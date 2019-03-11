#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>

struct data
{
	int id;
	char name[20];
	int update;
	char msg[50];
};

int main()
{
	struct sockaddr_in v;
	int sfd;
	struct data device_data;

/************* Create the broker which stores the data related to device ******************/

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0)
	{
		perror("socket");
		return 0;
	}

/*********** Bind the device with the IP ************************/

	v.sin_family = AF_INET;
	v.sin_port = htons(2500);
	v.sin_addr.s_addr = inet_addr("0.0.0.0");

	if(bind(sfd,(struct sockaddr *)&v, sizeof(v)))
	{
		perror("bind");
		return 0;
	}

/*********** Read the howmany client on the queue ******************/

	if(listen(sfd,10))
	{
		perror("listen");
		return 0;
	}

/************** Start the application loop *********************/

	v1.sin_family = AF_INET;
	v1.sin_port = htons(2500);
	v1.sin_addr.s_addr = inet_addr("0.0.0.0");
	size = sizeof(v);

	while(1)
	{
		nsfd = accept(sfd, (struct sockaddr *)&v1, &size);
		if(nsfd)
		{
			perror("accept");
			return 0;
		}

		

	}
}
