#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <iostream>
#include <cstring>

#define MAXVALUE 4096
#define PACKET_SIZE 1514

using namespace std;

/* RAW Packet과 Packet Size를 구조체로 저장 */
struct packet_data {
	int packet_size;
	u_char data[PACKET_SIZE];
	

	/* Struct Constructor in C++ */
	packet_data()
	{
		packet_size = 0;
		memset(data, 0, PACKET_SIZE);
	}

};


class Buffer {
	private:
		int front;
		int rear;
		int size;
		packet_data values[MAXVALUE];

	public:
		Buffer();
		void enQueue(packet_data temp_data);
		int deQueue(packet_data &temp_data);
		bool isEmpty();
		bool isFull();

};



#endif
