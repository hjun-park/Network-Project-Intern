//=============================
// arp.h
//=============================

#ifndef __ARP_H__
#define __ARP_H__

#define ARP_REQUEST	1
#define ARP_REPLY	2

//=============================
// 1 : Ethernet 헤더
//=============================
typedef struct _ether_header {
	u_char dest_ether[6];
	u_char src_ether[6];
	u_char *type[1];

} ether_header;
// 패킷을 받아오고 +14 하면 ARP 헤더 위치 도달 가능


//=============================
// 2 : ARP 헤더
//=============================
typedef struct _arp_header {
	u_int16_t hw_type;
	u_int16_t protocol_type;
	u_char hw_len;
	u_char protocol_len;
	u_int16_t opcode;
	u_char src_mac[6];
	u_char src_ip[4];
	u_char dst_mac[6];
	u_char dst_ip[4];
} arp_header;

#define MAXBYTE2CAPTURE 2048

//=============================
// 3 : ARP 관련 함수들
//=============================
void *PacketSniffer(void *dev);

void ArpParsing(u_char *useless, const struct pcap_pkthdr * packet_header, const u_char *uPacket);




#endif
