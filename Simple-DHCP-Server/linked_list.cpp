#include<iostream>
#include "linked_list.h"
using namespace std;

List::List()
{
	head = NULL;
	tail = NULL;
}


void List:: createnode(struct element *value)
{
	node *temp=new node;
//	temp->element->set(value);
	temp->element=value;
	temp->next=NULL;
	if(head==NULL)
	{
		head=temp;
		tail=temp;
		temp=NULL;
	}
	else
	{	
		tail->next=temp;
		tail=temp;
	}
}

void List::freeList(List *memory)
{
    node *tmpNode = new node;
	tmpNode = memory->head;

    node *prev = NULL;

	while(tmpNode != NULL) {
        prev = tmpNode;            
        tmpNode = tmpNode->next;
        free(prev);
    }
	free(tmpNode);
}
