//=============================
// buffer.h
//=============================

//=============================
// Queue 전처리문
//=============================
#ifndef __circle_queue_H__
#define __circle_queue_H__

#include "dhcp_server.h"

#define TRUE 1
#define FALSE 0
#define ERROR -1
#define MAX_SIZE 2000


using namespace std;

class CircularQueue{

private:
	struct dhcp_header *queue;

public:
	int front;
	int rear;
	int maxQueueSize;
	int iPoint;

	CircularQueue();
	~CircularQueue();
	bool isEmpty();
	bool isFull();
	void enQueue(struct dhcp_header item);
	struct dhcp_header  deQueue();
	//void showCircularQueue();

};


#endif
