int i;
pthread_t th[1000];

for (i=0; i<N; i++)
  if (pthread_create(&th[i], NULL, (void* (*) (void*)) increment, (void*) i) != 0) {
    perror("Cannot create threads");
    exit(1);
  }



