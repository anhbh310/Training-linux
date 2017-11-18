#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <string.h>
int main(int argc, char* argv[]){
	char buffer[1025];
	int master_socket=socket(AF_INET,SOCK_STREAM,0);
	if (master_socket==0){
		 printf("ERROR in socket() %s\n", strerror(errno));
		 return 0;
	}
	
	int client_socket[30];
	for (int i=0;i<30;i++){
		client_socket[i]=0;
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
	
	fd_set readfds;	
	int sd;
	int max_sd=0;
	int act;
	int new_socket;
	int addrlen=sizeof(addr);
	while (1){
		FD_ZERO(&readfds);
		FD_SET(master_socket,&readfds);
		max_sd=master_socket;
		for (int i=0;i<30;i++){
			sd = client_socket[i];
			if (sd>0){
				FD_SET(sd,&readfds);
			}
			if (sd>max_sd){
				max_sd=sd;
			}
		}
		act=select(max_sd+1,&readfds,NULL,NULL,NULL);
		if ((act<0)&(errno!=EINTR)){
			printf("ERROR in select() %s\n", strerror(errno));
			return 0;
		}
		
		if (FD_ISSET(master_socket,&readfds)){
			new_socket=accept(master_socket,(struct sockaddr *)&addr,(socklen_t*) &addrlen);
			if (new_socket<0){
				printf("ERROR in accept() %s\n", strerror(errno));
				return 0;
			}
			printf("New connection is connected\n");
			for (int i=0;i<30;i++){
				if (client_socket[i]==0){
					client_socket[i]=new_socket;
					break;
				}
			}
		}
		else{
			int num;
			for (int i=0;i<30;i++){
				sd = client_socket[i];
				if (FD_ISSET(sd,&readfds)){
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
	return 1;
}
