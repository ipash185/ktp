#include "ktp.h"

#define FLAG_START 0
#define FLAG_LENGTH 1
#define MAX_SEQ_NO 256

int get_flag(void *buffer)
{
    // Extract the first byte without assuming 'char' is 1 byte
    uint8_t first_byte = *((uint8_t *)buffer);
    return first_byte;
}

int get_seq_num(void *buffer)
{
    // Extract the second byte without assuming 'char' is 1 byte
    uint8_t second_byte = *((uint8_t *)buffer + 1);
    return second_byte;
}

int get_ack_num(void *buffer)
{
    // Extract the third byte without assuming 'char' is 1 byte
    uint8_t third_byte = *((uint8_t *)buffer + 2);
    return third_byte;
}

int has_data(void *buffer)
{
    // return 0 if buffer is of 3 bytes
    return sizeof(buffer) > 3*sizeof(uint8_t);
}

char *get_data(void *buffer)
{
    if (!has_data(buffer)) return NULL;
    char *data = (uint8_t *)buffer + 3;
    return data;
}

char* make_packet(uint8_t flag, uint8_t seq_no, uint8_t ack_no, char *data)
{
    // write the stuffs into a buffer
    if (data != NULL)
    {
        uint8_t* packet = (uint8_t*)malloc(sizeof(uint8_t) * 515);
        packet[0] = flag;
        packet[1] = seq_no;
        packet[2] = ack_no;
        memcpy(packet + 3, data, strlen(data));
        return (char *)packet;
    }
    else {
        uint8_t* packet = (uint8_t*)malloc(sizeof(uint8_t) * 3);
        packet[0] = flag;
        packet[1] = seq_no;
        packet[2] = ack_no;
        return (char *)packet;
    }
}

void send_ACK(k_sock *sock, char *buffer)
{
    char *ack = make_packet(ACK, sock->seq_no, sock->ack_no, NULL);
    // make the address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = sock->dest_ip;
    addr.sin_port = sock->dest_port;
    sendto(sock->fd, ack, sizeof(ack), 0, (struct sockaddr *)&addr, sizeof(addr));
    free(ack);
}

void handle_ACK(k_sock *sock, char *buffer)
{
    printf("received ack!: %s\n", buffer);
    if (sock->state == WAITING_FOR_ACK)
    {
        int ack_num = get_ack_num(buffer);
        if (ack_num == (sock->seq_no + 1) % MAX_SEQ_NO)
        {
            sock->seq_no = (sock->seq_no + 1) % MAX_SEQ_NO;
            sock->state = WAITING_FOR_DATA;
            dequeue(&sock->write_buf);
        }
    }
}

void handle_DATA(k_sock *sock, char *buffer)
{
    printf("received data!: %s\n", buffer);
    if (has_data(buffer))
    {
        int seq_no = get_seq_num(buffer);
        if (sock->ack_no == seq_no)
        {
            char *data = get_data(buffer);
            enqueue(&sock->read_buf, data); // handling overflow later
            sock->ack_no = (sock->ack_no + 1) % MAX_SEQ_NO;
        }
        if (seq_no == sock->ack_no || seq_no == (sock->ack_no - 1 + MAX_SEQ_NO) % MAX_SEQ_NO)       // to be noted
        {
            send_ACK(sock, buffer); // handle window later
        }
    }
}