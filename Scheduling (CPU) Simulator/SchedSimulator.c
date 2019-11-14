#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <unistd.h>
#include <windows.h>
#include "List1A.h"
#include "structPCB.h"

//define N number of threads, N is 2 (one to create PCB list and one to ensure I can create multiple threads)
#define NUM_THREADS 2

//declare the PCBlist as a struct with a mutex and 2 condition variables because it is a shared resource
typedef struct Mutex
{
   List list;
   int counter;
   pthread_mutex_t lock;
   pthread_cond_t emptied, filled;
} Mutex;

void deallocateMem(PCB process);

//PCBlist, normalReadyQ, and realTimeReadyQ are mutexes as they are shared resources
Mutex PCBlist;
Mutex waitList;
Mutex freeSpace;
Mutex normalReadyQ;
Mutex realTimeReadyQ;
Mutex finishedQ;
List CPU;

bool PCBlistFinished = false;
bool FinishedListFull = false;
int numOfProcesses = 0;
int timeQuantum;
int maxMemUnits;

void *fileReader();
void *scheduler(void *threadArg);
void *processManager(void *threadArg);
void *clocker(void *threadArg);

/*
MAIN
initialize the PCB list & ready queue's mutex, emptied, & filled condition variables
create thread 1, thread 2
join thread 1, thread 2
free the PCB list and ready queue list and destroy the mutex, emptied, & filled condition variables
*/
int main()
{
    atomic_int TheClock = 0;
    initializeList(&PCBlist.list);
    initializeList(&freeSpace.list);
    initializeList(&waitList.list);
    initializeList(&normalReadyQ.list);
    initializeList(&realTimeReadyQ.list);
    initializeList(&CPU);
    initializeList(&finishedQ.list);
    pthread_mutex_init(&PCBlist.lock, NULL);
    pthread_cond_init(&PCBlist.filled, NULL);
    pthread_mutex_init(&normalReadyQ.lock, NULL);
    pthread_mutex_init(&realTimeReadyQ.lock, NULL);
    pthread_mutex_init(&finishedQ.lock, NULL);
    pthread_mutex_init(&finishedQ.filled, NULL);
    pthread_t fileReaderThread;
    pthread_t schedulerThread;
    pthread_t CPUthread;
    pthread_t clockThread;
    void *status;
    FILE *fp;

    printf("Enter a time quantum (the total amount of time for the processes to spend on the CPU at once): ");
    scanf("%d", &timeQuantum);
    printf("\n");

    printf("How many memory units would you like to simulate? ");
    scanf("%d", &maxMemUnits);
    printf("\n");
    printf("Running simulation...\n");

    int rc = pthread_create(&fileReaderThread, NULL, fileReader(), NULL);
	if(rc)
	{
		printf( "ERROR. Return code from thread %d\n", rc);
		exit(-1);
	}

    rc = pthread_create(&schedulerThread, NULL, scheduler, (void *)&TheClock);
	if(rc)
	{
		printf( "ERROR. Return code from thread %d\n", rc);
		exit(-1);
	}
	rc = pthread_create(&CPUthread, NULL, processManager, (void *)&TheClock);
	if(rc)
	{
		printf( "ERROR. Return code from thread %d\n", rc);
		exit(-1);
	}
	rc = pthread_create(&clockThread, NULL, clocker, (void *)&TheClock);
	if(rc)
	{
		printf( "ERROR. Return code from thread %d\n", rc);
		exit(-1);
	}
    rc = pthread_join(fileReaderThread, &status);
	if(rc)
	{
        printf( "ERROR. Return code from thread join %d\n", rc);
		exit(-1);
	}

    rc = pthread_join(schedulerThread, &status);
	if(rc)
	{
        printf( "ERROR. Return code from thread join %d\n", rc);
		exit(-1);
	}
	rc = pthread_join(CPUthread, &status);
	if(rc)
	{
        printf( "ERROR. Return code from thread join %d\n", rc);
		exit(-1);
	}
	rc = pthread_join(clockThread, &status);
	if(rc)
	{
        printf( "ERROR. Return code from thread join %d\n", rc);
		exit(-1);
	}

	int totTime = 0;
	double avgTime = 0;
	pthread_mutex_lock(&finishedQ.lock);
    List temp = finishedQ.list->next;
    int t;
    while(temp != finishedQ.list)
    {
        t = (temp->info.finishTime) - (temp->info.startTime) - (temp->info.ST);
        totTime += t;
        temp = temp->next;
    }
    avgTime = (double)totTime/(double)numOfProcesses;
    pthread_mutex_unlock(&finishedQ.lock);

    if ((fp = fopen("SchedSimulatorReport.txt", "w")) != NULL)
    {
        fprintf(fp,"PROCESS SUMMARY REPORT\n");
        temp = finishedQ.list->next;
        int ready;
        int wait;
        while(temp != finishedQ.list)
        {
            fprintf(fp,"----------------------------------------------------\n");
            fprintf(fp,"PROCESS %d\n", temp->info.pid);
            fprintf(fp,"Time finished: %d\n", temp->info.finishTime);
            ready = (temp->info.finishTime) - (temp->info.startTime) - (temp->info.ST);
            fprintf(fp,"Time spent waiting on the ready list: %d\n", ready);
            if(temp->info.placedOnWaitlist == 1)
                wait = (temp->info.timeTakenOffWaitlist) - (temp->info.timePlacedOnWaitlist);
            else
                wait = 0;
            fprintf(fp,"Time spent waiting on the wait list: %d\n", wait);
            fprintf(fp,"----------------------------------------------------\n");
            temp = temp->next;
        }
        int time = getBack(finishedQ.list).finishTime;
        fprintf(fp,"The total time spent waiting on the ready list was %d \n",totTime);
        fprintf(fp,"The average wait time spent waiting the ready list was %.2f \n", avgTime);
        fprintf(fp,"All processes finished running at time %d\n",time);
    }
    fclose (fp);
    temp = NULL;
    pthread_mutex_unlock(&finishedQ.lock);
    printf("Simulation complete. Report file created and saved as \"SchedSimulatorReport.txt\".\n");

    freeList(&PCBlist.list);
    freeList(&freeSpace.list);
    freeList(&waitList.list);
    freeList(&normalReadyQ.list);
    freeList(&realTimeReadyQ.list);
    freeList(&CPU);
    freeList(&finishedQ.list);
    pthread_mutex_destroy(&PCBlist.lock);
    pthread_cond_destroy(&PCBlist.filled);
    pthread_mutex_destroy(&normalReadyQ.lock);
    pthread_mutex_destroy(&realTimeReadyQ.lock);
    pthread_mutex_destroy(&finishedQ.lock);
    pthread_mutex_destroy(&finishedQ.filled);
    return 0;
}

