
libmosq_EXPORT int mosquitto_lib_init(void)

libmosq_EXPORT int mosquitto_connect(struct mosquitto *mosq, const char *host, int port, int keepalive)

libmosq_EXPORT int mosquitto_reconnect(struct mosquitto *mosq)

libmosq_EXPORT int mosquitto_disconnect(struct mosquitto *mosq)

libmosq_EXPORT int mosquitto_publish(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain)

libmosq_EXPORT int mosquitto_subscribe(struct mosquitto *mosq, int *mid, const char *sub, int qos)

libmosq_EXPORT int mosquitto_unsubscribe(struct mosquitto *mosq, int *mid, const char *sub	)


