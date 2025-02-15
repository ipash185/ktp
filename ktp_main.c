#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <signal.h>
#include "ktp.h"
#include "ktp_util.h"

#define MAX_SOCK 5
#define SHM_SIZE 10
#define BUFFER_SIZE 1024
#define T 2     // T should be greater than 1 as slep(T/2) is used

fd_set read_fds;
int shm_id;
k_sock *shm;

void handle_sigint(int sig) {
    // Detach from the shared memory
    shmdt(shm);
    // close the shared memory
    shmctl(shm_id, IPC_RMID, NULL);
    exit(0);
}

// Thread function 1
void *R(void *arg)
{

    int max_fd = 0;
    k_sock *sockets;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);

        sockets = (k_sock *)shm;

        for (int i = 0; i < MAX_SOCK; i++)
        {
            if (sockets[i].fd > 0)
            {
                FD_SET(sockets[i].fd, &read_fds);
                max_fd = sockets[i].fd > max_fd ? sockets[i].fd : max_fd;
            }
        }
    }

    select(max_fd + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(0, &read_fds))
    {
        // Detach from the shared memory
        shmdt(shm);
        // close the shared memory
        shmctl(shm_id, IPC_RMID, NULL);
        // stop this process and end the program
        exit(0);
    }

    for (int i = 0; i < MAX_SOCK; i++)
    {
        if (FD_ISSET(sockets[i].fd, &read_fds))
        {
            printf("Thread 1 message: %s\n", sockets[i].read_buf.data[sockets[i].read_buf.front]);
            // recvfrom the message
            struct sockaddr_in src_addr;
            int src_len;
            char buffer[BUFFER_SIZE];
            int n = recvfrom(sockets[i].fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&src_addr, &src_len); // error handling later
            int flag = get_flag(buffer);

            if (flag == ACK)
            {
                handle_ACK(sockets+i, buffer);
            }
            handle_DATA(sockets+i, buffer);
        }
    }

    return NULL;
}

// Thread function 2
void *S(void *arg)
{
    while (1)
    {
        k_sock *sockets = (k_sock *)shm;
        for (int i = 0; i < MAX_SOCK; i++)
        {
            if (sockets[i].fd > 0)
            {
                if (sockets[i].state == WAITING_FOR_ACK)
                {
                    // check if timer timed out
                    struct timespec current_time;
                    clock_gettime(CLOCK_MONOTONIC, &current_time);

                    // Calculate the difference between current time and send_time
                    struct timespec diff;
                    diff.tv_sec = current_time.tv_sec - sockets[i].send_time.tv_sec;
                    diff.tv_nsec = current_time.tv_nsec - sockets[i].send_time.tv_nsec;
                    if (diff.tv_sec > T)
                    {
                        // resend the packet
                        char* packet = make_packet(DATA, sockets[i].seq_no, sockets[i].ack_no, sockets[i].write_buf.data[sockets[i].write_buf.front]);
                        struct sockaddr_in addr;
                        addr.sin_family = AF_INET;
                        addr.sin_addr.s_addr = sockets[i].dest_ip;
                        addr.sin_port = sockets[i].dest_port;
                        sendto(sockets[i].fd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
                        free(packet);
                        // set the timer again
                        clock_gettime(CLOCK_MONOTONIC, &sockets[i].send_time);
                        sockets[i].state = WAITING_FOR_ACK;
                    }
                }
                else if (!is_empty(&sockets[i].write_buf))
                {
                    char* packet = make_packet(DATA, sockets[i].seq_no, sockets[i].ack_no, sockets[i].write_buf.data[sockets[i].write_buf.front]);
                    struct sockaddr_in addr;
                    addr.sin_family = AF_INET;
                    addr.sin_addr.s_addr = sockets[i].dest_ip;
                    addr.sin_port = sockets[i].dest_port;
                    sendto(sockets[i].fd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
                    free(packet);
                    clock_gettime(CLOCK_MONOTONIC, &sockets[i].send_time);
                    sockets[i].state = WAITING_FOR_ACK;
                }
            }
        }
        sleep(T/2);
    }
    return NULL;
}

int main()
{
    signal(SIGINT, handle_sigint);
    // Create a shared memory key
    key_t shmkey = ftok("/", 'a');
    // get the shared memory
    shm_id = shmget(shmkey, sizeof(k_sock) * SHM_SIZE, IPC_CREAT | IPC_EXCL | 0777);

    if (shm_id == -1)
    {
        perror("shmget failed");
        return 1;
    }

    // Attach to the shared memory
    shm = shmat(shm_id, NULL, 0);

    // initialize shm to 0
    memset(shm, 0, sizeof(k_sock) * SHM_SIZE);

    pthread_t thread1, thread2;

    // Create the first thread
    if (pthread_create(&thread1, NULL, R, NULL) != 0)
    {
        perror("Failed to create thread 1");
        return 1;
    }

    // Create the second thread
    if (pthread_create(&thread2, NULL, S, NULL) != 0)
    {
        perror("Failed to create thread 2");
        return 1;
    }

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Both threads have finished execution.\n");

    for (int i = 0; i < MAX_SOCK; i++)
    {
        if (shm[i].fd > 0)
            close(shm[i].fd);
    }

    // Detach from the shared memory
    shmdt(shm);
    // close the shared memory
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
