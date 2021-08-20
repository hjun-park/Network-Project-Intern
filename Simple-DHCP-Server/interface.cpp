#include <iostream>
#include <thread>
#include <mysql.h>
#include <unistd.h> // using sleep
#include "interface.h"
#include "rapid.h"

using namespace std;
using namespace chrono;


//void updateLeaseTime(List *memory, uint32_t reqIP) {
//	node *curNode = new node;
//	curNode = memory->head;
//	
//	uint32_t ipaddr = reqIP;
//
//	printf("[+] updateLeaseTime = %u\n", ipaddr);
//
//	while(
//
//}


//  XXX : not verified */
//  Renewal Request or Rebinding 
int Interface::checkRenewal(List *memory, uint32_t ciaddr, char *reqMAC) {
	time_t now;
	time(&now);

	node *curNode = new node;
	curNode = memory->head;
	
	uint32_t ipaddr = ciaddr;
	int strResult = 0;

	printf("ipaddr = %u\n", ipaddr);
	printf("now = %u\n", now);
	printf("expire = %u\n", curNode->element->expire);
	

	printf("reqMAC = %s\n", reqMAC);

	cout << "strResult = " << strResult << endl;

	printf("ciaddr : %u\n", ipaddr);
	while(curNode != NULL) {
		printf("hwaddr = %s\n", curNode->element->hwaddr);
		strResult = strncmp(curNode->element->hwaddr, reqMAC, sizeof(curNode->element->hwaddr));
		if(curNode->element->address_int == ipaddr) {
			cout << "P1" << endl;
			if((strResult == 0) && (now <= curNode->element->expire))  {
				cout << "P2" << endl;
				return 0;
			} 

			else {
				cout << " other 1" << endl;
				return 1;
			}
		}
		
		curNode = curNode->next;
	}
	
	cout << " other second 1 " << endl;
	return 1;
}


void Interface::setIsusedValue(List *memory, uint32_t yiaddr, int value)
{	
	uint32_t ipaddr = yiaddr;
	char cZero[] = "0";
	char cUnknown[] = "?";

	cout << "ISusedValue Function" << endl;
	node *curNode = new node;
	curNode = memory->head;

	printf("setIsusedValue : %u\n", ipaddr);
	printf("setIsusedValue : %d\n", value);


	/* act on the occasion ( DECLINE, RELEASE ) */
	while(curNode != NULL) {
		if(curNode->element->address_int == ipaddr) {
			curNode->element->is_used = value; 
			memcpy(curNode->element->hwaddr, cZero, sizeof(cZero));

			/* if message == Decline */
			if(value == 1) {
				memcpy(curNode->element->hwaddr, cUnknown, sizeof(cUnknown));

			}	
			return;
		}
		curNode = curNode->next;
	}

	return;
}


void Interface::informToMemory(List *memory, uint32_t reqIP, char *reqMAC ) {
	time_t now;
	time(&now);

	node *curNode = new node;
	curNode = memory->head;

	/* set is_used == 1 */
	while(curNode != NULL) {

		/* register information */
		if(curNode->element->address_int == reqIP) {
			memcpy(curNode->element->hwaddr, reqMAC, sizeof(curNode->element->hwaddr));
			curNode->element->is_used = 1;
			curNode->element->expire = now+(curNode->element->valid_lifetime);

			return;
		}
		curNode = curNode->next;
	}
	return;
}

uint32_t Interface::findRequestIP(List *memory, uint32_t reqIP)
{
	node *curNode = new node;
	curNode = memory->head;
	//int strResult = 0;

//	time_t now;
//	time(&now);

	printf("reqIP : %u \n", reqIP);

	while(curNode != NULL) {
		if(curNode->element->address_int == reqIP) {
			if(curNode->element->is_used==0) {
				return reqIP;
			} else {
				return 0;
			}
		}
		curNode = curNode->next;
	}

//	printf("[+] reqMAC == %s\n", reqMAC);
//	while(curNode != NULL) {
//		strResult = strncmp(curNode->element->hwaddr, reqMAC, sizeof(curNode->element->hwaddr));
//		printf("[+] strResult == %d\n", strResult);
//		printf("[+] hwaddr = %s\n", curNode->element->hwaddr);
//		if(curNode->element->address_int == reqIP) {
//			cout << "P1" << endl;
//			//if((now <= curNode->element->expire) && (strResult <= 0))  {
//			if(strResult <= 0)  {
//				cout << "P2" << endl;	
//				return 1;
//			}
//
//			else {
//				cout << " other 1" << endl;
//				return 0;
//			}
//		}
//		curNode = curNode->next;
//	}
	return 0;
}


int Interface::findEmptyIP(List *memory)
{
	node *curNode = new node;
	curNode = memory->head;

	while(curNode != NULL) {
		if(curNode->element->is_used==0) {
			return curNode->element->address_int;
		}
		curNode = curNode->next;
	}
	return 0;
}	


void Interface::set_config(Parser *parser) {
	memcpy(this->interfaces, parser->interfaces, sizeof(parser->interfaces));
	this->valid_lifetime = parser->valid_lifetime;
	memcpy(this->pool, parser->pool, sizeof(parser->pool));
	memcpy(this->subnet, parser->subnet, sizeof(parser->subnet));
	memcpy(this->router, parser->router, sizeof(parser->router));
	memcpy(this->dns_server, parser->dns_server, sizeof(parser->dns_server));

}



int Interface::dbToMemory(List *memory)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query_buffer[2048];
	conn = mysql_init(NULL);

	if(!mysql_real_connect(conn, "localhost", "root", "root", NULL, 0, NULL, 0)) {
		cout << "[+] Can't Connect to DB" << endl;
		return 1;
	} 
	
	else {
		if(mysql_select_db(conn, "dhcp_db")) {
			cout <<  "[+] Can't use dhcp_db" << endl;
			return 1;
		}
	}
	
	snprintf(query_buffer, sizeof(query_buffer), "select interface, address_int, address, hwaddr, valid_lifetime, Unix_Timestamp(expire), is_used from lease_db order by address_int");

	if(mysql_query(conn, query_buffer)) {
		printf("query failed : %s\n", query_buffer);
		return 1;
	}

	res = mysql_store_result(conn);

	/* db to memory */
	while((row = mysql_fetch_row(res))) {

		struct element *element = (struct element*)malloc(sizeof(struct element));

		memcpy(element->interface, row[0], sizeof(element->interface));
		element->address_int = atoi(row[1]);
		memcpy(element->address, row[2], sizeof(element->address));
		memcpy(element->hwaddr, row[3], sizeof(element->hwaddr));
		element->valid_lifetime = atoi(row[4]);
		element->expire = atoi(row[5]);
		element->is_used = atoi(row[6]);

		memory->createnode(element);
	}	
	
	mysql_free_result(res);
	mysql_close(conn);

	return 0;
}



void Interface::thread_run(int num_thread)
{
    for(int i=0; i<num_thread; i++)
        process_pkt.push_back(thread(&send_packet, this));

    thread db_update(&memoryToDB, this);
    thread capture_func(&packet_capture, this);

    db_update.join();
    capture_func.join();
    for(auto &t : process_pkt)
        t.join();
}

