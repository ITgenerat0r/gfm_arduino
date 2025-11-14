#ifndef PID_H
#define PID_H

typedef struct {
    float kp;             // Пропорциональный коэффициент
    float ki;             // Интегральный коэффициент
    float kd;             // Дифференциальный коэффициент

    float setpoint;       // Заданное значение
    float input;          // Текущее значение (feedback)
    float output;         // Выход ПИД (временно float, потом округляется в int)

    float last_error;     // Предыдущая ошибка
    float integral;       // Интеграл ошибки
    float derivative;     // Производная ошибки

    float output_min;     // Минимальное значение выхода
    float output_max;     // Максимальное значение выхода
} PID_Controller;

void PID_Init(PID_Controller* pid, float kp, float ki, float kd,
              float min_output, float max_output);

int PID_Compute(PID_Controller* pid, float input);

#endif