/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__dsPIC33E__)
    	#include <p33Exxxx.h>
    #elif defined(__dsPIC33F__)
    	#include <p33Fxxxx.h>
    #endif
#endif


#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include <math.h>
#include "system.h"

/******************************************************************************/
/* Generic tasks                                                              */
/******************************************************************************/

#define TASK_1A "TASK_1A"
static string_data_t _TASK_1A = {TASK_1A, sizeof(TASK_1A)};
#define TASK_1B "TASK_1B"
static string_data_t _TASK_1B = {TASK_1B, sizeof(TASK_1B)};

#define TASK_2 "TASK_2"
static string_data_t _TASK_2 = {TASK_2, sizeof(TASK_2)};

void Task1(int argc, int *argv) {
    int data = argv[0];
    float p = cosf(data);
    float pp = expf(p);
    int a = 1;
    a++;
}

void Task2(int argc, int *argv) {
    int data = argv[0];
    float p = cosf(data);
    int b = 1;
    b++;
}

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

int16_t main(void)
{
    ConfigureOscillator();
    
    InitTimer1();           ///< Open Timer1 for clock system
    
    InitEvents();
    
    // Init task
    hModule_t module_1a = register_module(&_TASK_1A);
    hEvent_t event_1a = register_event_p(module_1a, &Task1, EVENT_PRIORITY_MEDIUM);
    hTask_t task_1a = task_load_data(event_1a, 1000, 1, 1);
    task_set(task_1a, RUN);
    
    // Init task
    hModule_t module_1b = register_module(&_TASK_1B);
    hEvent_t event_1b = register_event_p(module_1b, &Task1, EVENT_PRIORITY_MEDIUM);
    hTask_t task_1b = task_load_data(event_1b, 1000, 1, 2);
    task_set(task_1b, RUN);
    
    // Init task
    hModule_t module_2 = register_module(&_TASK_2);
    hEvent_t event_2 = register_event_p(module_2, &Task2, EVENT_PRIORITY_MEDIUM);
    hTask_t task_2 = task_load_data(event_2, 1000, 1, 10);
    task_set(task_2, RUN);
    
    while(1)
    {

    }
}