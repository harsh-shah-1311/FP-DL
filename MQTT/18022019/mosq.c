//#include"mosq.h"

int mqtt_connect(char *cname, char *sip, int port)
{
	struct mosq *broker;
	char packet[100];
	char msg[20];

	broker = malloc(sizeof(struct mosq)+1);
	if(broker == NULL)
	{
		printf("unable to allocate memory!\n\r");
		return 0;
	}

	broker->port = port;
	strcpy(broker->hname,sip);
	strcpy(broker->cname, cname);

	broker->socket = socket(AF_INET, SOCK_STREAM, 0);
	if(broker->socket < 0)
	{
		printf("unable to connect in socket\n\r");
		return 0;
	}

	broker->socket_addr.sin_family = AF_INET;
	broker->socket_addr.sin_port = htons(broker->port);
	broker->socket_addr.sin_addr.saddr = inet_addr(broker->hname);

	if(connect(broker->socket, (struct sockaddr *)&broker->socket_addr, sizeof(broker->socket_addr)) < 0)
	{
		printf("unable to connect in connect\n\r");
		return 0;
	}

	memset(packet,0,sizeof(packet));
	memcpy(packet, cname, sizeof(cname));
	memcpy(packet+cname, sip, sizeof(sip));
	memcpy(packet+cname+sip, topic, sizeof(topic));

	if(send(broker->socket, packet, sizeof(packet),0)<0)
	{
		printf("unable to send the message\n\r");
		return 0;
	}

	int size = recv(broker->socket, msg, sizeof(msg), 0);
	if(size<=0)
	{
		printf("unable to connect in recv\n\r");
		return 0;
	}

	if(!strcmp(msg,CONNACK))
	{
		printf("connect with %s ip with %d port\n\r",sip,port);	
	}	
}

int mqtt_publish(struct mosq *broker, char *topic, char *msg, int qos)
{
	char packet[100];
	char msg[20];
	
	char utf_topic[24];
	
	utf_topic[0]=0;
	utf_topic[1]=strlen(topic);
	memcpy(utf_topic+2, topic, sizeof(topic));

	int fixed_header[] = { SET_MESSAGE(PUBLISH)|(qos << 1), sizeof(utf_topic)+strlen(msg)};
	memset(packet ,0 ,sizeof(packet));
	memset(packet,(char *)&fixed_header, sizeof(fixed_header));
	memset(packet+sizeof(fixed_header), utf_topic, sizeof(utf_topic));
	memset(packet+sizeof(fixed_header)+sizeof(utf_topic), msg, sizeof(msg));

	if(send(broker->socket, packet, sizeof(packet), 0)<0)
	{
		printf("unable to publish\n\r");
		return 0;
	}

	int size = recv(broker->socket, msg, sizeof(msg));
	if(size<=0)
	{
		printf("unable to receive ack\n\r");
		return 0;
	}

	if(!strcmp(msg,PUCACK))
	{
		printf("publish message successfully\n\r");
		return 0;
	}
}


int mqtt_subscribe(struct mosq *broker, char *topic, int qos)
{
	if(!broker->connected)
	{
		printf("connection not established\n\r");
		return 0;
	}

	char packet[100];
	char msg[20];

	char var_name[24];
	var_name[0]=0;
	var_name[1] = strlen(topic);
	memcpy(var_name+2, topic, sizeof(topic));
	memcpy(var_name+2+topic, qos, sizeof(qos));

	int var_header[] = { MSB(MESSAGE_ID), LSB(MESSAGE_ID) };

	memset(packet,0,sizeof(packet));
	memcpy(packet,(char *) &fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), var_name, sizeof(var_name));

	if(send(broker->socket, packet, sizeof(packet),0)<0)
	{
		printf("unable to sunscribe\n\r");
		return 0;
	}

	int size = recv(broker->socket, msg, sizeof(msg),0);
	if(size<=0)
	{
		printf("unable to receive ack \n\r");
		return 0;
	}

	if(!strcmp(msg, "SUBACK"))
	{
		printf("subscribe sucessfully\n\r");
	}
}
