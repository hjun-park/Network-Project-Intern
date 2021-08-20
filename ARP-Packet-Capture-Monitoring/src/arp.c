//=============================
// arp.c
//=============================
#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h> // memcpy 
#include <arpa/inet.h> // ntohs 
#include <sys/timeb.h>
#include <time.h>
#include "arp.h"
#include "buffer.h"

extern Queue q;


void ArpParsing(u_char *useless, const struct pcap_pkthdr * packet_header, const u_char * uPacket)
{
	char timebuf[20];
	char arp_type_buf[10];
	struct timeb itb;
	struct tm *lt;

	element arp_elem;
	ether_header *etherheader = NULL;
	arp_header *arpheader = NULL;
	etherheader = (struct ether_header*)uPacket;
	arpheader = (struct arp_hdr*)(uPacket+14);

	int i=0;

	// ntohs ( network byte order -> host byte order ) ( big endian -> little endian )
	if (ntohs(arpheader->hw_type) == 1 && ntohs(arpheader->protocol_type) == 0x0800){

    	// Get TimeStamp
    	ftime(&itb);
    	lt = localtime(&itb.time);
    	sprintf(timebuf, "%02d:%02d:%02d.%03d", lt->tm_hour, lt->tm_min, lt->tm_sec, itb.millitm);
		memcpy(q.data->timestamp, timebuf, sizeof(q.data->timestamp));

    	// Get srcIP, srcMAC, dstIP, dstMAC
		memcpy(q.data->src_ip, arpheader->src_ip, sizeof(q.data->src_ip));
    	memcpy(q.data->src_mac, arpheader->src_mac, sizeof(q.data->src_mac));
    	memcpy(q.data->dst_ip, arpheader->dst_ip, sizeof(q.data->dst_ip));
    	memcpy(q.data->dst_mac, arpheader->dst_mac, sizeof(q.data->dst_mac));
		
		// Get OPcode 
    	sprintf(arp_type_buf, "%s", (ntohs(arpheader->opcode) == ARP_REQUEST)? "ARP Request" : "ARP Reply"); 
		memcpy(q.data->arp_type, arp_type_buf, sizeof(q.data->arp_type));

		// Get isGARP
		for (i=0; i<4; i++) {
			if(arpheader->src_ip[i] != arpheader->dst_ip[i]) {
				memcpy(q.data->is_garp, "N\0", sizeof(q.data->is_garp));
			} else {
				memcpy(q.data->is_garp, "Y\0", sizeof(q.data->is_garp));
			}
		}

		Enqueue(&q, q.data);
	}

	// 2020.06.25
	sleep(0);

}

void *PacketSniffer(void *dev)
{
	char *device = (char *)dev;		
	const unsigned char *uPacket = NULL; 	

	struct bpf_program filter;		
	char errbuf[PCAP_ERRBUF_SIZE];	
	pcap_t *handler = NULL;			// 인터페이스 핸들러(=packet capture descriptor)
	bpf_u_int32 net_mask = 0;

	memset(errbuf, 0, PCAP_ERRBUF_SIZE);

	// 패킷캡처 하기 위해 네트워크 디바이스 오픈
	if((handler = pcap_open_live(device, MAXBYTE2CAPTURE, 1, -1, errbuf))==NULL) {
		fprintf(stderr, "ERROR: %s\n", errbuf);
		printf("장치를 열 수 없습니다.\n");
		return 1;
	}

	// bpf 구조체 필터 설정
	if(pcap_compile(handler, &filter, "arp", 1, net_mask) == -1) {
		fprintf(stderr, "ERROR: %s\n", pcap_geterr(handler));
		printf("필터를 적용할 수 없습니다.\n");
		return 1;
	}

	// 핸들러에 필터 적용
	if(pcap_setfilter(handler, &filter) == -1) {
		fprintf(stderr, "ERROR : %s\n", pcap_geterr(handler));
		return 1;
	}

	// 패킷캡처 시작
	pcap_loop(handler, 0, ArpParsing, NULL);
}
	


