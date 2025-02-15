#ifndef KTP_H
#define KTP_H

#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/shm.h>

#include "queue.h"

#define SHM_SIZE 10
#define MESSAGE_SIZE 512
#define ACK 0
#define DATA 1
#define SOCK_KTP 100
#define WAITING_FOR_DATA 0
#define WAITING_FOR_ACK 1

typedef struct k_sock {
    int fd;
    uint16_t dest_port;
    uint32_t dest_ip;
    uint16_t src_port;
    uint32_t src_ip;
    uint8_t seq_no;
    uint8_t ack_no;
    uint8_t state;
    queue read_buf;
    queue write_buf;
    // sending time
    struct timespec send_time;
} k_sock;

int k_socket(int domain, int type, int protocol);
void k_bind(int fd, struct sockaddr *src_addr, socklen_t src_addrlen, struct sockaddr *dest_addr, socklen_t dest_addrlen);
int k_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen);
int k_recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen);
void k_close(int fd);
#endif