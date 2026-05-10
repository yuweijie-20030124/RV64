#include <common.h>
#include "syscall.h"
#include "fs.h"
#include <memory.h>

#define CONFIG_STRACE 1

#if CONFIG_STRACE
#define STRACE_LOG(...) Log(__VA_ARGS__)
#else
#define STRACE_LOG(...)
#endif

#if CONFIG_STRACE
#define STRACE_FD(fd) fs_fd_name(fd)
#else
#define STRACE_FD(fd) NULL
#endif

static const char *syscall_names[] = {
  [SYS_exit] = "exit",
  [SYS_yield] = "yield",
  [SYS_open] = "open",
  [SYS_read] = "read",
  [SYS_write] = "write",
  [SYS_kill] = "kill",
  [SYS_getpid] = "getpid",
  [SYS_close] = "close",
  [SYS_lseek] = "lseek",
  [SYS_brk] = "brk",
  [SYS_fstat] = "fstat",
  [SYS_time] = "time",
  [SYS_signal] = "signal",
  [SYS_execve] = "execve",
  [SYS_fork] = "fork",
  [SYS_link] = "link",
  [SYS_unlink] = "unlink",
  [SYS_wait] = "wait",
  [SYS_times] = "times",
  [SYS_gettimeofday] = "gettimeofday",
};

Context *do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  const char *name = "unknown";

  if (a[0] < LENGTH(syscall_names) && syscall_names[a[0]] != NULL) {
    name = syscall_names[a[0]];
  }
  (void)name;

  switch (a[0]) {
    case SYS_exit:
      STRACE_LOG("syscall: %s()", name);  
      halt(0);
    break;

    case SYS_yield:
      STRACE_LOG("syscall: %s()", name);
      yield();
      c->GPRx = 0;
      STRACE_LOG("syscall return: %s -> %d", name, c->GPRx);
      return c;
    break;

    case SYS_write:
      STRACE_LOG("syscall: %s(%d:%s, %p, %d)", name, a[1], STRACE_FD(a[1]), a[2], a[3]);
      c->GPRx = fs_write(a[1], (const void *)a[2], a[3]);
      STRACE_LOG("syscall return: %s -> %d", name, c->GPRx);
      return c;

    case SYS_brk:
      STRACE_LOG("syscall: %s(%p)", name, a[1]);
      c->GPRx = mm_brk(a[1]);
      STRACE_LOG("syscall return: %s -> %d", name, c->GPRx);
      return c;

    default:
      STRACE_LOG("syscall: %s(%d, %p, %p, %p)", name, a[0], a[1], a[2], a[3]);
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return c;
}
