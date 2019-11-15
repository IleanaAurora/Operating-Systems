/*#include "List1A.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    List L1;
    printf("Initializing list...");
    initializeList(&L1);
    printf("the list is initialized.\n");
    printf("Is the list empty? ");
    if(isEmpty(L1))
        printf("The list is empty.\n");
    pushFront(L1, 8);
    pushFront(L1, 9);
    printf("I'm front pushing an 8 followed by a 9. ");
    if(isEmpty(L1))
        printf("The list is empty.\n");
    else
        printf("The list is not empty.\n");
    printf("At the front is %d and ", getFront(L1));
    printf("now %d has been popped off. \n", popFront(L1));
    if(isEmpty(L1))
        printf("The list is empty.\n");
    else
        printf("The list is not empty.\n");
    printf("At the front is %d. \n", getFront(L1));
    pushFront(L1, 7);
    printf("After adding a 7 to the front of the list, the front is %d. \n", getFront(L1));
    printf("At the back is %d ", getBack(L1));
    printf("and now %d has been popped off the back.\n", popBack(L1));
    printf("At the back is now %d ", getBack(L1));
    pushBack(L1, 10);
    printf("but after adding 10 to the back, the back is now %d ", getBack(L1));
    printf("unless I pop %d off right now.\n", popBack(L1));
    printf("At the back is again %d ", getBack(L1));
    printf("but I just popped that same %d off from the front. \n", popFront(L1));
    printf("Now at the front is %d. ", getFront(L1));
    if(isEmpty(L1))
        printf("The list is empty.\n");
    else
        printf("The list is not empty.\n");
    printf("I'm pushing 4 to the front and 5 and 6 to the back of the list so that now ");
    pushFront(L1, 4);
    pushBack(L1, 5);
    pushBack(L1, 6);
    if(isEmpty(L1))
        printf("the list is empty.\n");
    else
        printf("the list is not empty.\n");
    freeList(&L1);
    printf("I've freed the list so that only the header remains, meaning ");
    if(isEmpty(L1))
        printf("the list is empty.\n");
    else
        printf("the list is not empty.\n");
    printf("I'm pushing 1 to the front and 2 and 3 to the back of the list. ");
    pushFront(L1, 1);
    pushBack(L1, 2);
    pushBack(L1, 3);
    if(findInList(L1,3))
        printf("The value of 3 is in the list. ");
    else
        printf("The value of 3 is not in the list. ");
    if(findInList(L1,4))
        printf("The value of 4 is in the list. \n");
    else
        printf("The value of 4 is not in the list. \n");
    if(isEmpty(L1))
        printf("The list is empty.\n");
    else
        printf("The list is not empty.\n");
    clearList(L1);
    printf("I've cleared the list so that nothing (neither the list nor the header) remains, meaning ");
    if(isEmpty(L1))
        printf("the list is empty ");
    else
        printf("the list is not empty ");
    printf("and all resources should be deallocated. \n");
    return 0;
}*/
