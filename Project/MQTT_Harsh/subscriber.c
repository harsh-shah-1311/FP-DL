#include"headers.h"

struct data mfg_data;
extern int is_new_conn;
int main()
{
	struct sockaddr_in v;
	int sfd;
	char buf[100];
	extern int is_new_conn;

	is_new_conn=1;	

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0)
	{
		perror("socket");
		return 0;
	}

	v.sin_family = AF_INET;
	v.sin_port = htons(2500);
	v.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sfd, (struct sockaddr *)&v, sizeof(v)))
	{
		perror("connect");
		return 0;
	}

	read(sfd,buf,sizeof(buf));

	printf("%s\n",buf);

	while(1)
	{
		read(sfd, mfg_data, sizeof(mfg_data));
		printf("id : %d\n",mfg_data.id);
		printf("name : %s\n",mfg_data.name);
		printf("update : %d\n",mfg_data.update);
	}
}
