#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int fd;
    char * myfifo = "/tmp/myfifo";

    mkfifo(myfifo, 0666);
    char* s=(char*) malloc( 100 );;
	printf("Send: ");
	fd = open(myfifo, O_WRONLY);
	scanf("%s",s);
	write(fd, s, sizeof(s));	
	
	close(fd);
    unlink(myfifo);

    return 0;
}
