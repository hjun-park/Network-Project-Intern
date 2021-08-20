#ifndef __RELAY_H__
#define __RELAY_H__

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <thread>
#include <vector>
#include <chrono>		// using timer

#include "Buffer.hpp"
#include "BuildPacket.hpp"

#include <ifaddrs.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define MAC_LEN 6
#define CMD_BUF_SIZE 256
#define STDOUT_BUF_SIZE 256


using namespace std;


class Relay{
	private:
		uint32_t my_ip_int;

	public:
		char *my_ip;
		uint8_t my_mac[MAC_LEN];
		uint8_t router_mac[MAC_LEN];

		void getMyIP(const char *);
		void getMyMAC(const char *);
		int getRouterMAC();
//		void testPrintMyInfo();
		bool isSpoofed(struct ether_header *, struct ip *);
		void relayPacket(pcap_t *, struct packet_data);

		// not used
//        uint32_t AddressToInt(char *prev_ip) {
//            uint32_t conv_ip = 0;
//            conv_ip = inet_addr(prev_ip);
//            conv_ip = ntohl(conv_ip);
//            return conv_ip;
//        }
//
//        char *intToAddress(uint32_t prev_ip) {
//            struct in_addr conv_ip;
//            char *string_ip;
//
//            prev_ip = htonl(prev_ip);
//            conv_ip.s_addr = prev_ip;
//
//            string_ip = inet_ntoa(conv_ip);
//            return string_ip;
//        }


};

#endif


