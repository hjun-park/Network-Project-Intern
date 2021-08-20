#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr
#include <net/if.h> // struct ifreq
#include <sys/ioctl.h> // macro ioctl 
#include <pcap.h>

#include <ifaddrs.h>
#include <errno.h>
#include <netinet/ip.h>
#include <net/ethernet.h>

#include <chrono>	// using timer
#include <mysql.h>

#include "dhcp_server.h"
#include "buffer.h"
#include "interface.h"

#define MAC_LEN 6

extern "C"

using namespace std;

u_int8_t mac[MAC_LEN];
char *myIP;

int iThreadCount = 0;

static int fill_dhcp_option(u_int8_t *packet, u_int8_t code, u_int8_t *data, u_int8_t len)
{
    packet[0] = code;				 // 옵션코드
    packet[1] = len; 				// 길이
    memcpy(&packet[2], data, len);  // 데이터

    return len + (sizeof(u_int8_t) * 2);
}


static unsigned short in_cksum(unsigned short *addr, int len)
{
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1)
    {
        *(u_char *)(&answer) = *(u_char *) w;
        sum += answer;
    }
    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
    sum += (sum >> 16); 	       		    /* add carry */
    answer = ~sum;          			    /* truncate to 16 bits */
    return (answer);
}



static void ether_output(u_char *frame, u_int8_t *mac, int *len, pcap_t *pcap_handle)
{
    struct ether_header *eframe = (struct ether_header *)frame;

    memcpy(eframe->ether_shost, mac, ETHER_ADDR_LEN);
    memset(eframe->ether_dhost, -1,  ETHER_ADDR_LEN);

    eframe->ether_type = htons(ETHERTYPE_IP);
		
    *len = *len + sizeof(struct ether_header);

	/* send packet */
    pcap_inject(pcap_handle, frame, *len);
}

static void ip_output(struct ip *ip_header, int *len)
{
    *len += sizeof(struct ip);

    ip_header->ip_hl = 5;
    ip_header->ip_v = IPVERSION;
    ip_header->ip_tos = 0x0;
    ip_header->ip_len = htons(*len);
    ip_header->ip_id = htons(0x0);
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 64;
    ip_header->ip_p = IPPROTO_UDP;
    ip_header->ip_sum = 0;
    ip_header->ip_src.s_addr = inet_addr(myIP);
    ip_header->ip_dst.s_addr = 0xFFFFFFFF;

    ip_header->ip_sum = in_cksum((unsigned short *) ip_header, sizeof(struct ip));
}

static void udp_output(struct udp_header *udp_header, int *len)
{
    if (*len & 1)
        *len += 1;
    *len += sizeof(struct udp_header);

    udp_header->us_sport = htons(DHCP_SERVER_PORT);
    udp_header->us_dport = htons(DHCP_CLIENT_PORT);
    udp_header->us_len = htons(*len);
	udp_header->us_sum = in_cksum((unsigned short *) udp_header, sizeof(struct udp_header));
}

static int process_decline(dhcp_header bf_packet, void *parg) {

	class Interface *pInterface = (Interface *)parg;
	dhcp_header dhcp_packet = bf_packet;

	uint32_t ciaddr = dhcp_packet.ciaddr;
	int is_used = 1;

	/* IP in use */
	pInterface->setIsusedValue(pInterface->memory, ciaddr, is_used);
	
	return 0;

}

static int process_release(dhcp_header bf_packet, void *parg) {

	class Interface *pInterface = (Interface *)parg;
	dhcp_header dhcp_packet = bf_packet;

	uint32_t ciaddr = ntohl(dhcp_packet.ciaddr);
	int is_used = 0;

	/* Unused IP */
	pInterface->setIsusedValue(pInterface->memory, ciaddr, is_used);

	return 0;
}


