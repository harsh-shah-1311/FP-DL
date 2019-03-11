#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
  
struct msgbuf { 
    long mtype; 
    char msg[100]; 
}; 
  
int main() 
{ 
    	int msgid;
	struct msgbuf message; 
    	msgid = msgget(1, 0666 | IPC_CREAT); 
  
  	msgrcv(msgid, &message, sizeof(message), 1, 0); 
  
    	printf("Data Received is : %s \n",message.msg); 
} 
