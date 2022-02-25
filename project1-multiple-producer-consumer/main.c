/*
Name - Abhishek Sinha
UIN: 232003268
People I collaborated with: Shubham Gupta, Rohit Sah, Rohan Chaudhury, Sherine Davis 
*/

#include "common.h"
#include "message.h"

sem_t* full;
sem_t* empty;
pthread_mutex_t mutex;

int main()
{
	sem_unlink(SEM_EMPTY);
	sem_unlink(SEM_FULL);
	sem_destroy(empty);
	sem_destroy(full);
	empty = sem_open(SEM_EMPTY, O_CREAT,0666, BUF_SIZE);
	full = sem_open(SEM_FULL, O_CREAT,0666, 0);
	int val1,val2; sem_getvalue(empty, &val1);sem_getvalue(full, &val2);
	
	pthread_mutex_init(&mutex, NULL);
	const char* namePipe = NAMED_PIPE;
	mkfifo(namePipe, 0666);
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
		unlink(namePipe);
	}

    return 0;
}
