#ifndef COMMUNICATION_H_INCLUDED
#define COMMUNICATION_H_INCLUDED

#include <stdint.h>  // for uint8_t

// ✅ Define the enum globally
typedef enum {
    forward_slow,
    forward_fast,
    backward_slow,
    backward_fast,
    stop,
    turn_left,
    turn_right,
    turn_left_H,
    turn_right_H
} tasks_bibo;

//Function declarations
void USART1_sendCommand(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);
void USART0_sendHexByte(uint8_t byte);
void USART0_sendString(const char* str);
void USART0_sendByte(uint8_t data);
void USART1_init(unsigned int baud);
void USART0_init(unsigned int baud);
uint8_t USART1_receiveByte(void);

//task manager for all tasks for slave
void task_manager(tasks_bibo current_task, uint8_t speed, uint8_t acceleration);

/*Declare external global variables
extern int standard_speed;
extern int standard_acceleration;
*/
#endif // COMMUNICATION_H_INCLUDED
