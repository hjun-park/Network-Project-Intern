#ifndef __CONFIG_H__
#define __CONFIG_H__


typedef struct _Parser {
	char interfaces[10];
	int valid_lifetime;
	char pool[30];
	char subnet[20];
	char router[20];
	char dns_server[20];
} Parser;


Parser parsing();





#endif
