#ifndef KTP_UTIL_H
#define KTP_UTIL_H

#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int get_flag(char *buffer);
int get_seq_num(void *buffer);
int get_ack_num(void *buffer);
int has_data(void *buffer);
char *get_data(void *buffer);
char* make_packet(uint8_t flag, uint8_t seq_no, uint8_t ack_no, char *data);
void send_ACK(k_sock *sock, char *buffer);
void handle_ACK(k_sock* sock, char* buffer);
void handle_DATA(k_sock* sock, char* buffer);

#endif