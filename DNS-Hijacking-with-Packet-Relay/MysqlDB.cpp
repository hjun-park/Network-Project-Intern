#include "MysqlDB.hpp"

int MysqlDB::initDB() {

	conn = mysql_init(NULL);

   if(!mysql_real_connect(conn, "localhost", "root", "root", NULL, 0, NULL, 0)) {
        cout << "[+] Cannot Connect to DB" << endl;
        return -1;
    }

    else {
        if(mysql_select_db(conn, "dns_db")) {
            cout << "[+] Cannot use dns_db" << endl;
            return -1;
        }
    }


	printf("[+] Connect To dns_db\n");

	return 0;

}

MysqlDB::~MysqlDB() {

	run = false;

	char query_buffer[2048];

	for(vector<int>::size_type i=0; i<ip_array_int.size(); i++) {
		snprintf(query_buffer, sizeof(query_buffer), "INSERT INTO dns_info(ipaddr, ipaddr_int, domain) VALUES('%s', '%u', '%s') ON DUPLICATE KEY UPDATE domain='%s'", ip_array[i].c_str(), ip_array_int[i], domain_array[i].c_str(), domain_array[i].c_str());

		if(mysql_query(conn, query_buffer)) {
			printf("Query Failed : %s\n", query_buffer);
			return ;
		}		
	}
	cout << "\nSave memoryToDB" << endl;
	// DB에 내용을 업데이트해야하며 끝나면 close문을 실행하게 한다.
	mysql_close(conn);
	mysql_library_end();
	cout << "Close MysqlDB" << endl;
}

void MysqlDB::showDomainInfo() {
	
	for(vector<int>::size_type i=0; i<ip_array_int.size(); i++) {
		cout << "IP_int is : " << ip_array_int[i] << endl;
		cout << "IP is : " << ip_array[i] << endl;
		cout << "domain is : " << domain_array[i] << endl;
	}
}

int MysqlDB::deleteDomain(const char *domain) {

	char query_buffer[2048]; 
		
	snprintf(query_buffer, sizeof(query_buffer), "DELETE FROM dns_info WHERE domain='%s'", domain);

	if(mysql_query(conn, query_buffer)) {
		printf("Query Failed : %s\n", query_buffer);
		return 1;
	}		
	
	// 벡터를 초기화 
	
	dbToMemory();

	return 0;

}


uint32_t MysqlDB::compareDomain(const char *target_domain) {

	for(vector<int>::size_type i=0; i<domain_array.size(); i++) {
		if(strcmp(target_domain, "") != 0 && strncmp(target_domain, domain_array[i].c_str(), sizeof(domain_array[i].length()))==0) {
			return htonl(ip_array_int[i]);
		}
	}

	return 0;
}

bool MysqlDB::isValidIP(const char *ip) {
	struct sockaddr_in sa;
	return inet_pton(AF_INET, ip, &(sa.sin_addr))==1 ? true :false;
}

int MysqlDB::inputMemory(const char *domain, const char *ip) {

	uint32_t ip_int;
	struct in_addr *ip_struct = (struct in_addr *)calloc(1, sizeof(in_addr));

	if(!isValidIP(ip)) {
		return 1;
	}

	inet_aton(ip, ip_struct);
	ip_int = ip_struct->s_addr;
	ip_int = ntohl(ip_int);

	ip_array.push_back(ip);
	ip_array_int.push_back(ip_int);
	domain_array.push_back(domain);
	

	for(vector<int>::size_type i=0; i<ip_array_int.size(); i++) {
		cout << "============== print ==============" << endl;
		cout << "IP_int is : " << ip_array_int[i] << endl;
		cout << "IP is : " << ip_array[i] << endl;
		cout << "domain is : " << domain_array[i] << endl;
		cout << "===================================" << endl;
	}

	memoryToDB();
	cout << "!!!!!!!!! end input memory func " << endl;
	dbToMemory();
	cout << "!!!!!!!!! Sync with DB to Memory" << endl;
	free(ip_struct);

	return 0;
}

//void MysqlDB::inputMemory() {
//
//	char domain[254];
//	char ip[20];
//	uint32_t ip_int;
//	struct in_addr *ip_struct = (struct in_addr *)calloc(1, sizeof(in_addr));
//
//	while(run) {
//		printf("[+] Input Domain : ");
//		scanf("%s", domain);
//
////		if(!strncmp(domain, "exit", sizeof(4))) {
////			cout << "XXX : make end" << endl;
////			// XXX : 종료 처리 필요 
////		}
//
//		printf("[+] Input IP : ");
//		scanf("%s", ip);
//		
//		if(!isValidIP(ip)) {
//			cout << "Invalid IP Address" << endl;
//			sleep(0);
//			continue;
//		}
//
//		inet_aton(ip, ip_struct);
//		ip_int = ip_struct->s_addr;
//		ip_int = ntohl(ip_int);
//
//		ip_array.push_back(ip);
//		ip_array_int.push_back(ip_int);
//		domain_array.push_back(domain);
//		
//
//		for(vector<int>::size_type i=0; i<ip_array_int.size(); i++) {
//			cout << "============== print ==============" << endl;
//			cout << "IP_int is : " << ip_array_int[i] << endl;
//			cout << "IP is : " << ip_array[i] << endl;
//			cout << "domain is : " << domain_array[i] << endl;
//			cout << "===================================" << endl;
//		}
//
//		
//		sleep(0);
//	}
//	cout << "!!!!!!!!! end input memory func " << endl;
//
//	free(ip_struct);
//
//}


int MysqlDB::dbToMemory() {
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query_buffer[2048];

	ip_array.clear();
	ip_array_int.clear();
	domain_array.clear();

	/*
	 * DB 내용 추출
	 */
	snprintf(query_buffer, sizeof(query_buffer), "SELECT ipaddr, ipaddr_int, domain from dns_info");

	if(mysql_query(conn, query_buffer)) {
		printf("query failed : %s\n", query_buffer);
		return -1;
	}

	res = mysql_store_result(conn);

	/*
	 * DB to memory 
	 */

	while((row = mysql_fetch_row(res))) {
		ip_array.push_back(row[0]);
		ip_array_int.push_back(atoi(row[1]));
		domain_array.push_back(row[2]);
	}

	mysql_free_result(res);

	
	return 0;

}


///* run by updateThread */
//void MysqlDB::timerSaveDB() {
//
//	time_t now;
//
//	while(run) {
//		this_thread::sleep_for(chrono::seconds(20));
//		time(&now);
//
//		printf("========= Start Synchronous =======\n");
//		memoryToDB();
//		
//		sleep(0);
//	}
//
//
//	// XXX : 자고 있는 스레드를 깨우는 방법 
//	cout << "STOP memoryToDB " << endl;
//}

void MysqlDB::memoryToDB() {

	char query_buffer[2048];

	for(vector<int>::size_type i=0; i<ip_array_int.size(); i++) {
			snprintf(query_buffer, sizeof(query_buffer), "INSERT INTO dns_info(ipaddr, ipaddr_int, domain) VALUES('%s', '%u', '%s') ON DUPLICATE KEY UPDATE domain='%s'", ip_array[i].c_str(), ip_array_int[i], domain_array[i].c_str(), domain_array[i].c_str());

		if(mysql_query(conn, query_buffer)) {
			printf("Query Failed : %s\n", query_buffer);
			return ;
		}		

	}

}


