/*
Assignment 4 Submission 
Name: Heyramb Agrawal 
Roll number: 22CS30030
*/
#include "ksocket.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <source_ip> <source_port> <destination_ip> <destination_port>\n", argv[0]);
        return 1;
    }

    printf("\n=== KTP Receiver Initialized ===\n");
    printf("Source: %s:%s\n", argv[1], argv[2]);
    printf("Destination: %s:%s\n\n", argv[3], argv[4]);

    int socket_fd = k_socket(AF_INET, SOCK_KTP, 0);
    if (socket_fd < 0) {
        perror("k_socket failed");
        return 1;
    }

    if (k_bind(socket_fd, argv[1], atoi(argv[2]), argv[3], atoi(argv[4])) < 0) {
        perror("k_bind failed");
        return 1;
    }

    printf("Waiting for incoming messages... (Press Ctrl+C to exit)\n");

    while (1) {
        char buffer[MAX_MSG_SIZE];
        if (k_recvfrom(socket_fd, buffer, MAX_MSG_SIZE) < 0) {
            perror("k_recvfrom failed");
            break;
        }

        printf("\n[RECEIVER] Message received: %s", buffer);
    }

    k_close(socket_fd);
    printf("\n=== KTP Receiver Terminated ===\n");
    return 0;
}
