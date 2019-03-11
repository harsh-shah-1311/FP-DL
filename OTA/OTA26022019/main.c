ota_download_sftp()
{
	ota_download_progress_cb();
}

ota_swupdate_progress_init()
{
	ota_swupdate_progress_loop();
}

ota_update_start()
{
	ota_swupdate_progress_init();
	ota_update_verify();
}

ota_start()
{
	ota_update_start();
}

ota_start_file()
{
	ota_update_start();
	ota_reboot_device();
}

ota_parse_json_data()
{
	ota_correct_uri_path();
	ota_download_sftp();
	ota_start();
	ota_start_file();
}

ota_ind_led_loop()
{
	ota_ind_led_ctrl();
}

ota_ind_start()
{
	ota_ind_led_loop();
}

/* It is a entry function of the thread */
ota_update_thread()
{
	ota_get_uboot_env()
	{
		ots_set_uboot_env();
	}
	ota_ind_start();
	ota_parse_json_data();
}

ota_update_init()
{
/* start the ota update thread and its entry function is given below */
	ota_update_thread();
}

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

int main()
{
	ota_set_lock();
	ota_set_gw_id();

/* Initilize the ota update process, start the ota update thread */
	ota_update_init();
	if(filetype == file)
		ota_start_file();
	else
		ota_start();
}
