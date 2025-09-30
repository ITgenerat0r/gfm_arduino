#include "pid.h"

void PID_Init(PID_Controller* pid, float kp, float ki, float kd,
              float min_output, float max_output) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->setpoint = 0.0f;
    pid->input = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;

    pid->output_min = min_output;
    pid->output_max = max_output;
}

int PID_Compute(PID_Controller* pid, float input) {
    pid->input = input;

    float error = pid->setpoint - pid->input;

    // Интеграл
    pid->integral += error;

    // Ограничение интеграла
    if (pid->integral > pid->output_max / pid->ki) {
        pid->integral = pid->output_max / pid->ki;
    } else if (pid->integral < pid->output_min / pid->ki) {
        pid->integral = pid->output_min / pid->ki;
    }

    // Производная
    pid->derivative = error - pid->last_error;
    pid->last_error = error;

    // Вычисление выхода
    pid->output = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * pid->derivative);

    // Ограничение выхода
    if (pid->output > pid->output_max) {
        pid->output = pid->output_max;
    } else if (pid->output < pid->output_min) {
        pid->output = pid->output_min;
    }

    return (int)pid->output;
}