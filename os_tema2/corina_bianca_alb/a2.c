#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "a2_helper.h"

#define MAX_TH_PROCESS_8    6
#define MAX_TH_PROCESS_7    38
#define MAX_TH_PROCESS_5    4

#define SEM_TH_8_3_END      0
#define SEM_TH_5_4_END      1

pthread_mutex_t lock, lock13;
pthread_cond_t cond, cond13;

int sem_id;
int thNo = 1;
int thNo13 = 0;

pthread_t th7[MAX_TH_PROCESS_7];

// decrement
void P(int semId, int semNr)
{
    struct sembuf op = {semNr, -1, 0};

    semop(semId, &op, 1);
}

// increment
void V(int semId, int semNr)
{
    struct sembuf op = {semNr, +1, 0};

    semop(semId, &op, 1);
}

void* thread5_exec(void* thread)
{
    int number = *((int *)thread);

    if (number == 4)
    {
        P(sem_id, SEM_TH_8_3_END);
        info(BEGIN, 5, number);

        info(END, 5, number);
        V(sem_id, SEM_TH_5_4_END);
    } else {
        info(BEGIN, 5, number);
    
        if (number == 2)
        {
            int i = 3;
            pthread_t thread;
            pthread_create(&thread, NULL, thread5_exec, &i);
            pthread_join(thread, NULL);
        }

        info(END, 5, number);
    }

    return NULL;
}

void* thread7_13_exec(void* thread)
{
    int number = *((int *)thread);

    thNo13++;
    info(BEGIN, 7, number);    

    if (pthread_mutex_lock(&lock13) != 0) {
        perror("Cannot take the lock");
        exit(1);
    }

    if (number == 13)
    {
        thNo13--;
        info(END, 7, number);
    }
    
    if (thNo < 6)
    {
        if (pthread_mutex_unlock(&lock13) != 0) 
        {
            perror("Cannot release the lock");
            exit(3);
        }
    }

    if (pthread_mutex_lock(&lock13) != 0) {
        perror("Cannot take the lock");
        exit(1);
    }

    if (number != 13)
    {
        thNo13--;
        info(END, 7, number);
    }
    
    if (pthread_mutex_unlock(&lock13) != 0) 
    {
        perror("Cannot release the lock");
        exit(3);
    }    

    return NULL;
}

void* thread7_exec(void* thread)
{
    int number = *((int *)thread);

    // Check if entrance in the limited area is allowed
    if (pthread_mutex_lock(&lock) != 0) {
    	perror("Cannot take the lock");
    	exit(1);
    }
    
    while (thNo >= 6)
    {
        if (pthread_cond_wait(&cond, &lock) != 0)
        {
            perror("Cannot wait for condition");
            exit(2);
        }
    }
    
    if (pthread_mutex_unlock(&lock) != 0) 
    {
    	perror("Cannot release the lock");
    	exit(3);
    }

    thNo++;
	info(BEGIN, 7, number);   

    // Check if exit from the limited area is allowed
    if (pthread_mutex_lock(&lock) != 0) {
    	perror("Cannot take the lock");
    	exit(10);
    }

    thNo--;
    info(END, 7, number);

    // signal that a new place is available
    if (pthread_cond_signal(&cond)  != 0) {
    	perror("Cannot signal the condition waiters");
    	exit(11);
    }

    if (pthread_mutex_unlock(&lock) != 0) {
    	perror("Cannot release the lock");
    	exit(12);
    }

    return NULL;
}

void* thread8_exec(void* thread)
{
    int number = *((int *)thread);

    if (number == 1)
    {
        P(sem_id, SEM_TH_5_4_END);
        info(BEGIN, 8, number);
    } else {
        info(BEGIN, 8, number);
    }

    if (number == 3)
    {      
        info(END, 8, number);
        V(sem_id, SEM_TH_8_3_END);
    } else {
        info(END, 8, number);
    }

    return NULL;
}

