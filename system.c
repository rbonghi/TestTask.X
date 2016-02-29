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

#include "system.h"

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

#define EVENT_PRIORITY_LOW_ENABLE IEC3bits.RTCIE
#define EVENT_PRIORITY_LOW_FLAG IFS3bits.RTCIF
#define EVENT_PRIORITY_LOW_P IPC15bits.RTCIP
hardware_bit_t RTCIF = REGISTER_INIT(IFS3, 14);

#define EVENT_PRIORITY_MEDIUM_ENABLE IEC0bits.OC1IE
#define EVENT_PRIORITY_MEDIUM_FLAG IFS0bits.OC1IF
#define EVENT_PRIORITY_MEDIUM_P IPC0bits.OC1IP
hardware_bit_t OC1IF = REGISTER_INIT(IFS0, 2);

#define EVENT_PRIORITY_HIGH_ENABLE IEC0bits.OC2IE
#define EVENT_PRIORITY_HIGH_FLAG IFS0bits.OC2IF
#define EVENT_PRIORITY_HIGH_P IPC1bits.OC2IP
hardware_bit_t OC2IF = REGISTER_INIT(IFS0, 6);

#define EVENT_PRIORITY_VERY_LOW_ENABLE IEC1bits.OC3IE
#define EVENT_PRIORITY_VERY_LOW_FLAG IFS1bits.OC3IF
#define EVENT_PRIORITY_VERY_LOW_P IPC6bits.OC3IP
hardware_bit_t OC3IF = REGISTER_INIT(IFS1, 9);

/******************************************************************************/
/* System Level Functions                                                     */
/******************************************************************************/

void ConfigureOscillator(void) {
    PLLFBD = 30; // M=32  //Old configuration: PLLFBD=29 - M=31
    CLKDIVbits.PLLPOST = 0; // N1=2
    CLKDIVbits.PLLPRE = 0; // N2=2
    // Disable Watch Dog Timer
    RCONbits.SWDTEN = 0;
    // Clock switching to incorporate PLL
    // Initiate Clock Switch to Primary
    __builtin_write_OSCCONH(0x03); // Oscillator with PLL (NOSC=0b011)
    __builtin_write_OSCCONL(0x01); // Start clock switching
    while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur
    while (OSCCONbits.LOCK != 1) {
    }; // Wait for PLL to lock
}

void InitEvents(void) {
    /// Register event controller
    init_events(&TMR1, &PR1);
    
    EVENT_PRIORITY_VERY_LOW_ENABLE = 0;
    EVENT_PRIORITY_VERY_LOW_P = EVENT_PRIORITY_VERY_LOW_LEVEL;
    register_interrupt(EVENT_PRIORITY_VERY_LOW, &RTCIF);
    EVENT_PRIORITY_VERY_LOW_ENABLE = 1;
    
    EVENT_PRIORITY_LOW_ENABLE = 0;
    EVENT_PRIORITY_LOW_P = EVENT_PRIORITY_LOW_LEVEL;
    register_interrupt(EVENT_PRIORITY_LOW, &RTCIF);
    EVENT_PRIORITY_LOW_ENABLE = 1;
    
    EVENT_PRIORITY_MEDIUM_ENABLE = 0;
    EVENT_PRIORITY_MEDIUM_P = EVENT_PRIORITY_MEDIUM_LEVEL;
    register_interrupt(EVENT_PRIORITY_MEDIUM, &OC1IF);
    EVENT_PRIORITY_MEDIUM_ENABLE = 1;
    
    EVENT_PRIORITY_HIGH_ENABLE = 0;
    EVENT_PRIORITY_HIGH_P = EVENT_PRIORITY_HIGH_LEVEL;
    register_interrupt(EVENT_PRIORITY_HIGH, &OC2IF);
    EVENT_PRIORITY_HIGH_ENABLE = 1;
    
    /// Initialization task controller
    task_init(FRTMR1);
}

/******************************************************************************/
/* InitTimer                                                                  */
/******************************************************************************/

void InitTimer1(void) {
    //T1CON = 10100000 00000000
    T1CONbits.TON = 0; // Disable Timer
    T1CONbits.TSIDL = 1; // Stop in Idle Mode bit
    T1CONbits.TGATE = 0; // Disable Gated Timer mode
    T1CONbits.TCKPS = 0b00; // Select 1:1 Prescaler
    T1CONbits.TSYNC = 0; // Disable Synchronization
    T1CONbits.TCS = 0; // Select internal clock source
    TMR1 = 0x00; // Clear timer register
    PR1 = TMR1_VALUE; // Load the period value

    IPC0bits.T1IP = SYS_TIMER_LEVEL; // Set Timer 1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer 1 Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt

    T1CONbits.TON = 1; // Start Timer
}

void __attribute__((interrupt, auto_psv)) _RTCCInterrupt(void) {
    event_manager(EVENT_PRIORITY_LOW);
    EVENT_PRIORITY_LOW_FLAG = 0; //interrupt flag reset
}

void __attribute__((interrupt, auto_psv)) _OC1Interrupt(void) {
    event_manager(EVENT_PRIORITY_MEDIUM);
    EVENT_PRIORITY_MEDIUM_FLAG = 0; // interrupt flag reset
}

void __attribute__((interrupt, auto_psv)) _OC2Interrupt(void) {
    event_manager(EVENT_PRIORITY_HIGH);
    EVENT_PRIORITY_HIGH_FLAG = 0; //interrupt flag reset
}

void __attribute__((interrupt, auto_psv)) _OC3Interrupt(void) {
    event_manager(EVENT_PRIORITY_VERY_LOW);
    EVENT_PRIORITY_VERY_LOW_FLAG = 0;
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    /// Execution task manager
    task_manager();
    IFS0bits.T1IF = 0; // Clear Timer 1 Interrupt Flag
}