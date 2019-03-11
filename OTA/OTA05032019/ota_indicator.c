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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <pthread.h>
#include "ota_utils.h"
#include "ota_indicator.h"

#if defined(LED_INDICATION)
#define LED_MSGQ_KEY        5335
#define LED_BURST_INTERVAL  10  /* seconds */
#define LED_1		        1   /* LED1 Identification */
#define LED_2		        2   /* LED2 Identification */
#define LED_TO_BLINK        LED_2
#endif

#if defined(LED_INDICATION)
enum LED2_STATE {
    SOM_ON,
    COMMISION_WINDOW,
    NFC_TAG_READ_DONE,
    DEVICE_COMMISIONED_SUCCESSFULLY,
    DEVICE_COMMISION_FAILED,
    MG_OTA_INDICATOR    /* will blink led for 5 seconds at interval of 0.5 sec*/
} LED2_STATES;

static int g_ledloop_cont;
static pthread_t ledloop_tid;
#endif

#if defined(LED_INDICATION)
static void ota_ind_led_ctrl() {
    message_buf  rbuf;
    static int msqid_led = 0;
    unsigned char buf[256];

    if (msqid_led == 0) {
        if ((msqid_led = msgget(LED_MSGQ_KEY, IPC_CREAT | 0666)) < 0) {
            LOGE("msgget() failed: %s\n", strerror(errno));
            return;
        }
    }

    /*Prepare packets*/
    sprintf(buf,"{\"LED\" : %d, \"state\" : %d}", LED_TO_BLINK,
            MG_OTA_INDICATOR);
    rbuf.mtype = 1;
    rbuf.mtext[0] = strlen(buf);
    memcpy(&rbuf.mtext[1],buf,strlen(buf));

    /*Send message in queue*/
    if (msgsnd(msqid_led, &rbuf, strlen(buf), IPC_NOWAIT) < 0) {
		LOGE("msgsnd() failed: %s\n", strerror(errno));
	}
    LOGD("sent\n");
}

/*
 * Loop will blink LED continuesly at interval of LED_BURST_INTERVAL
 * till it is stopped.
 * Blink pattern: <blink><blink><blink><blink><blink> <wait>
 *                ^---------------------------------^ ^----^
 *                      |                               |
 *                      `- 5 seconds                    `- 5 seconds
 */
static void *ota_ind_led_loop() {
    int count = 0;

    LOGD("start.\n");
    ota_ind_led_ctrl();

    while(g_ledloop_cont) {
        if (count == LED_BURST_INTERVAL) {
            ota_ind_led_ctrl();
            count = 0;
        }

        sleep(1);
        count++;
    }

    g_ledloop_cont = 0;
}
#endif

void ota_ind_start() {
#if defined(LED_INDICATION)
    int rc;

    LOGD("start.\n");
    g_ledloop_cont = 1;
    rc = pthread_create(&ledloop_tid, NULL, ota_ind_led_loop, NULL);
    if (rc != 0) {
		LOGE("pthread_create() failed: %s\n", strerror(errno));
    }
#else
    LOGD("built without LED indication");
#endif
}

void ota_ind_stop() {
#if defined(LED_INDICATION)
    LOGD("stop.\n");
    if (g_ledloop_cont) {
        g_ledloop_cont = 0;
        pthread_join(ledloop_tid, NULL);
    }
#else
    LOGD("built without LED indication");
#endif
}
