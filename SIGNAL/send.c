#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>

pid_t recvPID;

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

/*void signalHandler(){
	kill(recvPID,SIGINT);
	exit(1);
}*/

int main(){
	//signal(SIGINT, signalHandler);
	int p;
	printf("Recive PID = ");
	scanf("%d",&p);
	recvPID = p;
	while (1){
		printf("Send SIGSTOP \n");
		kill(recvPID,SIGSTOP);
		delay(5000);
		printf("Send SIGCONT \n");
		kill(recvPID,SIGCONT);
		delay(5000);
	}
	return 1;
}
