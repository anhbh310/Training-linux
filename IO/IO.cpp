#include <string>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
using namespace std;
int main(){
	cout << "Stream I/O" << endl;
	
	int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd <0){
		printf("ERROR");
	}
	int stt = write(fd, "SYSCALL IO \n", sizeof("SYSCALL IO \n"));
	if (stt <0){
		printf("ERROR");
	}
	
	close(fd);
	
	return 1;
}
