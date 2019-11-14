#ifndef LIST1A_H_INCLUDED
#define LIST1A_H_INCLUDED

#include <stdbool.h>
#include "structPCB.h"

  //typedef int ListInfo;
  typedef PCB ListInfo;

    typedef struct NodeType
    {
      ListInfo info;
      struct NodeType* next;
      struct NodeType* prev;
    }NodeType;

    typedef NodeType *NodePtr;  //a NodePtr is a ptr to a struct NodeType
    typedef NodePtr List;       //a List is a ptr to a struct NodeType (NodePointer to struct NodeType)

    //initialize the list with the header pointing at itself
     void initializeList(List *listPtr);

     //return true if the only element on the list is the header
     //otherwise return false
     bool isEmpty(const List list);

     //Remover all elements including the header from the list
     //set the List to null
     void clearList(List listPtr);

     //Remover all elements including the header from the list
     //Leave the list in an empty state.
     void freeList(List *listPtr);

     //Add the value to the front of the list
     void pushFront(const List list, ListInfo value);

     //Add the value to the back of the list
     void pushBack(const List list, ListInfo value);

     //Add the value to the list at specified index
     void push(const List list, int pos, ListInfo value);

     //Remove and return the value at the front of the list
     ListInfo popFront(const List list);

     //Remove and return the value at the back of the list
     ListInfo popBack(const List list);

     //Remove value from list at specified index
     ListInfo pop(const List list, int pos);

     //Return true if the value appears in the list,
     //otherwise return false
     bool findInList(const List list, int value);

     //Return the size of the list
     int sizeOfList(const List list);

     //Return the size of the list
     int sizeOfFreeSpaceList(const List list);

     //Return the value at the front of the list
     ListInfo getFront(const List list);

     //Return the value at the back of the list
     ListInfo getBack(const List list);

     //Return the value at the specified index of the list
     List getElement(const List list, int pos);


#endif
