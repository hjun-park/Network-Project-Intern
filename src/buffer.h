//=============================
// buffer.h
//=============================

//=============================
// Queue 전처리문
//=============================
#ifndef __circle_queue_H__
#define __circle_queue_H__

#define TRUE 1
#define FALSE 0
#define ERROR -1
#define MAX_SIZE 100

// boolean도 1/0으로 표시
//typedef int element;
typedef int boolean;
//=============================
// ARP Data Node 
//=============================
typedef struct _element {
	u_char arp_type[20];
	u_char src_mac[6];
	u_char src_ip[4];
	u_char dst_mac[6];
	u_char dst_ip[4];
	u_char timestamp[20];
	u_char	is_garp[1];
} element;


//=============================
// Queue Node
//  1. rear / front / data(struct)
//=============================
typedef struct _CircleQueue {
	int rear;
	int front;
	element *data;
} Queue;


//=============================
//  나머지 함수 입력 
//=============================

void Enqueue(Queue *q, element *data);
void InitQueue(Queue *q);
element Dequeue(Queue *q);
void DeleteQueue(Queue *q);
boolean IsFull(Queue *q);
boolean IsEmpty(Queue *q);


#endif


