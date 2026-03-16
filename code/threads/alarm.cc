// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "alarm.h"
#include "main.h"

//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom) { timer = new Timer(doRandom, this); sleepCount = 0;}

//------------------------------------------------------------------------
// Alarm:WaitUntill
// x = number of ticks to sleep
//------------------------------------------------------------------------

void Alarm::WaitUntil(int x) {
    // x is seconds directly — no conversion needed
    clock_t start = clock();
    while ((double)(clock() - start) / CLOCKS_PER_SEC < (double)x) {}

    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    sleepQueue[sleepCount].thread   = kernel->currentThread;
    sleepQueue[sleepCount].wakeTime = kernel->stats->totalTicks + (x * TimerTicks);
    sleepCount++;


    DEBUG('s', "[Sleep] Thread '" << kernel->currentThread->getName() 
              << "' sleeping for " << x << " second(s) [100 ticks = 1 sec]. "
              << "currentTick=" << kernel->stats->totalTicks 
              << " wakeTime=" << sleepQueue[sleepCount-1].wakeTime << "\n");


    kernel->currentThread->Sleep(false);
    kernel->interrupt->SetLevel(oldLevel);
}
//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as
//	if the interrupted thread called Yield at the point it is
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice
//      if we're currently running something (in other words, not idle).
//----------------------------------------------------------------------

void Alarm::CallBack() {
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();

    // Walk the array and wake any thread whose time has come
    for (int i = sleepCount - 1; i >= 0; i--) {
        if (kernel->stats->totalTicks >= sleepQueue[i].wakeTime) {
            
            DEBUG('s', "[Sleep] Thread '" << sleepQueue[i].thread->getName()
                      << "' woke up at tick=" << kernel->stats->totalTicks
                      << " (was scheduled for tick=" << sleepQueue[i].wakeTime << ")"
                      << " delta=" << (kernel->stats->totalTicks - sleepQueue[i].wakeTime)
                      << " tick(s) late\n");
            
            // Wake the thread
            kernel->scheduler->ReadyToRun(sleepQueue[i].thread);

            // Remove by swapping with the last element
            sleepQueue[i] = sleepQueue[sleepCount - 1];
            sleepCount--;
        }
    }

    if (status != IdleMode || sleepCount > 0) {
        interrupt->YieldOnReturn();
    }
}
