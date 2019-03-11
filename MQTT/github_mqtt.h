#ifndef mqtt_h
#define mqtt_h

#include <stdint.h>

typedef struct mqtt_broker_handle mqtt_broker_handle_t;

typedef enum {QoS0, QoS1, QoS2} QoS;

mqtt_broker_handle_t * mqtt_connect(const char* client, const char * server_ip, uint32_t port);
void mqtt_disconnect(mqtt_broker_handle_t *broker);

int mqtt_publish(mqtt_broker_handle_t *broker, const char *topic, const char *msg, QoS qos);

int mqtt_subscribe(mqtt_broker_handle_t *broker, const char *topic, QoS qos);
void mqtt_display_message(mqtt_broker_handle_t *broker, int (*print)(int));



#endif
