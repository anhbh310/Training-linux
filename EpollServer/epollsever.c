#include <stdio.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

void setnonblocking(int sock){
	int opts = fcntl(sock, F_GETFL);
	if (opts <0){
		printf("ERROR in GETFL() %s\n", strerror(errno));
		exit(0);
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts)<0){
		printf("ERROR in SETFL() %s\n", strerror(errno));
		exit(0);
	}
}

int main(){
	char buffer[1025];	
	int client_socket[30];
	for (int i=0;i<30;i++){
		client_socket[i]=0;
	}
	
	int master_socket=socket(AF_INET,SOCK_STREAM,0);
	if (master_socket==0){
		 printf("ERROR in socket() %s\n", strerror(errno));
		 return 0;
	}
	
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_port=htons(9090); 
	
	if (bind(master_socket,(struct sockaddr *) &addr,sizeof(addr))<0){
		printf("ERROR in bind() %s\n", strerror(errno));
		return 0;
	}
	
	if (listen(master_socket,5)<0){
		printf("ERROR in listen() %s\n", strerror(errno));
		return 0;
	}
	
	struct epoll_event ev, events[30];
	int epollfd = epoll_create1(0);
	if (epollfd == -1){
		printf("ERROR in epoll_create() %s\n", strerror(errno));
		return 0;
	}
	
	ev.events = EPOLLIN;
	ev.data.fd= master_socket;
	if (epoll_ctl(epollfd,EPOLL_CTL_ADD,master_socket, &ev)==-1){
		printf("ERROR in epoll_ctl() %s\n", strerror(errno));
		return 0;
	}
	
	int nfds;
	int conn_sock;
	int addrlen=sizeof(addr);
	int sd;
	while (1){
		nfds = epoll_wait(epollfd,events,30,-1);
		if (nfds==-1){
			printf("ERROR in epoll_wait() %s\n", strerror(errno));
			return 0;
		}
		for (int i=0;i<nfds;i++){
			if (events[i].data.fd = master_socket){
				conn_sock= accept(master_socket,(struct sockaddr *) &addr,(socklen_t*)  &addrlen);
				if (conn_sock==-1){
					printf("ERROR in accept() %s\n", strerror(errno));
					return 0;
				}
				printf("New connection is connected\n");
				setnonblocking(conn_sock);
				ev.events =  EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) ==-1){
					printf("ERROR in epoll_ctl() %s\n", strerror(errno));
					return 0;
				}
				for (int i=0;i<30;i++){
					if (client_socket[i]==0){
						client_socket[i]=conn_sock;
						break;
					}
				}
			}
			else {
				int num;
				for (int i=0;i<30;i++){
					sd = client_socket[i];
					if (events[i].events & EPOLLIN){
						num=read(sd,buffer,1024);
						if (num==0){
							printf("Host disconnected\n");
							close(sd);
							client_socket[i]=0;
						}
						else{
							buffer[num]='\0';
							send(sd,buffer,strlen(buffer),0);
						}
					}
				}
			}
		}	
	}
	return 1;
}
