#include "List1A.h"
#include "structPCB.h"
#include <stdlib.h>
#include <stdio.h>

//Documentation: If anyone wishes to use popFront(), popBack(), getFront(), or getBack(), they must first call isEmpty() and receive a boolean true

//initialize the list with the header pointing at itself
void initializeList(List *listPtr)
{
    *listPtr = (List)malloc(sizeof(NodeType));
    (*listPtr)->next = *listPtr;
    (*listPtr)->prev = *listPtr;
}

//return true if the only element on the list is the header
//otherwise return false
bool isEmpty(const List list)
{
    if((list->next == list) && (list->prev == list))
        return true;
    else
        return false;
}

//Remove all elements including the header from the list
//set the List to null
void clearList(List listPtr)
{
    List *Ptr = &listPtr;
    if(!(isEmpty(listPtr)))
        freeList(Ptr);
    if(isEmpty(listPtr) && (listPtr != NULL))
    {
        listPtr = NULL;
        free(listPtr);
    }
}

//Remove all elements except the header from the list
//Leave the list in an empty state.
void freeList(List *listPtr)
{
    if(!isEmpty((*listPtr)))
    {
        while((*listPtr)->next != (*listPtr)->prev)
            popFront(*listPtr);
        popFront(*listPtr);
    }
}

//Add the value to the front of the list
void pushFront(const List list, ListInfo value)
{
    List newNode = (List)malloc(sizeof(NodeType));
    newNode->info = value;
    newNode->next = list->next;
    newNode->prev = list;
    list->next = newNode;
    newNode->next->prev = newNode;
}

 //Add the value to the back of the list
 void pushBack(const List list, ListInfo value)
 {
     if(isEmpty(list))
    {
        pushFront(list, value);
    }
    else
    {
        List newNode = (List)malloc(sizeof(NodeType));
        newNode->info = value;
        newNode->next = list;
        newNode->prev = list->prev;
        newNode->prev->next = newNode;
        newNode->next->prev = newNode;
    }
 }

//must use sizeOfList first to ensure position exists
//Add the value to the list at specified index
void push(const List list, int pos, ListInfo value)
 {
    if(isEmpty(list))
    {
        pushFront(list, value);
    }
    else
    {
        List newNode = (List)malloc(sizeof(NodeType));
        newNode->info = value;
        List temp = getElement(list, pos);
        newNode->next = temp;
        newNode->prev = temp->prev;
        newNode->prev->next = newNode;
        newNode->next->prev = newNode;
        temp = NULL;
    }
 }

//Remove and return the value at the front of the list
ListInfo popFront(const List list)
{
    ListInfo val = list->next->info;
    List temp = list->next;
    list->next = list->next->next;
    list->next->prev = list;
    free(temp);
    return val;
}

//Remove and return the value at the back of the list
ListInfo popBack(const List list)
{
    ListInfo val = list->prev->info;
    List temp = list->prev;
    list->prev = list->prev->prev;
    list->prev->next = list;
    free(temp);
    return val;
}

//must use sizeOfList first to ensure position exists
//Remove value from list at specified index
ListInfo pop(const List list, int pos)
{
    List temp = getElement(list, pos);
    ListInfo val = temp->info;
    temp->next->prev = temp->prev;
    temp->prev->next = temp->next;
    free(temp);
    return val;
}

 //Return true if the value appears in the list,
 //otherwise return false

 //bool findInList(const List list, ListInfo value)
 bool findInList(const List list, int value)
 {
     List temp = list->next;
     while(temp != list)
     {
         //if(temp->info == value)
         if(temp->info.pid == value)
         {
             temp = NULL;
             return true;
         }
         temp = temp->next;
     }
     temp = NULL;
     return false;
 }

  //Return the size of the list
 int sizeOfList(const List list)
 {
     List temp = list->next;
     int count = 0;
     while(temp != list)
     {
         //if(temp->info == value)
         if(temp->info.pid != NULL)
            count++;
         temp = temp->next;
     }
     temp = NULL;
     return count;
 }

//Return the size of the list
int sizeOfFreeSpaceList(const List list)
{
     List temp = list->next;
     int count = 0;
     while(temp != list)
     {
         //if(temp->info == value)
         if(((temp->info.SA) || (temp->info.RS)) != NULL)
            count++;
         temp = temp->next;
     }
     temp = NULL;
     return count;
}

//Return the value at the front of the list
ListInfo getFront(const List list)
{
    return list->next->info;
}

//Return the value at the back of the list
ListInfo getBack(const List list)
{
    return list->prev->info;
}

//must use sizeOfList first to ensure position exists
//Return the value at the specified index of the list
List getElement(const List list, int pos)
 {
     List temp = list->next;
     int count = 0;
     while(temp != list)
     {
         if(count == pos)
            return temp;
         count++;
         temp = temp->next;
     }
     temp = NULL;
 }
