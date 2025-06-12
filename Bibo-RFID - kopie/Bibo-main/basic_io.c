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
    if(stop_pressed()){
        while(!plus_pressed_once() || stop_pressed()){
            // Stop steppers
            DDR_ST1_STEP &= ~(1<<PIN_ST1_STEP);
            DDR_ST2_STEP &= ~(1<<PIN_ST2_STEP);
            buzzer_enable(0);
            if(stop_pressed()){
                display_txt_stop();
            }
            else{
                if(enter_pressed_once() && !stop_pressed()){
                    // RESET arduino
                    cli();
                    // Enable Watchdog Timer and set minimum timeout
                    WDTCSR = (1 << WDE) | (1 << WDCE);
                    // Wait for the watchdog timer to reset the system
                    while (1) {}
                }
            }
        }
        // Re-enable steppers
        DDR_ST1_STEP |= (1<<PIN_ST1_STEP);
        DDR_ST2_STEP |= (1<<PIN_ST2_STEP);
    }
}

// --- Init all basic-io --- //
void basic_io_init(void){
    donk_init();
    //buttons_init();
    //init_stop_button();
    //led_init();
}
// --- E17-D80NK init -- ///
void donk_init(void){
    // Make input
    DDR_DONK1 &= ~(1<<PIN_DONK1);
    DDR_DONK2 &= ~(1<<PIN_DONK2);
    // Turn on pullup
    PORT_DONK1 |= (1<<PIN_DONK1);
    PORT_DONK2 |= (1<<PIN_DONK2);
}
// --- User interface buttons -- //
void buttons_init(void){
    // Make all input & enable pullup
    DDR_PLUS &= ~(1<<PIN_PLUS);
    PORT_PLUS |= (1<<PIN_PLUS);

    DDR_MINUS &= ~(1<<PIN_MINUS);
    PORT_MINUS |= (1<<PIN_MINUS);

    DDR_ENTER &= ~(1<<PIN_ENTER);
    PORT_ENTER |= (1<<PIN_ENTER);
}

// --- INIT INTERRUPT FOR STOP BUTTON --- //
void init_stop_button(void){
    DDR_STOP &= ~(1<<PIN_STOP);
    EICRA |= (1<<ISC01) | (1<<ISC00);  // Rising edge
    EIMSK |= (1<<INT0);
}
// --- INIT LEDS --- //
void led_init(void){
    DDR_LED |= (1<<PIN_LED);
    led_control(0);
}
// --- Functions for reading buttons and sensors --- //
char donk_left(void){
    return !(REG_DONK1 & (1<<PIN_DONK1));
}
char donk_right(void){
    return !(REG_DONK2 & (1<<PIN_DONK2));
}

char plus_pressed(void) {
    return !(REG_PLUS & (1 << PIN_PLUS));
}

char minus_pressed(void) {
    return !(REG_MINUS & (1 << PIN_MINUS));
}

char enter_pressed(void) {
    return !(REG_ENTER & (1 << PIN_ENTER));
}

char stop_pressed(void) {
    return (REG_STOP & (1 << PIN_STOP));
}
// --- Functions for reading single buttons presses with debounce --- //
char plus_pressed_once(void){
    static int pressed_before = 0;
    // Only return 1 when not pressed before
    if(plus_pressed()){
        if(!pressed_before){
            _delay_ms(DEBOUNCE_TIME);
            pressed_before = 1;
            return 1;
        }
    }
    else{
        if(pressed_before){
            _delay_ms(DEBOUNCE_TIME);
            pressed_before = 0;
        }
    }
    return 0;
}

char minus_pressed_once(void) {
    static int pressed_before = 0;
    // Only return 1 when not pressed before
    if (minus_pressed()) {
        if (!pressed_before) {
            _delay_ms(DEBOUNCE_TIME);
            pressed_before = 1;
            return 1;
        }
    } else {
        if (pressed_before) {
            _delay_ms(DEBOUNCE_TIME);
            pressed_before = 0;
        }
    }
    return 0;
}

char enter_pressed_once(void) {
    static int pressed_before = 0;
    // Only return 1 when not pressed before
    if (enter_pressed()) {
        if (!pressed_before) {
            _delay_ms(DEBOUNCE_TIME);
            pressed_before = 1;
            return 1;
        }
    } else {
        if (pressed_before) {
            _delay_ms(DEBOUNCE_TIME);
            pressed_before = 0;
        }
    }
    return 0;
}

void led_control(char power){
    if(power){
        PORT_LED &= ~(1<<PIN_LED);
    }
    else{
        PORT_LED |= (1<<PIN_LED);
    }
}
