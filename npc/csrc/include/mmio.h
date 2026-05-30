#ifndef __DEVICE_MMIO_H__
#define __DEVICE_MMIO_H__

#include <common.h>

#ifdef CONFIG_DIFFTEST
void difftest_skip_ref();
#endif
extern int difftest_skip_ref_flag;
typedef void(*io_callback_t)(uint32_t, int, bool);
uint8_t* new_space(int size);

/*在硬件交互中，某些寄存器的读取可能具有副作用（side effects），例如：
状态寄存器：读取后可能自动清除某些标志位（如中断状态寄存器）。
FIFO 缓冲区：读取数据后，硬件可能自动更新内部指针。
计数器/定时器：读取计数值可能影响其行为。
callback 允许在 map_read 执行真正的数据读取之前，先通知设备（或模拟器）：“有一个读取操作发生了，请做好相应处理”。*/

typedef struct {
  const char *name;   //设备名称，调试用
  // we treat ioaddr_t as paddr_t here
  paddr_t low;    //映射区的起始地址
  paddr_t high;   //映射区域的结束地址
  void *space;    //实际存储数据的指针
  io_callback_t callback;   //回调函数
} IOMap;
//包括名字映射的起始地址和结束地址，映射的目标空间和一个回调函数。
//low地址范围的下界--起始地址  high地址范围的上界--结束地址

//bool类型确认给定的物理地址是否位于IO的映射区域内
static inline bool map_inside(IOMap *map, paddr_t addr) {
  return (addr >= map->low && addr <= map->high);
}

//给你地址，范围和mmio，你要找出这个地址和范围在mmio中的id是多少
static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
  int i;
  for (i = 0; i < size; i ++) {//遍历一遍要找的addr中的size空间
    if (map_inside(maps + i, addr)) {//确认看的是IO的映射区域
      // printf("!!!!!!!!!!!!!!!!!!!!!******\n");

      #ifdef CONFIG_DIFFTEST
      difftest_skip_ref();
      difftest_skip_ref_flag = 1;
      #endif
      
      return i;
    }
  }
  return -1;
}

void add_pio_map(const char *name, ioaddr_t addr,
        void *space, uint32_t len, io_callback_t callback);
void add_mmio_map(const char *name, paddr_t addr,
        void *space, uint32_t len, io_callback_t callback);

word_t map_read(paddr_t addr, int len, IOMap *map);
void map_write(paddr_t addr, int len, word_t data, IOMap *map);

word_t mmio_read(paddr_t addr, int len);
void mmio_write(paddr_t addr, int len, word_t data);

#endif
