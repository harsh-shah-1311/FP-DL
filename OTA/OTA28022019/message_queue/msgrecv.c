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
	int msgid;
	struct msgbuf message;

	msgid = msgget(1,IPC_CREAT|0644);

	msgrcv(msgid,&message,sizeof(message),1,0);
	
	printf("message receive successfully : %s\n",message.msg);
}
