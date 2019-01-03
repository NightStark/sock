#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

int sock2(int port, const char *addr)
{
    int sock = socket(AF_INET,SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(addr);
    socklen_t len = sizeof(struct sockaddr_in);
    if(connect(sock, (struct sockaddr*)&server, len) < 0 ) {
        perror("connect");
        return -1;
    }

    return sock;
}

struct event_base *g_evt_base = NULL;

int g_sock = -1;
int g_sock_s = -1;

int data_in(int fd)
{
    char buf[4096] = {0};
    ssize_t _s = 0;
    int ret = 0;

    //_s = read(fd, buf, sizeof(buf)-1);
    _s = recv(fd, buf, sizeof(buf)-1, 0);
    if (_s <= 0) {
        printf("[%s][%d] peer is closed. \n", __func__, __LINE__);
        return -1;
    }
    printf("[%s][%d] r->s , g_sock_s:%d\n", __func__, __LINE__, g_sock_s);
    buf[_s] = 0;
    if (g_sock_s > 0) {
        printf("[%s][%d] r->s +[%d] \n", __func__, __LINE__, _s);
        //write(g_sock_s, buf, _s);
        ret = send(g_sock_s, buf, _s, 0);
        if (ret < 0) {
            perror("send errror");
        }
        printf("[%s][%d] r->s - ret[%d] errno[%d]\n", __func__, __LINE__, ret, errno);
    }

    return 0;
}

int data_in_s(int fd)
{
    char buf[4096] = {0};
    ssize_t _s = 0;
    int ret = 0;

    //_s = read(fd, buf, sizeof(buf)-1);
    _s = recv(fd, buf, sizeof(buf)-1, 0);
    if (_s <= 0) {
        printf("[%s][%d] peer is closed. \n", __func__, __LINE__);
        return -1;
    }
    buf[_s] = 0;
    //printf("[%s][%d] s [%s]\n", __func__, __LINE__, buf);
    printf("[%s][%d] s->r \n", __func__, __LINE__);
    if (g_sock > 0) {
        printf("[%s][%d] s->r + [%d] \n", __func__, __LINE__, _s);
        //write(g_sock, buf, _s);
        ret = send(g_sock, buf, _s, 0);
        printf("[%s][%d] s->r - ret=[%d] \n", __func__, __LINE__, ret);
    }

    return 0;
}

void *client_work(void *arg)
{
    signal(SIGPIPE, SIG_IGN);

    while (1) {
        if (data_in((int)arg) < 0) {
            break; 
        }
    }

    return NULL;
}

void *client_work_s(void *arg)
{
    int sock_s = -1;

    signal(SIGPIPE, SIG_IGN);

    while (1) {
        sock_s = sock2(23, "127.0.0.1");
        g_sock_s = sock_s;
        printf("[%s][%d] sock_s create. \n", __func__, __LINE__);

        while (1) {
            if (data_in_s(sock_s) < 0) {
                break; 
            }
        }
        g_sock_s = -1;
        close(sock_s);
    }

    return NULL;
}

int main(int argc,const char* argv[])
{
    int sock = -1;

    if(argc != 3) {
        printf("Usage:%s [ip] [port]\n",argv[0]);
        return 0;
    }

    signal(SIGPIPE, SIG_IGN);

    pthread_t tid;
    pthread_t tid_s;

    sock = sock2(atoi(argv[2]), argv[1]);

    g_sock = sock;

    pthread_create(&tid, NULL, client_work, (void *)sock);
    pthread_create(&tid_s, NULL, client_work_s, NULL);

    while (1) {
        sleep(1);
    }

    close(sock);

    return 0;
}
