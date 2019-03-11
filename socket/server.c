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

/******************* BIND THE SOCKET WITH IP ADDRESS ************************/

	s1.sin_family = AF_INET;
	s1.sin_port = atoi(argv[1]);
	s1.sin_addr.s_addr = inet_addr("0.0.0.0");

	if(bind(sfd, (struct sockaddr *)&s1, sizeof(struct sockaddr)))
	{
		perror("bind");
		return 0;
	}

	printf("bind the socket with IP sucessfully...\n");

/***************** HOW-MANY CONNECTIONS ARE IN THE RAW *********************/

	if(listen(sfd,1))
	{
		perror("listen");
		return 0;
	}

	printf("listen sucessfully...\n");

/********************ACCEPT THE CLIENT CONNECTION REQUEST ******************/

	s2.sin_family = AF_INET;
	s2.sin_port = atoi(argv[1]);
	s2.sin_addr.s_addr = inet_addr("0.0.0.0");
	size = sizeof(struct sockaddr);

	nsfd = accept(sfd, (struct sockaddr *)&s2, &size);
	if(nsfd < 0)
	{
		perror("accept");
		return 0;
	}

	printf("accept the client connection request sucessfully...\n");

/****************** RECEIVE THE DATA ***************************/

	recv(nsfd, str,sizeof(str),0);
	printf("the received data is : %s\n", str);

/******************* CONVERT THE DATA ***********************/

	for(int i = 0;str[i];i++)
	{
		if(str[i]>='a' && str[i]<='z')
			str[i]-=32;
	}

/***************** SEND THE DATA ***********************/

	send(nsfd, str, strlen(str), 0);

	return 0;
}
