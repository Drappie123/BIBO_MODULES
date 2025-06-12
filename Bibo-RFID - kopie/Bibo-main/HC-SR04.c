/*
 * This library is meant to be used with four ultrasonic distance sensors.
 * Two sensors can be used simultaneously, the other two can be used by using the selection function.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "HC-SR04.h"
#include <util/atomic.h>
// Distances in centimeters get updated with approx 30Hz
volatile unsigned int left_distance = 0;
volatile unsigned int right_distance = 0;

ISR(TIMER4_CAPT_vect){
    static volatile unsigned int start_time = 0;
    // If interrupt is set to rising edge
    if (TCCR4B & (1 << ICES4)) {
        // Store echo start value
        start_time = ICR4;
        // Switch to falling edge
        TCCR4B &= ~(1 << ICES4);
    }
    // If interrupt is set to falling edge
    else {
        // Switch back to rising edge
        TCCR4B |= (1 << ICES4);
        // distance in centimeters = (total_time/2)*0.01715 - to simplify this: distance = total_time/117
        left_distance = (ICR4 - start_time)/117;
    }
}

ISR(TIMER5_CAPT_vect){
    static volatile unsigned int start_time = 0;
    // If interrupt is set to rising edge
    if (TCCR5B & (1 << ICES5)) {
        // Store echo start value
        start_time = ICR5;
        // Switch to falling edge
        TCCR5B &= ~(1 << ICES5);
    }
    // If interrupt is set to falling edge
    else {
        // Switch back to rising edge
        TCCR5B |= (1 << ICES5);
        // distance in centimeters = (total_time/2)*0.01715 - to simplify this: distance = total_time/117
        right_distance = (ICR5 - start_time)/117;
    }
}

void ultrasonic_init(void){
    // Config Trig pin
    DDR_TRIG4 |= (1 << PIN_TRIG4);
    PORT_TRIG4 &= ~(1 << PIN_TRIG4);

    DDR_TRIG5 |= (1 << PIN_TRIG5);
    PORT_TRIG5 &= ~(1 << PIN_TRIG5);
    // Config Echo pin
    DDR_ECHO4 &= ~(1 << PIN_ECHO4);
    DDR_ECHO5 &= ~(1 << PIN_ECHO5);
    // Mode 15 fast PWM, non-inverting mode, prescaler 8
    TCCR4A = (1<< COM4B1) | (1 << WGM41) | (1 << WGM40);
    TCCR4B = (1 << ICES4) | (1<< WGM43) | (1<< WGM42) | (1 << CS41);
    TIMSK4 = (1 << ICIE4);

    TCCR5A = (1<< COM5B1) | (1 << WGM51) | (1 << WGM50);
    TCCR5B = (1 << ICES5) | (1<< WGM53) | (1<< WGM52) | (1 << CS51);
    TIMSK5 = (1 << ICIE5) | (1 << TOV5);
    // Set top value to max
    OCR4A = 0xFFFF;
    OCR5A = 0xFFFF;
    // Set trigger timing
    OCR4B = 20;
    OCR5B = 20;
    // Sync timers
    ultrasonic_sync();
    // Set trigger for primary sensor set
    ultrasonic_select(0);
}
// Sync timers for precise timing
void ultrasonic_sync(void){
    TCNT4 = 0;
    TCNT5 = 0;
}
// Select sensor set
void ultrasonic_select(char selection){
    // Trigger timer 4 enabled
    if(selection == 0){
        TCCR4A |= (1<< COM4B1);
        TCCR5A &= ~(1<< COM5B1);
        PORT_TRIG5 &= ~(1<<PIN_TRIG5);
    }
    // Trigger timer 5 enabled
    else{
        TCCR4A &= ~(1<< COM4B1);
        PORT_TRIG4 &= ~(1<<PIN_TRIG4);
        TCCR5A |= (1<< COM5B1);

    }
}
// Get distance functions to prevent interrupt during distance read
int get_left_distance(void){
        unsigned int value;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {  // Disables interrupts temporarily
        value = left_distance;
    }
    return value;
}

int get_right_distance(void){
        unsigned int value;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {  // Disables interrupts temporarily
        value = right_distance;
    }
    return value;
}
