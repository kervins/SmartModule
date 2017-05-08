/**@file		linked_list.h
 * @brief		Header file which defines linked lists for local and external use
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		April 03, 2017
 * @copyright	GNU Public License
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

// TYPE DEFINITIONS------------------------------------------------------------

/**@struct LinkedListNode
 * Defines a doubly linked list node
 */
typedef struct LinkedListNode
{
	void* data;						/**< Pointer to data */
	unsigned char memoryIndex;		/**< Internal use - DO NOT MODIFY */
	struct LinkedListNode* next;	/**< Pointer to the next node in the list */
	struct LinkedListNode* prev;	/**< Pointer to the previous node in the list */
} LinkedListNode;

/**@struct LinkedList_16Element
 * Defines a 16-element doubly linked list which maps to external SRAM
 */
typedef struct LinkedList_16Element
{
	LinkedListNode nodeMemory[16];	/**< An array of 16 <b>LinkedListNode</b>s */
	unsigned int memoryBitmap;		/**< Bitmap which tracks free nodes */
	unsigned char elementSize;		/**< Defines the size (in bytes) of each value */
	char* elementMemoryBaseAddr;	/**< Defines the SRAM base address at which the data are located */
	LinkedListNode* first;			/**< Pointer to the first node in the list */
	LinkedListNode* last;			/**< Pointer to the last node in the list */
} LinkedList_16Element;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Management Functions
void LinkedList_16Element_Initialize(LinkedList_16Element*, void*, unsigned char);
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