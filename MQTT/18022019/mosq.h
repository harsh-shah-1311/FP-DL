#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>

struct mosq
{
	int socket;
	//struct sockaddr_in socket_addr;
	struct sockaddr_in socket_addr;
	int connect;
	int port;
	char hname[16];
	char cname[24];
	int pubmsgid;
	int submsgid;
	char topic[24];
};

enum QOS
{
	QOS0=1,
	QOS1,
	QOS2
};
