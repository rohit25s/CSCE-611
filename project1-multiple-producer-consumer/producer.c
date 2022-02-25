#include "common.h"
#include "message.h"

extern pthread_mutex_t mutex;
extern sem_t *full;
extern sem_t *empty;
extern sem_t *mqAccess;


mqd_t mq;
// mqueue attributes
struct mq_attr attr;
	

void *produce(void *a)
{

	attr.mq_flags = 0;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = 8192;
	attr.mq_curmsgs = 0;

	// signal interrupt
	int sig;
	sigset_t signalset;
	struct sigevent event;

	// open the queue and create it if doesn't exist
	printf("Open mqueue producer %s\n", MQUEUE);
	if ((mq = mq_open(MQUEUE, O_RDWR)) == -1)
	{
		perror("open queue producer error");
		exit(EXIT_FAILURE);
	}
	printf("queue opened producer\n");
	
	int myID = *((int *)a);

	while(1)
	{	
		sem_wait(full);
		pthread_mutex_lock(&mutex);
		struct message mes;
		mes.status = 0;
		//int n = mq_receive(mq, (char *) &mes, sizeof(struct message), NULL);
		//printf("Waiting for message\n");
		if (mq_receive(mq, (char *) &mes, 8192, NULL) == -1)
		{
			perror("receive producer error");
			exit(EXIT_FAILURE);
		}
		//printf("Message received\n");
		sem_post(empty);
		int q = 0;
		if(mes.status == 1)
		{
			sem_wait(empty);
			mq_send(mq, (const char *) &mes, sizeof(mes)+1, 10);
			sem_post(full);
			pthread_mutex_unlock(&mutex);
			
		}
		else if(mes.status == 2)
		{
			sem_wait(empty);
			mq_send(mq, (const char *) &mes, sizeof(mes)+1, 10);
			sem_post(full);
			pthread_mutex_unlock(&mutex);
			printf("\nProducer %d exiting.\n", myID);
			break;
		}
		else
		{	
			mes.status=1;
			mes.data = rand()%100 + 1;
				
			mes.timestamp = time(NULL);
			sem_wait(empty);
			mq_send(mq, (const char *) &mes, sizeof(mes)+1, 10);
			printf("[Producer %d] Data produced = %d\n",myID, mes.data);
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
		pthread_create(&producer_thread[i], NULL, (void *)produce, (void *)&a[i]);
	}
	for(i=0;i<NUM_PRODUCERS;i++)
	{
		pthread_join(producer_thread[i], NULL);
	}
	printf("\nProducer done Producing. Exiting.\n");
	mq_close(mq);
	return;
}	
