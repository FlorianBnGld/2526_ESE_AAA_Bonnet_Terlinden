/*
 * motor.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */


#ifndef INC_MOTOR_CONTROL_MOTOR_H_
#define INC_MOTOR_CONTROL_MOTOR_H_

#include "user_interface/shell.h"
#include "tim.h"

int motor_init();
int motor_control(h_shell_t* h_shell, int argc, char** argv);

#endif /* INC_MOTOR_CONTROL_MOTOR_H_ */
