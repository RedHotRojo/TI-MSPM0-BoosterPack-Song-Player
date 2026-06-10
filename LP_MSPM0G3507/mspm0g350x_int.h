#ifndef MSPM0G350X_INT_H_
#define MSPM0G350X_INT_H_

typedef struct {
    void * CallbackObject;          // Callback function parameter pointer
    void (*CallbackFunc)(void *);   // Callback function pointer
} Callback_s;

// Function prototype
void Default_Callback(void *);

#endif /* MSPM0G350X_INT_H_ */
