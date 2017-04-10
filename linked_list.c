/* Project:	SmartModule
 * File:	linked_list.c
 * Author:	Jonathan Ruisi
 * Created:	April 03, 2017, 3:27 PM
 */

#include <stdint.h>
#include "linked_list.h"
#include "utility.h"

// MANAGEMENT FUNCTIONS--------------------------------------------------------

void LinkedList_16Element_Initialize(LinkedList_16Element* list,
									 void* elementMemory, unsigned char elementSize)
{
	if(list == 0 || elementMemory == 0)
		return;

	list->first = 0;
	list->last = 0;
	list->memoryBitmap = 0;
	list->elementMemoryBaseAddr = (char*) elementMemory;
	list->elementSize = elementSize;

	// Initialize memory indices
	uint8_t i;
	for(i = 0; i < 16; i++)
	{
		list->nodeMemory[i].memoryIndex = i;
		list->nodeMemory[i].data = (unsigned char*) elementMemory + (i * elementSize);
		list->nodeMemory[i].next = 0;
		list->nodeMemory[i].prev = 0;
	}
}

LinkedListNode* LinkedListNewNode(LinkedList_16Element* list)
{
	if(list == 0)
		return 0;

	uint16_t y;
	uint8_t bz, b3, b2, b1, b0, result;

	// Gaudet's algorithm
	y = isolate_rightmost_zero(list->memoryBitmap);
	bz = y ? 0 : 1;
	b3 = (y & 0x00FF) ? 0 : 8;
	b2 = (y & 0x0F0F) ? 0 : 4;
	b1 = (y & 0x3333) ? 0 : 2;
	b0 = (y & 0x5555) ? 0 : 1;
	result = bz + b3 + b2 + b1 + b0;

	if(result == 16)	// No free memory blocks
		return 0;

	bit_set(list->memoryBitmap, result);
	return (void*) &list->nodeMemory[result];
}

void LinkedListFreeNode(LinkedList_16Element* list, LinkedListNode* node)
{
	if(list == 0 || node == 0)
		return;

	node->next = 0;
	node->prev = 0;
	bit_clear(list->memoryBitmap, node->memoryIndex);
}

// MANIPULATION FUNCTIONS------------------------------------------------------

void LinkedListInsert(LinkedList_16Element* list, LinkedListNode* node, void* data, bool insertBefore)
{
	if(list == 0)
		return;

	// Get next unallocated node
	LinkedListNode* newNode = LinkedListNewNode(list);
	if(newNode == 0)
		return;

	// Copy data
	uint8_t i;
	for(i = 0; i < list->elementSize; i++)
	{
		*((unsigned char*) newNode->data + i) = *((unsigned char*) data + i);
	}

	// If the list is empty, set first/last node pointers to the inserted node
	if(list->first == 0)
	{
		list->first = newNode;
		list->last = newNode;
	}

	// If the specified target node does not exist (null reference), there is nothing left to do.
	if(node == 0)
		return;

	// Update node pointers
	if(insertBefore)
	{
		newNode->next = node;
		newNode->prev = node->prev;
		node->prev->next = newNode;
		node->prev = newNode;
		if(newNode->prev == 0)
			list->first = newNode;
	}
	else
	{
		newNode->next = node->next;
		newNode->prev = node;
		node->next->prev = newNode;
		node->next = newNode;
		if(newNode->next == 0)
			list->last = newNode;
	}
}

void LinkedListReplace(LinkedList_16Element* list, LinkedListNode* node, void* data)
{
	if(node == 0 || list == 0)
		return;

	// Save a local copy of the target node, then delete it
	LinkedListNode oldNode = *node;
	LinkedListFreeNode(list, node);

	// Get next unallocated node
	LinkedListNode* newNode = LinkedListNewNode(list);
	if(newNode == 0)
		return;

	// Copy data
	uint8_t i;
	for(i = 0; i < list->elementSize; i++)
	{
		*((unsigned char*) newNode->data + i) = *((unsigned char*) data + i);
	}

	// Update node pointers
	newNode->next = oldNode.next;
	oldNode.next->prev = newNode;
	newNode->prev = oldNode.prev;
	oldNode.prev->next = newNode;
	if(newNode->prev == 0)
		list->first = newNode;
	if(newNode->next == 0)
		list->last = newNode;
}

void LinkedListRemove(LinkedList_16Element* list, LinkedListNode* node)
{
	if(node == 0 || list == 0)
		return;

	// Update first/last node pointers
	if(list->first == node)
		list->first = node->next;
	if(list->last == node)
		list->last = node->prev;

	// Update node pointers
	node->next->prev = node->prev;
	node->prev->next = node->next;

	// Delete target node
	LinkedListFreeNode(list, node);
}

// SEARCH FUNCTIONS------------------------------------------------------------

LinkedListNode* LinkedListFindFirst(LinkedList_16Element* list, void* data)
{
	if(list == 0)
		return 0;

	LinkedListNode* result = list->first;
	while(result)
	{
		uint8_t i = 0;
		while(i < list->elementSize &&
			*((unsigned char*) result->data + i) == *((unsigned char*) data + i))
		{
			i++;
		}

		if(i == list->elementSize)
			return result;
		result = result->next;
	}
	return 0;
}

LinkedListNode* LinkedListFindLast(LinkedList_16Element* list, void* data)
{
	if(list == 0)
		return 0;

	LinkedListNode* result = list->last;
	while(result)
	{
		uint8_t i = 0;
		while(i < list->elementSize &&
			*((unsigned char*) result->data + i) == *((unsigned char*) data + i))
		{
			i++;
		}

		if(i == list->elementSize)
			return result;
		result = result->prev;
	}
	return 0;
}