/* 
 * File:   system.h
 * Author: Raffaello
 *
 * Created on February 29, 2016, 1:50 PM
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <system/events.h>
#include <system/task_manager.h>
    
    /* Microcontroller MIPs (FCY) */
#define SYS_FREQ        80000000
#define FCY             SYS_FREQ/2

#define FRTMR1 1000             // Timer1 - Value in herz [Hz]
#define TCTMR1 1/FRTMR1         // Timer1 - Value in seconds [s]
#define TMR1_VALUE FCY/FRTMR1   // Timer1 - Value in CLK
#define FRTMR2 SYS_FREQ
#define TMR2_VALUE 0xFFFF       // Timer2 - Value for overflow

#define PWM_TIMER_LEVEL 7
#define INPUT_CAPTURE_LEVEL 6
#define SYS_TIMER_LEVEL 5


#define EVENT_PRIORITY_HIGH_LEVEL 4
#define EVENT_PRIORITY_MEDIUM_LEVEL 3
#define EVENT_PRIORITY_LOW_LEVEL 2
#define EVENT_PRIORITY_VERY_LOW_LEVEL 1

void ConfigureOscillator(void);
void InitEvents(void);
void InitTimer1(void);
void Peripherals_Init(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEM_H */

