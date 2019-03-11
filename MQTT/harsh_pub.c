#include"main.h"

char cname = "publish";
int port = 1883;
int sip = "127.0.0.1";

struct mosq mqtt_connect(char *cname, char *sip, int port)
{
	struct mosq *broker;
	broker = malloc(sizeof(struct mosq)+1);
	if(broker == NULL)
	{
		printf("memory is not free\n\r");
		return 0;
	}

	strcpy(broker->cid,cname);
	strcpy(broker->hname,sip);
	broker->port=port;

	broker->socket = socket(AFF_INET, SOCK_STREAM, 0);
	if(broker->socket < 0)
	{
		printf("unable to connect!\n\r");
		return 0;
	}

	broker->socket_addr.sin_family = AF_INET;
	broker->socket_addr.sin_port = htons(broker->port);
	broker->socker_addr.sin_addr.saddr = inet_addr(broker->hname);

	if(connect(broker->socket, (struct sockaddr *)& broker->socket_addr, sizeof(struct socket_addr))<0)
	{
		printf("unable to connect!\n\r");
		return 0;
	}

	if(send(broker->socket, msg,sizeof(msg),0)<sizeof(msg))
	{
		printf("unable to send connect msg!\n\r");
		return 0;
	}

	int size = recv(broker->socket, msg, sizeof(msg), 0);
	
	if(size<=0)
	{
		printf("unable to connect!\n\r");
		return 0;
	}

	if(msg == CONNACK)
	{
		printf("MQTT connected with : %s with %d port\n\r",sip,port);
	}
}

int mqtt_publish(struct mosq *broker, char *topic, char *msg, int qos)
{
	char  packet[64];
	
	 uint8_t utf_topic[2+strlen(topic)+2]; // 2 extra for message size > QoS0 for msg ID

        // set up topic payload
        utf_topic[0] = 0;                       // MSB(strlen(topic));
        utf_topic[1] = LSB(strlen(topic));
        memcpy((char *)&utf_topic[2], topic, strlen(topic));
        utf_topic[sizeof(utf_topic)-2] = MSB(broker->pubMsgID);
        utf_topic[sizeof(utf_topic)-1] = LSB(broker->pubMsgID);

        uint8_t fixed_header[] = { SET_MESSAGE(PUBLISH)|(qos << 1), sizeof(utf_topic)+strlen(msg)};

	memset(packet,0,sizeof(packet));
	memcpy(packet, &fixed_header, sizeof(fixed_header));
	memcpy(packet+fixed_header, utf_topic, sizeof(utf_topic));
	memcpy(packet+fixed_header+utf_topic, msg, sizeof(msg));

	if(send(broker->socket, packet, sizeof(packet), 0)<0)
	{
		printf("unable to publish the message!\n\r");
		return 0;
	}

	int size = recv(broker->socket, msg, sizeof(msg), 0);
	if(size<=0)
	{
		printf("unable to publish the message\n\r");
		return 0;
	}

	if(msg == PUBACK)
	{
		printf("publish message sucessfully\n\r");
	}
}

int main()
{
	struct mosq *broker;
	int id;	

	broker = malloc(sizeof(struct mosq)+1);
	if(broker == NULL)
	{
		printf("insufficient memory! Please try again...\n\r");
		return 0;
	}

	broker = mqtt_connect(cname, sip, port);
	if(broker == NULL)
	{
		printf("unable to connect!\n\r");
		return 0;
	}

	id = mqtt_publish(broker, topic, msg, QOS1);
	if(id == -1)
	{
		printf("publish failed\n\r");
		return 0;
	}
}
	
