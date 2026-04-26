#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "common.h"

#define IRINGBUF_SIZE 16
#define LOGBUF_SIZE 128

typedef struct {
    char buffer[IRINGBUF_SIZE][LOGBUF_SIZE]; // 每条指令一个字符串
    int head;
    int count;
} CircularBuffer;

void initBuffer(CircularBuffer *cb);

void enqueue(CircularBuffer *cb, const char *logbuf);

void printBuffer(CircularBuffer *cb);

#endif