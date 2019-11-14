#ifndef STRUCTPCB_H_INCLUDED
#define STRUCTPCB_H_INCLUDED
#include <stdbool.h>

//define what a PCB is
//pid is the process id, AT is arrival time, ST is service time, SA is starting address in mem, RS is requested size
typedef struct PCB
{
   int pid;
   int AT;
   int ST;
   int SA;
   int RS;
   char Type;
   int waitTime;
   int timePlacedOnWaitlist;
   int timeTakenOffWaitlist;
   int placedOnWaitlist;
   bool new;
   int startTime;
   int runtime;
   int finishTime;
} PCB;

#endif
