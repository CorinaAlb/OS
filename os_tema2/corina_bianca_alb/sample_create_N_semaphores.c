#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int sem_id;

int main(int argc, char *argv[])
{
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem_id < 0) {
        perror("Error creating the semaphore set");
        exit(2);
    }
}

