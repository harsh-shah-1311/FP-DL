/* there are 3 message queue are created.
* 1 for send the command.
* 2 for send the notification to the application.
* 3. send the data.
*/ 

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "ota_download.h"
#include "ota_update.h"
#include "ota_utils.h"
#include "ota_indicator.h"
#include "jsmn.h"

#define GW_OTA_UPDATE_SEND_KEY          4657

#define DOWNLOAD_DESTINATION_DIR                "/media/userdata"
#define DOWNLOAD_DESTINATION_FILE_PATH  DOWNLOAD_DESTINATION_DIR"/update.swu"

#define OTA_UPDATE_SECURITY_KEY_FILE_PATH       "/etc/keys/public.pem"
#define PRE_REBOOT_DELAY        10 /*in seconds*/
#define OTA_COMPLETED_UBOOT_VAR         "is_ota_done"

static char ota_uri[JSON_URI_KEY_MAX_SIZE + 1];
static char ota_msg_type[JSON_OTA_MSG_TYPE_MAX_SIZE + 1];
static char ota_cmd[JSON_OTA_CMD_TYPE_MAX_SIZE + 1];

int ota_start(char *src_url);
int ota_start_file(char *src_url);

// to control update thread loop
static int g_update_loop;

/*
struct msgbuf
{
        long mtype;
        char msg[100];
}message,url;*/

message_buf message,url;

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
        if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
                        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
                return 0;
        }
        return -1;
}

