/* 
 * File:   motor_init.h
 * Author: Raffaello
 *
 * Created on February 29, 2016, 3:14 PM
 */

#ifndef MOTOR_INIT_H
#define	MOTOR_INIT_H

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * Numbers of motors available in this board
     */
#define NUM_MOTORS 2
#define MOTOR_ZERO 0
#define MOTOR_ONE 1
    
void InitPWM(void);
void InitQEI(short motIdx);
void InitIC(short motIdx);


#ifdef	__cplusplus
}
#endif

#endif	/* MOTOR_INIT_H */

