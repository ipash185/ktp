/*
Assignment 4 Submission 
Name: Heyramb Agrawal 
Roll number: 22CS30030
*/
#include "ksocket.h"

static int udp_socket_fd = -1;
static struct sockaddr_in local_address, remote_address;
static uint8_t current_sequence = 1;

int should_drop_packet(float probability) {
    return ((float)rand() / RAND_MAX) < probability;
}

int k_socket(int domain, int type, int protocol) {
    if (type != SOCK_KTP) {
        errno = EINVAL;
        return -1;
    }
    
    udp_socket_fd = socket(domain, SOCK_DGRAM, protocol);
    if (udp_socket_fd < 0) return -1;
    
    srand(time(NULL));  // Initialize random seed
    return udp_socket_fd;
}

int k_bind(int socket_fd, const char* src_ip, int src_port, const char* dest_ip, int dest_port) {
    if (socket_fd != udp_socket_fd) {
        errno = EBADF;
        return -1;
    }

    memset(&local_address, 0, sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(src_port);
    inet_pton(AF_INET, src_ip, &local_address.sin_addr);

    memset(&remote_address, 0, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(dest_port);
    inet_pton(AF_INET, dest_ip, &remote_address.sin_addr);

    return bind(socket_fd, (struct sockaddr*)&local_address, sizeof(local_address));
}

int k_sendto(int socket_fd, const void *buffer, size_t length) {
    if (length > MAX_MSG_SIZE) {
        errno = EMSGSIZE;
        return -1;
    }
    
    ktp_message_t message;
    message.header.sequence_number = current_sequence;
    message.header.is_acknowledgment = 0;
    memcpy(message.data, buffer, length);
    
    struct timeval timeout;
    fd_set read_fds;
    int attempts = 1;

    do {
        printf("[SENDER] Attempt %d: Sending sequence %d\n", attempts, current_sequence);
        sendto(socket_fd, &message, HEADER_SIZE + length, 0, 
               (struct sockaddr*)&remote_address, sizeof(remote_address));

        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        
        int select_result = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result > 0) {
            ktp_message_t ack_message;
            socklen_t addr_len = sizeof(remote_address);
            recvfrom(socket_fd, &ack_message, sizeof(ack_message), 0,
                     (struct sockaddr*)&remote_address, &addr_len);
            
            if (!should_drop_packet(PACKET_DROP_PROB) && ack_message.header.is_acknowledgment &&
                ack_message.header.sequence_number == current_sequence) {
                printf("[SENDER] ACK received for sequence %d\n", current_sequence);
                current_sequence++;
                return length;
            }
        }
        printf("[SENDER] Timeout! Retransmitting...\n");
        attempts++;
    } while (1);
}

int k_recvfrom(int socket_fd, void *buffer, size_t length) {
    ktp_message_t message;
    socklen_t addr_len = sizeof(remote_address);

    do {
        recvfrom(socket_fd, &message, sizeof(message), 0,
                 (struct sockaddr*)&remote_address, &addr_len);

        if (!should_drop_packet(PACKET_DROP_PROB)) {
            message.header.is_acknowledgment = 1;
            sendto(socket_fd, &message, HEADER_SIZE, 0,
                   (struct sockaddr*)&remote_address, sizeof(remote_address));
            
            memcpy(buffer, message.data, length < MAX_MSG_SIZE ? length : MAX_MSG_SIZE);
            return length < MAX_MSG_SIZE ? length : MAX_MSG_SIZE;
        }
    } while (1);
}

int k_close(int socket_fd) {
    if (socket_fd == udp_socket_fd) {
        udp_socket_fd = -1;
        return close(socket_fd);
    }
    errno = EBADF;
    return -1;
}
