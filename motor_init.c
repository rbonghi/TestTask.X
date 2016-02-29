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


#include <pwm12.h>

#include "system.h"
#include "motor_init.h"

typedef struct _ICdata {
        volatile unsigned int overTmr; //Overflow timer
        volatile unsigned long timePeriod; //Time period from Input Capture
        volatile int SIG_VEL; //Sign of versus rotation motor
    } ICdata;

/*****************************************************************************/
/* Global Variable Declaration                                               */
/*****************************************************************************/

// Dynamic Interrupt Capture
#define IC_MODE0    0b001 // 2X mode (default)
#define IC_MODE1    0b011 // 1X mode
#define IC_MODE2    0b100 // 1/4X mode
#define IC_MODE3    0b101 // 1/16X mode
#define IC_DISABLE  0b00
const int IcMode[4] = {IC_MODE0, IC_MODE1, IC_MODE2, IC_MODE3};

ICdata ICinfo[NUM_MOTORS];

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

void InitQEI(short motIdx) {
    switch (motIdx) {
        case MOTOR_ZERO:
            //QEI1CONbits.CNTERR= 0; // No position count error has occurred
            QEI1CONbits.QEISIDL = 1; // Discontinue module operation when device enters Idle mode
            QEI1CONbits.QEIM = 7; // Quadrature Encoder Interface enabled (x4 mode) with position counter reset by match (MAXxCNT)
            //QEI1CONbits.SWPAB = (get_motor_parameters(motIdx).rotation >= 1) ? 1 : 0; // Phase A and Phase B inputs swapped
            QEI1CONbits.SWPAB = 1;
            QEI1CONbits.PCDOUT = 0; // Position counter direction status output disabled (Normal I/O pin operation)
            //QEI1CONbits.TQGATE= 0  // Timer gated time accumulation disabled
            //QEI1CONbits.TQCKPS = 0b00	// 1:1 prescale value
            QEI1CONbits.POSRES = 0; // Index pulse does not reset position counter

            DFLT1CONbits.QEOUT = 1; // Digital filter outputs enabled on QEAx/QEBx/INDXx pins
            DFLT1CONbits.QECK = 6; // 1:128 Clock divide for QEAx/QEBx/INDXx

            MAX1CNT = 0xFFFF;
            POS1CNT = 0;
            break;
        case MOTOR_ONE:
            //QEI2CONbits.CNTERR= 0; // No position count error has occurred
            QEI2CONbits.QEISIDL = 1; // Discontinue module operation when device enters Idle mode
            QEI2CONbits.QEIM = 7; // Quadrature Encoder Interface enabled (x4 mode) with position counter reset by match (MAXxCNT)
            //QEI2CONbits.SWPAB = (get_motor_parameters(motIdx).rotation >= 1) ? 1 : 0; // Phase A and Phase B inputs swapped
            QEI1CONbits.SWPAB = 1;
            QEI2CONbits.PCDOUT = 0; // Position counter direction status output disabled (Normal I/O pin operation)
            //QEI2CONbits.TQGATE= 0  // Timer gated time accumulation disabled
            //QEI2CONbits.TQCKPS = 0b00	// 1:1 prescale value
            QEI2CONbits.POSRES = 0; // Index pulse does not reset position counter

            DFLT2CONbits.QEOUT = 1; // Digital filter outputs enabled on QEAx/QEBx/INDXx pins
            DFLT2CONbits.QECK = 6; // 1:128 Clock divide for QEAx/QEBx/INDXx

            MAX2CNT = 0xFFFF;
            POS2CNT = 0;
            break;
    }
}

void InitIC(short motIdx) {
    switch (motIdx) {
        case MOTOR_ZERO:
            // Initialize Capture Module
            IC1CONbits.ICM = IC_DISABLE; // Disable Input Capture 1 module
            IC1CONbits.ICTMR = 1; // Select Timer2 as the IC1 Time base
            IC1CONbits.ICI = 0b01; // Interrupt on every second capture event
            IC1CONbits.ICM = IcMode[0]; // Generate capture event on every Rising edge

            // Enable Capture Interrupt And Timer2
            IPC0bits.IC1IP = INPUT_CAPTURE_LEVEL; // Setup IC1 interrupt priority level
            IFS0bits.IC1IF = 0; // Clear IC1 Interrupt Status Flag
            IEC0bits.IC1IE = 1; // Enable IC1 interrupt
            break;
        case MOTOR_ONE:
            // Initialize Capture Module
            IC2CONbits.ICM = IC_DISABLE; // Disable Input Capture 2 module
            IC2CONbits.ICTMR = 1; // Select Timer2 as the IC1 Time base
            IC2CONbits.ICI = 0b01; // Interrupt on every second capture event
            IC2CONbits.ICM = IcMode[0]; // Generate capture event on every Rising edge

            // Enable Capture Interrupt And Timer2
            IPC1bits.IC2IP = INPUT_CAPTURE_LEVEL; // Setup IC2 interrupt priority level
            IFS0bits.IC2IF = 0; // Clear IC2 Interrupt Status Flag
            IEC0bits.IC2IE = 1; // Enable IC2 interrupt
            break;
    }
}

void __attribute__((interrupt, auto_psv, shadow)) _IC1Interrupt(void) {
    unsigned int t1, t2;
    t2 = IC1BUF; // IC1BUF is a FIFO, each reading is a POP
    t1 = IC1BUF;

    ICinfo[MOTOR_ZERO].timePeriod += ICinfo[MOTOR_ZERO].overTmr * PR2 + t2 - t1; // PR2 is 0xFFFF
    ICinfo[MOTOR_ZERO].overTmr = 0;

    (QEI1CONbits.UPDN ? ICinfo[MOTOR_ZERO].SIG_VEL++ : ICinfo[MOTOR_ZERO].SIG_VEL--); //Save sign Vel motor 0
    ICinfo[MOTOR_ZERO].SIG_VEL = (QEI1CONbits.UPDN ? 1 : -1); //Save sign Vel L
    IFS0bits.IC1IF = 0;
}

void __attribute__((interrupt, auto_psv, shadow)) _IC2Interrupt(void) {
    unsigned int t1, t2;
    t2 = IC2BUF; // IC1BUF is a FIFO, each reading is a POP
    t1 = IC2BUF;

    ICinfo[MOTOR_ONE].timePeriod += ICinfo[MOTOR_ONE].overTmr * PR2 + t2 - t1; // PR2 is 0xFFFF
    ICinfo[MOTOR_ONE].overTmr = 0;
    
    //	if(QEI2CONbits.UPDN) SIG_VELR++;		//Save sign Vel R
    //	else SIG_VELR--;
    (QEI2CONbits.UPDN ? ICinfo[MOTOR_ONE].SIG_VEL++ : ICinfo[MOTOR_ONE].SIG_VEL--); //Save sign Vel motor 1
//    ICinfo[MOTOR_ONE].SIG_VEL = (QEI2CONbits.UPDN ? 1 : -1); //Save sign Vel R
    IFS0bits.IC2IF = 0;
}

void __attribute__((interrupt, auto_psv, shadow)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0; // interrupt flag reset
    if (ICinfo[MOTOR_ZERO].timePeriod)
        ICinfo[MOTOR_ZERO].overTmr++; // timer overflow counter for Left engines
    if (ICinfo[MOTOR_ONE].timePeriod)
        ICinfo[MOTOR_ONE].overTmr++; // timer overflow counter for Right engines
}