static int dhcp_inform(dhcp_header bf_packet, void *parg) {

	class Interface *pInterface = (Interface *)parg;

	int len = 0;
	u_char packet[4096];

	struct udp_header *udp_header;
	struct ip *ip_header;
	dhcp_header *dhcp;

	pcap_t *pcap_handler = pInterface->pcap_handle;
	dhcp_header dhcp_packet = bf_packet;

	cout << "############ Sending DHCP_INFORM ############" << endl;

	ip_header = (struct ip *)(packet + sizeof(struct ether_header));
	udp_header = (struct udp_header *)(((char *)ip_header) + sizeof(struct ip));
	dhcp = (dhcp_header *)(((char *)udp_header) + sizeof(struct udp_header));

	
    // ==========================
    // fill_dhcp_discovery_options
    // ==========================
    u_int8_t option;
    option = DHCP_OPTION_ACK; 
    len += fill_dhcp_option(&dhcp->bp_options[len], 53, &option, sizeof(option));

	u_int32_t subnet_mask;
	subnet_mask = inet_addr(pInterface->subnet);
	len += fill_dhcp_option(&dhcp->bp_options[len], 1, (u_int8_t *)&subnet_mask, sizeof(subnet_mask));

	u_int32_t router;
	router = inet_addr(pInterface->router);
	len += fill_dhcp_option(&dhcp->bp_options[len], 3, (u_int8_t *)&router, sizeof(router));

	u_int32_t dns_server;
	dns_server = inet_addr(pInterface->dns_server);
	len += fill_dhcp_option(&dhcp->bp_options[len], 6, (u_int8_t *)&dns_server, sizeof(dns_server));

    option = 0;
    len += fill_dhcp_option(&dhcp->bp_options[len], MESSAGE_TYPE_END, &option, sizeof(option));


    // ============================
    // dhcp_output
	// ============================
	len += sizeof(dhcp_header);
	memset(dhcp, 0, sizeof(dhcp_header));
	
	dhcp->opcode = DHCP_BOOTREPLY;
	dhcp->htype = DHCP_HARDWARE_TYPE_10_EHTHERNET;
	dhcp->hlen = 6;
	dhcp->xid = dhcp_packet.xid;
	dhcp->flags = dhcp_packet.flags;
	memcpy(dhcp->chaddr, dhcp_packet.chaddr, DHCP_CHADDR_LEN);

//	// XXX : 
//	dhcp->yiaddr = htonl(dhcp_packet.yiaddr);
//	printf(">>>> dhcp_ack yiaddr = %u<<<<<<\n", dhcp_packet.yiaddr);
//	if(dhcp_packet.ciaddr != 0) {
//		dhcp->yiaddr = dhcp_packet.ciaddr;
//		printf(">>>> dhcp_ack ciaddr = %u<<<<<<\n", htonl(dhcp_packet.ciaddr));
//	}

	dhcp->magic_cookie = htonl(DHCP_MAGIC_COOKIE);


	//=============================
	// udp_output
	//==============================
    udp_output(udp_header, &len);

	//===============================
	// ip_output
	//===============================
    ip_output(ip_header, &len);

	//==============================
	// ether output
	//=============================
    ether_output(packet, mac, &len, pcap_handler);

	return 0;
}


