#include "ktp.h"

#define MAX_SEQ_NO 100

int k_socket(int domain, int type, int protocol)
{
    if (type == SOCK_KTP)
    {
        key_t shmkey = ftok("/", 'a');
        // get the shared memory
        int shm_id = shmget(shmkey, sizeof(k_sock) * SHM_SIZE, IPC_CREAT | 0777);

        if (shm_id == -1)
        {
            perror("shmget failed");
            return -1;
        }

        // Attach to the shared memory
        void *shm = shmat(shm_id, NULL, 0);

        // see if another socket can fit
        for (int i = 0; i < SHM_SIZE; i++)
        {
            k_sock *sock = (k_sock *)shm + i;
            int retval;
            if (sock->fd == 0)
            {
                sock->fd = socket(domain, SOCK_DGRAM, protocol);
                retval = sock->fd;
                struct sockaddr_in addr;
                int addr_len;
                getsockname(sock->fd, (struct sockaddr *)&addr, &addr_len);
                sock->src_port = addr.sin_port;
                sock->src_ip = addr.sin_addr.s_addr;
                // destination address not set here
                sock->seq_no = 1;
                sock->ack_no = 1;
                sock->state = WAITING_FOR_DATA;
                shmdt(shm);
                return retval;
            }
        }
        shmdt(shm);
    }
    else
    {
        printf("invalid socket type\n");
    }

    return -1;
}

void k_bind(int fd, struct sockaddr *src_addr, socklen_t src_addrlen, struct sockaddr *dest_addr, socklen_t dest_addrlen)
{
    if (bind(fd, src_addr, src_addrlen) == -1)
    {
        perror("k_bind failed");
        exit(EXIT_FAILURE);
    }

    key_t shmkey = ftok("/", 'a');
    // get the shared memory
    int shm_id = shmget(shmkey, sizeof(k_sock) * SHM_SIZE, IPC_CREAT | 0777);

    if (shm_id == -1)
    {
        perror("shmget failed");
        return;
    }

    // Attach to the shared memory
    void *shm = shmat(shm_id, NULL, 0);

    for (int i = 0; i < SHM_SIZE; i++)
    {
        k_sock *sock = (k_sock *)shm + i;
        if (sock->fd == fd)
        {
            sock->src_port = ((struct sockaddr_in *)src_addr)->sin_port;
            sock->src_ip = ((struct sockaddr_in *)src_addr)->sin_addr.s_addr;
            sock->dest_port = ((struct sockaddr_in *)dest_addr)->sin_port;
            sock->dest_ip = ((struct sockaddr_in *)dest_addr)->sin_addr.s_addr;
            break;
        }
    }

    shmdt(shm);
}

int k_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen)
{
    key_t shmkey = ftok("/", 'a');
    // get the shared memory
    int shm_id = shmget(shmkey, sizeof(k_sock) * SHM_SIZE, IPC_CREAT | 0777);

    if (shm_id == -1)
    {
        perror("shmget failed");
        return -1;
    }

    // Attach to the shared memory
    void *shm = shmat(shm_id, NULL, 0);

    for (int i = 0; i < SHM_SIZE; i++)
    {
        k_sock *sock = (k_sock *)shm + i;
        if (sock->fd == fd)
        {
            if (sock->dest_port == ((struct sockaddr_in *)dest_addr)->sin_port && sock->dest_ip == ((struct sockaddr_in *)dest_addr)->sin_addr.s_addr)
            {
                if (!is_full(&sock->write_buf))
                {
                    enqueue(&sock->write_buf, (char *)buf);
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
            break;
        }
    }

    shmdt(shm);
}

int k_recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen)
{
    key_t shmkey = ftok("/", 'a');
    // get the shared memory
    int shm_id = shmget(shmkey, sizeof(k_sock) * SHM_SIZE, IPC_CREAT | 0777);

    if (shm_id == -1)
    {
        perror("shmget failed");
        return -1;
    }

    // Attach to the shared memory
    void *shm = shmat(shm_id, NULL, 0);

    for (int i = 0; i < SHM_SIZE; i++)
    {
        k_sock *sock = (k_sock *)shm + i;
        if (sock->fd == fd)
        {
            if (!is_empty(&sock->read_buf))
            {
                strncpy(buf, sock->read_buf.data[sock->read_buf.front], len);
                dequeue(&sock->read_buf);
                return len;
            }
            else
            {
                return -1;
            }
            break;
        }
    }

    shmdt(shm);
}

void k_close(int fd)
{
    key_t shmkey = ftok("/", 'a');
    // get the shared memory
    int shm_id = shmget(shmkey, sizeof(k_sock) * SHM_SIZE, IPC_CREAT | 0777);

    if (shm_id == -1)
    {
        perror("shmget failed");
        return;
    }

    // Attach to the shared memory
    void *shm = shmat(shm_id, NULL, 0);

    for (int i = 0; i < SHM_SIZE; i++)
    {
        k_sock *sock = (k_sock *)shm + i;
        if (sock->fd == fd)
        {
            close(fd);
            sock->fd = 0;
            break;
        }
    }

    shmdt(shm);
}