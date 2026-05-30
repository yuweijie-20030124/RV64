/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory.h>
#include <mmio.h>

#define IO_SPACE_MAX (32 * 1024 * 1024)

static uint8_t *io_space = NULL;
static uint8_t *p_space = NULL;//初始化map的空间

//分配一块大小为 size 字节的连续内存空间，并返回指向这块空间的指针（类型为 uint8_t*，即无符号8位整数指针）。
//作用是将 size 向上取整到最近的 PAGE_SIZE 的倍数。
//CPU 访问内存时，某些操作（如 DMA、MMU 映射）要求地址必须是页对齐的。
uint8_t* new_space(int size) {  
  uint8_t *p = p_space; //p = p_space 记录分配前的指针位置，后续作为返回值。
  // page aligned;
  size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;//将 size 向上取整到最近的页面大小（PAGE_SIZE）的倍数。
  p_space += size; //将全局指针向后移动 size 字节，标记为已分配。
  assert(p_space - io_space < IO_SPACE_MAX);
  return p;
}

static void check_bound(IOMap *map, paddr_t addr) {//检查是否溢出
  if (map == NULL) {
    Assert(map != NULL, "address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
  } else {
    Assert(addr <= map->high && addr >= map->low,
        "address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, map->name, map->low, map->high, cpu.pc);
  }
}
//offset：相对于map->low的偏移量 len是读取的字节数 is_write false是表示读取操作，true时表示写操作
static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
  if (c != NULL) { c(offset, len, is_write); }
}

void init_map() {
  io_space = (uint8_t *)malloc(IO_SPACE_MAX);
  assert(io_space);
  p_space = io_space;
}

word_t map_read(paddr_t addr, int len, IOMap *map) {
  assert(len >= 1 && len <= 8);
  check_bound(map, addr);
  paddr_t offset = addr - map->low;  //将物理地址转换为映射区域内的相对偏移量
  invoke_callback(map->callback, offset, len, false); // 如果map->callback存在，调用它并传入参数（offset、len、false 表示读操作）。
  //callback用于模拟硬件设备的副作用（例如，读取某个寄存器可能自动清除状态位）。
  //map->space + offset：定位到映射区域中的目标地址。
  //host_read：从指针处读取 len 字节并返回 word_t 类型的地址。
  word_t ret = host_read(map->space + offset, len);
  //如果启用调试（CONFIG_DTRACE），记录读取操作的设备名、地址和长度。
  IFDEF(CONFIG_DTRACE, Log("read device %s : address in  = " FMT_PADDR ", len = %d\n", map->name , addr, len));
  return ret;
}
//其中map_read()和map_write()用于将地址addr映射到map所指示的目标空间, 并进行访问. 
//每次进行I/O读写的时候, 才会调用设备提供的回调函数(callback).
void map_write(paddr_t addr, int len, word_t data, IOMap *map) {
  assert(len >= 1 && len <= 8);
  check_bound(map, addr);
  paddr_t offset = addr - map->low;
  host_write(map->space + offset, len, data);
  invoke_callback(map->callback, offset, len, true);
  IFDEF(CONFIG_DTRACE, Log("write device %s : address in = " FMT_PADDR ", len = %d\n",  map->name , addr, len));
}
