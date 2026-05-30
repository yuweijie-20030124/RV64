#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define IRINGBUF_SIZE 16
#define LOGBUF_SIZE 128

// 定义环形缓冲区结构体
typedef struct {
    char buffer[IRINGBUF_SIZE][LOGBUF_SIZE]; // 每个环形缓冲器的格子里面放一个字符串
    int head;
    int count;
} CircularBuffer;

void initBuffer(CircularBuffer *cb){
    //strcpy(cb->buffer , (char *)malloc(sizeof(char) * IRINGBUF_SIZE * LOGBUF_SIZE));
    cb->head = 0;
    cb->count = 0;
}

void enqueue(CircularBuffer *cb, const char *logbuf) {
    strncpy(cb->buffer[cb->head], logbuf, LOGBUF_SIZE - 1);
    cb->buffer[cb->head][LOGBUF_SIZE - 1] = '\0';
    cb->head = (cb->head + 1) % IRINGBUF_SIZE; //能直接15+1变成0
    if (cb->count < IRINGBUF_SIZE) {
        cb->count++;
    }
}

void printBuffer(CircularBuffer *cb) {
    if (cb->count == 0) {
        //printf("缓冲区为空\n");
        return;
    }
    int idx = (cb->head + IRINGBUF_SIZE - cb->count) % IRINGBUF_SIZE;
    for (int i = 0; i < cb->count; i++) {
        if(i == cb->count-1){ 
        printf("->%s\n", cb->buffer[(idx + i) % IRINGBUF_SIZE]);
        }
        else{
        printf("  %s\n", cb->buffer[(idx + i) % IRINGBUF_SIZE]);
        }
    }
}