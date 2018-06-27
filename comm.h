#ifndef __COMM_H__
#define __COMM_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>


typedef struct response
{
	char msg[128];
	char ip[16];
	int port;
}response;


#endif
