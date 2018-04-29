// Initialize the first semaphore in the set with 1
semctl(sem_id, 0, SETVAL, 1);

// Initialize the second semaphore in the set with 10
semctl(sem_id, 1, SETVAL, 10);

// Initialize the third semaphore in the set with 0
semctl(sem_id, 2, SETVAL, 0);


