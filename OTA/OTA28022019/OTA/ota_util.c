static int ota_init_gw_id()
{
    	FILE *fp;
    	int fd = 0;
    	char buf[MAX_ID_LEN];
    	char macaddr[MAX_MAC_LEN];
    	struct ifreq s;

    	/* Reset buffer */
    	memset(g_gatewayid, 0, MAX_ID_LEN);
    	memset(buf, 0, MAX_ID_LEN);
    	memset(macaddr, 0, MAX_MAC_LEN);

    	fp = fopen(DEFAULT_GWID_CONFIGFILE, "r");
    	if(fp == NULL) 
	{
        	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
        	strcpy(s.ifr_name, "eth0");
        	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) 
		{
            		snprintf(macaddr, MAX_MAC_LEN, "%02X%02X%02X%02X%02X%02X",
            		(unsigned char)s.ifr_addr.sa_data[0],
            		(unsigned char)s.ifr_addr.sa_data[1],
            		(unsigned char)s.ifr_addr.sa_data[2],
            		(unsigned char)s.ifr_addr.sa_data[3],
            		(unsigned char)s.ifr_addr.sa_data[4],
            		(unsigned char)s.ifr_addr.sa_data[5]);
        	}

        	/* Create Gateway GUID */
        	sprintf(g_gatewayid, DEFAULT_GWID_PREFIX"%s",macaddr);
        	LOGD("[using eth0]Gateway ID: %s\n", g_gatewayid);

    	} 
	else 
	{
        	fscanf(fp, "%s", buf);
        	strncpy(g_gatewayid, buf, MAX_ID_LEN);
       		fclose(fp);
        	LOGD("[using config]Gateway ID: %s\n", g_gatewayid);
    	}

    	return 0;
}
void ota_set_gw_id(char *id) 
{
    	if (id == NULL) 
	{
        	/* get gw id from file or generate using eth0 mac*/
        	ota_init_gw_id();
    	} 
	else 
	{
        	/* init gw id using provided arg */
        	strncpy(g_gatewayid, id, MAX_ID_LEN);
        	LOGD("[using arg]Gateway ID: %s\n", g_gatewayid);
    	}
}
void ota_get_gw_id(char *id, int len) 
{
    	if (id != NULL) 
	{
        	memset(id, 0, len);
        	strncpy(id, g_gatewayid, len);
    	} 
	else 
	{
        	LOGE("Invalid input buffer\n");
    	}
}
int ota_set_lock(void) 
{
        int fd, rc;

        rc = access(OTA_LOCK_FILE, F_OK);
        if (rc == 0) 
	{
                /*file already present*/
                LOGE("can't lock ota app instance\n");
                rc = -1;
        } 
	else 
	{
                /*not present*/
                fd = open(OTA_LOCK_FILE, O_CREAT | O_WRONLY);
                if (fd <= 0) 
		{
                        LOGE("open() failed: %s\n", __FILE__, __LINE__,
                                   strerror(errno));
                        rc = -1;
                } 
		else 
		{
                        /* write pid in file */
                        char buf[PID_BUF_SIZE];
                        sprintf(buf, "%d\n", getpid());
                        write(fd, buf, strlen(buf));
                        close(fd);
                        rc = 0;
                }
        }
        return rc;
}
void ota_reboot_device(int delay_in_seconds, int send_status) 
{
        int count = delay_in_seconds;
        int post_reboot_delay = POST_REBOOT_DELAY;

        if (send_status) 
	{
                char buf[128];
                sprintf(buf, "Rebooting device in %d seconds.", delay_in_seconds);
                ota_send_status(OTA_SEND_STATUS_SUCCESS, buf);
        }

        if (count > 0) 
	{
                LOGD("rebooting device in %d seconds\n", count);
                while (count--) 
		{
                        sleep(1);
                }
        }

        LOGD("rebooting now...");
        system("reboot");

        while(post_reboot_delay--) 
	{
                /* do nothing, wait for device to be rebooted*/
                sleep(1);
        }

        if (send_status) 
	{
                ota_send_status(OTA_SEND_STATUS_SUCCESS, "Couldn't reboot the device, Try manual reboot");
        }
        /*flow should not come here...*/
        LOGD("Please reboot the device manually\n");
}

int ota_send_status(const char *status, const char *data) 
{
        message_buf sbuf_wsn;
    	char gwid[MAX_ID_LEN];
        static int ota_msqid_cloudres = -1;

        sbuf_wsn.mtype = 1;
        memset(&sbuf_wsn.mtext[0], 0, MSGSZ);
        LOGD("Sending OTA status [%s] to cloud\n", status);

        if (ota_msqid_cloudres < 0) 
	{
                if ((ota_msqid_cloudres = msgget(WSN_KEY, IPC_CREAT | 0666)) < 0) 
		{
                        LOGE("msgget() failed: %s\n", strerror(errno));
                        return -1;
                }
        }

    	ota_get_gw_id(gwid, MAX_ID_LEN);
        if (data == NULL) 
	{
                sprintf(&sbuf_wsn.mtext[0],
                                "{\"Ctype\":%d,\"br_guid\":\"%s\", \"MsgType\":\"RES\",\"Status\":\"%s\"}",
                                CTYPE_CMD_GW_OTA_UPDATE, gwid, status);
        } 
	else 	
	{
                sprintf(&sbuf_wsn.mtext[0],
                                "{\"Ctype\":%d,\"br_guid\":\"%s\",\"MsgType\":\"RES\",\"Status\":\"%s\",\"Data\":\"%s\"}",
                                CTYPE_CMD_GW_OTA_UPDATE, gwid, status, data);
        }

        /*
         * Send a message.
        */
        if (msgsnd(ota_msqid_cloudres, &sbuf_wsn, strlen(&sbuf_wsn.mtext[0]) + 1, IPC_NOWAIT) < 0) 
	{
                LOGE("%d, %d, %s, %d\n",  ota_msqid_cloudres, sbuf_wsn.mtype, sbuf_wsn.mtext, strlen(&sbuf_wsn.mtext[0]));
                LOGE("msgsnd() failed: %s\n", strerror(errno));
                return -1;
        } 
	else 
	{
                LOGD("Message: \"%s\" Sent\n", sbuf_wsn.mtext);
        }

        return 0;
}

int ota_get_uboot_env(char *var, char *val) {
        FILE *p_fp = NULL;
        char cmd[128];
        char buf[128];
        char parser[128];
        int rc;

        sprintf(cmd, "fw_printenv %s", var);
        sprintf(parser, "%%%ds=%%s", strlen(var));
        p_fp = popen(cmd, "r");
        if (p_fp != NULL) 
	{
                rc = fscanf(p_fp, parser, buf, val);
                if (rc != 2) 
		{
                        LOGE("can't find %s arg\n", var);
                        return -1;
                }
                pclose(p_fp);
                p_fp = NULL;
        }

        return 0;
}
int ota_set_uboot_env(char *var, char *val) {
        FILE *p_fp = NULL;
        char cmd[128];
        char buf[128];
        int rc;

        sprintf(cmd, "fw_setenv %s \"%s\"", var, val);
        rc = system(cmd);
        if (rc != 0) 
	{
                LOGE("can't set %s to %s\n", var, val);
        }

        return rc;
}

