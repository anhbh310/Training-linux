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

static int make_socket_non_blocking (int sfd){
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}

int main(){
	int s;
	char buffer[1025];	
	
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
	int sd;
	while (1){
		nfds = epoll_wait(epollfd,events,30,-1);
		if (nfds==-1){
			printf("ERROR in epoll_wait() %s\n", strerror(errno));
			return 0;
		}
		for (int i=0;i<nfds;i++){
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
				printf("ERROR in epoll error %s\n", strerror(errno));
				close(events[i].data.fd);
				continue;
			}
			else {
				if (events[i].data.fd == master_socket){
					struct sockaddr in_addr;
					int conn_sock;
					socklen_t in_len=sizeof(in_addr);
					conn_sock= accept(master_socket,(struct sockaddr *) &in_addr,(socklen_t*)  &in_len);
					if (conn_sock==-1){
						if ((errno ==EAGAIN) || (errno == EWOULDBLOCK)){
								break;
						}
						else {
							printf("ERROR in accept() %s\n", strerror(errno));
							break;
						}
					}
					printf("New connection is connected\n");
					s=make_socket_non_blocking(conn_sock);
					if (s==-1) abort();
					ev.data.fd=conn_sock;
					ev.events =  EPOLLIN | EPOLLET;
					ev.data.fd = conn_sock;
					if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) ==-1){
						printf("ERROR in epoll_ctl() %s\n", strerror(errno));
						return 0;
					}
					continue;
				}
				else{
					int done =0;
					while (1){
						sd = events[i].data.fd;
						int num=read(sd,buffer,1024);
						if (num ==-1){
							if (errno != EAGAIN){
								perror("read");
								done =1;
							}
							break;
						}
						else{
							if (num==0){
								done =1;
								break;
							}
						}
						buffer[num]='\0';
						send(sd,buffer,strlen(buffer),0);
					}
					if (done){
						printf("Host disconnected\n");
						close(sd);
					}
				}
			}
		}	
	}
	free(events);
	close(nfds);
	return 1;
}
