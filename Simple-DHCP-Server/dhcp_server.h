#ifndef __DHCP__CAPTURE_H__
#define __DHCP__CAPTURE_H__

#define DHCP_CHADDR_LEN 16
#define DHCP_SNAME_LEN  64
#define DHCP_FILE_LEN   128

#define DHCP_SNAME_LEN  64
#define DHCP_FILE_LEN   128
#define DHCP_HWADDR_LEN	6

#include "interface.h"
#include <cstring>

struct udp_header
{
	u_short us_sport;
	u_short us_dport;
	u_short us_len;
	u_short us_sum;
};

struct dhcp_header
{
    u_int8_t    opcode;
    u_int8_t    htype;
    u_int8_t    hlen;
    u_int8_t    hops;
    u_int32_t   xid;
    u_int16_t   secs;
    u_int16_t   flags;
    u_int32_t   ciaddr;
    u_int32_t   yiaddr;
    u_int32_t   siaddr;
    u_int32_t   giaddr;
	u_int8_t	chaddr[DHCP_HWADDR_LEN];
	u_int8_t	offset[DHCP_CHADDR_LEN-DHCP_HWADDR_LEN];
    char        bp_sname[DHCP_SNAME_LEN];
    char        bp_file[DHCP_FILE_LEN];
    uint32_t    magic_cookie;
	u_int8_t	bp_options[0];


	void set(struct dhcp_header temp) {
		opcode = temp.opcode;
		htype = temp.htype;
		hlen = temp.hlen;
		hops = temp.hops;
		xid = temp.xid;
		secs = temp.secs;
		flags = temp.flags;
		ciaddr = temp.ciaddr;
		yiaddr = temp.yiaddr;
		siaddr = temp.siaddr;
		giaddr = temp.giaddr;
		
		for(int i=0; i<DHCP_HWADDR_LEN; i++) {
			chaddr[i] = temp.chaddr[i];
		}

		memcpy(bp_sname, temp.bp_sname, DHCP_SNAME_LEN);
		memcpy(bp_file, temp.bp_file, DHCP_FILE_LEN);
		
		magic_cookie = temp.magic_cookie;
		bp_options[0] = temp.bp_options[0];
	}

};

struct dhcp_header_option
{
	// type
    u_int8_t    bp_value;
    u_int8_t    bp_length;
    u_int8_t    bp_type;

	// hw address 
	u_int8_t	bp_value2;
	u_int8_t	bp_length2;
	u_int8_t	bp_type2;
	u_int8_t	bp_hwaddr[DHCP_HWADDR_LEN];
	
	// requested ip address
	u_int8_t	bp_value3;
	u_int8_t	bp_length3;
	u_int8_t	bp_ipAddr[4];

	void set(struct dhcp_header_option temp) {
		bp_value = temp.bp_value;
		bp_length = temp.bp_length;
		bp_type = temp.bp_type;
	}
};

#define DHCP_BOOTREQUEST                    1
#define DHCP_BOOTREPLY                      2

#define DHCP_HARDWARE_TYPE_10_EHTHERNET     1

#define MESSAGE_TYPE_PAD                    0
#define MESSAGE_TYPE_REQ_SUBNET_MASK        1
#define MESSAGE_TYPE_ROUTER                 3
#define MESSAGE_TYPE_DNS                    6
#define MESSAGE_TYPE_DOMAIN_NAME            15
#define MESSAGE_TYPE_REQ_IP                 50
#define MESSAGE_TYPE_DHCP                   53
#define MESSAGE_TYPE_PARAMETER_REQ_LIST     55
#define MESSAGE_TYPE_END                    255

#define DHCP_OPTION_DISCOVER                1
#define DHCP_OPTION_OFFER                   2
#define DHCP_OPTION_REQUEST                 3
#define DHCP_OPTION_DECLINE					4
#define DHCP_OPTION_ACK						5
#define DHCP_OPTION_NACK					6
#define DHCP_OPTION_RELEASE					7
#define DHCP_OPTION_INFORM					8


typedef enum {
    VERBOSE_LEVEL_NONE,
    VERBOSE_LEVEL_ERROR,
    VERBOSE_LEVEL_INFO,
    VERBOSE_LEVEL_DEBUG,
}verbose_level_t;

#define DHCP_SERVER_PORT    67
#define DHCP_CLIENT_PORT    68

#define DHCP_MAGIC_COOKIE   0x63825363

#define PCKT_LEN 8192

void packet_capture(void *parg);
int send_packet(void *parg);
void memoryToDB(void *parg);


#endif
