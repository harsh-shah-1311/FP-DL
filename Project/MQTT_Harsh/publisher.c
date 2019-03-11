#include"headers.h"

struct data mfg_data;
int is_new_conn;

int main()
{
	int sfd,nsfd[10];
	int i=0,j=1,size;
	struct sockaddr_in v,v1;
	char welcome_msg[50]="Wel-come, Thank you for subscribing our channel!";
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0)
	{
		perror("socket");
		return 0;
	}

	v.sin_family = AF_INET;
	v.sin_port = htons(2500);
	v.sin_addr.s_addr = inet_addr("0.0.0.0");

	if(bind(sfd,(struct sockaddr *)&v, sizeof(v)))
	{
		perror("bind");
		return 0;
	}

	if(listen(sfd,10))
	{
		perror("listen");
		return 0;
	}

	while(1)
	{
//		printf("in main application loop\n");
		v1.sin_family = AF_INET;
		v1.sin_port = htons(2500);
		v1.sin_addr.s_addr = inet_addr("0.0.0.0");
		size = sizeof(v1);

		if(is_new_conn == 1)
		{
			printf("in main  application loop\n");
			nsfd[i] = accept(sfd, (struct sockaddr *)&v1, &size);
			if(nsfd<0)
			{
				perror("accept");
				return 0;
			}
	
			write(nsfd[i],welcome_msg,strlen(welcome_msg));
			is_new_conn = 0;
			i++;
		}

		mfg_data.id = j;
		strcpy(mfg_data.name,"harsh");
		mfg_data.update = 0;

		write(nsfd,mfg_data,sizeof(mfg_data));
	}
}
