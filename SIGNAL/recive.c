#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

int main(){
	pid_t p = getpid();
	while (1){
		printf("Program PID = %d is running ... \n",p);
		delay(1000);
	}
	return 1;
}
