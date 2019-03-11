void init_config(struct mosq_config *cfg)
{
        memset(cfg, 0, sizeof(*cfg));
        cfg->port = 1883;
        cfg->max_inflight = 20;
        cfg->keepalive = 60;
        cfg->clean_session = true;
        cfg->eol = true;
        cfg->protocol_version = MQTT_PROTOCOL_V31;
}

void client_config_cleanup(struct mosq_config *cfg)
{
        int i;
        free(cfg->id);
        free(cfg->id_prefix);
        free(cfg->host);
        free(cfg->file_input);
        free(cfg->message);
        free(cfg->topic);
        free(cfg->bind_address);
        if(cfg->topics){
                for(i=0; i<cfg->topic_count; i++){
                        free(cfg->topics[i]);
                }
                free(cfg->topics);
        }
	if(cfg->filter_outs){
                for(i=0; i<cfg->filter_out_count; i++){
                        free(cfg->filter_outs[i]);
                }
                free(cfg->filter_outs);
        }
}

int client_config_load(struct mosq_config *cfg, int pub_or_sub, int argc, char *argv[])
{
        int rc;
        FILE *fptr;
        char line[1024];
        int count;
        char *loc = NULL;
        int len;
        char *args[3];

        args[0] = NULL;

        init_config(cfg);

        rc = client_config_line_proc(cfg, pub_or_sub, argc, argv);
        if(rc) 
		return rc;

        if(pub_or_sub == CLIENT_SUB)
	{
 		if(cfg->clean_session == false && (cfg->id_prefix || !cfg->id))
		{
                        if(!cfg->quiet) 
				printf("Error: You must provide a client id if you are using the -c option.\n");
                        return 1;
                }
                if(cfg->topic_count == 0)
		{
                        if(!cfg->quiet) 
				printf("Error: You must specify a topic to subscribe to.\n");
                        return 1;
                }
        }

        if(!cfg->host){
                cfg->host = "localhost";
        }
        return MOSQ_ERR_SUCCESS;
}