static int dhcp_nack(dhcp_header bf_packet, void *parg) {

	class Interface *pInterface = (Interface *)parg;

	int len = 0;
	u_char packet[4096];

	struct udp_header *udp_header;
	struct ip *ip_header;
	dhcp_header *dhcp;

	pcap_t *pcap_handler = pInterface->pcap_handle;
	dhcp_header dhcp_packet = bf_packet;

	cout << "############ Sending DHCP_NACK ############" << endl;

	ip_header = (struct ip *)(packet + sizeof(struct ether_header));
	udp_header = (struct udp_header *)(((char *)ip_header) + sizeof(struct ip));
	dhcp = (dhcp_header *)(((char *)udp_header) + sizeof(struct udp_header));

	
    // ==========================
    // fill_dhcp_discovery_options
    // ==========================
    u_int8_t option;
    option = DHCP_OPTION_NACK; 
    len += fill_dhcp_option(&dhcp->bp_options[len], 53, &option, sizeof(option));

    option = 0;
    len += fill_dhcp_option(&dhcp->bp_options[len], MESSAGE_TYPE_END, &option, sizeof(option));


    // ============================
    // dhcp_output
	// ============================
	len += sizeof(dhcp_header);
	memset(dhcp, 0, sizeof(dhcp_header));
	
	dhcp->opcode = DHCP_BOOTREPLY;
	dhcp->htype = DHCP_HARDWARE_TYPE_10_EHTHERNET;
	dhcp->hlen = 6;
	dhcp->xid = dhcp_packet.xid;
	dhcp->flags = dhcp_packet.flags;
	dhcp->yiaddr = inet_addr("0.0.0.0");
	memcpy(dhcp->chaddr, dhcp_packet.chaddr, DHCP_CHADDR_LEN);

	dhcp->magic_cookie = htonl(DHCP_MAGIC_COOKIE);


	//=============================
	// udp_output
	//==============================
    udp_output(udp_header, &len);

	//===============================
	// ip_output
	//===============================
    ip_output(ip_header, &len);

	//==============================
	// ether output
	//=============================
    ether_output(packet, mac, &len, pcap_handler);


	return 0;

}

static int dhcp_ack(dhcp_header bf_packet, void *parg) {	

	class Interface *pInterface = (Interface *)parg;
	dhcp_header dhcp_packet = bf_packet;


	// =======================
	// Extract User inform and save to Memory [ Linked List ]
	// =======================
	char reqMAC[18];
	uint32_t reqIP = 0;

	//memcpy(reqMAC, dhcp_packet.chaddr, DHCP_HWADDR_LEN);
	snprintf(reqMAC, sizeof(reqMAC), "%02x:%02x:%02x:%02x:%02x:%02x", dhcp_packet.chaddr[0], dhcp_packet.chaddr[1], dhcp_packet.chaddr[2], dhcp_packet.chaddr[3], dhcp_packet.chaddr[4], dhcp_packet.chaddr[5]);

	
	// ========================
	// Check packet for reassignment ( Renewal or Rebinding )
	// =======================
	uint32_t ciaddr = ntohl(dhcp_packet.ciaddr); 

	if(ciaddr != 0) {
		if(pInterface->checkRenewal(pInterface->memory, ciaddr, reqMAC) == 1) {
			cout << "[+] send nack, Invalid Rebind/Renewal Request" << endl;
			dhcp_nack(dhcp_packet, pInterface);

			// XXX : 에러 시 지우기
			return 0;
		}
	}

	int len = 0;
	u_char packet[4096];

	struct udp_header *udp_header;
	struct ip *ip_header;
	dhcp_header *dhcp;

	pcap_t *pcap_handler = pInterface->pcap_handle;

	cout << "############ Sending DHCP_ACK ############" << endl;

	ip_header = (struct ip *)(packet + sizeof(struct ether_header));
	udp_header = (struct udp_header *)(((char *)ip_header) + sizeof(struct ip));
	dhcp = (dhcp_header *)(((char *)udp_header) + sizeof(struct udp_header));

	
    // ==========================
    // fill_dhcp_discovery_options
    // ==========================
    u_int8_t option;
    option = DHCP_OPTION_ACK; 
    len += fill_dhcp_option(&dhcp->bp_options[len], 53, &option, sizeof(option));

    u_int32_t server_id;
    server_id = inet_addr(myIP);
    len += fill_dhcp_option(&dhcp->bp_options[len], 54, (u_int8_t *)&server_id, sizeof(server_id));

	u_int32_t lease_time;
	lease_time = htonl(pInterface->valid_lifetime);
	len += fill_dhcp_option(&dhcp->bp_options[len], 51, (u_int8_t *)&lease_time, sizeof(lease_time));

	u_int32_t subnet_mask;
	subnet_mask = inet_addr(pInterface->subnet);
	len += fill_dhcp_option(&dhcp->bp_options[len], 1, (u_int8_t *)&subnet_mask, sizeof(subnet_mask));

	u_int32_t router;
	router = inet_addr(pInterface->router);
	len += fill_dhcp_option(&dhcp->bp_options[len], 3, (u_int8_t *)&router, sizeof(router));

	u_int32_t dns_server;
	dns_server = inet_addr(pInterface->dns_server);
	len += fill_dhcp_option(&dhcp->bp_options[len], 6, (u_int8_t *)&dns_server, sizeof(dns_server));

    option = 0;
    len += fill_dhcp_option(&dhcp->bp_options[len], MESSAGE_TYPE_END, &option, sizeof(option));


    // ============================
    // dhcp_output
	// ============================
	len += sizeof(dhcp_header);
	memset(dhcp, 0, sizeof(dhcp_header));
	
	dhcp->opcode = DHCP_BOOTREPLY;
	dhcp->htype = DHCP_HARDWARE_TYPE_10_EHTHERNET;
	dhcp->hlen = 6;
	dhcp->xid = dhcp_packet.xid;
	dhcp->flags = dhcp_packet.flags;

	dhcp->yiaddr = htonl(dhcp_packet.yiaddr);

	printf(">>>> dhcp_ack yiaddr = %u<<<<<<\n", dhcp_packet.yiaddr);
	reqIP = dhcp_packet.yiaddr;
	if(dhcp_packet.ciaddr != 0) {
		dhcp->yiaddr = dhcp_packet.ciaddr;
		printf(">>>> dhcp_ack ciaddr = %u<<<<<<\n", htonl(dhcp_packet.ciaddr));
		reqIP = htonl(dhcp_packet.ciaddr);
	}
	memcpy(dhcp->chaddr, dhcp_packet.chaddr, DHCP_CHADDR_LEN);

	dhcp->magic_cookie = htonl(DHCP_MAGIC_COOKIE);


	//=============================
	// udp_output
	//==============================
    udp_output(udp_header, &len);

	//===============================
	// ip_output
	//===============================
    ip_output(ip_header, &len);

	//==============================
	// ether output
	//=============================
    ether_output(packet, mac, &len, pcap_handler);


	// ======================
	// Update Lease Time & Save to memory
	// =====================
	printf("<<<<<<<<<<<<<<<<<< reqIP is : %u\n", reqIP);
	pInterface->informToMemory(pInterface->memory, reqIP, reqMAC);
	printf("[+] Done \n");

	return 0;
}



