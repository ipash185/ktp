#include "queue.h"
// void init_queue(queue *q);
// void enqueue(queue *q, char *data);
// char *dequeue(queue *q);
// int is_empty(queue *q);
// int is_full(queue *q);

int is_empty(queue *q)
{
    return q->front == (q->back+1)%BUF_SIZE;
}

int is_full(queue *q)
{
    return q->front == (q->back+2)%BUF_SIZE;
}

void init_queue(queue *q)
{
    q->front = 0;
    q->back = BUF_SIZE-1;
    memset(q->data, 0, BUF_SIZE*MESSAGE_SIZE*sizeof(char));
}

void enqueue(queue *q, char *data)
{
    if (is_full(q))
    {
        printf("queue is full\n");
        return;
    }
    q->back = (q->back + 1) % BUF_SIZE;
    memcpy(q->data[q->back], data, MESSAGE_SIZE*sizeof(char));
}

void dequeue(queue *q)
{
    if (is_empty(q))
    {
        printf("queue is empty\n");
        return;
    }
    q->front = (q->front + 1) % BUF_SIZE;
}