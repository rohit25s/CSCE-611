#include "common.h"
#include "message.h"

extern pthread_mutex_t mutex;
extern sem_t* full;
extern sem_t* empty;
extern sem_t* mqAccess;

mqd_t mq;
// mqueue attributes
struct mq_attr attr;
	

void sendRequestMessage(int myID)
{
	sem_wait(empty);
	pthread_mutex_lock(&mutex);
	struct message mes;
	mes.cid = myID;
	mes.timestamp = time(NULL);
	mes.status = 0;
	
	mq_send(mq, (const char *) &mes, sizeof(mes)+1, 0);
	sem_post(full);
	pthread_mutex_unlock(&mutex);
	return;
}


void *consume(void *a)
{
	mq_getattr(mq, &attr);
	
	// message queue buffer to store incoming and outgoing messages
	
	// open the queue and create it if doesn't exist

	// printf("Open mqueue consumer %s\n", MQUEUE);
	if ((mq = mq_open(MQUEUE, O_RDWR)) == -1)
	{
		perror("open queue consumer error");
		exit(EXIT_FAILURE);
	}

	// printf("queue open consumer\n");

	bool canConsume = false;
	int terminate = 3;
	int myID = *((int *)a);

	while(1)
	{	
		if(canConsume)
		{
			sem_wait(full);
			pthread_mutex_lock(&mutex);
			struct message mes;

			//printf("Waiting for message\n");
			if (mq_receive(mq, (char *) &mes, 8192, NULL) == -1)
			{
				perror("receive consumer");
				exit(EXIT_FAILURE);
			}
			//printf("Message received\n");
				if(terminate<=0)
					break;	
			sem_post(empty);
			if(mes.cid != myID)
			{
				sem_wait(empty);
				mq_send(mq, (const char *) &mes, sizeof(mes)+1, 10);
				sem_post(full);
				pthread_mutex_unlock(&mutex);
			}
			else 
			{
				if(mes.status == 1)
				{
					pthread_mutex_unlock(&mutex);
					printf("[Consumer %i] Data consumed = %d\n",myID, mes.data);
					canConsume = !canConsume;
					terminate--;
					if(terminate<=0)
						break;
				}
				else
				{
					sem_wait(empty);
					mq_send(mq, (const char *) &mes, sizeof(mes)+1, 10);
					sem_post(full);
					pthread_mutex_unlock(&mutex);
				}
			}
			
		}
		else if(!canConsume && terminate>0)
		{
			sendRequestMessage(myID);
			canConsume = !canConsume;
		}
		usleep(50);
	}
	printf("\nconsumer %d exiting.\n", myID);
	//close(fd);
	return NULL;
}

void start_consumer()
{


	int i=0;
	int a[NUM_CONSUMERS];
	pthread_t cthread[NUM_CONSUMERS];
	for(i=0;i<NUM_CONSUMERS;i++)
	{
		a[i] = i+1;
		printf("[Consumer %d] created\n", a[i]);
		pthread_create(&cthread[i], NULL, (void *)consume, (void *)&a[i]);
	}
	for(i=0;i<NUM_CONSUMERS;i++)
	{
		pthread_join(cthread[i], NULL);
	}

	struct message mes;
	mes.status = 2;

	sem_wait(empty);
	mq_send(mq, (const char *) &mes, sizeof(mes), 10);
	sem_post(full);
	
	printf("\nConsumer done consuming.\n");
	mq_close(mq);
	return;

}
