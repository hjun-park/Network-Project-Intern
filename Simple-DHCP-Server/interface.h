#ifndef __INTERFACE__H__
#define __INTERFACE__H__

#include <pcap.h>
#include <vector>
#include <thread>
#include <arpa/inet.h>
#include "interface.h"
#include "buffer.h"
#include "rapid.h"
#include "linked_list.h"

using namespace std;

//========================
// 스레드 클래스 선언
//=======================
class Interface
{
    private:
		vector<thread> process_pkt;


    public:
		char interfaces[10];
		int valid_lifetime;
		char pool[30];
		char subnet[20];
		char router[20];
		char dns_server[20];

        void thread_run(int num_thread);
		void print_func();
		void set_config(Parser *parser);
		

		void setIsusedValue(List *memory, uint32_t ciaddr, int value);
		void informToMemory(List *memory, uint32_t reqIP, char *reqMAC);
		int dbToMemory(List *list);

		int checkRenewal(List *memory, uint32_t ciaddr, char *reqMAC);
		int findEmptyIP(List *memory);
		uint32_t findRequestIP(List *memory, uint32_t reqIP);

		uint32_t AddressToInt(char *prev_ip) {
			uint32_t conv_ip = 0;
			conv_ip = inet_addr(prev_ip);
			conv_ip = ntohl(conv_ip);
			return conv_ip;
		}

		char *intToAddress(uint32_t prev_ip) {
			struct in_addr conv_ip;
			char *string_ip;

			prev_ip = htonl(prev_ip);
			conv_ip.s_addr = prev_ip;

			string_ip = inet_ntoa(conv_ip);
			return string_ip;
		}
		
		class CircularQueue *packet_buffer;
		class List *memory;
		
		pcap_t *pcap_handle;
		
};


# endif
