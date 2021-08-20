#include <signal.h>
#include <unistd.h>
#include "DNSHijack.hpp"

static void handler(int sig);

DNSHijack *dns_hijack = new DNSHijack();
bool run = 1;

int main(int argc, char *argv[])
{

	/* set signal is <Ctrl> + C or killall */
	signal(SIGINT, handler);
	signal(SIGTERM, handler);
	
	/*
	 * exract interface name from argument 
	 */
	if(argc < 2) {
		printf("[+] usuage : %s [Interface]\n", argv[0]);
		return -1;
	}
	
	printf("Interface name : %s\n", argv[1]);


	/*
	 * DNSHijack->startLoop()
	 */
	dns_hijack->startLoop(argv[1]);

	char domain[254];
	char ip[20];

	int input = 0;

	while(run) {
		memset(domain, 0x00, sizeof(domain));

		printf("\n[+] Select Menu \n");
	   	printf("[1] Add Domain/IP \n");
		printf("[2] Delete Domain/IP \n");
		printf("[3] Show Domain/IP \n");
		printf("Write Number : ");

		rewind(stdin);
		scanf("%d", &input);
		getchar();
		printf("\n\n");

		switch(input) {
			case 1:	{
				printf("[+] Input Domain : ");
				scanf("%s", domain);

				printf("[+] Input IP : ");
				scanf("%s", ip);
		
				if((dns_hijack->mysql_db->inputMemory(domain, ip)) == 1) {
					cout << "Invalid IP Address" << endl;
					continue;
				}
				break;
			}

			case 2: {
				printf("[+] Delete Domain : ");
				scanf("%s", domain);

				if(dns_hijack->mysql_db->compareDomain(domain) == 0) {
					cout << "Not exist domain" << endl;	
					continue;
				}

				else {
					if(dns_hijack->mysql_db->deleteDomain(domain) == 0) {
						printf("[+] Delete \"%s\"\n", domain);
					}
				}
				break;
			}				

			case 3: {
				dns_hijack->mysql_db->showDomainInfo();
				break;
			}

			default: {
				cout << "No Option Selected" << endl;
				break;
			}
		}
	}

	return 0;
}


static void handler(int sig) {
	delete dns_hijack->mysql_db;
	run = false;
	dns_hijack->run = false;
	printf("EXIT %d\n", sig);

	sleep(1);
	exit(0);
}
