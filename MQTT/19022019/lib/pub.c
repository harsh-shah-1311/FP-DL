#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include "mosquitto.h"
#include "main.h"
#include"main.c"

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2

static char *topic = NULL;
static char *message = NULL;
static long msglen = 0;
static int qos = 0;
static int retain = 0;
static int mode = MSGMODE_NONE;
static int status = STATUS_CONNECTING;
static int mid_sent = 0;
static int last_mid = -1;
static int last_mid_sent = -1;
static bool connected = true;
static bool disconnect_sent = false;
static bool quiet = false;

void my_connect_callback(struct mosquitto *mosq,void *obj, int result)
{
        int rc = MOSQ_ERR_SUCCESS;

        if(!result)
	{
                switch(mode)
		{
                        case MSGMODE_CMD:
                        case MSGMODE_FILE:
                                rc = mosquitto_publish(mosq, &mid_sent, topic, msglen, message, qos, retain);
                                break;
                        case MSGMODE_NULL:
                                rc = mosquitto_publish(mosq, &mid_sent, topic, 0, NULL, qos, retain);
                                break;
                        case MSGMODE_STDIN_LINE:
                                status = STATUS_CONNACK_RECVD;
                                break;
                }
                if(rc)
		{
                        if(!quiet)
			{
                                switch(rc)
				{
                                        case MOSQ_ERR_INVAL:
                                                printf("Error: Invalid input. Does your topic contain '+' or '#'?\n");
                                                break;
                                        case MOSQ_ERR_NOMEM:
                                                printf("Error: Out of memory when trying to publish message.\n");
                                                break;
                                        case MOSQ_ERR_NO_CONN:
                                                printf("Error: Client not connected when trying to publish.\n");
                                                break;
                                        case MOSQ_ERR_PROTOCOL:
                                                printf("Error: Protocol error when communicating with broker.\n");
                                                break;
                                        case MOSQ_ERR_PAYLOAD_SIZE:
                                                printf("Error: Message payload is too large.\n");
                                                break;
                                }
                        }
                        mosquitto_disconnect(mosq);
                }
        }
	else
	{
                if(result && !quiet)
		{
                        printf("%s\n", mosquitto_connack_string(result));
                }
        }
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
        connected = false;
}

void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
        last_mid_sent = mid;
        if(mode == MSGMODE_STDIN_LINE)
	{
                if(mid == last_mid)
		{
                        mosquitto_disconnect(mosq);
                        disconnect_sent = true;
                }
        }
	else if(disconnect_sent == false)
	{
                mosquitto_disconnect(mosq);
                disconnect_sent = true;
        }
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
        printf("%s\n", str);
}

int load_file(const char *filename)
{
        long pos, rlen;
        FILE *fptr = NULL;

        fptr = fopen(filename, "rd");
        if(!fptr)
	{
                if(!quiet) 
			printf("Error: Unable to open file \"%s\".\n", filename);
                return 1;
        }
        mode = MSGMODE_FILE;
        fseek(fptr, 0, SEEK_END);
        msglen = ftell(fptr);
        if(msglen > 268435455)
	{
                fclose(fptr);
                if(!quiet) 
			printf("Error: File \"%s\" is too large (>268,435,455 bytes).\n", filename);
                return 1;
        }
	else if(msglen == 0)
	{
                fclose(fptr);
                if(!quiet) 
			printf("Error: File \"%s\" is empty.\n", filename);
                return 1;
        }
	else if(msglen < 0)
	{
                fclose(fptr);
                if(!quiet) 
			printf("Error: Unable to determine size of file \"%s\".\n", filename);
                return 1;
        }
        fseek(fptr, 0, SEEK_SET);
        message = malloc(msglen);
        if(!message)
	{
                fclose(fptr);
                if(!quiet) 
			printf("Error: Out of memory.\n");
                return 1;
        }
        pos = 0;
        while(pos < msglen)
	{
                rlen = fread(&(message[pos]), sizeof(char), msglen-pos, fptr);
                pos += rlen;
        }
        fclose(fptr);
        return 0;
}



int main(int argc, char **argv)
{
	struct mosq_config cfg;
        char buf[1024];
        struct mosquitto *mosq = NULL;
        int rc;
        int rc2;

        rc = client_config_load(&cfg, CLIENT_PUB, argc, argv);
        if(rc)
	{
                client_config_cleanup(&cfg);
        }
 	
	topic = cfg.topic;
        message = cfg.message;
        msglen = cfg.msglen;
        qos = cfg.qos;
        retain = cfg.retain;
        mode = cfg.pub_mode;
        quiet = cfg.quiet;

	if(cfg.file_input)
	{
                if(load_file(cfg.file_input))
		{
                        printf("Error loading input file \"%s\".\n", cfg.file_input);
                        return 1;
                }
        }

	if(!topic || mode == MSGMODE_NONE)
	{
                printf("Error: Both topic and message must be supplied.\n");
                return 1;
        }

	mosquitto_lib_init();

        if(client_id_generate(&cfg, "mosqpub"))
	{
                return 1;
        }

	mosq = mosquitto_new(cfg.id,true, NULL);
        if(!mosq)
	{
                switch(errno)
		{
                        case ENOMEM:
                                if(!quiet) 
					printf("Error: Out of memory.\n");
                                break;
                        case EINVAL:
                                if(!quiet) 
					printf("Error: Invalid id.\n");
                                break;
                }
                mosquitto_lib_cleanup();
                return 1;
        }
        
	if(cfg.debug)
	{
                mosquitto_log_callback_set(mosq, my_log_callback);
        }

        mosquitto_connect_callback_set(mosq, my_connect_callback);
        mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
        mosquitto_publish_callback_set(mosq, my_publish_callback);

	rc = client_connect(mosq, &cfg);
        if(rc) 
		return rc;

	if(mode == MSGMODE_STDIN_LINE)
	{
                mosquitto_loop_start(mosq);
        }

        do{
                if(mode == MSGMODE_STDIN_LINE)
		{
                        if(status == STATUS_CONNACK_RECVD)
			{
                                if(fgets(buf, 1024, stdin))
				{
                                        buf[strlen(buf)-1] = '\0';
                                        rc2 = mosquitto_publish(mosq, &mid_sent, topic, strlen(buf), buf, qos, retain);
                                        if(rc2)
					{
                                                if(!quiet) 
							printf("Error: Publish returned %d, disconnecting.\n", rc2);
                                                mosquitto_disconnect(mosq);
                                        }
                                }
				else if(feof(stdin))
				{
                                        last_mid = mid_sent;
                                        status = STATUS_WAITING;
                                }
                        }
			else if(status == STATUS_WAITING)
			{
                                if(last_mid_sent == last_mid && disconnect_sent == false)
				{
                                        mosquitto_disconnect(mosq);
                                        disconnect_sent = true;
                                }
 				Sleep(100);
			}
                        rc = MOSQ_ERR_SUCCESS;
                }
		else
		{
                        rc = mosquitto_loop(mosq, -1,1);
                }
        }
	while(rc == MOSQ_ERR_SUCCESS && connected);

        if(mode == MSGMODE_STDIN_LINE)
	{
                mosquitto_loop_stop(mosq, false);
        }

        if(message && mode == MSGMODE_FILE)
	{
                free(message);
        }
        
	mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
	
	if(rc)
	{
                printf("Error: %s\n", mosquitto_strerror(rc));
        }
        return rc;
}

