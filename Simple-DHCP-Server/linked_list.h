#include<iostream>
#include <cstring>
#include <ctime>
using namespace std;

struct element {
	char		interface[10];
	uint32_t	address_int;
	char 		address[20];
	char 		hwaddr[20];
	uint32_t	valid_lifetime;
	uint32_t	expire;
	char		hostname[255];
	int			is_used;

	void set(struct element *temp) {
		memcpy(interface, temp->interface, sizeof(interface));
		address_int = temp->address_int;
		memcpy(address, temp->address, sizeof(address));
		memcpy(hwaddr, temp->hwaddr, sizeof(hwaddr));
		valid_lifetime = temp->valid_lifetime;
		expire = temp->expire;
		memcpy(hostname, temp->hostname, sizeof(hostname));
	}

};


struct node
{
 	struct element *element;	
    node *next;
};

class List
{
    public:
		node *head, *tail;
	    List();
		void createnode(struct element *value);
		void freeList(List *memory);
};