static int dhcp_offer( dhcp_header bf_packet, void *parg) {	
	class Interface *pInterface = (Interface *)parg;

	dhcp_header dhcp_packet = bf_packet;



	int len = 0;
	u_char packet[4096];

	struct udp_header *udp_header;
	struct ip *ip_header;
	dhcp_header *dhcp;

	pcap_t *pcap_handler = pInterface->pcap_handle;

	cout << "############ Sending DHCP_OFFER ############" << endl;

	ip_header = (struct ip *)(packet + sizeof(struct ether_header));
	udp_header = (struct udp_header *)(((char *)ip_header) + sizeof(struct ip));
	dhcp = (dhcp_header *)(((char *)udp_header) + sizeof(struct udp_header));

	
    // ==========================
    // fill_dhcp_discovery_options
    // ==========================
    u_int8_t option;
    option = DHCP_OPTION_OFFER; 
    len += fill_dhcp_option(&dhcp->bp_options[len], 53, &option, sizeof(option));

    u_int32_t server_id;

    server_id = inet_addr(myIP);
    len += fill_dhcp_option(&dhcp->bp_options[len], 54, (u_int8_t *)&server_id, sizeof(server_id));

	u_int32_t lease_time;
	lease_time = htonl(pInterface->valid_lifetime);
	len += fill_dhcp_option(&dhcp->bp_options[len], 51, (u_int8_t *)&lease_time, sizeof(lease_time));

	u_int32_t subnet_mask;
	subnet_mask = inet_addr(pInterface->subnet);
	len += fill_dhcp_option(&dhcp->bp_options[len], 1, (u_int8_t *)&subnet_mask, sizeof(subnet_mask));

	u_int32_t router;
	router = inet_addr(pInterface->router);
	len += fill_dhcp_option(&dhcp->bp_options[len], 3, (u_int8_t *)&router, sizeof(router));

	u_int32_t dns_server;
	dns_server = inet_addr(pInterface->dns_server);
	len += fill_dhcp_option(&dhcp->bp_options[len], 6, (u_int8_t *)&dns_server, sizeof(dns_server));

    option = 0;
    len += fill_dhcp_option(&dhcp->bp_options[len], MESSAGE_TYPE_END, &option, sizeof(option));


    // ============================
    // dhcp_output
	// ============================
	len += sizeof(dhcp_header);
	memset(dhcp, 0, sizeof(dhcp_header));
	
	dhcp->opcode = DHCP_BOOTREPLY;
	dhcp->htype = DHCP_HARDWARE_TYPE_10_EHTHERNET;
	dhcp->hlen = 6;
	dhcp->xid = dhcp_packet.xid;
	dhcp->flags = dhcp_packet.flags;
	

	// ===========================
	// Find usable IP in DB
	// ===========================
	uint32_t lease_ip;
	lease_ip = pInterface->findEmptyIP(pInterface->memory);
	if(lease_ip == 0) {	/* not exists then send Nack */
		cout << "[+] No usable IP in DB, send NACK" << endl;
		dhcp_nack(dhcp_packet, pInterface );
	}
	dhcp->yiaddr = htonl(lease_ip);

	// =========================
	// Set Requested IP if yiaddr==reqIP
	// =========================
	if(dhcp_packet.yiaddr != 0) {
		dhcp->yiaddr = htonl(dhcp_packet.yiaddr);
	}

	memcpy(dhcp->chaddr, dhcp_packet.chaddr, DHCP_CHADDR_LEN);

	dhcp->magic_cookie = htonl(DHCP_MAGIC_COOKIE);


	//=============================
	// udp_output
	//==============================
    udp_output(udp_header, &len);

	//===============================
	// ip_output
	//===============================
    ip_output(ip_header, &len);

	//==============================
	// ether output
	//=============================
    ether_output(packet, mac, &len, pcap_handler);

	return 0;
}


