#ifndef __PACKET_STRUCT_H__
#define __PACKET_STRUCT_H__

#include <arpa/inet.h>		// ntohl, ntohs, htonl, htons, inet_aton, inet_ntoa 
#include <sys/ioctl.h>		// macro ioctl 
#include <net/ethernet.h>	// ARP Ethernet
#include <netinet/ip.h>		// ip header 
#include <netinet/in.h> 	// socketaddr_in, in_addr
#include <net/if.h>			// struct ifreq 
#include <atomic>			// C++ atomic<bool> run {true};

#include <pcap.h>

#define DNS_QUERY 0x8000

struct udphdr {
	u_short	uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	short	uh_ulen;		/* udp length */
	u_short	uh_sum;			/* udp checksum */
};

struct dns_header { 
    uint16_t xid;     	   	// transaction ID 
    uint16_t flags;    	  	// dns flags 
    uint16_t q_count;  	 	// number of question entries
    uint16_t ans_count;	  	// number of answer entries
    uint16_t auth_count;	// number of authority entries 
    uint16_t add_count;  	// number of additional resource entries 
};


struct dns_query {
    uint8_t     *q_name; 	// name, string part
    uint16_t    q_type;     // query type
    uint16_t    q_class;    // query class
};


struct dns_answer {
    uint16_t ans_name;		// answer name 
    uint16_t ans_type;		// answer type 
    uint16_t ans_class;		// answer class  
    uint32_t ans_ttl;		// TTL 
    uint16_t data_len;		// data len ( IP = 4 )
    uint32_t ans_address;	//	IP address
};

#endif
