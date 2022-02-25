#ifndef COMMON_H_
#define COMMON_H_

#define BUF_SIZE 10
#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 10
#define NAMED_PIPE "./foo"
#define MQUEUE "/bar3"
#define MAX_SIZE 1000
#define SLEEP_TIME_MICRO 50
#define SEM_FULL "/full"
#define SEM_EMPTY "/empty"
#define SEM_MQ_ACCESS "/mqAccess"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <mqueue.h>
#include "message.h"
#include <semaphore.h>
#include <string.h>

void start_consumer();
void start_producer();
void *consume(void *a);
void *produce(void *a);
#endif
