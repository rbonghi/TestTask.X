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
#include <pwm12.h>
#include "system.h"


void InitTimer2(void) {
    //T2CON = 10100000 00000000
    T2CONbits.TON = 0; // Disable Timer
    T2CONbits.TSIDL = 1; // Stop in Idle Mode bit
    T2CONbits.TGATE = 0; // Disable Gated Timer mode
    T2CONbits.TCKPS = 0b00; // Select 1:1 Prescaler
    T2CONbits.TCS = 0; // Select internal clock source
    TMR2 = 0x00; // Clear timer register
    PR2 = TMR2_VALUE; // Load the period value

    IPC1bits.T2IP = PWM_TIMER_LEVEL; // Set Timer 1 Interrupt Priority Level
    IFS0bits.T2IF = 0; // Clear Timer 1 Interrupt Flag
    IEC0bits.T2IE = 1; // Enable Timer1 interrupt

    T2CONbits.TON = 1; // Start Timer
}

void InitPWM(void) {
    // Holds the value to be loaded into dutycycle register
    unsigned int period;
    // Holds the value to be loaded into special event compare register
    unsigned int sptime;
    // Holds PWM configuration value
    unsigned int config1;
    // Holds the value be loaded into PWMCON1 register
    unsigned int config2;
    // Holds the value to config the special event trigger postscale and duty cycle
    unsigned int config3;
    // Config PWM
    period = 2048; // PWM F=19,340Hz counting UP 12bit resolution @ Fcy=39.628 MHz
    sptime = 0x0;
    // 1:1 postscaler, 1:1 prescale, free running mode
    // PWM time base ON, count up
    config1 = PWM1_DIS & PWM1_IDLE_CON & PWM1_OP_SCALE1 & PWM1_IPCLK_SCALE1 &
            PWM1_MOD_FREE;
    // PWM1H e PWM1L enabled in complementar mode
    // dsPICs with 3 pairs of PWM pins have one timer only (A)
    config2 = PWM1_MOD1_COMP & PWM1_PEN1L & PWM1_PEN1H &
            PWM1_MOD2_COMP & PWM1_PEN2L & PWM1_PEN2H &
            PWM1_PDIS3H & PWM1_PDIS3L;
    config3 = PWM1_SEVOPS1 & PWM1_OSYNC_PWM & PWM1_UEN;
    OpenMCPWM1(period, sptime, config1, config2, config3);
    // Dead Time Unit A assigned to both 1 & 2 PWM pairs
    /* SetMCPWM1DeadTimeAssignment(PWM1_DTS1A_UA & PWM1_DTS1I_UA & PWM1_DTS2A_UA & PWM1_DTS2I_UA); */
    P1DTCON2bits.DTS1A = 0;
    P1DTCON2bits.DTS1I = 0;
    P1DTCON2bits.DTS2A = 0;
    P1DTCON2bits.DTS2I = 0;
    // Dead time 100ns = 0.2% of PWM period
    SetMCPWM1DeadTimeGeneration(PWM1_DTA4 & PWM1_DTAPS1);
    // duty cycle reg=1, duty cycle=50% (motore fermo in LAP mode , update disable=0
    SetDCMCPWM1(1, 2048, 0);
    SetDCMCPWM1(2, 2048, 0);

    ConfigIntMCPWM1(PWM1_INT_DIS);
}

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
//    float p = cosf(data);
//    float pp = expf(p);
    int a = 1;
    a = data;
    SetDCMCPWM1(1, 2548, 0);
}

void Task2(int argc, int *argv) {
    int data = argv[0];
//    float p = cosf(data);
    int b = 1;
    b = data;
}

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

int16_t main(void)
{
    ConfigureOscillator();
    
    // Encoders
    _TRISB10 = 1;
    _TRISB11 = 1;
    _TRISB6 = 1;
    _TRISB5 = 1;
    _TRISB12 = 0; // PWM1 +
    _TRISB13 = 0; // PWM1 -
    _TRISB14 = 0; // PWM2 +
    _TRISB15 = 0; // PWM2 -
    
    _TRISA7 = 0;
    _TRISA10 = 0;
    _LATA7 = 1;
    _LATA10 = 1;
    
    InitTimer1();           ///< Open Timer1 for clock system
    
    InitEvents();
    
    InitTimer2();
    InitPWM();
    PTCONbits.PTEN = 1;
    
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

void __attribute__((interrupt, auto_psv, shadow)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0; // interrupt flag reset
}