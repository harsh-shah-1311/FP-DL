int ota_start(char *src_url)
{
    	int res;

    	res = ota_download_sftp(src_url, DOWNLOAD_DESTINATION_FILE_PATH);
        if(res != 0) 
	{
        	LOGE("ERROR: ota_download_sftp\n");
        	goto error;
    	}

    	res = ota_update_verify(DOWNLOAD_DESTINATION_FILE_PATH, OTA_UPDATE_SECURITY_KEY_FILE_PATH);
    	if(res != 0) 
	{
       	 	LOGE("ERROR: ota_update_verify\n");
        	goto error;
    	}

        res = ota_update_start(DOWNLOAD_DESTINATION_FILE_PATH, OTA_UPDATE_SECURITY_KEY_FILE_PATH);
        if(res != 0) 
	{
        	LOGE("ERROR: ota_update_start\n");
        	goto error;
    	}

    	ota_reboot_device(PRE_REBOOT_DELAY, 1);

error:
    	return -1;
}

int ota_start_file(char *src_url)
{
    	int res;

    	res = ota_update_verify(src_url,OTA_UPDATE_SECURITY_KEY_FILE_PATH);
    	if(res != 0) 
	{
        	LOGE("ERROR: ota_update_verify\n");
        	goto error;
    	}

        res = ota_update_start(src_url,OTA_UPDATE_SECURITY_KEY_FILE_PATH);
        if(res != 0) 
	{
        	LOGE("ERROR:  ota_update_start\n");
        	goto error;
    	}

        ota_reboot_device(PRE_REBOOT_DELAY, 1);

error:
    return -1;
}

void *ota_update_thread(void *arg)
{
        int ota_msq_id;
        key_t ota_key;
        message_buf rbuf;
        int data_recv_len = 0;
        char is_ota_done[16];

        LOGD(" Gateway OTA Handler thread started\n");

        ota_key = GW_OTA_UPDATE_SEND_KEY;
        if ((ota_msq_id = msgget(ota_key, IPC_CREAT | 0666)) < 0) 
	{
                LOGE("msgget() failed: %s\n", strerror(errno));
                return NULL;
        }

        if (ota_get_uboot_env(OTA_COMPLETED_UBOOT_VAR, is_ota_done) == 0) 
	{
                if (strncmp(is_ota_done, "y", 1) == 0) 
		{
                        LOGD("gateway update applied successfully\n");
                        ota_send_status(OTA_SEND_STATUS_SUCCESS, "OTA completed successfully");
                        ota_set_uboot_env(OTA_COMPLETED_UBOOT_VAR, "n");
                } 
		else 
		{
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
        while(g_update_loop) 
	{
                int i;

                memset(&rbuf, 0, sizeof(rbuf));
                LOGD("Waiting for Message\n");
                /*
                 * Receive an answer of message type 1.
                 */
                if ((data_recv_len = msgrcv(ota_msq_id, &rbuf, MSGSZ, 1, 0)) < 0) 
		{
                        LOGE("msgrcv() failed: %s\n", strerror(errno));
                        continue;
                }
                LOGD("Message arrived\n");
                LOGD("JSON DATA:%s\n",rbuf.mtext);
                ota_parse_json_data(&rbuf.mtext[0]);
        }
}

static int ota_update_init() 
{
        pthread_t tid;

        g_update_loop = 1;
        if (pthread_create(&tid, NULL, &ota_update_thread, NULL) != 0) 
	{
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

        if (ota_set_lock() != 0) 
	{
                LOGE("Another ota app instance alread running...\n");
                return 1;
        }

    	ota_set_gw_id(NULL);
    	if (argc != 2) 
	{
                LOGD("Starting OTA thread: \n\n");
                ota_update_thread_task_id = ota_update_init();
    	} 
	else 
	{
                if (argv[1] == NULL) 
		{
                	LOGD("Usage: %s sftp://user:passwd@192.168.0.1/inputfile\n",argv[0]);
                    	LOGE("ERROR: URL is empty \n\n");
                } 
		else 
		{
                        sscanf(argv[1], "%99[^:]:/%99[^\n]", filetype, filename);
                        if (strcmp(filetype, "file") == 0) 
			{
                                LOGD("Using file: %s\n", filename);
                                ota_start_file(filename);
                        } 
			else 
			{
                         	LOGD("Downloading file: %s \n\n",argv[1]);
                                ota_start(argv[1]);
                        }
                }
    	}

        if (ota_update_thread_task_id > 0) 
	{
                pthread_join( ota_update_thread_task_id, NULL);
        }

        closelog();
        return 0;
}


