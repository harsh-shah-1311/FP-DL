/*
* Copyright (c) 2016 NXP
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

#ifndef _H_OTA_UTILS_H_
#define _H_OTA_UTILS_H_

#include <syslog.h>

#define MSGSZ		512
#define JSON_CTYPE_KEY	"Ctype"				///< device id key name in json string
#define JSON_CTYPE_KEY_VALUE_SIZE	3		///< device Ctype max string size

#define JSON_OTA_MSG_TYPE_KEY	"MsgType"	///< OTA msg type in json string, REQ or RES
#define JSON_OTA_MSG_TYPE_MAX_SIZE	3		///< OTA msg type max string size

#define JSON_OTA_CMD_TYPE_KEY	"Cmd"		///< OTA cmd type in json string, onlineUpdate or offlineUpdate
#define JSON_OTA_CMD_TYPE_MAX_SIZE	15		///< OTA cmd tye max string size

#define JSON_URI_KEY_NAME	"Uri"			///< URI in json string
#define JSON_URI_KEY_MAX_SIZE	256			///< URI max string size

#define MAX_RX_DATA_SIZE_CL	1024			/// < Max received data from cloud
#define MAX_JSON_TOKEN	20					///< Maximum JSON token in OTA json

#define CTYPE_CMD_GW_OTA_UPDATE	51

#define OTA_SEND_STATUS_STARTED		"started"
#define OTA_SEND_STATUS_INPROGRESS	"inprogress"
#define OTA_SEND_STATUS_SUCCESS		"success"
#define OTA_SEND_STATUS_FAILED		"failed"


#define OTA_SEND_ERR_DW_AUTH_FAILED			"Download error: [101] Authenticatatoin failed"
#define OTA_SEND_ERR_DW_FILE_NOT_FOUND		"Download error: [102] File not found"
#define OTA_SEND_ERR_DW_FILE_WRITE_FAILED	"Download error: [103] File write failed"
#define OTA_SEND_ERR_DW_NW_FAILED			"Download error: [104] Network isuue"
#define OTA_SEND_ERR_DW_FAILED				"Download error: [105] "


#define OTA_SEND_STATUS_VERIFICATION_KEY	"Verification:"
#define OTA_SEND_STATUS_UPDATE_KEY			"Update:"
#define OTA_UPDATE_ERROR_VERIFICATION	\
					"Verification error: [201] Verification failed"
#define OTA_UPDATE_ERROR_FAILED \
					"Update error [301] Update failed"


#define DEBUG 1
#if DEBUG
	#define LOGD(fmt, ...)	syslog(LOG_DEBUG,"%s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
	#define LOGD(...) do{} while (0)
#endif

#define LOGE(fmt, ...)		syslog(LOG_ERR,"%s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

/*
 * Declare the message structure.
 */
typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;

void ota_set_gw_id(char *id);
void ota_get_gw_id(char *id, int len);
int ota_set_lock(void);
void ota_reboot_device(int delay_in_seconds, int send_status);
int ota_send_status(const char *status, const char *data);
int ota_get_uboot_env(char *var, char *val);
int ota_set_uboot_env(char *var, char *val);

#endif
