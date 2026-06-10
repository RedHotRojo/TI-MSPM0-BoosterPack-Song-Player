#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <ti/devices/msp/msp.h>
#include "../../LP_MSPM0G3507/bsp.h"

typedef struct {
    void (*Task)(void *);           // Pointer to function task
    uint32_t TaskCycleCounter;      // Task is executed when (TaskCycleCounter == TaskExecutionPeriod)
    int32_t TaskExecutionPeriod;    // An integer multiple of the scheduler execution period
    void *TaskObject;               // Task object information
} TaskType;


// Function prototypes
void TaskSchedulerISR(void);

#endif /* SCHEDULER_H_ */
