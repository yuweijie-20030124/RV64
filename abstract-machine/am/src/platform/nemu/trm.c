#include <am.h>
#include <nemu.h>
#include <stdio.h>

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END); //结构用于指示堆区的启始和末尾

static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

void putch(char ch) { //输出一个字符
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
  //printf("halt called with code: %d\n", code);
  //fflush(stdout);
  nemu_trap(code);
  // should not reach here
  while (1);
}

void _trm_init() {
  int ret = main(mainargs); //用户程序
  halt(ret);
}
