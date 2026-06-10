#include "scheduler.h"

//extern TaskType Tasks[NUMBER_OF_TASKS];
//extern TaskType *TaskTable;
//extern uint32_t TaskTableSize;

void TaskSchedulerISR(void)
{
    uint32_t i;

    /*for (i = 0; i < TaskTableSize; i++) {
        (TaskTable+i)->TaskCycleCounter++;
        if ((TaskTable+i)->TaskCycleCounter >= (TaskTable+i)->TaskExecutionPeriod) {

            // If current task is to be executed, reset task counter.
            (TaskTable+i)->TaskCycleCounter = 0;

            // And then call the task for execution (to completion).
            (*((TaskTable+i)->Task))((void *) (TaskTable+i)->TaskObject);
        }
    }*/
}

void SysTick_Handler(void)
{
    TaskSchedulerISR();
}
