/*
Name - Abhishek Sinha
UIN: 232003268
People I collaborated with: Shubham Gupta, Rohit Sah, Rohan Chaudhury, Sherine Davis 
*/

#include "common.h"
#include "message.h"

sem_t* full;
sem_t* empty;
sem_t* mqAccess;
pthread_mutex_t mutex;

int main()
{
	mq_unlink(MQUEUE);
	sem_unlink(SEM_EMPTY);
	sem_unlink(SEM_FULL);
	sem_destroy(empty);
	sem_destroy(full);
	sem_unlink(SEM_MQ_ACCESS);
	sem_destroy(mqAccess);
	empty = sem_open(SEM_EMPTY, O_CREAT,0666, BUF_SIZE);
	full = sem_open(SEM_FULL, O_CREAT,0666, 0);
	mqAccess = sem_open(SEM_MQ_ACCESS, O_CREAT, 0666, 1);

	pthread_mutex_init(&mutex, NULL);
	//const char* namePipe = NAMED_PIPE;
	//mkfifo(namePipe, 0666);

	mqd_t mq;
	if ((mq = mq_open(MQUEUE, O_CREAT, 777, 0)) == -1)
	{
		perror("open queue error");
		exit(EXIT_FAILURE);
	}
	printf("queue opened");

	int producer, consumer;

	producer = fork();

	if(producer == 0)
		start_producer();
	else if (producer>0)
		consumer = fork();
	else
	{
		printf("Could not create the producer. Terminating ...\n");
		exit(EXIT_FAILURE);
	}

	if(producer != 0 && consumer == 0)
		start_consumer();
	else if(producer > 0 && consumer > 0)
	{
		
		while(wait(NULL)>0);
		// printf("here\n");
		pthread_mutex_destroy(&mutex);
		sem_destroy(empty);
		sem_destroy(full);
		sem_destroy(mqAccess);
		//unlink(namePipe);
		mq_unlink(MQUEUE);

	}

    return 0;
}