/*
THREAD 1
opens input file
locks PCBlist mutex
for each line:
    create a PCB and add the 4 values to the PCB (initialization)
    the PCB is then added to the back of the PCBlist queue
signals that the list is full
unlocks the PCBlist mutex
print the contents of PCB list to ensure the list works
close the file
*/
void *fileReader()
{
    char line[50];
    FILE *fp;

    if ((fp = fopen("Simulation-data-2.txt", "r")) == NULL)
    {
        printf("The file could not be opened for reading.");
        exit(1);
    }
    //reads file by line until end of file
    char *element;
    pthread_mutex_lock(&PCBlist.lock);
    while (fgets(line, sizeof line, fp) != NULL )
    {
        PCB process;
        PCBlist.counter++;
        element = (char*)strtok (line," ");
        //for each item element on a line
        int count = 0;
        while (element != NULL)
          {
            //save the element 1 as pid, element 2 as AT, element 3 as ST, and element 4 as process Type
            if(count == 0)
                process.pid = atoi(element);
            if(count == 1)
                process.AT = atoi(element);
            if(count == 2)
                process.ST = atoi(element);
            if(count == 3)
                process.RS = atoi(element);
            if(count == 4)
                process.Type = element[0];
            count += 1;
            process.new = true;
            process.placedOnWaitlist = 0;
            process.runtime = process.ST;
            element = (char*)strtok (NULL, " ");
          }
        pushBack(PCBlist.list, process);
    }
    numOfProcesses = PCBlist.counter;
    pthread_cond_signal(&PCBlist.filled);
    pthread_mutex_unlock(&PCBlist.lock);
    PCBlistFinished = true;
    fclose(fp);
    return NULL;
}

