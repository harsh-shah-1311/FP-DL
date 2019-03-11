#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>

int main()
{
	int sfd,sz;
	struct sockaddr_in v,v1;
	int size,i=0;
	int nsfd[10];
	char buf[100];
	char msg[100],main_msg[100];

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0)
	{
		perror("socket\n\r");
		return 0;
	}

	v.sin_family = AF_INET;
	v.sin_port = htons(1883);
	v.sin_addr.s_addr = inet_addr("0.0.0.0");

	if(bind(sfd,(struct sockaddr *)&v, sizeof(v)))
	{
		perror("bind\n\r");
		return 0;
	}

	if(listen(sfd,1))
	{
		perror("listen\n\r");
		return 0;
	}
//	printf("listen successfully\n\r");
	v1.sin_family = AF_INET;
	v1.sin_port = htons(1883);
	v1.sin_addr.s_addr = inet_addr("0.0.0.0");
	size = sizeof(v1);

	while(1)
	{
		nsfd[i] = accept(sfd, (struct sockaddr *)&v1, &size);
		if(nsfd[i]>0)
		{
			send(nsfd[i],"CONNACK",sizeof("CONNACK"),0);
		}
		printf("connected with client successfully\n\r");

		sz = recv(nsfd[i],buf,sizeof(buf),0);
		if(size<=0)
		{
			perror("recv\n\r");
			return 0;
		}

		puts(buf);		

		if(recv(nsfd[i],msg,sizeof(msg),0))
		{
			if(strcmp(msg,"PUBLISH")==0)
			{
				printf("publish the message req\n\r");
				if(recv(nsfd[i],msg,sizeof(msg),0))
				{
					strcpy(main_msg,msg);
					puts(main_msg);
				}
				send(nsfd[i],"PUBACK",sizeof("PUBACK"),0);
			}
			else if(strcmp(msg,"SUBSCRIBE")==0)
			{
				printf("subscribe the topic req\n\r");
				if(recv(nsfd[i],msg,sizeof(msg),0))
                                {
//                                        strcpy(main_msg,msg);
                                        puts(msg);
                                }

				send(nsfd[i],"SUBACK",sizeof("SUBACK"),0);
				sleep(2);
				send(nsfd[i],main_msg,sizeof(main_msg),0);
			}
		}
	}
}
