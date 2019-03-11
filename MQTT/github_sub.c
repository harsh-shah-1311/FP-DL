
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "github_mqtt.h"
#include"github_mqtt.c"


const char *client_name = "default_sub"; 	// -c
const char *ip_addr     = "127.0.0.1";		// -i
uint32_t    port        = 1883;			// -p
const char *topic       = "hello/world";	// -t

void parse_options(int argc, char** argv);

int main (int argc, char** argv)
{
    puts("MQTT SUB Test Code");

    if(argc > 1) {
	parse_options(argc, argv);
    }

//  mqtt_broker_handle_t *broker = mqtt_connect("default_sub","127.0.0.1", 1883);
    mqtt_broker_handle_t *broker = mqtt_connect(client_name, ip_addr, port);

    if(broker == 0) {
        puts("Failed to connect");
        exit(1);
    }
  
    int result = mqtt_subscribe(broker, topic, QoS0);
    
    if(result != 1) {
        puts("failed to Subscribe");
        exit(1);
    }

    while(1)
    {
        mqtt_display_message(broker, &putchar);
    }
    return 0;
}

void parse_options(int argc, char** argv)
{
	int i;
   for(i = 1; i < argc; ++i) {
	if(strcmp("-c",argv[i]) == 0) {
		printf("client:%s ",argv[++i]);
		client_name = argv[i];
	}	
	if(strcmp("-i",argv[i]) == 0) {
		printf("ip:%s ",argv[++i]);
		ip_addr = argv[i];
	}
	if(strcmp("-p",argv[i]) == 0) {
		printf("port:%s ", argv[++i]);
		port = atoi(argv[i]);
	}
	if(strcmp("-t",argv[i]) == 0) {
		printf("topic:%s ",argv[++i]);
		topic = argv[i];
	}
   }
   puts("");
}