int client_config_line_proc(struct mosq_config *cfg, int pub_or_sub, int argc, char *argv[])
{
        int i;

        for(i=1; i<argc; i++)
	{
                if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
                {
                        if(i==argc-1)
                        {
                                printf("Error: -p argument given but no port specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->port = atoi(argv[i+1]);
                                if(cfg->port<1 || cfg->port>65535)
                                {
                                        printf("Error: Invalid port given: %d\n", cfg->port);
                                        return 1;
                                }
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "-A"))
                {
                        if(i==argc-1)
                        {
                                printf("Error: -A argument given but no address specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->bind_address = strdup(argv[i+1]);
                        }
                        i++;
		}
                else if(!strcmp(argv[i], "-C"))
                {
                        if(pub_or_sub == CLIENT_PUB)
                        {
                                goto unknown_option;
                        }
                        else
                        {
                                if(i==argc-1)
                                {
                                        printf("Error: -C argument given but no count specified.\n\n");
                                        return 1;
                                }
                                else
                                {
                                        cfg->msg_count = atoi(argv[i+1]);
                                        if(cfg->msg_count < 1)
                                        {
                                                printf("Error: Invalid message count \"%d\".\n\n", cfg->msg_count);
                                                return 1;
                                        }
                                }
                                i++;
                        }
                }
                else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug"))
                {
                        cfg->debug = true;
                }
                else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file"))
                {
                        if(pub_or_sub == CLIENT_SUB)
                        {
                                goto unknown_option;
                        }
                        if(cfg->pub_mode != MSGMODE_NONE)
                        {
                                printf("Error: Only one type of message can be sent at once.\n\n");
                                return 1;
                        }
                        else if(i==argc-1)
                        {
                                printf("Error: -f argument given but no file specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->pub_mode = MSGMODE_FILE;
                                cfg->file_input = strdup(argv[i+1]);
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "--help"))
                {
                        return 2;
                }
                else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host"))
                {
                        if(i==argc-1)
                        {
                                printf("Error: -h argument given but no host specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->host = strdup(argv[i+1]);
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--id"))
                {
                        if(cfg->id_prefix)
                        {
                                printf("Error: -i and -I argument cannot be used together.\n\n");
                                return 1;
                        }
                        if(i==argc-1)
                        {
                                printf("Error: -i argument given but no id specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->id = strdup(argv[i+1]);
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "-I") || !strcmp(argv[i], "--id-prefix"))
                {
                        if(cfg->id)
                        {
                                printf("Error: -i and -I argument cannot be used together.\n\n");
                                return 1;
                        }
                        if(i==argc-1)
                        {
                                printf("Error: -I argument given but no id prefix specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->id_prefix = strdup(argv[i+1]);
                        }
                        i++;
                }
            	else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepalive"))
                {
                        if(i==argc-1)
                        {
                                printf("Error: -k argument given but no keepalive specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->keepalive = atoi(argv[i+1]);
                                if(cfg->keepalive>65535)
                                {
                                        printf("Error: Invalid keepalive given: %d\n", cfg->keepalive);
                                        return 1;
                                }
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--message"))
                {
                        if(pub_or_sub == CLIENT_SUB)
                        {
                                goto unknown_option;
                        }
                        if(cfg->pub_mode != MSGMODE_NONE)
                        {
                                printf("Error: Only one type of message can be sent at once.\n\n");
                                return 1;
                        }
                        else if(i==argc-1)
                        {
                                printf("Error: -m argument given but no message specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                cfg->message = strdup(argv[i+1]);
                                cfg->msglen = strlen(cfg->message);
                                cfg->pub_mode = MSGMODE_CMD;
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "-V") || !strcmp(argv[i], "--protocol-version"))
                {
                        if(i==argc-1)
                        {
                                printf("Error: --protocol-version argument given but no version specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                if(!strcmp(argv[i+1], "mqttv31"))
                                {
                                        cfg->protocol_version = MQTT_PROTOCOL_V31;
                                }
                                else if(!strcmp(argv[i+1], "mqttv311"))
                                {
                                        cfg->protocol_version = MQTT_PROTOCOL_V311;
                                }
                                else
                                {
                                        printf("Error: Invalid protocol version argument given.\n\n");
                                        return 1;
                                }
                                i++;
                        }
          	}
                else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--qos"))
                {
                       if(i==argc-1)
                       {
                               	printf("Error: -q argument given but no QoS specified.\n\n");
                               	return 1;
                       }
                       else
                       {
                                cfg->qos = atoi(argv[i+1]);
                                if(cfg->qos<0 || cfg->qos>2)
                                {
                                        printf("Error: Invalid QoS given: %d\n", cfg->qos);
                                        return 1;
                                }
                        }
                        i++;
                }
                else if(!strcmp(argv[i], "--quiet"))
                {
                        cfg->quiet = true;
                }
                else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--retain"))
                {
                        if(pub_or_sub == CLIENT_SUB)
                        {
                                goto unknown_option;
                        }
                        cfg->retain = 1;
                }
                else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--topic"))
                {
                        if(i==argc-1)
                        {
                                printf("Error: -t argument given but no topic specified.\n\n");
                                return 1;
                        }
                        else
                        {
                                if(pub_or_sub == CLIENT_PUB)
                                {
                                        if(mosquitto_pub_topic_check(argv[i+1]) == MOSQ_ERR_INVAL)
                                        {
                                                printf("Error: Invalid publish topic '%s', does it contain '+' or '#'?\n", argv[i+1]);
                                                return 1;
                                        }
                                        cfg->topic = strdup(argv[i+1]);
                                }
                                else
                                {
                                        if(mosquitto_sub_topic_check(argv[i+1]) == MOSQ_ERR_INVAL)
                                        {
                                                printf("Error: Invalid subscription topic '%s', are all '+' and '#' wildcards correct?\n", argv[i+1]);
                                                return 1;
                                        }
                                        cfg->topic_count++;
                                        cfg->topics = realloc(cfg->topics, cfg->topic_count*sizeof(char *));
                                        cfg->topics[cfg->topic_count-1] = strdup(argv[i+1]);
                                }
                                i++;
                        }
                }
        }

        return MOSQ_ERR_SUCCESS;

unknown_option:
        printf("Error: Unknown option '%s'.\n",argv[i]);
        return 1;
}

int client_id_generate(struct mosq_config *cfg, const char *id_base)
{
        int len;
        char hostname[256];

        if(cfg->id_prefix)
	{
                cfg->id = malloc(strlen(cfg->id_prefix)+10);
                if(!cfg->id)
		{
                        if(!cfg->quiet) 
				printf("Error: Out of memory.\n");
                        mosquitto_lib_cleanup();
                        return 1;
                }
                snprintf(cfg->id, strlen(cfg->id_prefix)+10, "%s%d", cfg->id_prefix, getpid());
        }
	else if(!cfg->id)
	{
                hostname[0] = '\0';
                gethostname(hostname, 256);
                hostname[255] = '\0';
                len = strlen(id_base) + strlen("/-") + 6 + strlen(hostname);
                cfg->id = malloc(len);
                if(!cfg->id)
		{
                        if(!cfg->quiet) 
				printf("Error: Out of memory.\n");
                        mosquitto_lib_cleanup();
                        return 1;
                }
                snprintf(cfg->id, len, "%s/%d-%s", id_base, getpid(), hostname);
                if(strlen(cfg->id) > MOSQ_MQTT_ID_MAX_LENGTH)
		{
                        cfg->id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
                }
        }
        return MOSQ_ERR_SUCCESS;
}


int client_connect(struct mosquitto *mosq, struct mosq_config *cfg)
{
        char err[1024];
        int rc;

        rc = mosquitto_connect_bind(mosq, cfg->host, cfg->port, cfg->keepalive, cfg->bind_address);
        if(rc>0)
	{
                if(!cfg->quiet)
		{
                        if(rc == MOSQ_ERR_ERRNO)
			{
                                fprintf(stderr, "Error: %s\n", err);
                        }
			else
			{
                                fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(rc));
                        }
                }
                mosquitto_lib_cleanup();
                return rc;
        }
        return MOSQ_ERR_SUCCESS;
}

