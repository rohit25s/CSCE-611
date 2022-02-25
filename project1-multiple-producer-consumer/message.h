#ifndef MESSAGE_H_
#define MESSAGE_H_


struct message
{
	time_t timestamp;
	int cid;
	int data;
	int status;
};

#endif