/*
THREAD 2
declare current time variable
declare boolean added = false
lock the PCB list mutex
make freeSpace list entry where SA=0 and size=maxMemUnits
while the PCB list and the waitList aren't empty:
    get and save the current time from the clock
    if the getFront(PCBlist.list.AT) == currentTime or the waitList.list is not empty:
        added = false
        for each element in the freeSpace list:
            if the process' requested size (RS) < element size:
                process.SA = element SA and element SA += process.RS and element size -= RS and added = true
                if the PCB.list.Type is N:
                    lock the normal ready queue mutex, popFront() of the PCBlist.list, and push the PCB to the back of the normal ready queue
                    increment the normal ready queue counter and unlock the normal ready queue mutex
                if the PCB.list.Type is R:
                    lock the real time ready queue mutex, popFront() of the PCBlist.list, and push the PCB to the back of the real time ready queue
                    increment the real time ready queue counter and unlock the real time ready queue mutex
            if the process' RS == element size:
                process.SA = element SA and remove element and added = true
                if the PCB.list.Type is N:
                    lock the normal ready queue mutex, popFront() of the PCBlist.list, and push the PCB to the back of the normal ready queue
                    increment the normal ready queue counter and unlock the normal ready queue mutex
                if the PCB.list.Type is R:
                    lock the real time ready queue mutex, popFront() of the PCBlist.list, and push the PCB to the back of the real time ready queue
                    increment the real time ready queue counter and unlock the real time ready queue mutex
        if added == false:
            if the PCB.list.Type is N:
                add process to back of waitList
            else
                add process to front of waitList
unlock the PCB list mutex
*/
void *scheduler(void *threadArg)
{
    PCB process;
    process.SA = 0;
    process.RS = maxMemUnits;
    pthread_mutex_lock(&freeSpace.lock);
    pushFront(freeSpace.list, process);
    pthread_mutex_unlock(&freeSpace.lock);
    atomic_int *schedulerCurrentTime;
    bool added = false;
    int list;
    pthread_mutex_lock(&PCBlist.lock);
    while(!(isEmpty(PCBlist.list)) || !(isEmpty(waitList.list)))
    {
        //printf("The front of the PCBlist.list is %d\n", getFront(PCBlist.list).pid);
        if(!(isEmpty(waitList.list)))
        {
            pthread_mutex_lock(&waitList.lock);
            process = getFront(waitList.list);
            pthread_mutex_unlock(&waitList.lock);
            list = 0;
        }
        else if(!(isEmpty(PCBlist.list)))
        {
            process = getFront(PCBlist.list);
            list = 1;
        }
        schedulerCurrentTime = (atomic_int *)threadArg;
        if(process.AT <= (int)*schedulerCurrentTime)
        {
            added = false;
            pthread_mutex_lock(&freeSpace.lock);
            List temp = freeSpace.list->next;
            pthread_mutex_unlock(&freeSpace.lock);
            int size = 0;
            while(temp != freeSpace.list)
                {
                 size = temp->info.RS - process.RS;
                 if(((process.RS <= temp->info.RS) || (size >= 0)) && (temp->info.RS <= maxMemUnits) && (temp->info.RS >= 0))
                 {
                    if(list == 1)
                       {process = popFront(PCBlist.list);}
                    else if(list == 0)
                    {
                        pthread_mutex_lock(&waitList.lock);
                        process = popFront(waitList.list);
                        process.timeTakenOffWaitlist = (int)*schedulerCurrentTime;
                        pthread_mutex_unlock(&waitList.lock);
                    }
                    pthread_mutex_lock(&freeSpace.lock);
                    process.SA = temp->info.SA;
                    temp->info.SA += process.RS;
                    temp->info.RS -= process.RS;
                    pthread_mutex_unlock(&freeSpace.lock);
                    added = true;
                     if((process.Type) == 'N')
                        {
                            pthread_mutex_lock(&normalReadyQ.lock);
                            process.startTime = (int)*schedulerCurrentTime;
                            pushBack(normalReadyQ.list, process);
                            normalReadyQ.counter++;
                            pthread_mutex_unlock(&normalReadyQ.lock);
                        }
                     else if((process.Type) == 'R')
                        {
                            pthread_mutex_lock(&realTimeReadyQ.lock);
                            process.startTime = (int)*schedulerCurrentTime;
                            pushBack(realTimeReadyQ.list, process);
                            realTimeReadyQ.counter++;
                            pthread_mutex_unlock(&realTimeReadyQ.lock);
                        }
                 }
                 temp = temp->next;
                }
            temp = NULL;
            if((added == false) && !(findInList(waitList.list, process.pid)))
            {
                if(list == 1)
                       process = popFront(PCBlist.list);
                else if(list == 0)
                {
                    pthread_mutex_lock(&waitList.lock);
                    process = popFront(waitList.list);
                    process.timeTakenOffWaitlist = (int)*schedulerCurrentTime;
                    pthread_mutex_unlock(&waitList.lock);
                }
                if((process.Type) == 'N')
                {
                    pthread_mutex_lock(&waitList.lock);
                    process.placedOnWaitlist = 1;
                    process.timePlacedOnWaitlist = (int)*schedulerCurrentTime;
                    pushBack(waitList.list, process);
                    pthread_mutex_unlock(&waitList.lock);
                }
                else
                {
                    pthread_mutex_lock(&waitList.lock);
                    process.placedOnWaitlist = 1;
                    process.timePlacedOnWaitlist = (int)*schedulerCurrentTime;
                    pushFront(waitList.list, process);
                    pthread_mutex_unlock(&waitList.lock);
                }
            }
        }
        list = -1;
    }
    pthread_mutex_unlock(&PCBlist.lock);
    return NULL;
}

