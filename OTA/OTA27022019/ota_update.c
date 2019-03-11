int ota_update_verify(char *update_file, char *key_file)
{
    	int child_res, child_pid;

        ota_send_status(OTA_SEND_STATUS_INPROGRESS, OTA_SEND_STATUS_VERIFICATION_KEY" [0%] Verification started");
    	child_pid = fork();
    	if(child_pid == 0)
    	{
        	execlp("/usr/bin/swupdate", "swupdate", "-c", "-i", update_file, "-v", "-k", key_file, NULL);
        	exit(-1);
    	}
    	else
    	{
        	LOGD("[%d]:[%d]Verifying OTA update image\n", getpid(), child_pid);
        	waitpid(child_pid, &child_res, 0);
        	if (WIFEXITED(child_res)) 
		{
            		int rc = WEXITSTATUS(child_res);
                        if (rc)
                                ota_send_status(OTA_SEND_STATUS_FAILED, OTA_UPDATE_ERROR_VERIFICATION);
                        else
                                ota_send_status(OTA_SEND_STATUS_INPROGRESS, OTA_SEND_STATUS_VERIFICATION_KEY" [100%] Verification completed");
                        return rc;
        	} 
		else 
		{
            		LOGE("swupdate did not terminate with exit\n");
                        ota_send_status(OTA_SEND_STATUS_FAILED, OTA_UPDATE_ERROR_VERIFICATION);
            		return -1;
        	}
    	}
}

static void *ota_swupdate_progress_loop(void *arg) 
{
        int connfd, ret, is_done;
        struct progress_msg msg;
        struct sockaddr_un servaddr;
        char buf[512];
        int is_finalize = 0;

        connfd = -1;
        is_done = 0;

        ota_send_status(OTA_SEND_STATUS_INPROGRESS, OTA_SEND_STATUS_UPDATE_KEY" [1%] Preparing for update...");
        while (g_progress_loop) 
	{
                if (connfd < 0) 
		{
                        /* connect to swupdate socket */
                        connfd = socket(AF_LOCAL, SOCK_STREAM, 0);
                        bzero(&servaddr, sizeof(servaddr));
                        servaddr.sun_family = AF_LOCAL;
                        strcpy(servaddr.sun_path, SOCKET_PROGRESS_PATH);

                        LOGD("Trying to connect to SWUpdate...\n");
                        ret = connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
                        if (ret != 0) 
			{
                                /* try again...*/
                                connfd = -1;
                                sleep(1);
                                continue;
                        }
                }

                ret = read(connfd, &msg, sizeof(msg));
                if (ret != sizeof(msg)) 
		{
                        /* connect again */
                        LOGD("Connection closing..\n");
                        close(connfd);
                        connfd = -1;
                        continue;
                }
                if (!is_finalize) 
		{
                        sprintf(buf, "%s [%d/%d] [%d%] %s", OTA_SEND_STATUS_UPDATE_KEY,
                                        msg.cur_step, msg.nsteps, msg.cur_percent, msg.cur_image);
                        ota_send_status(OTA_SEND_STATUS_INPROGRESS, buf);
                        LOGD("%s\n", buf);
                }

                if (msg.cur_step == msg.nsteps && msg.cur_percent == 100) 
		{
                        /*finalizing update, partition resize can take time*/
                        ota_send_status(OTA_SEND_STATUS_INPROGRESS, OTA_SEND_STATUS_UPDATE_KEY" [99%] Finalizing update...");
                        is_finalize = 1;
                }

                switch (msg.status) 
		{
                        case SUCCESS:
                                is_done = 1;
                                ota_send_status(OTA_SEND_STATUS_INPROGRESS, OTA_SEND_STATUS_UPDATE_KEY" [100%] Update success");
                                break;
                        case FAILURE:
                                ota_send_status(OTA_SEND_STATUS_FAILED, OTA_UPDATE_ERROR_FAILED);
                                is_done = 1;
                                break;
                        case DONE:
                                LOGD("Update completed\n");
                                is_done = 1;
                                break;
                        default:
                                break;
                }

                if (is_done)
                        break;
        }

        if (connfd > 0)
                close(connfd);

        return 0;
}

static int ota_swupdate_progress_init() 
{
        pthread_t tid;

        g_progress_loop = 1;
        if (pthread_create(&tid, NULL, ota_swupdate_progress_loop, NULL) != 0) 
	{
                LOGE("pthread_create() failed: %s\n", strerror(errno));
                g_progress_loop = 0;
                return -1;
        }

        return tid;
}


int ota_update_start(char *update_file, char *key_file)
{
	int child_res, child_pid;
	pthread_t tid;

	ota_send_status(OTA_SEND_STATUS_INPROGRESS, OTA_SEND_STATUS_UPDATE_KEY" [0%] Initiating update");
    	child_pid = fork();
    	if(child_pid == 0)
    	{
		/* it means "swupdate -i update_file -v -k key_file"*/
        	execlp("/usr/bin/swupdate","swupdate", "-i", update_file, "-v", "-k", key_file, NULL);
        	exit(-1);
    	}
    	else
    	{
        	LOGD("[%d]:[%d] Installing OTA update image\n", getpid(), child_pid);
        	tid = ota_swupdate_progress_init();
        	waitpid(child_pid, &child_res, 0);
                if (tid > 0) 
		{
                        g_progress_loop = 0;
                        pthread_join(tid, NULL);
                }
        	if(WIFEXITED(child_res)) 
		{
			/* If child terminate normally, it returns true */
            		return WEXITSTATUS(child_res);
        	} 
		else 
		{
            		LOGE("swupdate did not terminate with exit\n");
            		return -1;
        	}
    	}
}

