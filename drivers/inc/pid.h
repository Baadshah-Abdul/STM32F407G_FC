/*
 * pid.h
 *
 *  Created on: Jan 12, 2026
 *      Author: ACER1
 */

#ifndef INC_PID_H_
#define INC_PID_H_

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float prev_error;
    float integral;
    float output_limit;
} PID_Config_t;

float PID_Compute(PID_Config_t* pid, float setpoint, float measured_value, float dt);

#endif /* INC_PID_H_ */
