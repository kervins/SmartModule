/* Project:	SmartModule
 * File:	linked_list.h
 * Author:	Jonathan Ruisi
 * Created:	April 03, 2017, 3:27 PM
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

// TYPE DEFINITIONS------------------------------------------------------------
// Doubly linked list node

typedef struct LinkedListNode
{
	void* data;
	unsigned char memoryIndex;
	struct LinkedListNode* next;
	struct LinkedListNode* prev;
} LinkedListNode;

// Doubly linked list

typedef struct LinkedList_16Element
{
	LinkedListNode nodeMemory[16];
	unsigned int memoryBitmap;
	LinkedListNode* first;
	LinkedListNode* last;
} LinkedList_16Element;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Management Functions
void LinkedList_16Element_Initialize(LinkedList_16Element*);
LinkedListNode* LinkedListNewNode(LinkedList_16Element*);
void LinkedListFreeNode(LinkedList_16Element*, LinkedListNode*);
// Manipulation Functions
void LinkedListInsert(LinkedList_16Element*, LinkedListNode*, void*, bool);
void LinkedListReplace(LinkedList_16Element*, LinkedListNode*, void*);
void LinkedListRemove(LinkedList_16Element*, LinkedListNode*);
// Search Functions
LinkedListNode* LinkedListFindFirst(LinkedList_16Element*, void*);
LinkedListNode* LinkedListFindLast(LinkedList_16Element*, void*);

#endif