/* run by Thread ( process packet thread ) */
int send_packet(void *parg) 
{
	class Interface *pInterface = (Interface *)parg;
	dhcp_header dhcp_packet;

    struct ifreq ifr;
    int sd;
    int one = 1;
    const int *val = &one;

    if((sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) <0) {
        perror("socket() error");
        return 0;
    }
	printf("[+] Process Packet Thread %d is Created.\n", ++iThreadCount);

    //==========================
    // Set interface
    // =========================
    const char *dev;
    dev = pInterface->interfaces;
    memset(&ifr, 0, sizeof(ifr));

    strcpy(ifr.ifr_name, dev);
    setsockopt(sd, SOL_SOCKET, SO_BROADCAST, val, sizeof(one));
    if(setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0) {
        perror("setsockopt() error");
        return 0;
    }

    //==========================
    // Find my IP & MAC address
    //==========================
	ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name, dev);

	ioctl(sd, SIOCGIFADDR, &ifr); 
	myIP = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    ioctl(sd, SIOCGIFHWADDR, &ifr);
    memcpy((void *)mac, ifr.ifr_addr.sa_data, sizeof(u_int8_t)*MAC_LEN);
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	close(sd);

	

    //=============================
    // Send DHCP Packet
    // ============================
	while(1) {
		if(pInterface->packet_buffer->isEmpty()){
			sleep(1);
			continue;
		}

		cout << "start DEQUEUE" << endl;
		
		// TODO : if문 이용해서 검증 한번 더 하기 
		dhcp_packet.set(pInterface->packet_buffer->deQueue());
		cout << "done Dequeue " << endl;
		switch(dhcp_packet.htype) {
		
			case DHCP_OPTION_DISCOVER: 
				printf("[+] Receive Discover Packet\n");
				dhcp_offer(dhcp_packet, pInterface);
				break;

			case DHCP_OPTION_REQUEST:
				printf("[+] Receive Request Packet\n");
				dhcp_ack(dhcp_packet, pInterface);
				break;

			case DHCP_OPTION_DECLINE:
				printf("[+] Receive Decline Packet\n");
				process_decline(dhcp_packet, pInterface);
				break;
				
			case DHCP_OPTION_RELEASE:
				printf("[+] Receive Release Packet\n");
				process_release(dhcp_packet, pInterface);
				break;
			
			case DHCP_OPTION_INFORM:
				printf("[+] Receive Inform Packet\n");
				dhcp_inform(dhcp_packet, pInterface);
				break;
		}
		sleep(0);
	}
	return 0;
}

