#include <fs.h>

//读写函数指针
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;         // 文件名（如 "/dev/fb"）
  size_t size;        // 文件大小（字节）
  size_t disk_offset; // 在 ramdisk 中的起始偏移
  size_t open_offset; // 当前读写位置
  ReadFn read;        // 自定义读函数指针
  WriteFn write;      // 自定义写函数指针
} Finfo;

enum {
  FD_STDIN,     // = 0  标准输入
  FD_STDOUT,    // = 1  标准输出
  FD_STDERR,    // = 2  标准错误
  FD_EVENTS,    // = 3  事件设备（键盘输入事件）
  FD_DISPINFO,  // = 4  显示信息（屏幕分辨率等）
  FD_FB,        // = 5  帧缓冲（显存）
};

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t stdin_read(void *buf, size_t offset, size_t len) {
  (void)buf;
  (void)offset;
  (void)len;
  return 0;
}

///dev/events 不是一个真实的磁盘文件，而是一个虚拟设备文件（device file），类似 Linux 的 /dev/input/events。

// 不理解qaq
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]    = {"stdin", 0, 0, 0, stdin_read, invalid_write},
  [FD_STDOUT]   = {"stdout", 0, 0, 0, invalid_read, serial_write},
  [FD_STDERR]   = {"stderr", 0, 0, 0, invalid_read, serial_write},
  [FD_EVENTS]   = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  [FD_FB]       = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
#include "files.h"
};
// 如果不用指定初始化器，编译器自动分配下一个连续下标
// 如果后面没写
// open_offset 自动 = 0
// read        自动 = NULL
// write       自动 = NULL

static size_t file_size(int fd) {
  return file_table[fd].size;
}
//防止读越界，如果请求读的字节数超出了文件剩余内容，就自动截断。
  // offset = 文件内部偏移
  // len    = 读的字节数
  // size   = 文件总大小
static size_t clamp_len(size_t offset, size_t len, size_t size) {
  if (offset >= size) {
    return 0;
  }
  size_t rest = size - offset;
  return len < rest ? len : rest;
}

static int find_file(const char *pathname) {
  for (size_t i = 0; i < LENGTH(file_table); i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      return i;
    }
  }
  return -1;
}

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags;
  (void)mode;
  
  int fd = find_file(pathname);
  if (fd >= 0) {
    file_table[fd].open_offset = 0;
    return fd;
  }

  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(fd >= 0 && fd < (int)LENGTH(file_table));
  Finfo *file = &file_table[fd];
  size_t ret = 0;

  if (file->read != NULL) {
    ret = file->read(buf, file->open_offset, len);
  } else { //bin文件会进入这里
    ret = clamp_len(file->open_offset, len, file->size);
    ret = ramdisk_read(buf, file->disk_offset + file->open_offset, ret);
  }

  file->open_offset += ret;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < (int)LENGTH(file_table)); //遍历大于0且不超出范围
  Finfo *file = &file_table[fd];
  size_t ret = 0;

  if (file->write != NULL) {
    ret = file->write(buf, file->open_offset, len);
  } else {
    ret = clamp_len(file->open_offset, len, file->size);
    ret = ramdisk_write(buf, file->disk_offset + file->open_offset, ret);
  }

  file->open_offset += ret;
  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < (int)LENGTH(file_table));
  Finfo *file = &file_table[fd];
  size_t new_offset = 0;

  switch (whence) {
    case SEEK_SET: new_offset = offset; break; //从文件头
    case SEEK_CUR: new_offset = file->open_offset + offset; break; //从当前位置算起
    case SEEK_END: new_offset = file_size(fd) + offset; break;  //从文件尾算起
    default: assert(0);
  }

  assert(new_offset <= file_size(fd));
  file->open_offset = new_offset;
  return new_offset;
}

int fs_close(int fd) {
  assert(fd >= 0 && fd < (int)LENGTH(file_table));
  return 0;
}

int fs_fstat(int fd, struct stat *buf) {
  assert(fd >= 0 && fd < (int)LENGTH(file_table));
  memset(buf, 0, sizeof(*buf));
  buf->st_mode = (fd <= FD_STDERR || fd == FD_EVENTS || fd == FD_DISPINFO || fd == FD_FB)
    ? S_IFCHR
    : S_IFREG;
  buf->st_size = file_size(fd);
  buf->st_blksize = 512;
  return 0;
}

const char *fs_fd_name(int fd) {
  if (fd < 0 || fd >= (int)LENGTH(file_table)) {
    return "<bad-fd>";
  }
  return file_table[fd].name;
}
//init_fs() 初始化**帧缓冲（framebuffer）**的大小。
void init_fs() {
  file_table[FD_FB].size = 0x200000;
}