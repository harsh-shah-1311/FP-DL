#ifndef _CLIENT_CONFIG_H
#define _CLIENT_CONFIG_H
#endif
#include <stdio.h>
#define MSGMODE_NONE 0
#define MSGMODE_CMD 1
#define MSGMODE_STDIN_LINE 2
#define MSGMODE_FILE 4
#define MSGMODE_NULL 5

#define CLIENT_PUB 1
#define CLIENT_SUB 2

//#define MQTT_PROTOCOL_V311 32
//#define MQTT_PROTOCOL_V31 31
//#define MOSQ_MQTT_ID_MAX_LENGTH 24

struct mosq_config {
        char *id;
        char *id_prefix;
        int protocol_version;
        int keepalive;
        char *host;
        int port;
        int qos;
        bool retain;
        int pub_mode; /* pub */
        char *file_input; /* pub */
        char *message; /* pub */
        long msglen; /* pub */
        char *topic; /* pub */
        char *bind_address;
        bool debug;
        bool quiet;
        unsigned int max_inflight;
        bool clean_session; /* sub */
        char **topics; /* sub */
        int topic_count; /* sub */
        bool no_retain; /* sub */
        char **filter_outs; /* sub */
        int filter_out_count; /* sub */
        bool eol; /* sub */
        int msg_count; /* sub */
};

int client_config_load(struct mosq_config *config, int pub_or_sub, int argc, char *argv[]);
void client_config_cleanup(struct mosq_config *cfg);
int client_id_generate(struct mosq_config *cfg, const char *id_base);
int client_connect(struct mosquitto *mosq, struct mosq_config *cfg);