void ether_input(u_char *args, const struct pcap_pkthdr *header, const u_char *frame)
{
	//==========================
	// Start Packet processing
	// =========================
	cout << "============================" << endl;
	cout << "[+] start pakcket processing" << endl;
	cout << "============================" << endl;
   	class Interface *pInterface = (Interface *)args;
    
   	struct ip *ipHeader = NULL;
   	struct udp_header *udpHeader = NULL;
   	dhcp_header *dhcpHeader = NULL;
   	dhcp_header_option dhcp_option;

   	int iPoint = 0;
	dhcp_header tempDHCP;

	//=============================
	// Calculate Offset 
	// ============================
   	iPoint = sizeof(ether_header);
 	ipHeader = (struct ip *)(frame+iPoint);
   
   	iPoint = iPoint + sizeof(ip);
   	udpHeader = (struct udp_header *)(frame+iPoint);
   
   	iPoint = iPoint + sizeof(udp_header);
   	dhcpHeader = (dhcp_header *)(frame+iPoint);
   
	memcpy(&tempDHCP, dhcpHeader, sizeof(dhcp_header));
	

	// ==============================
	// Copy DHCP Option 
	// ==============================
	iPoint = iPoint + sizeof(dhcp_header);
	
	memcpy(&dhcp_option, (frame + iPoint), sizeof(dhcp_option));

	/* Check if DHCP Type field exists */
	if(dhcp_option.bp_value == 53) {
		if(dhcp_option.bp_type == NULL) {
			printf(" IS NULL POINTER \n");
		}
		printf("BPTYPE : %d\n", dhcp_option.bp_type);
		tempDHCP.htype = dhcp_option.bp_type;
		tempDHCP.bp_options[0] = dhcp_option.bp_type;
	
		/* Check if reqIP field exists */
		if(dhcp_option.bp_value3 == 50) {
			char reqIP[4];
		//	char reqMAC[18];

			uint32_t uReqIP;

			sprintf(reqIP, "%d.%d.%d.%d", dhcp_option.bp_ipAddr[0], dhcp_option.bp_ipAddr[1], dhcp_option.bp_ipAddr[2], dhcp_option.bp_ipAddr[3]);

//			snprintf(reqMAC, sizeof(reqMAC), "%02x:%02x:%02x:%02x:%02x:%02x", tempDHCP.chaddr[0], tempDHCP.chaddr[1], tempDHCP.chaddr[2], tempDHCP.chaddr[3], tempDHCP.chaddr[4], tempDHCP.chaddr[5]);

			uReqIP = pInterface->AddressToInt(reqIP);

			/* Validate that IP exists in DB */
			//if(pInterface->findRequestIP(pInterface->memory, uReqIP) == 0) {
			tempDHCP.yiaddr = pInterface->findRequestIP(pInterface->memory, uReqIP);
			printf("[+] It has Requested IP = %u\n", tempDHCP.yiaddr);
			//printf("[+] Invalid Requested IP Field, send NACK\n");
			//dhcp_nack(tempDHCP, pInterface);
		
//			tempDHCP.yiaddr = uReqIP; 
		}
   		pInterface->packet_buffer->enQueue(tempDHCP);
	}

	// XXX:  flag값 조정해서 종료 시키기
//	sleep(0);


	//pcap_breakloop(pInterface->pcap_handle);

}

