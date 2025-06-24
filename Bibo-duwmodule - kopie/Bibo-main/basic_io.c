/*
 * Functions for buttons and E18-D80NK sensors. Button functions with debouncing included
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "TM1637.h"
#include "basic_io.h"
#include "config.h"
#include "stepper.h"
#include "music.h"

// --- STOP KNOP --- //
ISR(INT0_vect){
        while(1){
            // STOP SLAVE
            PORTA &= ~(1<<PA0);
            buzzer_enable(0);
            if(stop_pressed()){
                display_txt_stop();
            }
            else{
                if(starting_button()){
                    // Reset slave
                    PORTA |= (1<<PA0);
                    // RESET arduino
                    cli();
                    // Enable Watchdog Timer and set minimum timeout
                    WDTCSR = (1 << WDE) | (1 << WDCE);
                    // Wait for the watchdog timer to reset the system
                    while (1) {}
                }
            }
        }
}

// --- Init all basic-io --- //
void basic_io_init(void){
    init_stop_button();
    init_limit_switches();
}

// --- INIT INTERRUPT FOR STOP BUTTON --- //
void init_stop_button(void){
    DDR_STOP &= ~(1<<PIN_STOP);
    PORT_STOP |= (1<<PIN_STOP);
    DDRA |= (1<<PA0);
    PORTA |= (1<<PA0);

    EICRA |= (1<<ISC01) | (1<<ISC00);  // Rising edge
    EIMSK |= (1<<INT0);
}
// --- Functions for reading buttons and sensors --- //
char stop_pressed(void) {
    return (REG_STOP & (1 << PIN_STOP));
}

/// --- limit_switches---///
void init_limit_switches(void) {
    // Set PD0 and PD1 (pins 25, 26) as input
    DDRA &= ~((1 << PA3) | (1 << PA4));

    // Optional: Enable pull-up resistors
    PORTA |= (1 << PA3) | (1 << PA4);
    // Set PD1 startbutton
    DDRD &= ~((1 << PD1));

    // Optional: Enable pull-up resistors startbutton
    PORTD |= (1 << PD1);
}

uint8_t light_limit_switch_25(void) {
    return !(PINA & (1 << PA3));
}

uint8_t heavy_limit_switch_26(void) {
    return !(PINA & (1 << PA4));
}
//starting button
uint8_t starting_button(void) {
    return !(PIND & (1 << PD1));
}
