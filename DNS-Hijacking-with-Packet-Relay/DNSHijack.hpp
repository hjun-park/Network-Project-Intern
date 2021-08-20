#ifndef __DNS_HIJACK_H__
#define __DNS_HIJACK_H__

#include "Relay.hpp"
#include "Buffer.hpp"
#include "MysqlDB.hpp"
#include "BuildPacket.hpp"

#define DGRAM_SIZE 8192
#define IP_LEN 20
#define UDP_PORT 53

using namespace std;

class DNSHijack {
	private:
		vector<thread> process_pkt;
		vector<thread> process_pkt2;
		const char *interface;
		pcap_t *pcap_handle;

	public:

		class Buffer *buffer = new Buffer();
		class MysqlDB *mysql_db = new MysqlDB();
		class Relay *relay = new Relay();
		atomic<bool> run {true};

		void makeDatagram(uint8_t *datagram, unsigned int len, struct ether_header *, struct ip *, struct udphdr *);
		unsigned int makeDNSAnswer(struct dns_header *, uint8_t *, char *, uint32_t);
		void runThread();
		void parseQuery(struct packet_data *n_packet, char *extract_domain, int iPoint);
		int sendDNS(struct packet_data *n_packet, char *extract_domain, uint32_t extract_domain_ip);
		bool isUDP(struct ip *);
		bool isDNS(struct udphdr *);
		int startLoop(const char*);
		void packetCapture();
		void processPacket();
		void terminateDB();


};

#endif
