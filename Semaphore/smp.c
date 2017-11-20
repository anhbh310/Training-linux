#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

void* create_shared_memory(size_t size) {
  int protection = PROT_READ | PROT_WRITE;
  int visibility = MAP_ANONYMOUS | MAP_SHARED;
  return mmap(NULL, size, protection, visibility, 0, 0);
}

int main() {
  sem_t mutex;

  void* shmem = create_shared_memory(512);
  sem_init(&mutex,1,1);

  

  int pid = fork();

  if (pid == 0) {
	printf("parent say: ");
	sem_wait(&mutex);
	memcpy(shmem, "parent using shared memory", sizeof("using shared memory"));
    printf("%s \n", shmem);
    sem_post(&mutex);
  } else {
	printf("child say: ");
	sem_wait(&mutex);
	memcpy(shmem, "child using shared memory", sizeof("child using shared memory"));
    printf("%s \n", shmem);
    sem_post(&mutex);   
  }
}
