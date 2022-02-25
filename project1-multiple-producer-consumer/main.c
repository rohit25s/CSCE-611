/*
Name - Rohit Sah
UIN: 831000250
People I collaborated with: Abhishek Sinha, Shubham Gupta, Rohan Chaudhury
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

	mqd_t mq;
	static struct mq_attr attr;
	memset(&attr, 0x00, sizeof(struct mq_attr));
	attr.mq_flags = 0;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = 1024;
	attr.mq_curmsgs = 0;

	printf("Create message queue for communication\n");
	mq = mq_open(MQUEUE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, NULL);

	if (mq == (mqd_t) -1)
    {
        perror("queue open error");
		exit(EXIT_FAILURE);
    }
	printf("message queue create\n");



	if (mq_getattr(mq, &attr) == -1)
    {
    	perror("attr fetch error");
		exit(EXIT_FAILURE);
    }

    printf("Maximum # of messages on queue:   %ld\n", attr.mq_maxmsg);
    printf("Maximum message size:             %ld\n", attr.mq_msgsize);

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

		printf("reached end of program, destroy\n");

		pthread_mutex_destroy(&mutex);
		sem_destroy(empty);
		sem_destroy(full);
		sem_destroy(mqAccess);
		mq_unlink(MQUEUE);

		printf("destroyed resources\n");

	}

    return 0;
}
