/*
* Copyright (c) 2017 NXP
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of the copyright holder nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////
#define GW_OTA_UPDATE_SEND_KEY		4657

#define DOWNLOAD_DESTINATION_DIR		"/media/userdata"
#define DOWNLOAD_DESTINATION_FILE_PATH	DOWNLOAD_DESTINATION_DIR"/update.swu"

#define OTA_UPDATE_SECURITY_KEY_FILE_PATH	"/etc/keys/public.pem"
#define PRE_REBOOT_DELAY	10 /*in seconds*/
#define OTA_COMPLETED_UBOOT_VAR		"is_ota_done"

static char ota_uri[JSON_URI_KEY_MAX_SIZE + 1];
static char ota_msg_type[JSON_OTA_MSG_TYPE_MAX_SIZE + 1];
static char ota_cmd[JSON_OTA_CMD_TYPE_MAX_SIZE + 1];

// to control update thread loop
static int g_update_loop;

static void ota_correct_uri_path(char *source_str)
{
	char *tempstr = source_str;
	do {
		tempstr = strstr(tempstr, "\\/");
		if (tempstr != NULL) {
			strncpy(tempstr, tempstr + 1, strlen(tempstr + 1));
			source_str[strlen(source_str) - 1] = '\0';
		}
	} while (tempstr != NULL);
}

// Validate json string contains specified key
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
        if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
                        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
                return 0;
        }
        return -1;
}
//////////////////////////////////////////////////////////////
// Global Functions
//////////////////////////////////////////////////////////////

/// \brief  This function start OTA update using online update file.
///
/// \param[in] *arg 		Sftp url path for online update bundle.
/// \return  Status of update process.
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

/// \brief  This function start OTA update using local update file.
///
/// \param[in] *arg 		Path of local update bundle.
/// \return  Status of update process.
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

