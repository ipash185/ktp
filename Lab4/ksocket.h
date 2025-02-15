/*
Assignment 4 Submission 
Name: Heyramb Agrawal 
Roll number: 22CS30030
*/
#ifndef KSOCKET_H
#define KSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define SOCK_KTP 3
#define TIMEOUT_SEC 5  // Timeout in seconds
#define PACKET_DROP_PROB 0.2 // Packet loss probability
#define MAX_MSG_SIZE 512

// KTP packet header structure
typedef struct {
    uint8_t sequence_number;
    uint8_t is_acknowledgment;
} ktp_header_t;

// Now HEADER_SIZE is valid because the struct is fully defined
#define HEADER_SIZE sizeof(ktp_header_t)

// KTP message structure
typedef struct {
    ktp_header_t header;
    char data[MAX_MSG_SIZE];
} ktp_message_t;

// Function prototypes
int k_socket(int domain, int type, int protocol);
int k_bind(int socket_fd, const char* src_ip, int src_port, const char* dest_ip, int dest_port);
int k_sendto(int socket_fd, const void *buffer, size_t length);
int k_recvfrom(int socket_fd, void *buffer, size_t length);
int k_close(int socket_fd);
int should_drop_packet(float probability);

#endif
