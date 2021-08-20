
#include "Buffer.hpp"
#include "BuildPacket.hpp"

/*
 * 가변 버퍼를 만들 수도 있음
 */

Buffer::Buffer() {
	size = MAXVALUE;
	front = 0;
	rear = 0;
}

void Buffer::enQueue(packet_data temp_data) 
{
	if(!isFull()) {
		//values[rear].set(value);
		values[rear] = temp_data;
		rear = (rear + 1) % size;
	}

	else
		return ;
//		cout << "Queue Full" << endl;
}


int Buffer::deQueue(packet_data &temp_data)
{
	int bf_front = 0;

	if(!isEmpty()) {
		bf_front = front;
		front = (front + 1) % size;
		temp_data = values[bf_front];
		return 1;

	}
		
	else {
//		cout << "Queue Empty" << endl;
		return 0;
	}
}

bool Buffer::isEmpty()
{
	if(rear == front)
		return true;
	else
		return false;
}

bool Buffer::isFull()
{
	if((rear + 1) % size == front)
		return true;
	else
		return false;
}

