#include "list2.h"

#include <assert.h>

static List Lists[LIST_MAX_NUM_HEADS];
static Node Nodes[LIST_MAX_NUM_NODES];

static int numHeads = 0;
static int numNodes = 0;

static List freeList = {0};

static void FreeList_create() {
	// Create the free list
	freeList.head = &Nodes[0];
	freeList.tail = &Nodes[LIST_MAX_NUM_NODES - 1];
	freeList.numItems = LIST_MAX_NUM_NODES;
	freeList.OOB = LIST_OOB_START;
	freeList.currentItem = NULL;

	// Add all the nodes to the free list
	for (int i = 0; i < LIST_MAX_NUM_NODES - 1; i++) {
		Nodes[i].next = &Nodes[i + 1];
	}
	Nodes[LIST_MAX_NUM_NODES - 1].next = NULL;
}
static Node* FreeList_malloc() {
	if (freeList.numItems == 0) return NULL;

	Node* newNode = freeList.head;
	freeList.head = freeList.head->next;
	freeList.numItems--;

	return newNode;
}
static void FreeList_free(Node* node) {
	if (node == NULL) return;

	// Basically just add the node to the front of the free list
	node->next = freeList.head;
	node->prev = NULL;
	node->item = NULL;
	freeList.head = node;
	freeList.numItems++;
}
static void RemoveList(List** ppList) {
	if (*ppList == NULL || numHeads == 0) return;
	if (*ppList != &Lists[numHeads - 1]) {
		// Swap the target list with the last list
		List* temp = *ppList;
		**ppList = Lists[numHeads - 1];
		Lists[numHeads - 1] = *temp;
	}
	// Decrement the number of heads
	numHeads--;
	*ppList = NULL;
}

List* List_create() {
	if (numHeads == 0 && numNodes == 0) {
		FreeList_create();

	} else if (numHeads == LIST_MAX_NUM_HEADS) {
		return NULL;
	}

	Lists[numHeads].head = NULL;
	Lists[numHeads].tail = NULL;
	Lists[numHeads].numItems = 0;
	Lists[numHeads].OOB = LIST_OOB_START;
	Lists[numHeads].currentItem = NULL;

	return &Lists[numHeads++];
}
int List_count(List* pList) {
	assert(pList != NULL);
	return pList->numItems;
}
void* List_first(List* pList) {
	assert(pList != NULL);

	if (pList->head == NULL) {
		pList->currentItem = NULL;
		return NULL;
	}
	pList->currentItem = pList->head;
	return pList->head->item;
}
void* List_last(List* pList) {
	assert(pList != NULL);

	if (pList->tail == NULL) {
		pList->currentItem = NULL;
		return NULL;
	}
	pList->currentItem = pList->tail;
	return pList->tail->item;
}
void* List_next(List* pList) {
	assert(pList != NULL);

	if (pList->currentItem == NULL) {
		if (pList->OOB == LIST_OOB_START) {
			pList->currentItem = pList->head;
			if (pList->head == NULL) return NULL;
			return pList->head->item;
		}
		return NULL;
	} else {
		if (pList->currentItem == pList->tail) {
			pList->currentItem = NULL;
			pList->OOB = LIST_OOB_END;
			return NULL;
		}

		pList->currentItem = pList->currentItem->next;
		return pList->currentItem->item;
	}
}
void* List_prev(List* pList) {
	assert(pList != NULL);

	if (pList->currentItem == NULL) {
		if (pList->OOB == LIST_OOB_END) {
			pList->currentItem = pList->tail;
			if (pList->tail == NULL) return NULL;
			return pList->tail->item;
		}
		return NULL;
	} else {
		if (pList->currentItem == pList->head) {
			pList->currentItem = NULL;
			pList->OOB = LIST_OOB_START;
			return NULL;
		}

		pList->currentItem = pList->currentItem->prev;
		return pList->currentItem->item;
	}
}
void* List_curr(List* pList) {
	if (pList->currentItem == NULL) {
		return NULL;
	}
	return pList->currentItem->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item.
// If the current pointer is before the start of the pList, the item is added at the start. If
// the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem) {
	assert(pList != NULL);

	Node* newNode = FreeList_malloc();
	if (newNode == NULL) {
		return LIST_FAIL;
	}

	if (pList->currentItem == NULL) {
		if (pList->OOB == LIST_OOB_START) {
			// Make a new head of the pList
			newNode->next = pList->head;
			newNode->prev = NULL;
			if (pList->head != NULL) {
				pList->head->prev = newNode;
			} else {
				pList->tail = newNode;
			}

			pList->head = newNode;
		} else {
			// Make a new tail of the pList
			newNode->next = NULL;
			newNode->prev = pList->tail;

			if (pList->tail != NULL) {
				pList->tail->next = newNode;
			} else {
				pList->head = newNode;
			}

			pList->tail = newNode;
		}
	} else {
		Node* before = pList->currentItem;
		Node* after = pList->currentItem->next;

		newNode->next = after;
		newNode->prev = before;

		// Edge cases to handle the ends of the list

		// Inserting when just 1 item in list
		// before and after are NULL
		if (before != NULL) before->next = newNode;
		//		else pList->head = newNode;

		if (after != NULL)
			after->prev = newNode;
		else
			pList->tail = newNode;
	}

	newNode->item = pItem;
	pList->currentItem = newNode;
	pList->numItems++;
	numNodes++;
	return LIST_SUCCESS;
}
int List_insert_before(List* pList, void* pItem) {
	List_prev(pList);
	return List_insert_after(pList, pItem);
}
int List_append(List* pList, void* pItem) {
	List_last(pList);
	return List_insert_after(pList, pItem);
}
int List_prepend(List* pList, void* pItem) {
	List_first(pList);
	return List_insert_before(pList, pItem);
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
	assert(pList != NULL);

	if (pList->currentItem == NULL) {
		return NULL;
	} else {
		Node* before = pList->currentItem->prev;
		Node* after = pList->currentItem->next;

		if (before != NULL)
			before->next = after;
		else
			pList->head = after;

		if (after != NULL) {
			after->prev = before;
		} else {
			pList->tail = before;
			pList->OOB = LIST_OOB_END;
		}

		void* item = List_curr(pList);
		FreeList_free(pList->currentItem);

		pList->currentItem->item = NULL;
		pList->currentItem->next = NULL;
		pList->currentItem->prev = NULL;

		pList->currentItem = after;
		pList->numItems--;

		numNodes--;
		return item;
	}
}
void* List_trim(List* pList) {
	List_last(pList);
	return List_remove(pList);
}
void List_concat(List* pList1, List* pList2) {
	if (List_count(pList2) == 0) {
		// do nothing
	} else if (List_count(pList1) == 0) {
		pList1->head = pList2->head;
		pList1->tail = pList2->tail;
		pList1->numItems = pList2->numItems;
	} else {
		pList1->tail->next = pList2->head;
		pList1->tail->next->prev = pList1->tail;

		pList1->tail = pList2->tail;
		pList1->numItems += pList2->numItems;
	}

	RemoveList(&pList2);
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
	assert(pList != NULL);

	List_first(pList);
	while (List_count(pList) != 0) {
		(*pItemFreeFn)(List_curr(pList));
		List_remove(pList);
	}
	RemoveList(&pList);
}