/* run by thread ( packet capture thread ) */
void packet_capture(void *parg)
{

	class Interface *pInterface = (Interface *)parg;

	// XXX : 
	/* Set Interface */
	const char *inf = NULL;
	inf = "enp0s8";
//    const char *inf;
//    inf = pInterface->interfaces;

	char errbuf[PCAP_ERRBUF_SIZE];


	/* set Filter */
	struct bpf_program filter;
	bpf_u_int32 net_mask = 0;
	

	if(pInterface->packet_buffer == nullptr) {
		cout << "packet error" << endl;
	}

	if((pInterface->pcap_handle = pcap_open_live(inf, BUFSIZ, 1, 10, errbuf))==NULL) {
		fprintf(stderr, "ERROR :%s\n", errbuf);
		cout << "Can't Open device" << endl;
		return;
	}

	if(pcap_compile(pInterface->pcap_handle, &filter, "udp and (port 67 or port 68)", 1, net_mask) == -1) {
		fprintf(stderr, "ERROR: %s\n", pcap_geterr(pInterface->pcap_handle));
		cout << "Can't Set Filter" << endl;
		return;
	}

	if(pcap_setfilter(pInterface->pcap_handle, &filter)==-1) {
		fprintf(stderr, "ERROR : %s\n", pcap_geterr(pInterface->pcap_handle));
		return;
	}

	//==========================
	// Start Packet Capture 
	// =========================
	pcap_loop(pInterface->pcap_handle, 0, ether_input, (u_char *)pInterface);

	//===========================
	// Close Packet Capture 
	// =========================
	pcap_close(pInterface->pcap_handle);

}

/* run by thread ( DB Update thread ) */
void memoryToDB(void *parg) {
	class Interface *pInterface = (Interface *)parg;

	MYSQL *conn;
	char query_buffer[2048];
	conn = mysql_init(NULL);

	char cZero[] = "0";
	time_t now;

	node *curNode = new node;

	curNode = pInterface->memory->head;
	
	if(!mysql_real_connect(conn, "localhost", "root", "root", NULL, 0, NULL, 0)) {
		cout << "[+] Can't connect to DB" << endl;
		return ;
	}

	else {
		if(mysql_select_db(conn, "dhcp_db")) {
			cout << "[+] Can't use dhcp_db" << endl;
			return ;
		}
	}


	/* Timer run every 1 minutes */
	while(1) {
		curNode = pInterface->memory->head;
		this_thread::sleep_for(chrono::seconds(20));
		time(&now);
		printf("=========== Start Clock =========\n");

		while(curNode != NULL) {
			// XXX : delete here
//			printf("================= ===============\n");
//			printf("address : %s\n", curNode->element->address);
//			printf("hwaddr : %s\n", curNode->element->hwaddr);
//			printf("valid_lifetime : %d\n", curNode->element->valid_lifetime);
//			printf("expire : %d\n", curNode->element->expire);
//			printf("is_used : %d\n", curNode->element->is_used);

			/* Initialize if lease time is exceeded */
			if((now > curNode->element->expire) && curNode->element->is_used == 1) {
				memcpy(curNode->element->hwaddr, cZero, sizeof(cZero));
				curNode->element->is_used = 0;
			}

			curNode = curNode->next;
		}
	
		/* Init */
		curNode = pInterface->memory->head;

		/* Update memory To Database */
		while(curNode != NULL) {
			snprintf(query_buffer, sizeof(query_buffer), "UPDATE lease_db SET hwaddr=\"%s\", valid_lifetime=%d, expire=FROM_UNIXTIME(%d), is_used=%d WHERE address_int=%u", curNode->element->hwaddr, curNode->element->valid_lifetime, curNode->element->expire, curNode->element->is_used, curNode->element->address_int);

			if(mysql_query(conn, query_buffer)) {
				printf("Query Failed : %s\n", query_buffer);
				return ;
			}
			curNode = curNode->next;
		}
	}
	sleep(0);
	mysql_close(conn);
	
}
