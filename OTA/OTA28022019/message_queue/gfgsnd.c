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
    	message.mtype = 1; 
  
    	printf("Write Data : "); 
    	gets(message.msg); 
  
    	msgsnd(msgid, &message, sizeof(message), 0); 
  
    	printf("Data send is : %s \n", message.msg);  
} 
