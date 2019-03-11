#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
struct mosq
{
	int socket;
	struct sockaddr_in sock_addr;
	int port;
	char cname[24];
	char hname[16];
};

enum QOS
{
	QOS0=1,
	QOS1,
	QOS2
};

int mqtt_connect(struct mosq *broker, char *sid, int port, char *topic)
{
	char buf[20];
	char packet[100];
	int size;
	
//	puts(broker->hname);	

	broker->socket = socket(AF_INET, SOCK_STREAM, 0);
	if(broker->socket<0)
	{
		perror("socket\n\r");
		return 0;
	}
	broker->sock_addr.sin_family = AF_INET;
	broker->sock_addr.sin_port = htons(port);
	broker->sock_addr.sin_addr.s_addr = inet_addr(sid);

	if(connect(broker->socket,(struct sockaddr *)&broker->sock_addr, sizeof(struct sockaddr)))
	{
		perror("connect\n\r");
		return 0;
	}

	memset(packet, 0 , sizeof(packet));
	memcpy(packet,sid, strlen(sid)+1);
	memcpy(packet+sizeof(sid)+1,topic, strlen(topic)+1);
	puts(packet);
	if(send(broker->socket,packet,sizeof(packet),0) < 0)
	{
		perror("send\n\r");
		return 0;
	}

	printf("send successfully\n\r");	

	size = recv(broker->socket, buf, sizeof(buf), 0);
	if(size<=0)
	{
		perror("recv\n\r");
		return 0;
	}

	if(strcmp(buf,"CONNACK")==0)
	{
		printf("connection established\n\r");
	}
	return 1;
}

int mqtt_publish(struct mosq *broker, char *topic, char *msg, enum QOS qos)
{
	char buf[20];
	char packet[100];

	send(broker->socket,"PUBLISH",sizeof("PUBLISH"),0);
//	sleep(5);
	memset(packet,0,sizeof(packet));

	memcpy(packet,msg,strlen(msg)+1);
	puts(packet);
	memcpy(packet+strlen(msg),topic,strlen(topic)+1);
	puts(packet);
	memcpy(packet+sizeof(topic)+strlen(msg)+3,broker->hname, strlen(broker->hname)+1);
	puts(packet);
//	memcpy(packet+sizeof(broker->hname)+sizeof(topic),msg,sizeof(msg));
//	puts(packet);

	if(send(broker->socket, packet, sizeof(packet), 0) < 0)
	{
		perror("send\n");
		return 0;
	}

	printf("send successfully\n\r");
	int size;
	size = recv(broker->socket, buf, sizeof(buf), 0);
	if(size<=0)
	{
		perror("unable to receive puback\n\r");
		return 0;
	}

	if(strcmp(buf,"PUBACK")==0)
		printf("publish message successfully\n\r");
	while(1)
	{
		sleep(5);
	}
}
	
		

int main()
{
	struct mosq *broker;
	broker = malloc(sizeof(struct mosq));
	char *sid = "127.0.0.1";
	int port = 1883;
	char topic[32] = "hello/world";
	mqtt_connect(broker, sid,port,topic);
	//printf("%d\n",broker->socket);
	sleep(2);
	strcpy(broker->hname,sid);
	mqtt_publish(broker, topic, "message",QOS1);
}
