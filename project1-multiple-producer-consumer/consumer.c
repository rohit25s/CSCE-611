#include "common.h"
#include "message.h"

extern pthread_mutex_t mutex;
extern sem_t* full;
extern sem_t* empty;

void sendRequestMessage(int *fd, int myID)
{
	sem_wait(empty);
	pthread_mutex_lock(&mutex);
	struct message mes;
	mes.cid = myID;
	mes.timestamp = time(NULL);
	mes.status = 0;
	
	write(*fd, &mes, sizeof(struct message));
	sem_post(full);
	pthread_mutex_unlock(&mutex);
	return;
}


void *ResourceRequester(void *a)
{
	bool canConsume = false;
	int terminate = 3;
	int myID = *((int *)a);
	int fd = open(NAMED_PIPE, O_RDWR);
	if (fd < 0)
	{
		printf("Unable to open pipe in consumer...Terminating\n");
		exit(EXIT_FAILURE);
	}
	while(1)
	{	
		if(canConsume)
		{
			sem_wait(full);
			pthread_mutex_lock(&mutex);
			struct message mes;
			if(read(fd, &mes, sizeof(struct message))<0 && terminate<=0)
				break;	
			sem_post(empty);
			if(mes.cid != myID)
			{
				sem_wait(empty);
				write(fd, &mes, sizeof(struct message));
				sem_post(full);
				pthread_mutex_unlock(&mutex);
			}
			else 
			{
				if(mes.status == 1)
				{
					pthread_mutex_unlock(&mutex);
					printf("[Consumer %i] data consumed = %d\n",myID, mes.data);
					canConsume = !canConsume;
					terminate--;
					if(terminate<=0)
						break;
				}
				else
				{
					sem_wait(empty);
					write(fd, &mes, sizeof(struct message));
					sem_post(full);
					pthread_mutex_unlock(&mutex);
				}
			}
			
		}
		else if(!canConsume && terminate>0)
		{
			sendRequestMessage(&fd, myID);
			canConsume = !canConsume;
		}
		usleep(SLEEP_TIME_MICRO);
	}
	printf("\nconsumer %d exiting...\n", myID);
	close(fd);
	return NULL;
}

void start_consumer()
{
	int i=0;
	int a[NUM_CONSUMERS];
	pthread_t consumer_thread[NUM_CONSUMERS];
	for(i=0;i<NUM_CONSUMERS;i++)
	{
		a[i] = i+1;
		pthread_create(&consumer_thread[i], NULL, (void *)ResourceRequester, (void *)&a[i]);
	}
	for(i=0;i<NUM_CONSUMERS;i++)
	{
		pthread_join(consumer_thread[i], NULL);
	}

	int fd = open(NAMED_PIPE, O_RDWR);
	struct message mes;
	mes.status = 2;

	sem_wait(empty);
	write(fd, &mes, sizeof(struct message));
	sem_post(full);
	
	printf("\nConsumer done consuming...\n");
	unlink(NAMED_PIPE);
	return;

}
