#include "common.h"
#include "message.h"

extern pthread_mutex_t mutex;
extern sem_t *full;
extern sem_t *empty;

void *ResourceGenerator(void *a)
{
	int myID = *((int *)a);
	int fd = open(NAMED_PIPE, O_RDWR);
	if (fd < 0)
	{
		printf("Unable to open pipe in producer...Terminating\n");
		exit(EXIT_FAILURE);
	}

	while(1)
	{	
		sem_wait(full);
		pthread_mutex_lock(&mutex);
		struct message mes;
		mes.status = 0;
		read(fd, &mes, sizeof(struct message));
		sem_post(empty);
		int q = 0;
		if(mes.status == 1)
		{
			sem_wait(empty);
			write(fd, &mes, sizeof(struct message));
			sem_post(full);
			pthread_mutex_unlock(&mutex);
			
		}
		else if(mes.status == 2)
		{
			sem_wait(empty);
			write(fd, &mes, sizeof(struct message));
			sem_post(full);
			pthread_mutex_unlock(&mutex);
			printf("\nProducer %d exiting...\n", myID);
			break;
		}
		else
		{	
			mes.status=1;
			mes.data = rand()%100 + 1;
				
			mes.timestamp = time(NULL);
			sem_wait(empty);
			write(fd, &mes, sizeof(struct message));
			printf("[Producer %d] Data produced =%d\n",myID, mes.data);
			sem_post(full);
			pthread_mutex_unlock(&mutex);
		}
		usleep(10);
	}
	return NULL;
}
	
void start_producer()
{
	int i=0;
	int a[NUM_PRODUCERS];
	pthread_t producer_thread[NUM_PRODUCERS];
	
	for(i=0;i<NUM_PRODUCERS;i++)
	{
		a[i] = i+1;
		pthread_create(&producer_thread[i], NULL, (void *)ResourceGenerator, (void *)&a[i]);
	}
	for(i=0;i<NUM_PRODUCERS;i++)
	{
		pthread_join(producer_thread[i], NULL);
	}
	printf("\nProducer done Producing...\n");
	return;
}	
