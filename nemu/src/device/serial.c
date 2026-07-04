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
#include <sys/select.h>
#include <unistd.h>

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

#define CH_OFFSET 0

static uint8_t *serial_base = NULL;

// 键盘输入队列（环形缓冲区）
#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;

static void key_enqueue(uint8_t ch) {
  key_queue[key_r] = ch;
  key_r = (key_r + 1) % KEY_QUEUE_LEN;
  Assert(key_r != key_f, "key queue overflow!");
}

// 非阻塞轮询 stdin，把输入放入队列
static void poll_stdin_key() {
  while (1) {
    fd_set readfds;
    struct timeval timeout = {0, 0};
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    if (ret <= 0 || !FD_ISSET(STDIN_FILENO, &readfds)) {
      break;
    }

    uint8_t ch = 0;
    if (read(STDIN_FILENO, &ch, 1) != 1) {
      break;
    }
    key_enqueue(ch);
  }
}

static void serial_putc(char ch) {
  MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
}

// 读串口：有输入返回字符，无输入返回 0xff（RT-Thread _uart_getc 以此判断无数据）
static void serial_read() {
  poll_stdin_key();
  if (key_f != key_r) {
    serial_base[0] = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  } else {
    serial_base[0] = 0xff;  // 无数据：RT-Thread 的 _uart_getc 收到 0xff 会返回 -1
  }
}

static void serial_io_handler(uint32_t offset, int len, bool is_write) {
  assert(len == 1);  //检查访问长度（len 必须是 1，因为串口通常按字节操作）
  switch (offset) {
    /* We bind the serial port with the host stderr in NEMU. */
    case CH_OFFSET:
      if (is_write) serial_putc(serial_base[0]); //如果偏移是0且写操作，那就直接给串口发送一个字节。
      else 
      // panic("do not support read");      //如果读操作就直接报错。
      serial_read();
      break;
    default: panic("do not support offset = %d", offset);
  }
}

void init_serial() {
  serial_base = new_space(8);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("serial", CONFIG_SERIAL_PORT, serial_base, 8, serial_io_handler);
#else
  add_mmio_map("serial", CONFIG_SERIAL_MMIO, serial_base, 8, serial_io_handler);
#endif

}