int i;
pthread_t th[1000];

for (i=0; i<N; i++)
  pthread_join(th[i], NULL);