int ota_parse_json_data(char *json_data)
{
        jsmn_parser parser;
        jsmntok_t tokens[MAX_JSON_TOKEN];
        int token_cnt;
        int i,res;
        char is_online_update = 1;

        char ctype_char[JSON_CTYPE_KEY_VALUE_SIZE];
        int ctype_cmd = -1;

        memset(&parser, 0, sizeof(parser));
        memset(&tokens[0], 0, sizeof(tokens));
        token_cnt = 0;

        LOGD("Initializing the json parser\n");
        //initialize json parser
        jsmn_init(&parser);

        token_cnt = jsmn_parse(&parser, json_data, strlen(json_data), tokens, sizeof(tokens) / sizeof(tokens[0]));

        //Assume the top-level element is an object
        if (token_cnt < 1 || tokens[0].type != JSMN_OBJECT)
        {
                LOGE("Object expected\n");
                return -1;
        }

        //Parse Ctype key value
        for (i = 0; i < token_cnt; i++)
        {
                if (jsoneq(json_data, &tokens[i], JSON_CTYPE_KEY) == 0)
                {
                        break;
                }
        }

        bzero(ctype_char, JSON_CTYPE_KEY_VALUE_SIZE);
        strncpy(ctype_char, json_data + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
        ctype_cmd = atoi(ctype_char);
       LOGD("Ctype received - %d\n", ctype_cmd);

        /**
         *  Now parse msg type
         */
        for (i = 0; i < token_cnt; i++)
        {
                if (jsoneq(json_data, &tokens[i], JSON_OTA_MSG_TYPE_KEY) == 0)
                {
                        break;
                }
        }

        bzero(ota_msg_type,JSON_OTA_MSG_TYPE_MAX_SIZE+1);
        strncpy(ota_msg_type, json_data + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
        LOGD("OTA msg type:%s\n", ota_msg_type);

        if(strcmp(ota_msg_type,"REQ") != 0)
                return -1;

        /**
         *      MsgType : REQ
         *      Now parse Cmd type
         */
        for (i = 0; i < token_cnt; i++)
        {
                if (jsoneq(json_data, &tokens[i], JSON_OTA_CMD_TYPE_KEY) == 0)
                {
                        break;
                }
        }

        bzero(ota_cmd,JSON_OTA_CMD_TYPE_MAX_SIZE+1);
        strncpy(ota_cmd, json_data + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
        LOGD("OTA cmd:%s\n", ota_cmd);

        if(strcmp(ota_cmd,"onlineUpdate") == 0)
        {
                is_online_update = 1;
                LOGD("Online Update\n");
        }
        else
        {
                is_online_update = 0;
                LOGD("Offline Update\n");
        }

        /**
        *      Cmd : onlineUpdate
         *      Now parse download URI in the command
         */
        for (i = 0; i < token_cnt; i++)
        {
                if (jsoneq(json_data, &tokens[i], JSON_URI_KEY_NAME) == 0)
                {
                        break;
                }
        }

        bzero(ota_uri, JSON_URI_KEY_MAX_SIZE + 1);
        strncpy(ota_uri, json_data + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
        /*Fix "\" in url*/

        LOGD("URI:%s\n", ota_uri);

        if(is_online_update)
        {
                ota_send_status(OTA_SEND_STATUS_STARTED, NULL);
                res = ota_start(ota_uri);
        }
        else
        {
                ota_send_status(OTA_SEND_STATUS_STARTED, NULL);
                res = ota_start_file(ota_uri);
        }

        return res;
}


int ota_start_file(char *src_url)
{
    int res;

    res = ota_update_verify(src_url,OTA_UPDATE_SECURITY_KEY_FILE_PATH);
    if(res != 0) {
        LOGE("ERROR: ota_update_verify\n");
        goto error;
    }

        res = ota_update_start(src_url,OTA_UPDATE_SECURITY_KEY_FILE_PATH);
        if(res != 0) {
        LOGE("ERROR:  ota_update_start\n");
        goto error;
    }

        ota_reboot_device(PRE_REBOOT_DELAY, 1);

error:
    return -1;
}


int ota_start(char *src_url)
{
    int res;

    res = ota_download_sftp(src_url, DOWNLOAD_DESTINATION_FILE_PATH);
        if(res != 0) {
        LOGE("ERROR: ota_download_sftp\n");
        goto error;
    }

    res = ota_update_verify(DOWNLOAD_DESTINATION_FILE_PATH,
                                                        OTA_UPDATE_SECURITY_KEY_FILE_PATH);
    if(res != 0) {
        LOGE("ERROR: ota_update_verify\n");
        goto error;
    }

        res = ota_update_start(DOWNLOAD_DESTINATION_FILE_PATH,
                                                   OTA_UPDATE_SECURITY_KEY_FILE_PATH);
        if(res != 0) {
        LOGE("ERROR: ota_update_start\n");
        goto error;
    }

    ota_reboot_device(PRE_REBOOT_DELAY, 1);

error:
    return -1;
}

void *ota_update_thread(void *msg)
{
        int uid;
        uid = msgget(2,IPC_CREAT|0644);
        if(uid<0)
        {
                perror("msgget");
                //return -1;
        }

        if(msgrcv(uid,&url,sizeof(url),1,0)>0)
        {
                printf("the url is : %s\n",url.mtext);
        }

        ota_parse_json_data(url.mtext);
}


int firmware_update_thread_init(void)
{
	pthread_t tid;
	
	if(pthread_create(&tid,NULL, ota_update_thread,NULL))
	{
		printf("thread not created\n\r");
		return -1;
	}

	return tid;
}

void *fwupdate_thread(void *data)
{
	struct msgbuf v;
	pthread_t tid;	
	int uid;
	uid = *(int *)data;
	
	if(msgrcv(uid, &v, sizeof(v), 1, 0)>0)
	{
		if(strncmp(v.mtext,"dwfw",4) == 0)
		{
			tid = firmware_update_thread_init();
			if(tid != 0)
			{
				printf("ERROR: firmware update thread create process failed\n\r");
				//return -1;
			}
		//	return tid;
		}
	}
}
		

int fwupdate_thread_init(void)
{
	pthread_t tid;
	int uid;

	uid = msgget(1,IPC_CREAT|0644);	
	if(uid<0)
	{
		perror("msgget");
		return -1;
	}

	if(pthread_create(&tid,NULL,fwupdate_thread,&uid)!=0)
	{
		printf("ERROR: not create fwupdate thread\n\r");
		return -1;
	}

	return tid;
}	 

int main(int argc, char **argv)
{
	pthread_t tid;
	int uid;
	
	tid = fwupdate_thread_init();
}
