ota_set_gw_id()
{
	ota_init_gw_id();
}

ota_init_gw_id()
{

}

ota_set_lock()
{

}

json_parse_data()
{
	if()
		ota_start();
	else
		ota_file_start();
}	

ota_update_thread()
{
	/* create the message queue */
	json_parse_data();
}

ota_update_init()
{
	thread_create(&thread_name, 0, opa_update_thread, 0);
}

ota_start()
{
	ota_start_download_sftp();
	ota_update_verify();
	ota_update_start();
	uboot_start();
}

ota_start_file()
{
	ota_update_verify();
	ota_update_start();
}

int main()
{
	ota_set_lock();
	ota_set_gw_id();

	if(argc != 2)
	{
		ota_update_init();
	}
	else
	{
		if(argv[1] == NULL)
		{
			logd("invalid argvs\n\r");
		}
		else
		{	
			if(argv[2] == file)
				ota_start_file();
			else
				ota_start();
		}
	}
}
