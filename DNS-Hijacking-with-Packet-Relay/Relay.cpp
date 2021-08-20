#include "Relay.hpp"


void Relay::getMyIP(const char *interface) {
	
	struct ifreq ifr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	uint32_t temp_ip;

    strcpy(ifr.ifr_name, interface);
    if (!ioctl(fd, SIOCGIFADDR, &ifr)) {
		my_ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
		temp_ip = inet_addr(my_ip);
		my_ip_int = ntohl(temp_ip);
	}
}

void Relay::getMyMAC(const char *interface) {

    struct ifreq ifr;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strcpy(ifr.ifr_name, interface);
    if (!ioctl(fd, SIOCGIFHWADDR, &ifr)) {
		memcpy((void *)my_mac, ifr.ifr_addr.sa_data, sizeof(uint8_t)*MAC_LEN);
	}
}

int Relay::getRouterMAC() {
	FILE *fp;
	char cmdbuf[CMD_BUF_SIZE];
	char stdout_buf[STDOUT_BUF_SIZE];

//	sprintf(cmdbuf, "ip neigh|grep \"$(ip -4 route list 0/0|cut -d' ' -f3) \"|cut -d' ' -f5|tr '[a-f]' '[A-F]' | sed -e 's/://g'");
	sprintf(cmdbuf, "ip neigh|grep \"$(ip -4 route list 0/0|cut -d' ' -f3) \"|cut -d' ' -f5|tr '[a-f]' '[A-F]'");
	fp = popen(cmdbuf, "r");
	if(fp==NULL) {
		perror("Fail to find Router MAC\n");
		return -1;
	}

	fgets(stdout_buf, STDOUT_BUF_SIZE -1, fp);
	pclose(fp);

	if(stdout_buf == NULL) {
		cout << " Not found Router MAC Address" << endl;
		return 0;
	}

//	cout << sizeof(stdout_buf) << " " << sizeof(router_mac) << endl;
//	printf("result = %s\n", stdout_buf);
	sscanf(stdout_buf, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &router_mac[0], &router_mac[1], &router_mac[2], &router_mac[3], &router_mac[4], &router_mac[5]);

//	memcpy(router_mac, stdout_buf, MAC_LEN);
		
	return 0;	

}

//void Relay::testPrintMyInfo() {
//	printf("%s\n", my_ip);
//	printf("my_IP_int : %u\n", my_ip_int);
//	printf("%02X:%02X:%02X:%02X:%02X:%02X\n", my_mac[0], my_mac[1], my_mac[2], my_mac[3], my_mac[4], my_mac[5]);
//	printf("%02X:%02X:%02X:%02X:%02X:%02X\n", router_mac[0], router_mac[1], router_mac[2], router_mac[3], router_mac[4], router_mac[5]);
//}

 
bool Relay::isSpoofed(struct ether_header *etherHeader, struct ip *ipHeader) {

	/* Destination MAC != Attacker MAC >> not spoofed */
	if(memcmp(etherHeader->ether_dhost, my_mac, MAC_LEN) != 0) {
		return false;

		/* Destination IP != Attacker IP >> don't need to relay */
		if(ntohl(ipHeader->ip_dst.s_addr) != my_ip_int) {
			return false;
		}

	}

	return true;
}

void Relay::relayPacket(pcap_t *pcap_handle, struct packet_data n_packet) {

	/* Modify destination MAC ( Attacker mac ) to router mac */ 
	memcpy(((struct ether_header *)(uint8_t *)n_packet.data)->ether_dhost, router_mac, MAC_LEN * sizeof(uint8_t));
	
	pcap_inject(pcap_handle, n_packet.data, n_packet.packet_size);
	
}

