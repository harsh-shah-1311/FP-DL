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
	memcpy(packet, sid, strlen(sid)+1);
	memcpy(packet+sizeof(sid)+1,topic, strlen(topic)+3);
//	memcpy(packet+sizeof(sid)+sizeof(topic),(char *)port,sizeof(port));

	if(send(broker->socket,packet,sizeof(packet),0) < 0)
	{
		perror("send\n\r");
		return 0;
	}

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

#define MSB(val) val>>4
#define LSB(val) val<<4

int mqtt_subscribe(struct mosq *broker, char *topic, int port, enum QOS qos)
{
//	struct mosq *broker;
	char buf[20];
	char packet[100];
	char msg[100];

	if(send(broker->socket, "SUBSCRIBE",sizeof("SUBSCRIBE"),0)<0)
	{
		perror("send\n\r");
		return 0;
	}

	memset(packet,0,sizeof(packet));
	memcpy(packet,broker->hname, strlen(broker->hname));
	puts(packet);
	memcpy(packet+strlen(broker->hname),topic,strlen(topic));
	puts(packet);
	//memcpy(packet+2+sizeof(broker->hname)+sizeof(topic),msg,sizeof(msg));

	if(send(broker->socket, packet, sizeof(packet), 0) < 0)
	{
		perror("send\n");
		return 0;
	}

	int size;
	size = recv(broker->socket, buf, sizeof(buf), 0);
	if(size<=0)
	{
		perror("unable to receive suback\n\r");
		return 0;
	}

	if(strcmp(buf,"SUBACK")==0)
		printf("subscribe message successfully\n\r");
	
	while(1)
	{
		if(recv(broker->socket,msg,sizeof(msg),0))
		{
			puts(msg);
		}
	}
}
	
		

int main()
{
	struct mosq *broker;
	broker = malloc(sizeof(struct mosq));
	char *sid = "127.0.0.1";
	int port = 1883;
	char *topic = "hello/world";
	mqtt_connect(broker, sid,port,topic);

	strcpy(broker->hname,sid);
	mqtt_subscribe(broker, topic, port,QOS1);
}
