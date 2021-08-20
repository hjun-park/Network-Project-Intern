//=============================
// insert.c
//=============================
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <unistd.h>	// sleep
#include "buffer.h"

extern Queue q;
extern MYSQL *conn;

int *DBInsert()
{

	element arp_elem;
	char query_buffer[2048] = {0,};

	u_char src_ip[20] = {0, };
	u_char src_mac[20] = {0, };
	u_char dst_ip[20] = {0, };
	u_char dst_mac[20] = {0, };

	while(1) {
		if(IsEmpty(&q)) {
			sleep(1);
			continue;
		}

    	arp_elem = Dequeue(&q);	
    
		// 큐에서 꺼낸 구조체 데이터 DB Insert 하기 위해 따로 저장
		sprintf(src_mac, "%02X:%02X:%02X:%02X:%02X:%02X", arp_elem.src_mac[0], arp_elem.src_mac[1], arp_elem.src_mac[2], arp_elem.src_mac[3], arp_elem.src_mac[4], arp_elem.src_mac[5]);
		sprintf(src_ip, "%d.%d.%d.%d", arp_elem.src_ip[0], arp_elem.src_ip[1], arp_elem.src_ip[2], arp_elem.src_ip[3]);
		sprintf(dst_mac, "%02X:%02X:%02X:%02X:%02X:%02X", arp_elem.dst_mac[0], arp_elem.dst_mac[1], arp_elem.dst_mac[2], arp_elem.dst_mac[3], arp_elem.dst_mac[4], arp_elem.dst_mac[5]);
		sprintf(dst_ip, "%d.%d.%d.%d", arp_elem.dst_ip[0], arp_elem.dst_ip[1], arp_elem.dst_ip[2], arp_elem.dst_ip[3]);

		// DB Insert 구문
		sprintf(query_buffer, "insert into arp_db(arp_type, send_mac, recv_mac, send_ip, recv_ip, timestamp, is_garp) values ('%s', '%s', '%s', '%s', '%s', '%s', '%s')", arp_elem.arp_type, src_mac, dst_mac, src_ip, dst_ip, arp_elem.timestamp, arp_elem.is_garp);

		if(mysql_query(conn, query_buffer)) {
			printf("query failed : %s\n", query_buffer);

			break;
		} 

		sleep(0);
	}
	return 0;
}

