#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>

struct mosq
{
	int socket;
	struct sockaddr_in socket_addr;
	int port;
	char hname[16];
	char cId[24];
	int connect;
	char topic[50];
	int pubid;
	int subid;
};
