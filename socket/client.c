#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string.h>
#include<stdlib.h>

int main(int argc, char **argv)
{
	struct sockaddr_in s1,s2;
	int sfd,nsfd,size;
	char str[100];

/*********************** CREATE THE SOCKET *********************/

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0)
	{
		perror("socket");
		return 0;
	}

	printf("socket created sucess fully\n");

/******************** SEND THE CONNECTION REQUEST ******************/

	s2.sin_family = AF_INET;
	s2.sin_port = atoi(argv[1]);
	s2.sin_addr.s_addr = inet_addr("127.0.0.1");
	size = sizeof(struct sockaddr);

	if(connect(sfd, (struct sockaddr *)&s2, size))
	{
		perror("connect");
		return 0;
	}

	printf("connect the client connection request sucessfully...\n");

/******************* SEND THE DATA TO SERVER **********************/

	printf("enter the data :");
	scanf("%s",str);
	send(sfd, str, strlen(str), 0);

/****************** RECEIVE THE DATA ***************************/

	recv(sfd, str,sizeof(str),0);
	printf("the received data is : %s\n", str);

	return 0;
}
