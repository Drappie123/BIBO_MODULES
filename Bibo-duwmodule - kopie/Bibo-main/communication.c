/*
 * This library is meant to be used for communication with a master and a slave arduino
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "communication.h"
#include "config.h"


// Init USART0 (USB naar PC - voor PuTTY)
void USART0_init(unsigned int baud) {
    unsigned int ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (3 << UCSZ00); // 8-bit data
}

// Init USART1 (naar Slave)
void USART1_init(unsigned int baud) {
    unsigned int ubrr = F_CPU / 16 / baud - 1;
    UBRR1H = (unsigned char)(ubrr >> 8);
    UBRR1L = (unsigned char)ubrr;
    UCSR1B = (1 << RXEN1) | (1 << TXEN1);
    UCSR1C = (3 << UCSZ10); // 8-bit data
}

// USART0: zend byte naar PC
void USART0_sendByte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

// USART0: zend string naar PC
void USART0_sendString(const char* str) {
    while (*str) {
        USART0_sendByte(*str++);
    }
}

// USART0: zend hexwaarde als ASCII naar PC
void USART0_sendHexByte(uint8_t byte) {
    char hex[] = "0123456789ABCDEF";
    USART0_sendByte(hex[(byte >> 4) & 0x0F]);
    USART0_sendByte(hex[byte & 0x0F]);
}

// USART1: zend 4-byte commando naar Slave
void USART1_sendCommand(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
    USART0_sendString("MASTER: Sending to Slave: ");
    USART0_sendHexByte(b1); USART0_sendByte(' ');
    USART0_sendHexByte(b2); USART0_sendByte(' ');
    USART0_sendHexByte(b3); USART0_sendByte(' ');
    USART0_sendHexByte(b4); USART0_sendString("\r\n");

    while (!(UCSR1A & (1 << UDRE1))) {}
    UDR1 = b1;
    while (!(UCSR1A & (1 << UDRE1))) {}
    UDR1 = b2;
    while (!(UCSR1A & (1 << UDRE1))) {}
    UDR1 = b3;
    while (!(UCSR1A & (1 << UDRE1))) {}
    UDR1 = b4;
    USART0_sendString("MASTER: Command sent.\r\n");
}

// USART1: ontvang byte van Slave
uint8_t USART1_receiveByte(void) {
    while (!(UCSR1A & (1 << RXC1)));
    return UDR1;
}



void task_manager(tasks_bibo current_task, uint8_t speed, uint8_t acceleration) {
    switch (current_task) {
        case forward_slow:  USART1_sendCommand(0x01, 0x80, speed, acceleration); break;
        case forward_fast:  USART1_sendCommand(0x01, 0xFF, speed, acceleration); break;
        case backward_slow: USART1_sendCommand(0x01, 0x00, speed, acceleration); break;
        case backward_fast: USART1_sendCommand(0x01, 0x7E, speed, acceleration); break;
        case stop:          USART1_sendCommand(0x01, 0x7F, speed, acceleration); break;
        case turn_left:     USART1_sendCommand(0x02, 0x01, speed, acceleration); break;
        case turn_right:    USART1_sendCommand(0x02, 0x02, speed, acceleration); break;
        case turn_left_H:   USART1_sendCommand(0x03, 0x01, speed, acceleration); break;
        case turn_right_H:  USART1_sendCommand(0x03, 0x02, speed, acceleration); break;
    }
}
