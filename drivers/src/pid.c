/*
 * pid.c
 *
 *  Created on: Jan 12, 2026
 *      Author: ACER1
 */

#include "pid.h"

float PID_Compute(PID_Config_t *pid, float setpoint, float measured_value, float dt)
{
	float error = setpoint - measured_value;

	// Proportional
	float P = pid->Kp * error;

	// Integral
	pid->integral += error * dt;
	// Limit the accumulated integral to prevent "Windup"
	float i_limit = (pid->i_limit > 0.0f) ? pid->i_limit : 150.0f;

	if (pid->integral > i_limit)
		pid->integral = i_limit;
	if (pid->integral < -i_limit)
		pid->integral = -i_limit;

	float I = pid->Ki * pid->integral;

	// Derivative
	float D = 0.0f;
	if (pid->prev_error != 0.0f || pid->integral != 0.0f)
	{
	    // Only compute derivative after first real sample
	    D = pid->Kd * (error - pid->prev_error) / dt;
	}
	pid->prev_error = error;

	//PID value
	float output = P + I + D;

	// External Output Limiting
	if (output > pid->output_limit)
		output = pid->output_limit;
	if (output < -pid->output_limit)
		output = -pid->output_limit;

	return output;
}
