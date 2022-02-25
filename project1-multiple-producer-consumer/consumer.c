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
	
	//write(*fd, &mes, sizeof(struct message));
	mq_send(mq, (const char *) &mes, attr.mq_msgsize, 0);
	sem_post(full);
	pthread_mutex_unlock(&mutex);
	return;
}


void *ResourceRequester(void *a)
{
	attr.mq_flags = 0;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = 1000;
	attr.mq_curmsgs = 0;
	// message buffer to store incoming and outgoing messages
	
	// open the queue and create it if doesn't exist
	printf("Open mqueue consumer %s\n", MQUEUE);
	if ((mq = mq_open(MQUEUE, O_RDWR)) == -1)
	{
		perror("open queue consumer error");
		exit(EXIT_FAILURE);
	}

	printf("queue open consumer");
	usleep(1000);

	bool canConsume = false;
	int terminate = 3;
	int myID = *((int *)a);
	// int fd = open(NAMED_PIPE, O_RDWR);
	// if (fd < 0)
	// {
	// 	printf("Unable to open pipe in consumer...Terminating\n");
	// 	exit(EXIT_FAILURE);
	// }
	while(1)
	{	
		if(canConsume)
		{
			sem_wait(full);
			pthread_mutex_lock(&mutex);
			struct message mes;

			printf("Waiting for message\n");
			if (mq_receive(mq, (char *) &mes, attr.mq_msgsize, NULL) == -1)
			{
				perror("receive");
				exit(EXIT_FAILURE);
			}
			printf("Message received\n");
				if(terminate<=0)
					break;	
			sem_post(empty);
			if(mes.cid != myID)
			{
				sem_wait(empty);
				//write(fd, &mes, sizeof(struct message));
				mq_send(mq, (const char *) &mes, attr.mq_msgsize, 10);
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
					//write(fd, &mes, sizeof(struct message));
					mq_send(mq, (const char *) &mes, attr.mq_msgsize, 10);
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
		usleep(SLEEP_TIME_MICRO);
	}
	printf("\nconsumer %d exiting...\n", myID);
	//close(fd);
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

	//int fd = open(NAMED_PIPE, O_RDWR);
	struct message mes;
	mes.status = 2;

	sem_wait(empty);
	mq_send(mq, (const char *) &mes, sizeof(struct message), 10);
	//write(fd, &mes, sizeof(struct message));
	sem_post(full);
	
	printf("\nConsumer done consuming...\n");
	//unlink(NAMED_PIPE);
	mq_close(mq);
	return;

}
