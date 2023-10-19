#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>


static bool initilaized = false;



static Node nodePool[LIST_MAX_NUM_NODES];

static List listPool[LIST_MAX_NUM_HEADS];


//static int freeNodeIndex = 0;
//static int freeListIndex = 0;

static Node* freeNode = NULL;  
static List* freeListHead = NULL;

void initilaizePool(){
    int i = 0;
    if(LIST_MAX_NUM_HEADS > 0){
        for (i = 0; i < LIST_MAX_NUM_HEADS-1; i++) {
            listPool[i].current = NULL;
            listPool[i].head = NULL;
            listPool[i].tail = NULL;
            listPool[i].state = LIST_OOB_START;
            listPool[i].nextFree = &listPool[i+1];
        }
        listPool[i].current = NULL;
        listPool[i].head = NULL;
        listPool[i].tail = NULL;
        listPool[i].state = LIST_OOB_START;
        listPool[i].nextFree = NULL;
        freeListHead = &listPool[0];
        initilaized = true;
    }

    if(LIST_MAX_NUM_NODES > 0){

    
        for (i = 0; i < LIST_MAX_NUM_NODES-1; i++) {
            nodePool[i].item = NULL;
            nodePool[i].next = NULL;
            nodePool[i].prev = NULL;
            nodePool[i].nextNode = &nodePool[i+1];
        }
        nodePool[i].item = NULL;
        nodePool[i].next = NULL;
        nodePool[i].prev = NULL;
        nodePool[i].nextNode = NULL;
        freeNode = &nodePool[0];
    }
    
}


Node* getFreeNode() {
    if (!freeNode) return NULL;

    Node* newNode = freeNode;
    newNode->item = NULL;
    newNode->next = NULL;
    newNode->prev = NULL;
    freeNode = freeNode->nextNode;  

    return newNode;
}


List* getFreeList() {
    if (!freeListHead) return NULL;

    List* newList = freeListHead;
    freeListHead = freeListHead->nextFree;  

      
    return newList;
}

List* List_create() {
    if(initilaized == false){
        initilaizePool();
    }

    List* newList = getFreeList();
    
    if (!newList) 
        return NULL;
    
    return newList;
    
}

int List_count(List* pList) {
    assert(pList != NULL);
    return pList->count;
}

void* List_last(List* pList) {
    assert(pList != NULL);
    if (!pList->tail) {
        pList->current = NULL;
        return NULL;
    }

    pList->current = pList->tail;   
    return pList->current->item;
}

void* List_first(List* pList) {
    assert(pList != NULL);
    
    if (pList->head == NULL) {
        pList->current = NULL;
        return NULL;
    }
    pList->current = pList->head;
    return pList->current->item;
}

void* List_next(List* pList) {
    assert(pList != NULL);
    if (pList->current == pList->tail) {
        pList->state = LIST_OOB_END;
        pList->current = NULL;
        return NULL;
    }else if(pList->current == NULL && pList->state == LIST_OOB_END){
        return NULL;
    }else if(pList->current == NULL && pList->state == LIST_OOB_START){
        pList->current = pList->head;
        return pList->current->item;
    }

    pList->current = pList->current->next;
    return pList->current->item;
}

void* List_prev(List* pList){
    assert(pList != NULL);
    if(pList->current == pList->head){
        pList->state = LIST_OOB_START;
        pList->current = NULL;
        return NULL;
    }else if(pList->current == NULL && pList->state == LIST_OOB_START){
        return NULL;
    }else if(pList->current == NULL && pList->state == LIST_OOB_END){
        pList->current = pList->tail;
        return pList->current->item;
    }
    pList->current = pList->current->prev;
    return pList->current->item;
}

void* List_curr(List* pList){
    assert(pList != NULL);
    if(pList->current == NULL)
    	return NULL;
    return pList->current->item;
}


int List_append(List* pList, void* pItem) {
    assert(pList != NULL);

    Node* newNode = getFreeNode();
    if (!newNode)
        return LIST_FAIL; 

    newNode->item = pItem;
    newNode->next = NULL;
    newNode->prev = pList->tail;

    if (!pList->head) { 
        pList->head = newNode;
        pList->current = newNode;
        pList->tail= newNode;
    } else { 
        pList->tail->next = newNode;
        
        pList->tail = newNode;
        
    }
    pList->current = newNode;
    pList->count++;
    return LIST_SUCCESS;
}

int List_prepend(List* pList, void* pItem){
    assert(pList != NULL);

    Node* newNode = getFreeNode();

    if(!newNode)
        return LIST_FAIL;

    newNode->item = pItem;
    newNode->next = pList->head;
    newNode->prev = NULL;

    if(!pList->head){
         pList->head = newNode;
         pList->current = newNode;
         pList->tail= newNode;
    }else{
        pList->head->prev = newNode;
        pList->head = newNode;
    }

    pList->current = newNode;
    pList->count++;
    return LIST_SUCCESS;
}

