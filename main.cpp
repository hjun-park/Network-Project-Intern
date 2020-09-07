#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>
#include "rapid.h"
#include "buffer.h"
#include "dhcp_server.h"
#include "interface.h"
#include <mysql.h>
#include <arpa/inet.h> // ip addr to int

#include <sys/stat.h>

using namespace std;


int main(int argc, char *argv[])
{

	// 객체 생성
	int num_thread = 5;
	char start_ip[16];
	char dest_ip[16];
	uint32_t uStart_ip = 0;
	uint32_t uDest_ip = 0;
	//int count = 0;

	// ==========================
	// Determine number of thread 
	// ==========================
	if(argc == 2) {
		num_thread = atoi(argv[1]);
		if(num_thread > 50) {
			printf("[+] usuage %s [thread <= 50]\n", argv[0]);
			return 0;
		}
	}

	// =========================
	// 1. config File Parser 
	// 2. program Interface 
	// 3. packet Buffer (packet capture) - Circular Queue
	// 4. Memory ( extract from DB )  - Linked List 
	// =========================
	Parser *config = (Parser *)malloc(sizeof(Parser));
	Interface *interface = new Interface();
	interface->packet_buffer = new CircularQueue();
	interface->memory = new List();



	//====================
	// Read Config Json File and save to Interface member variable
	// ==================
	*config = parsing();
	interface->set_config(config);

	// ===================
	// strtok : parsing start_ip and dest_ip 
	// ===================
	char *token = strtok(config->pool, "-");	
	snprintf(start_ip, strlen(token)+1, "%s", token);
	token = strtok(NULL, " ");
	snprintf(dest_ip, strlen(token)+1, "%s", token);

	// XXX :
	uStart_ip = interface->AddressToInt(start_ip);
	uDest_ip = interface->AddressToInt(dest_ip);

	if(uStart_ip > uDest_ip) {
		printf("[+] Check your dhcp_config.json Files ( IP range ) \n");
		return 0;
	}


	//====================
	// Integer to Address
	//====================
	//char *str1 = intToAddress(uStart_ip);
	//char *str2 = intToAddress(uDest_ip);
   	

	//=====================
	//DB connect
	//=====================
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query_buffer[2048] = {0, };
	conn = mysql_init(NULL);
	uint32_t min_address, max_address = 0;

    if(!mysql_real_connect(conn, "localhost", "root", "root", NULL, 0, NULL, 0)) {
        cout << "[+] Can't Connect" << endl;
        return 1;
    } 
	
	else {
        if(mysql_select_db(conn, "dhcp_db")) {
            cout << "[+] Can't use dhcp_db" << endl;
            return 1;
        }
    }      


	// ================================
	// Find min and max address from DB
	// ================================ 
	snprintf(query_buffer, sizeof(query_buffer), "SELECT MIN(address_int), MAX(address_int) FROM lease_db WHERE interface='%s'", config->interfaces);
	mysql_query(conn, query_buffer);
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);
	mysql_free_result(res);

	if(row[0] != NULL && row[1] != NULL) {
		min_address = atoi(row[0]);
		max_address = atoi(row[1]);
	}


	// ===============================
	// XXX INSERT INTO DB 
	// ==============================
	for(uint32_t i=uStart_ip; i<=uDest_ip; i++) {
		string s = to_string(i);

		// 생성 먼저 하고
		sprintf(query_buffer, "insert into lease_db(interface, address_int, address, valid_lifetime) values ('%s', '%s', '%s', '%d')", config->interfaces, s.c_str(), interface->intToAddress(i), config->valid_lifetime);
		mysql_query(conn, query_buffer);

//		if(mysql_query(conn, query_buffer)) {
//			cout << mysql_error(conn) << endl;
//		}
	}
	

	// ==============================
	// Delete IP ( compare before config file )
	// =============================
	if(min_address < uStart_ip) {
		cout << "[+] Low Deleted" << endl;
		snprintf(query_buffer, sizeof(query_buffer), "DELETE FROM lease_db WHERE address_int < %u", uStart_ip);
		mysql_query(conn, query_buffer);
	}
	
	if(max_address > uDest_ip) {
		cout << "[+] High Deleted" << endl;
		snprintf(query_buffer, sizeof(query_buffer), "DELETE FROM lease_db WHERE address_int > %u", uDest_ip);
		mysql_query(conn, query_buffer);
	}

	// ==============================
	// Retrieve valid_lifetime 
	// =============================
	// update lease_db set valid_lifetime=3000 WHERE interface="enp0s8";
	// XXX : valid_lifetime이 여러 개라면 ? => 아래와 같이 쿼리문을 수정 
	int flag_valid_lifetime = 0;

	snprintf(query_buffer, sizeof(query_buffer), "SELECT DISTINCT valid_lifetime FROM lease_db");
	mysql_query(conn, query_buffer);
	res = mysql_store_result(conn);

	while((row = mysql_fetch_row(res))) {
		if(atoi(row[0]) != config->valid_lifetime) {
			printf("row[0] = %d\n", atoi(row[0]));
			cout << "different valid_lifetime " << endl;
			flag_valid_lifetime = 1; 
			break;
		}
	}

	mysql_free_result(res);

	// ==============================
	// If valid_lifetime is different with before valid_lifetime
	// ============================= 
	if(flag_valid_lifetime == 1) {
		cout << "[+] Modify Lifetime " << endl;
		snprintf(query_buffer, sizeof(query_buffer), "UPDATE lease_db SET valid_lifetime=%d WHERE interface='%s'", config->valid_lifetime, config->interfaces);
		mysql_query(conn, query_buffer);
		cout << mysql_error(conn) << endl;
		
		/* Modify expire because the valid_lifetime has changed */
		cout << "[+] Update lease time " << endl;
		snprintf(query_buffer, sizeof(query_buffer), "UPDATE lease_db SET expire=DATE_ADD(NOW(), INTERVAL valid_lifetime SECOND)");
		mysql_query(conn, query_buffer);
		cout << mysql_error(conn) << endl;
	}

	
	// ============================
	// Load data ( DB to Memory )
	// & thread run 
	// 1. DB thread x 1
	// 2. packet capture thread x1
	// 3. packet process thread xnum
	// ============================
	interface->dbToMemory(interface->memory);
	interface->thread_run(num_thread);
	

	// =========================
	// Close ( XXX : When ? )
	// =========================
	mysql_close(conn);

	delete interface;
	free(config);

	delete interface->packet_buffer;
	interface->memory->freeList(interface->memory);
	delete interface;

	return 0;
}


