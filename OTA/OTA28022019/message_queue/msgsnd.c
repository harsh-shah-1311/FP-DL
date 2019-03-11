#include<stdio.h>
#include<sys/types.h>
#include<sys/msg.h>

struct msgbuf
{
	long mtype;
	char msg[100];
};

int main(int argc, char **argv)
{
	int id;
	struct msgbuf v;

	id = msgget(1,IPC_CREAT|0644);

	v.mtype=1;
	printf("enter the message for send\n");
	gets(v.msg);

	msgsnd(id,&v,sizeof(v),0);
	printf("message send successfully\n");
}