/*
THREAD 3
declare the current time variable and lock the finishedQ mutex
while the finished.counter is not equal to the numOfProcesses:
    if isEmpty(CPU):
        get and save the current time from the clock
        if the real time queue is not empty:
            popFront() of the realTimeReadyQ.list and pushFront in the CPU
            get and save the current time from the clock (make =)
                while ST > 0:
                    if current time is not equal to the clock:
                        decrement the ST and make the current time = the clock
                if ST == 0, get and save the current time as finishTime, popFront of the CPU, deallocate memory, and pushBack of the finished.list, finished.counter++
        otherwise if the normal queue is not empty:
                popFront() of the normalReadyQ.list and pushFront in the CPU
                get and save the current time from the clock (make =)
                while ST > 0 and runtime <= timeQuantum and isEmpty(realTimeReadyQ.list):
                    if current time is not equal to the clock:
                        decrement the ST, increment the runtime, and make the current time = the clock
                if ST == 0, get and save the current time as finishTime, popFront of the CPU, deallocate memory, and pushBack of the finished.list, finished.counter++
                else if runtime == timeQuantum, popFront of the CPU, set runtime to 0, and pushBack the PCB on the normalReadyQ.list
                else if realTimeReadyQ.list is not empty:
                    popFront of the CPU, set runtime to 0, and pushBack the PCB on the normalReadyQ.list
unlock the mutex
*/
void *processManager(void *threadArg)
{
    atomic_int *clockTime = (atomic_int *)threadArg;
    int currentTime;
    PCB process;
    pthread_mutex_lock(&finishedQ.lock);
    while(finishedQ.counter != numOfProcesses)
    {
        if(isEmpty(CPU))
        {
            if(!(isEmpty(realTimeReadyQ.list)))
            {
                pthread_mutex_lock(&realTimeReadyQ.lock);
                process = popFront(realTimeReadyQ.list);
                pthread_mutex_unlock(&realTimeReadyQ.lock);
                pushFront(CPU, process);
                currentTime = (int)*clockTime;
                while(CPU->next->info.runtime > 0)
                {
                    if(currentTime < (int)*clockTime)
                    {
                        CPU->next->info.runtime--;
                        currentTime = (int)*clockTime;
                    }
                }
                if(CPU->next->info.runtime == 0)
                {
                    currentTime = (int)*clockTime;
                    CPU->next->info.finishTime = (int)*clockTime;
                    process = popFront(CPU);
                    deallocateMem(process);
                    pushBack(finishedQ.list, process);
                    finishedQ.counter++;
                }
            }
            else if(!(isEmpty(normalReadyQ.list)))
            {
                int count = 0;
                pthread_mutex_lock(&normalReadyQ.lock);
                process = popFront(normalReadyQ.list);
                pthread_mutex_unlock(&normalReadyQ.lock);
                pushFront(CPU, process);
                currentTime = (int)*clockTime;
                while((CPU->next->info.runtime > 0) && (count < timeQuantum) && (isEmpty(realTimeReadyQ.list)))
                {
                    if(currentTime < (int)*clockTime)
                    {
                        CPU->next->info.runtime--;
                        count++;
                        currentTime = (int)*clockTime;
                    }
                }
                if(CPU->next->info.runtime == 0)
                {
                    currentTime = (int)*clockTime;
                    CPU->next->info.finishTime = (int)*clockTime;
                    deallocateMem(process);
                    process = popFront(CPU);
                    pushBack(finishedQ.list, process);
                    finishedQ.counter++;
                    count = 0;
                }
                else if(count == timeQuantum)
                {
                    process = popFront(CPU);
                    count = 0;
                    pthread_mutex_lock(&normalReadyQ.lock);
                    pushBack(normalReadyQ.list, process);
                    pthread_mutex_unlock(&normalReadyQ.lock);
                }
                else if(!(isEmpty(realTimeReadyQ.list)))
                {
                    process = popFront(CPU);
                    count = 0;
                    pthread_mutex_lock(&normalReadyQ.lock);
                    pushBack(normalReadyQ.list, process);
                    pthread_mutex_unlock(&normalReadyQ.lock);
                }
            }
        }
    }
    pthread_cond_signal(&finishedQ.filled);
    pthread_mutex_unlock(&finishedQ.lock);
    FinishedListFull = true;
    return NULL;
}