int List_insert_after(List* pList, void* pItem){
    assert(pList != NULL);
    if(pList->count == 0){
        return List_append(pList, pItem);
    }
    else if(pList->current == NULL && pList->state == LIST_OOB_START){
        return List_prepend(pList,pItem);
    }
    else if( (pList->current == NULL && pList->state == LIST_OOB_END) || pList->current == pList->tail){
        return List_append(pList,pItem);
    }
    else {
        Node* newNode = getFreeNode();

        if(!newNode)
            return LIST_FAIL;

        newNode->item = pItem;
        newNode->next = NULL;
        newNode->prev = NULL;

        
        pList->current->next->prev = newNode;
        newNode->next = pList->current->next;
        newNode->prev = pList->current;
        pList->current->next = newNode;
        pList->current = newNode;
        pList->count++;
        return LIST_SUCCESS;
        
    }

}

int List_insert_before(List* pList, void* pItem){
    assert(pList != NULL);
    if(pList->count == 0){
        return List_append(pList,pItem);
    }
    else if( (pList->current == NULL && pList->state == LIST_OOB_START) || pList->current == pList->head){
        return List_prepend(pList,pItem);
    }
    else if( (pList->current == NULL && pList->state == LIST_OOB_END)){
        return List_append(pList, pItem);
    }
    else{
         Node* newNode = getFreeNode();

        if(!newNode)
            return LIST_FAIL;

        newNode->item = pItem;
        newNode->next = NULL;
        newNode->prev = NULL;

        pList->current->prev->next = newNode;
        newNode->next = pList->current;
        newNode->prev = pList->current->prev;
        pList->current->prev = newNode;
        pList->current = newNode;
        pList->count++;
        return LIST_SUCCESS;
    }
}

void returnNode(Node* node) {
    if (!node) return;

    node->item = NULL;
    node->next = NULL;
    node->prev = NULL;
    node->nextNode = freeNode;
    freeNode = node;
    
}


void* List_remove(List* pList){
    assert(pList != NULL);

    if(pList->current == NULL){
        
        return NULL;  
    }else{
        Node* to_remove = pList->current;
        void* currentItem = pList->current->item;

        if(pList->count == 1 ){
            
            pList->head = NULL;
            pList->tail = NULL;
            pList->current = NULL;
            pList->state = LIST_OOB_END;

        }else if(to_remove == pList->head){
            
            pList->head = to_remove->next;
            pList->head->prev = NULL;
            pList->current = pList->head;            
        }
        else if(to_remove == pList->tail){
            
            pList->tail = to_remove->prev;
            pList->tail->next = NULL;
            pList->current = NULL;
            pList->state = LIST_OOB_END;
        }
        else{
            
            to_remove->prev->next = to_remove->next;
            to_remove->next->prev = to_remove->prev;
            pList->current = to_remove->next;
        }

       pList->count--;

       returnNode(to_remove);  
       return currentItem; 


    }
}

void* List_trim(List* pList){
    assert(pList != NULL);
    if(pList->count == 0){
        return NULL;
    }
    List_last(pList);
    void* lastItem = List_remove(pList);
    List_last(pList);
    return lastItem;
}

void returnList(List* list){
    if (!list) return;

    
    list->head = NULL;
    list->tail = NULL;
    list->current = NULL;
    list->state = LIST_OOB_START;
    list->count = 0;
    
    list->nextFree = freeListHead;
    freeListHead = list;
}


void List_concat(List* pList1, List* pList2){
    assert(pList1 != NULL);
    assert(pList2 != NULL);

    if(pList1->head == NULL && pList2->head != NULL){
        pList1->head = pList2->head;
        pList1->tail = pList2->tail;
    }
    else if(pList1->tail != NULL && pList2->head != NULL){
        pList1->tail->next = pList2->head;
        pList2->head->prev = pList1->tail;
        pList1->tail = pList2->tail;
    }
    pList1->count += pList2->count;
    pList1->current = pList1->current;
    returnList(pList2);
    pList2->head = NULL;
    pList2->tail = NULL;
    pList2->current = NULL;
    pList2->count = 0;
}

void List_free(List* pList, FREE_FN pItemFreeFn){
    assert(pList != NULL);
    Node* traversal = pList->head;
    while (traversal) {
        Node* next = traversal->next;

        
        if (traversal->item) {
            pItemFreeFn(traversal->item);
        }

        
        returnNode(traversal);

        traversal = next;
    }

   
    returnList(pList);
}

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    assert(pList != NULL);
    if (!pList->current && pList->state == LIST_OOB_START) {
        pList->current = pList->head;
    }
    while (pList->current) {
        
        if (pComparator(pList->current->item, pComparisonArg)) {
            return pList->current->item;
        }
        pList->current = pList->current->next;
    }
    pList->state = LIST_OOB_END;
    
    return NULL;

}
