#ifndef _CALLBACKACTION_H
#define _CALLBACKACTION_H

void TIM_Init();
void Sensor_Handler();
void Uart_Findborder_Receive(void);
void Uart_Fine_Tuning_Receive(void);
void Uart_Findborder_Hard_Receive(void);

#endif