void deallocateMem(PCB process)
{
    pthread_mutex_lock(&freeSpace.lock);
    int count = sizeOfFreeSpaceList(freeSpace.list);
    for(int k=0; k<count; k++)
    {
        if((process.SA + process.RS) <= getElement(freeSpace.list, k)->info.SA)
        {
            if((process.SA + process.RS) == getElement(freeSpace.list, k)->info.SA)
            {
                if(k == 0)
                {
                    getElement(freeSpace.list, k)->info.SA = process.SA;
                    getElement(freeSpace.list, k)->info.RS = process.RS;
                }
                else if(process.SA == (getElement(freeSpace.list, k-1)->info.SA + getElement(freeSpace.list, k-1)->info.RS))
                {
                    getElement(freeSpace.list, k-1)->info.RS += process.RS;
                    getElement(freeSpace.list, k-1)->info.RS += getElement(freeSpace.list, k)->info.RS;
                    pop(freeSpace.list, k);
                }
                else
                {
                    getElement(freeSpace.list, k)->info.SA = process.SA;
                    getElement(freeSpace.list, k)->info.RS = process.RS;
                }
            }
            else
            {
                if(k == 0)
                {
                    PCB ps;
                    push(freeSpace.list, 0, ps);
                    getElement(freeSpace.list, 0)->info.SA = process.SA;
                    getElement(freeSpace.list, 0)->info.RS = process.RS;
                }
                else if(process.SA == (getElement(freeSpace.list, k-1)->info.SA + getElement(freeSpace.list, k-1)->info.RS))
                {
                    getElement(freeSpace.list, k-1)->info.RS += process.RS;
                }
                else
                {
                    PCB ps;
                    push(freeSpace.list, k, ps);
                    getElement(freeSpace.list, k)->info.SA = process.SA;
                    getElement(freeSpace.list, k)->info.RS = process.RS;
                }
            }
        }
        else if(k == (count-1))
        {
            PCB ps;
            push(freeSpace.list, count+1, ps);
            getElement(freeSpace.list, count+1)->info.SA = process.SA;
            getElement(freeSpace.list, count+1)->info.RS = process.RS;
        }
    }
    pthread_mutex_unlock(&freeSpace.lock);
}

/*
THREAD 4
while the finished.list count is not equal to the numOfProcesses, increment the clock every second
*/
void *clocker(void *threadArg)
{
    atomic_int *countPtr = (atomic_int *)threadArg;
    while(finishedQ.counter != numOfProcesses)
	{
		atomic_fetch_add(countPtr, 1);
		Sleep(10);
		//for every sec:
		//Sleep(1000);
	}
    return NULL;
}
