#include"mosq.h"
#include"mosq.c"

char *cname = "publish";
char *sip = "127.0.0.1";
int port = 1883;
char *topic = "hello/world";

void parse_msg(struct mosq *, int, char **);
int main(int argc, char **argv)
{
	struct mosq *broker;
	int i;
	char msg[100];
	broker = malloc(sizeof(struct mosq)+1);
	if(broker == NULL)
	{
		printf("memory is not allocated\n\r");
		return 0;
	}

	if(argc==1)
	{
		printf("wrong syntax\n\r");
		return 0;
	}

	parse_msg(broker, argc, argv);

	if(mqtt_connect(cname, sip, port))
	{
		printf("unable to connect\n\r");
		return 0;
	}

	for(i=0;i<10;i++)
	{
		sprintf(msg,"message num %d",i);
		mqtt_publish(broker,topic,msg,QOS1);
	}	
}

void parse_msg(struct mosq *broker, int argc, char **argv)
{
	int i;
	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-c")==0)
		{
			printf("client name is : %s\n\r",argv[++i]);
			strcpy(broker->cname,argv[i]);
			i--;
			continue;
		}
		else if(strcmp(argv[i],"-i")==0)
		{
			printf("server ip : %s\n\r",argv[++i]);
			strcpy(broker->hname,argv[i]);
			i--;
			continue;
		}
		else if(strcmp(argv[i],"-p")==0)
		{
			printf("port number : %s\n",argv[++i]);
			//strcpy((char*)broker->port,argv[i]);
			broker->port = atoi(argv[i]);
			i--;
			continue;
		}
		else if(strcmp(argv[i],"-t")==0)
		{
			printf("topic name : %s\n",argv[++i]);
			strcpy(broker->topic,argv[i]);
			i--;
			continue;
		}
	}
}
