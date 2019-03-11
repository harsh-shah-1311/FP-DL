int StartFirmwareUpgradeTask(void)
{
	pthread_t tid;
	
	tid = pthread_create(&tid,NULL,fw_upgrade_func_new,NULL);
	if(tid<0)
	{
		LOGD("Error: fw upgrade thread failed\n\r");
		return -1;
	}
	return tid;
}



int main(int argc, char **argv)
{
	StartFirmwareUpgradeTask();
}