/// \brief  This function parse json data for OTA update.
///
/// \param[in] *arg 		json data
/// \return  json data parsing status
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

	token_cnt = jsmn_parse(&parser, json_data, strlen(json_data), tokens,
						   sizeof(tokens) / sizeof(tokens[0]));
	
	//Assume the top-level element is an object
	if (token_cnt < 1 || tokens[0].type != JSMN_OBJECT) {
		LOGE("Object expected\n");
		return -1;
	}

	//Parse Ctype key value
	for (i = 0; i < token_cnt; i++) {
		if (jsoneq(json_data, &tokens[i], JSON_CTYPE_KEY) == 0) {
			break;
		}
	}

	if (i >= token_cnt) {
		LOGE("Invalid JSON (%s key not found)\n", JSON_CTYPE_KEY);
		return -1;
	}

	// Validate Ctype length
	if(JSON_CTYPE_KEY_VALUE_SIZE < (tokens[i + 1].end - tokens[i + 1].start)) {
		LOGE("Invalid JSON (Device Ctype lenght is too long)\n");
		return -1;
	}

	bzero(ctype_char, JSON_CTYPE_KEY_VALUE_SIZE);
	strncpy(ctype_char, json_data + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
	ctype_cmd = atoi(ctype_char);
	LOGD("Ctype received - %d\n", ctype_cmd);

	/**
	 *  Now parse msg type
	 */
	for (i = 0; i < token_cnt; i++) {
		if (jsoneq(json_data, &tokens[i], JSON_OTA_MSG_TYPE_KEY) == 0) {
			break;
		}
	}

	if (i >= token_cnt) {
		LOGE("Invalid JSON (%s key not found)\n", JSON_OTA_MSG_TYPE_KEY);
		return -1;
	}

	if (JSON_OTA_MSG_TYPE_MAX_SIZE < (tokens[i + 1].end - tokens[i + 1].start) ) {
		LOGE("Invalid download OTA message type\n");
		return -1;
	}

	bzero(ota_msg_type,JSON_OTA_MSG_TYPE_MAX_SIZE+1);
	strncpy(ota_msg_type, json_data + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
	LOGD("OTA msg type:%s\n", ota_msg_type);

	if(strcmp(ota_msg_type,"REQ") != 0)
		return -1;

	/**
	 *	MsgType : REQ
	 *	Now parse Cmd type
	 */
	for (i = 0; i < token_cnt; i++) {
		if (jsoneq(json_data, &tokens[i], JSON_OTA_CMD_TYPE_KEY) == 0) {
			break;
		}
	}

	if (i >= token_cnt) {
		LOGD("Invalid JSON (%s key not found)\n", JSON_OTA_CMD_TYPE_KEY);
		return -1;
	}

	if (JSON_OTA_CMD_TYPE_MAX_SIZE < (tokens[i + 1].end - tokens[i + 1].start)) {
		LOGE("Invalid download OTA message type\n");
		return -1;
	}

	bzero(ota_cmd,JSON_OTA_CMD_TYPE_MAX_SIZE+1);
	strncpy(ota_cmd, json_data + tokens[i + 1].start,
			tokens[i + 1].end - tokens[i + 1].start);
	LOGD("OTA cmd:%s\n", ota_cmd);
	
	if(strcmp(ota_cmd,"onlineUpdate") == 0) {
		is_online_update = 1;
		LOGD("Online Update\n");
	} else {
		is_online_update = 0;
		LOGD("Offline Update\n");
	}
		
	/**
	 *	Cmd : onlineUpdate
	 *	Now parse download URI in the command
	 */
	for (i = 0; i < token_cnt; i++) {
		if (jsoneq(json_data, &tokens[i], JSON_URI_KEY_NAME) == 0) {
			break;
		}
	}

	if (i >= token_cnt) {
		LOGE("Invalid JSON ( %s key not found)\n",JSON_URI_KEY_NAME);
		return -1;
	}

	if (JSON_URI_KEY_MAX_SIZE < (tokens[i + 1].end - tokens[i + 1].start)) {
		LOGE("Invalid download URI size\n");
		return -1;
	}

	bzero(ota_uri, JSON_URI_KEY_MAX_SIZE + 1);
	strncpy(ota_uri, json_data + tokens[i + 1].start,
			tokens[i + 1].end - tokens[i + 1].start);
	/*Fix "\" in url*/
	ota_correct_uri_path(ota_uri);
	LOGD("URI:%s\n", ota_uri);

	if(is_online_update) {
		ota_send_status(OTA_SEND_STATUS_STARTED, NULL);
		res = ota_start(ota_uri);
	} else {
		ota_send_status(OTA_SEND_STATUS_STARTED, NULL);
		res = ota_start_file(ota_uri);
	}

	return res;
}

/// \brief   This thread is responsible for gateway OTA update handling
///
/// \param[in] *arg 		pthread input arguments
/// \return  None this thread will run forever
void *ota_update_thread(void *arg)
{
	int ota_msq_id;
	key_t ota_key;
	message_buf rbuf;
	int data_recv_len = 0;
	char is_ota_done[16];

	LOGD(" Gateway OTA Handler thread started\n");

	ota_key = GW_OTA_UPDATE_SEND_KEY;
	if ((ota_msq_id = msgget(ota_key, IPC_CREAT | 0666)) < 0) {
		LOGE("msgget() failed: %s\n", strerror(errno));
		return NULL;
	}

	if (ota_get_uboot_env(OTA_COMPLETED_UBOOT_VAR, is_ota_done) == 0) {
		if (strncmp(is_ota_done, "y", 1) == 0) {
			LOGD("gateway update applied successfully\n");
			ota_send_status(OTA_SEND_STATUS_SUCCESS, "OTA completed successfully");
			ota_set_uboot_env(OTA_COMPLETED_UBOOT_VAR, "n");
		} else {
			LOGD("no gateway updates in previous boot\n");
		}
	}

#if defined(TEST_IMAGE)
    /*
     * This will blink led when ota service is running.
     * FOR SPECIAL TEST IMAGE ONLY TO TEST OTA UPDATES. Enable TEST_IMAGE and
     * LED_INDICATION CFLAGS to enable this feature in recipe or makefile.
     */
    ota_ind_start();
#endif
	LOGD("Gateway OTA Queue Initialized successfully..\n");
	while(g_update_loop) {
		int i;

		memset(&rbuf, 0, sizeof(rbuf));
		LOGD("Waiting for Message\n");
		/*
		 * Receive an answer of message type 1.
		 */
		if ((data_recv_len = msgrcv(ota_msq_id, &rbuf, MSGSZ, 1, 0)) < 0) {
			LOGE("msgrcv() failed: %s\n", strerror(errno));
			continue;
		}
		LOGD("Message arrived\n");
		LOGD("JSON DATA:%s\n",rbuf.mtext);
		ota_parse_json_data(&rbuf.mtext[0]);
	}
}

/// \brief   Start update thread to receive json data from device control service
///
/// \return  pthread_t
static int ota_update_init() {
	pthread_t tid;

	g_update_loop = 1;
	if (pthread_create(&tid, NULL, &ota_update_thread, NULL) != 0) {
		LOGE("pthread_create() failed: %s\n", strerror(errno));
		g_update_loop = 0;
		return -1;
	}

	return tid;
}

int main(int argc, char *argv[])
{
	char filename[100];
	char filetype[100];
	pthread_t ota_update_thread_task_id;

	openlog(argv[0], LOG_CONS | LOG_NDELAY, LOG_USER);

	if (ota_set_lock() != 0) {
		LOGE("Another ota app instance alread running...\n");
		return 1;
	}

    ota_set_gw_id(NULL);
    if (argc != 2) {
		LOGD("Starting OTA thread: \n\n");
		ota_update_thread_task_id = ota_update_init();
    } else {
		if (argv[1] == NULL) {
	        LOGD("Usage: %s sftp://user:passwd@192.168.0.1/inputfile\n",argv[0]);
		    LOGE("ERROR: URL is empty \n\n");
		} else {
			sscanf(argv[1], "%99[^:]:/%99[^\n]", filetype, filename);
			if (strcmp(filetype, "file") == 0) {
				LOGD("Using file: %s\n", filename);
				ota_start_file(filename);
			} else {
			    LOGD("Downloading file: %s \n\n",argv[1]);
				ota_start(argv[1]);
			}
		}
    }

	if (ota_update_thread_task_id > 0) {
		pthread_join( ota_update_thread_task_id, NULL);
	}

	closelog();
  	return 0;
}