// Search pList, starting at the current item, until the end is reached or a match is found.
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match,
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator.
//
// If a match is found, the current pointer is left at the matched item and the pointer to
// that item is returned. If no match is found, the current pointer is left beyond the end of
// the list and a NULL pointer is returned.
//
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
	List_first(pList);
	while (List_curr(pList) != NULL) {
		if ((*pComparator)(List_curr(pList), pComparisonArg) == 1) {
			return List_curr(pList);
		}
		List_next(pList);
	}
	return NULL;
}

void print() {
	printf("\nnumHeads: %d\n", numHeads);
	printf("[\n");

	for (int i = 0; i < numHeads; i++) {
		List* pList = &Lists[i];
		int elements = pList->numItems;

		if (pList->head != NULL) {
			printf("\thead: %s", (char*)pList->head->item);
			printf("\ttail: %s", (char*)pList->tail->item);
		}
		printf("\tsize: %d\n", elements);
		printf("\t[ ");

		Node* curr = pList->head;
		if (curr == NULL) {
			printf(" ]\n\n\n");
			continue;
		}
		for (int j = 0; j < elements; j++) {
			printf("%s", (char*)curr->item);
			if (j == elements - 1) {
				printf(" ]\n\n\n");
			} else {
				printf(" -> ");
			}

			curr = curr->next;
		}
	}

	printf("]\n");
}
void printAllHeads() {
	printf("\nnumHeads: %d\n", numHeads);
	printf("[\n");

	for (int i = 0; i < numHeads; i++) {
		List head = Lists[i];

		printf("\t%*d:", 4, i);
		printf("\t -> [%s]\n", (char*)head.head->item);
	}

	printf("]\n");
}
void printAllNodes() {
	printf("\nnumNodes: %d\n", numNodes);
	printf("[\n");

	for (int i = 0; i < numNodes; i++) {
		Node node = Nodes[i];

		printf("\titem: %s", (char*)node.item);
		if (node.next == NULL) {
			printf("\tnext: NULL");
		} else {
			printf("\tnext: %*p", 4, node.next);
		}
		if (node.prev == NULL) {
			printf("\tprev: NULL\n");
		} else {
			printf("\tprev: %*p\n", 4, node.prev);
		}
	}

	printf("]\n");
}