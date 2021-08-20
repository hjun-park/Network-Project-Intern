//=============================
// buffer.cpp
//=============================
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "buffer.h"
//#include "dhcp_server.h"

CircularQueue::CircularQueue()
{
	front = -1;
	rear = -1;
	iPoint = 0;
	queue = new struct dhcp_header[MAX_SIZE];

}

CircularQueue::~CircularQueue()
{
	delete queue;
}

bool CircularQueue::isEmpty() {
	// XXX
	if (iPoint == 0) {
		return true;
	}
	return false;
}

bool CircularQueue::isFull() {
	if(iPoint == MAX_SIZE) {
		return true;
	}
	return  false;
}

void CircularQueue::enQueue(struct dhcp_header item) {
	if(isFull() == true)
	{
		return;
	}
	else
	{
		queue[iPoint].set(item);
		iPoint++;
	}

}

struct dhcp_header CircularQueue::deQueue() {

	// 비어 있지 않으면 
	if(!isEmpty()) {
		iPoint--;
		
		printf("@@@@@@ iPoint : %d\n", iPoint);
		printf("deQueue data : %d\n", queue[iPoint].htype);
		printf("!!!!!! Dequeue Done \n");
	
		return queue[iPoint];
	}
}

//void CircularQueue::showCircularQueue() {
//	if (front == rear) {
//		isEmpty();
//	}
//	else {
//		cout << "현재 원형 큐에 존재하는 수는:\n";
//		for (int i = 0; i < MAX_SIZE; i++) {
//		}
//		cout << "\n";
//	}
//
//
//}