int main()
{
    init();

    sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    if (sem_id < 0) {
        perror("Error creating the semaphore set");
        exit(2);
    }

    semctl(sem_id, SEM_TH_8_3_END, SETVAL, 0);
    semctl(sem_id, SEM_TH_5_4_END, SETVAL, 0);

    info(BEGIN, 1, 0);
    
    int pid_2 = fork();

    if (pid_2 == 0)
    {
        info(BEGIN, 2, 0);

        int pid_3 = fork();

        if (pid_3 == 0)
        {
            info(BEGIN, 3, 0);

            int pid_4 = fork();
            
            if (pid_4 == 0)
            {
                info(BEGIN, 4, 0);

                int pid_8 = fork();

                if (pid_8 == 0)
                {
                    info(BEGIN, 8, 0);

                    if (pthread_mutex_init(&lock, NULL) != 0) {
                        perror("Cannot initialize the lock");
                        exit(2);
                    }
                    
                    if (pthread_cond_init(&cond, NULL) != 0) {
                        perror("Cannot initialize the condition variable");
                        exit(3);
                    }

                    int i;
                    pthread_t th[MAX_TH_PROCESS_8];
                    int th_args[MAX_TH_PROCESS_8];    

                    // Create the N threads
                    for (i=1; i<=MAX_TH_PROCESS_8; i++) {
                        th_args[i-1] = i;
                        if (pthread_create(&th[i-1], NULL, thread8_exec, & th_args[i-1]) != 0 ) {
                            perror("cannot create threads");
                            exit(4);
                        }                            
                    }

                    // Wait for the termination of the N threads created
                    for (i=1; i<=MAX_TH_PROCESS_8; i++) {
                        pthread_join(th[i-1], NULL);
                    }

                    if (pthread_mutex_destroy(&lock) != 0) {
                        perror("Cannot destroy the lock");
                        exit(8);
                    }

                    if (pthread_cond_destroy(&cond) != 0) {
                        perror("Cannot destroy the condition variable");
                        exit(9);
                    }

                    info(END, 8, 0);                  
                }
                else
                {
                    int pid_9 = fork();

                    if (pid_9 == 0)
                    {
                        info(BEGIN, 9, 0);
                      
                        info(END, 9, 0);                  
                    }
                    else
                    {
                        waitpid(pid_8, NULL, 0);
                        waitpid(pid_9, NULL, 0);

                        info(END, 4, 0);
                    }
                }
            }
            else
            {
                int pid_6 = fork();
                if (pid_6 == 0)
                {
                    info(BEGIN, 6, 0);

                    int pid_7 = fork();

                    if (pid_7 == 0)
                    {
                        info(BEGIN, 7, 0);
                        
                        // Create the lock to provide mutual exclusion for the concurrent threads
                        if (pthread_mutex_init(&lock, NULL) != 0) {
                            perror("Cannot initialize the lock");
                            exit(2);
                        }

                        if (pthread_mutex_init(&lock13, NULL) != 0) {
                            perror("Cannot initialize the lock");
                            exit(2);
                        }
                        
                        if (pthread_cond_init(&cond, NULL) != 0) {
                            perror("Cannot initialize the condition variable");
                            exit(3);
                        }

                        if (pthread_cond_init(&cond13, NULL) != 0) {
                            perror("Cannot initialize the condition variable");
                            exit(3);
                        }

                        int i;
                        int th_args[MAX_TH_PROCESS_7];                        
                        th_args[12] = 13;

                        if (pthread_create(&th7[12], NULL, thread7_13_exec, & th_args[12]) != 0 ) {
                            perror("cannot create threads");
                            exit(4);
                        }

                        for (i=1; i<6; i++) {
                            th_args[i-1] = i;
                            if (pthread_create(&th7[i-1], NULL, thread7_13_exec, & th_args[i-1]) != 0 ) {
                                perror("cannot create threads");
                                exit(4);
                            }                
                        }

                        pthread_join(th7[12], NULL);

                        for (i=1; i<6; i++) 
                        {
                            pthread_join(th7[i-1], NULL);
                        }

                        if (pthread_mutex_destroy(&lock13) != 0) {
                            perror("Cannot destroy the lock");
                            exit(8);
                        }

                        if (pthread_cond_destroy(&cond13) != 0) {
                            perror("Cannot destroy the condition variable");
                            exit(9);
                        }

                        // Create the N threads
                        for (i=6; i<=MAX_TH_PROCESS_7; i++) 
                        {
                            if (i != 13)
                            {
                                th_args[i-1] = i;
                                if (pthread_create(&th7[i-1], NULL, thread7_exec, & th_args[i-1]) != 0 ) {
                                    perror("cannot create threads");
                                    exit(4);
                                }                
                            }                                        
                        }

                        // Wait for the termination of the N threads created
                        for (i=6; i<=MAX_TH_PROCESS_7; i++) 
                        {
                            if (i != 13)
                            {
                                pthread_join(th7[i-1], NULL);
                            }
                        }

                        // Remove the lock
                        if (pthread_mutex_destroy(&lock) != 0) {
                            perror("Cannot destroy the lock");
                            exit(8);
                        }                          

                        if (pthread_cond_destroy(&cond) != 0) {
                            perror("Cannot destroy the condition variable");
                            exit(9);
                        }

                        info(END, 7, 0);
                    }
                    else
                    {
                        waitpid(pid_7, NULL, 0);

                        info(END, 6, 0);
                    }       
                }
                else
                {
                    waitpid(pid_4, NULL, 0);
                    waitpid(pid_6, NULL, 0);

                    info(END, 3, 0);
                }                                                        
            }
        }
        else
        {
            waitpid(pid_3, NULL, 0);

            info(END, 2, 0);
        }
    }
    else
    {
        int pid_5 = fork();

        if (pid_5 == 0)
        {
            info(BEGIN, 5, 0);
            
            int i;
            pthread_t th[MAX_TH_PROCESS_5];
            int th_args[MAX_TH_PROCESS_5];

            for (i=1; i<=MAX_TH_PROCESS_5; i++)
            {
                th_args[i-1] = i;
                if (i != 3) 
                {
                    pthread_create(&th[i-1], NULL, thread5_exec, & th_args[i-1]);
                }                
            }

            // Wait for the termination of the N threads created
            for (i=1; i<=MAX_TH_PROCESS_5; i++) 
            {
                if (i != 3) 
                {
                    pthread_join(th[i-1], NULL);
                }
            }

            info(END, 5, 0);
        }
        else
        {
            waitpid(pid_5, NULL, 0);
            waitpid(pid_2, NULL, 0);
    
            info(END, 1, 0);
        }
    }
    
    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}
