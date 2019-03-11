static int ota_download_progress_cb(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    	struct progress_s *pg = (struct progress_s *)p;
    	CURL *curl = pg->curl;
    	double curtime = 0;
        float percent_process = 0;

    	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);
        /*
         * under certain circumstances it may be desirable for certain functionality
         * to only run every N seconds, in order to do this the transaction time can
         * be used
         */
    	if((curtime - pg->last_runtime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) 
	{
        	pg->last_runtime = curtime;
        	LOGD( "TOTAL TIME: %f \r\n", curtime);
    	}

        if(dltotal != 0)
                percent_process = (float)(dlnow * 100.0) / dltotal;
        else
                percent_process = 0;

        if(pg->last_percentage != (int)percent_process)
        {
                pg->last_percentage = (int)percent_process;
                LOGD("Downloading Update[%.2f %%]: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T"",
                         percent_process, dlnow, dltotal);

                bzero(response_buf, sizeof(response_buf));
                sprintf(response_buf, "Download: [%d%%] %s", (int)percent_process, ota_src_file_name);
                ota_send_status(OTA_SEND_STATUS_INPROGRESS,response_buf);
        }

    return 0;
}


int ota_download_sftp(char *src_url, char *dest_file)
{
    	CURL *curl;
    	CURLcode res = CURLE_OK;
        static char errbuf[1024];
    	FILE *out_file=NULL;
    	struct progress_s progress;
        char *file_name;
        const char *err_str;

        file_name = strrchr(src_url,'/');
        if(file_name != NULL)
        {
                file_name++; //To escape '/' char
                strcpy(ota_src_file_name,file_name);
        }

    	curl_global_init(CURL_GLOBAL_DEFAULT);
    	curl = curl_easy_init();
    	if (curl)
    	{
        	curl_easy_setopt(curl, CURLOPT_URL, src_url);
        	curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);

        	/* first remove file */
        	unlink(dest_file);

        	out_file = fopen(dest_file, "w");
        	if (!out_file)
        	{
            		curl_easy_cleanup(curl);
            		return -1;
        	}

        	curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_file);
        	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ota_download_progress_cb);
        	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);
                 /* clearing NO_PROGRESS --> progess enabled !! */
        	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        	/* enable TCP keep-alive for this transfer */
        	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        	curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, OTA_DOWNLOAD_KEEPALIVE_IDLE_TIME);
        	curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, OTA_DOWNLOAD_KEEPALIVE_INTVL_TIME);
        	res = curl_easy_perform(curl);
        	if(res > 0)
        	{
                        err_str = curl_easy_strerror(res);
            		LOGE("ERROR(%d): %s \n ", res, err_str);
                        LOGE("ERROR: Failed to download file \n ");
                        bzero(errbuf,sizeof(errbuf));
                        sprintf(errbuf,OTA_SEND_ERR_DW_FAILED"%s", err_str);
                        if((res == CURLE_REMOTE_ACCESS_DENIED) || (res == CURLE_LOGIN_DENIED))
                        {
                                ota_send_status(OTA_SEND_STATUS_FAILED,OTA_SEND_ERR_DW_AUTH_FAILED);
                        }
                        else if(res == CURLE_REMOTE_FILE_NOT_FOUND)
                        {
                                ota_send_status(OTA_SEND_STATUS_FAILED,OTA_SEND_ERR_DW_FILE_NOT_FOUND);
                        }
                        else if((res == CURLE_WRITE_ERROR) || (res == CURLE_REMOTE_DISK_FULL))
                        {
                                ota_send_status(OTA_SEND_STATUS_FAILED,OTA_SEND_ERR_DW_FILE_WRITE_FAILED);
                        }
                        else if(res == CURLE_COULDNT_CONNECT)
                        {
                                ota_send_status(OTA_SEND_STATUS_FAILED,OTA_SEND_ERR_DW_NW_FAILED);
                        }
                        else
                        {
                                ota_send_status(OTA_SEND_STATUS_FAILED,errbuf);
                        }
        	}	

        	curl_easy_cleanup(curl);
        	curl_global_cleanup();
        	fclose(out_file);

        	return res;
    	}
    	return 0;
}

