#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

void signalHandler(){
	
	int fd;
    char * myfifo = "/tmp/myfifo";
    char buf[10];

    fd = open(myfifo, O_RDONLY);
	read(fd, buf, 10);
	close(fd);
	
	int pid =atoi(buf);
	
	printf("%d",pid);
	
	char* mem_file_name=malloc(50);
	char* maps_file_name=malloc(50);
	sprintf(maps_file_name, "/proc/%d/maps", pid);
	sprintf(mem_file_name, "/proc/%d/mem", pid);
	FILE* file_maps = fopen(maps_file_name, "r"); 
	size_t len =0;
	char* st=NULL;
	FILE* dumpmem = fopen("dumpmem","wb");

	while (getline(&st, &len, file_maps) != -1) {
		char *ch;
		ch = strtok(st, " ");
		char* strings[6];
		int i=0;
		while (ch != NULL) {
			strings[i]=ch;
			i++;
			ch = strtok(NULL, " ,");
		}
		char* offset[2];
		i=0;
		ch = strtok(strings[0],"-");
		while (ch != NULL){
			offset[i]=ch;
			i++;
			ch=strtok(NULL," ,");
		}
		long long int start=0;
		long long int end=0;
		for (int i=0;i<2;i++){
			long long int number = (long long int)strtol(offset[i], NULL, 16);
			if (i==0){
				start=number;
			}
			else{
				end=number;
			}
		}
		char buf[end-start+1];
		char *te=strings[0];
		char *t=strings[1];
		if (*t == 'r' && *te!='f') {
			int file_mem;
			file_mem = open(mem_file_name, O_RDONLY);
			if (file_mem==-1){
				printf("ERROR in open %s \n", strerror(errno));
			}
			if (ptrace(PTRACE_ATTACH, pid, NULL, NULL)==-1){
				printf("ERROR in ptrace %s \n", strerror(errno));
			}
			waitpid(pid, NULL, 0);
			lseek(file_mem, start, SEEK_SET);
			read(file_mem, buf, end-start);
			if (ptrace(PTRACE_DETACH, pid, NULL, NULL)==-1){
				printf("ERROR in ptrace %s \n", strerror(errno));
			}
			fputs(strings[5],dumpmem);
			fputs("\n",dumpmem);
			for (int i=0;i<end-start;i++){
				fputc(buf[i],dumpmem);
			}
			fputs("\n",dumpmem);
			close(file_mem);
		}
	}
	fclose(file_maps);
	fclose(dumpmem);
	exit(1);
}

int main(){
	pid_t p = getpid();
	signal(SIGSEGV, signalHandler);
	while (1){
		printf("Program PID = %d is running ... \n",p);
		delay(1000);
	}
	return 1;
}
