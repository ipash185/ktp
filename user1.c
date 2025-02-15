#include "ktp.h"
#include <signal.h>

#define USER1_PORT 5000
#define USER2_PORT 5001
#define IP "127.0.0.1"
#define BUFFER_SIZE 512

int sockfd;

void handle_sigint(int sig) {
    k_close(sockfd);
    exit(0);
}

int main ()
{

    signal(SIGINT, handle_sigint);

    sockfd = k_socket(AF_INET, SOCK_KTP, 0);
    struct sockaddr_in user1_addr, user2_addr;
    user1_addr.sin_family = AF_INET;
    user1_addr.sin_port = htons(USER1_PORT);
    user1_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int user1_len = sizeof(user1_addr);

    user2_addr.sin_family = AF_INET;
    user2_addr.sin_port = htons(USER2_PORT);
    user2_addr.sin_addr.s_addr = inet_addr(IP);
    int user2_len = sizeof(user2_addr);
    k_bind(sockfd, (struct sockaddr *)&user1_addr, user1_len, (struct sockaddr *)&user2_addr, sizeof(user2_addr));

    char buf[BUFFER_SIZE] = "hello world";
    while (1)
    {
        if (k_sendto(sockfd, buf, 512, 0, (struct sockaddr *)&user2_addr, sizeof(user2_addr)) < 0)
        {
            // error
            perror("k_sendto");
            exit(1);
        }
        printf("sent %s\n", buf);
    }
}