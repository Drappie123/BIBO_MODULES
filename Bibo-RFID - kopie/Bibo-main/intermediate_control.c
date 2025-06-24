/*
 * Functions to help control the functions from the other libraries
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "intermediate_control.h"
#include "config.h"
#include "stepper.h"
#include "HC-SR04.h"
#include "basic_io.h"
#include "music.h"
#include "TM1637.h"
// --- Init all --- //
void init(void){
    basic_io_init();
    display_init();
    itermediate_control_init();
    music_init();
    sei();
}
// --- Init ic functions --- //
void itermediate_control_init(void){
    init_gp_timer();
    init_gp_timer2();
}
// --- Init general purpose timer --- //
void init_gp_timer(void){
    // Mode 4 CTC, prescaler 64
    TCCR1A = 0;
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
    // OCR1A is top in mode 4
    OCR1A = GP_TIMER_TOP;
}
void init_gp_timer2(void){
    // Mode 4 CTC, prescaler 64
    TCCR5A = 0;
    TCCR5B = (1<<WGM52) | (1<<CS51) | (1<<CS50);
    // OCR1A is top in mode 4
    OCR5A = GP_TIMER_TOP;
}
// --- Genral purpose timer --- //
// Timers only returns 1 after it initially reaches 0. Then it can be set again
char gp_timer(int set_value){
    static int count = -1;
    // Reset timer
    if(set_value == -1){
        count = -1;
    }
    // Set value if not running
    if(count == -1){
        count = set_value;
        TIFR1 = (1<<OCF1A);
        TCNT1 = 0;
    }
    // If running, count down on flag
    else if(count > 0){
        if(TIFR1 & (1<<OCF1A)){
            TIFR1 = (1<<OCF1A);
            count--;
        }
    }
    // If 0 is reached, return 1
    else if(count == 0){
        count = -1;
        return 1;
    }
    return 0;
}
char gp_timer2(int set_value){
    static int count = -1;
    // Reset timer
    if(set_value == -1){
        count = -1;
    }
    // Set value if not running
    if(count == -1){
        count = set_value;
        TIFR5 = (1<<OCF5A);
        TCNT5 = 0;
    }
    // If running, count down on flag
    else if(count > 0){
        if(TIFR5 & (1<<OCF5A)){
            TIFR5 = (1<<OCF5A);
            count--;
        }
    }
    // If 0 is reached, return 1
    else if(count == 0){
        count = -1;
        return 1;
    }
    return 0;
}
// --- detect packages --- //
char donk_detection(char *mem_left, char *mem_right){
    // Mem can only be reset, not set
    if(!donk_left()){
        *mem_left = 0;
    }
    if(!donk_right()){
        *mem_right = 0;
    }
    // Return 1 if detected and not detected before in previous cycle
     if((donk_left() && !*mem_left) ||(donk_right() && !*mem_right)){
        //*mem_left = donk_left();
        //*mem_right = donk_right();
        return 1;
     }
    return 0;
}
