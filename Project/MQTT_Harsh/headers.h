#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

struct data
{
	int id;
	char name[20];
	int update;
};

extern struct data mfg_data;
extern int is_new_conn;

