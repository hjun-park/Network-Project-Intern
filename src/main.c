//=============================
// main.c
//=============================

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mysql.h>
#include "arp.h"
#include "buffer.h"
#include "insert.h"

Queue q;
MYSQL *conn;

int main(int argc, char *argv[])
{
	char query_buffer[2048];
	conn = mysql_init(NULL); 
	
	pthread_t thread[2];
	int th_result = 0;

	int user_select = 0;
	char user_command[100];
	char *end_command = "end\0";
	int check_thread = 0;
	int ch;
    
	if(!mysql_real_connect(conn, "localhost", "root", "", NULL, 0, NULL, 0)) {
		printf("[+] Can't Connect");
		return 1;
	} else {
		if(mysql_select_db(conn, "arp_db")) {
			printf("[+] Can't use arp_db");
			return 1;
		}
	}

	// 인자가 1개 이하면 사용법 설명
	if(argc <= 1) {
		printf("Usuage : %s [interface]\n", argv[0]);
		return 0;
	}

	// Queue 초기화
	InitQueue(&q);

	// thread 생성
	check_thread = pthread_create(&thread[0], NULL, PacketSniffer, (void*)argv[1]);
	if(check_thread < 0) {
		printf("PacketSniffer thread is not created");
		exit(0);
	}
	pthread_detach(thread[0]);


	check_thread = pthread_create(&thread[1], NULL, DBInsert, NULL);
	if(check_thread < 0) {
		printf("DBInsert thread is not created");
		exit(0);
	}
	pthread_detach(thread[1]);	

	// 입력 buffer 지우기
	while ((ch = getchar()) != '\n' && ch != EOF);

	while(strncmp(user_command, end_command, sizeof(end_command))) {
		printf("[+] 'end'를 입력하면 종료합니다: ");

        //=============================
        // 명령어를 입력 받고 쿼리 보낸다.
        // 받아온 쿼리 내용을 출력한다.
        //=============================
		// printf("[+] 필터구문을 입력하세요 : "); 

		//=====================================
		// 필터구문 미완성 
		//=====================================

		scanf("%[^\n]s", user_command);
		
	}

	// 접속 종료 
	printf("\nTerminate Program\n");
	printf("[+] Clear Memory\n");
	DeleteQueue(&q);
	pthread_join(thread[0], (void *)&th_result);
	pthread_join(thread[1], (void *)&th_result);

	printf("[+] Disconnect from Database\n");
	mysql_close(conn);
	
	printf("\nBye\n");

	return 0;
}





