//=============================
// buffer.c
//=============================
#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"

void Enqueue(Queue *q, element *data) {
	if(IsFull(q)) {
		printf("Queue is Full\n");
		return;
	}
	else {
		q->rear = (q->rear+1) % (MAX_SIZE);
		q->data[q->rear] = *data;
	}
	return;
}

void InitQueue(Queue *q) {
	q->front = 0;
	q->rear = 0;
	q->data = (element *)malloc(sizeof(element)*MAX_SIZE);
}

element Dequeue(Queue *q) {
	if(IsEmpty(q)) {
		printf("Queue is Empty\n");
	}
	q->front = (q->front+1) % MAX_SIZE;
	return q->data[q->front];
}

void DeleteQueue(Queue *q) {
	free(q->data);
}

boolean IsEmpty(Queue *q) {
	if(q->front==q->rear) 
		return TRUE;
	else
		return FALSE;
}

boolean IsFull(Queue *q) {
	if(((q->rear+1)%MAX_SIZE) == q->front)
		return TRUE;
	else
		return FALSE;
}


	
