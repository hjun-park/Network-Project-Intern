#ifndef __MYSQL_DB_H__
#define __MYSQL_DB_H__

#include <iostream>
#include <vector>
#include <mysql.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <cstring> 		// strncmp
#include <netinet/in.h>	// socketaddr_in, in_addr
#include <arpa/inet.h> // inet_aton
#include <atomic>

using namespace std;

class MysqlDB {
	private:
		MYSQL *conn;
		vector<string> ip_array;
		vector<uint32_t> ip_array_int;
		vector<string> domain_array;
			
	public:
		int initDB();
		~MysqlDB();
		void showDomainInfo();
		int deleteDomain(const char *);
		uint32_t compareDomain(const char *);
		bool isValidIP(const char *);
		int inputMemory(const char *, const char *);
		int dbToMemory();
//		void timerSaveDB();
		void memoryToDB();
		
		atomic<bool> run { true };

};

#endif
