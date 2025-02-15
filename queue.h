#ifndef QUEUE_H
#define QUEUE_H
#define BUF_SIZE 10
#define MESSAGE_SIZE 512
#include <string.h>
#include <stdio.h>

typedef struct queue {
    char data[BUF_SIZE][MESSAGE_SIZE];
    int front;
    int back;
} queue;

void init_queue(queue *q);
void enqueue(queue *q, char *data);
void dequeue(queue *q);
int is_empty(queue *q);
int is_full(queue *q);

#endif