//19112 19113
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

struct event_base *g_evt_base = NULL; 
int g_sock = -1;
int g_sock_l = -1;
int g_c_sock = -1;
int g_c_sock_l = -1;
struct event *g_c_recv_evt;
struct event *g_c_recv_evt_l;

int startup(int _port,const char* _ip)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0) {
		perror("socket");
		exit(1);
	}

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons( _port);
	local.sin_addr.s_addr = inet_addr(_ip);
	socklen_t len = sizeof(local);

	if(bind(sock,(struct sockaddr*)&local , len) < 0) {
		perror("bind");
		exit(2);
	}

	if(listen(sock, 5) < 0) {
		perror("listen");
		exit(3);
	}

	return sock;
}

void *listen_work(void *arg)
{
	struct sockaddr_in remote;
	socklen_t len = sizeof(struct sockaddr_in);
    int listen_sock = (int)arg;

	while(1) {
		int sock = accept(listen_sock, (struct sockaddr*)&remote, &len);
		if(sock < 0) {
			perror("accept");
			continue;
		}
        g_c_sock = sock;
		printf("get a client, ip:%s, port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
		char buf[4096];
		while(1) {
			ssize_t _s = 0;
            int ret = 0;
            //_s = read(sock, buf, sizeof(buf)-1);
            _s = recv(sock, buf, sizeof(buf)-1, 0);
			if(_s > 0) {
				buf[_s] = 0;
				//printf("client:%s\n",buf);
                if (g_c_sock_l > 0) {
                    printf("s->l+ _s[%d]\n", _s);
                    //write(g_c_sock_l, buf, _s);
                    ret = send(g_c_sock_l, buf, _s, 0);
                    printf("s->l- ret[%d]\n", ret);
                }
			} else {
				printf("client is quit!\n");
				break;
			}
		}
	}

    return NULL;
}

void *listen_work_l(void *arg)
{
	struct sockaddr_in remote;
	socklen_t len = sizeof(struct sockaddr_in);
    int listen_sock = (int)arg;

	while(1) {
		int sock = accept(listen_sock, (struct sockaddr*)&remote, &len);
		if(sock < 0) {
			perror("accept");
			continue;
		}
        g_c_sock_l = sock;
		printf("get a client l, ip:%s, port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
		char buf[4096];
        int ret = 0;

		while(1) {
			ssize_t _s =0;
            //_s = read(sock, buf, sizeof(buf)-1);
            _s = recv(sock, buf, sizeof(buf)-1, 0);
			if(_s > 0) {
				buf[_s] = 0;
				//printf("client l:%s",buf);
                if (g_sock > 0) {
                    printf("l->s+ _s[%d]\n", _s);
                    //ret = write(g_c_sock, buf, _s);
                    ret = send(g_c_sock, buf, _s, 0);
                    printf("l->s-, ret[%d}\n", ret);
                }
			} else {
				printf("client l is quit!\n");
				break;
			}
		}

	}
    return NULL;
}

int main(int argc,const char* argv[])
{
	if(argc != 3) {
		printf("Usage:%s [loacl_ip] [loacl_port]\n",argv[0]);
		return 1;
	}



	int listen_sock = startup(atoi(argv[2]),argv[1]);
	int listen_sock_l = startup(19112, "0.0.0.0");


	g_sock = listen_sock;
	g_sock = listen_sock_l;

    pthread_t tid;
    pthread_t tid_l;

    pthread_create(&tid, NULL, listen_work, (void *)listen_sock);
    pthread_create(&tid_l, NULL, listen_work_l, (void *)listen_sock_l);


	#if 0
	struct sockaddr_in remote;
	socklen_t len = sizeof(struct sockaddr_in);

	while(1) {
		int sock = accept(listen_sock, (struct sockaddr*)&remote, &len);
		if(sock < 0) {
			perror("accept");
			continue;
		}
		printf("get a client, ip:%s, port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
		char buf[1024];
		while(1) {
			ssize_t _s = read(sock, buf, sizeof(buf)-1);
			if(_s > 0) {
				buf[_s] = 0;
				printf("client:%s",buf);
			} else {
				printf("client is quit!\n");
				break;
			}
		}
	}
	#endif

    while (1) {
        sleep(1);
    }

	return 0;
}

