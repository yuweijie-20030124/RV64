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

#include <utils.h>
#include <device/map.h>
#ifndef CONFIG_TARGET_AM
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

#define CH_OFFSET 0

static uint8_t *serial_base = NULL;


static void serial_putc(char ch) {
  MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr)); //如果没用am那就用标准io库，如果用了am那就用自己实现的putch
}

static uint8_t serial_getc() {
#ifdef CONFIG_TARGET_AM
  return 0xff;
#else
  uint8_t ch;
  ssize_t nread = read(STDIN_FILENO, &ch, 1);
  if (nread == 1) {
    return ch;
  }
  if (nread == 0) {
    return 0xff;
  }
  assert(nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
  return 0xff;
#endif
}

static void serial_io_handler(uint32_t offset, int len, bool is_write) {
  assert(len == 1);  //检查访问长度（len 必须是 1，因为串口通常按字节操作）
  switch (offset) {
    /* We bind the serial port with the host stderr in NEMU. */
    case CH_OFFSET:
      if (is_write) serial_putc(serial_base[0]); //如果偏移是0且写操作，那就直接给串口发送一个字节。
      else serial_base[0] = serial_getc();
      break;
    default: panic("do not support offset = %d", offset);
  }
}

void init_serial() {
  serial_base = new_space(8);
#ifndef CONFIG_TARGET_AM
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  assert(flags >= 0);
  assert(fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == 0);
#endif
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("serial", CONFIG_SERIAL_PORT, serial_base, 8, serial_io_handler);
#else
  add_mmio_map("serial", CONFIG_SERIAL_MMIO, serial_base, 8, serial_io_handler);
#endif

}
