/*
 * Library written to control 2 steppers with variable speed, steering and speed ramping
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "stepper.h"
#include "config.h"
// --- Variables --- //
// Global variables needed for handling in ISR
volatile int step_count_left = 0;
volatile int step_count_right = 0;
volatile int freq_target_left = 0xFF;
volatile int freq_target_right = 0xFF;
volatile char direction_change_left = 0;
volatile char direction_change_right = 0;
/// --- Step counting --- ///
// Left stepper
ISR(TIMER0_OVF_vect){
    // Count down till 0
    if(step_count_left > 0){
        step_count_left--;
    }
    // Stepper does not stop at count 0, only sets count to -1 as indication
    else if(step_count_left == 0){
        step_count_left = -1;
        step_count_right = -1;
        // Disable interupts
        TIMSK0 &= ~(1<<TOIE0);
        TIMSK2 &= ~(1<<TOIE2);
    }
}
// Right stepper
ISR(TIMER2_OVF_vect){
    // Count down till 0
    if(step_count_right > 0){
        step_count_right--;
    }
    // Stepper does not stop at count 0, only sets count to -1 as indication
    else if(step_count_right == 0){
        step_count_left = -1;
        step_count_right = -1;
        // Disable interupts
        TIMSK0 &= ~(1<<TOIE0);
        TIMSK2 &= ~(1<<TOIE2);
    }
}
/// --- Ramping --- ///
//uses timer from HC-SR04
ISR(TIMER5_OVF_vect){
    static int count1 = 0;
    static int count2 = 0;
    // --- left stepper --- //
    // Direction change ramping
    if(direction_change_left){
        // Slow down
        if (OCR0A < 0xFF) {
            // Prevent overshooting
            if (OCR0A + RAMPING_SPEED > 0xFF) {
                OCR0A = 0xFF;  // Directly set it to target to avoid overshooting
            }
            else {
                OCR0A += RAMPING_SPEED;  // Increment by step
            }
        }
        // Change direction or stop
        else{
            // Disable stepper
            PORT_ST1_EN |= (1<<PIN_ST1_EN);
            if(direction_change_left == 3){
                // Direction backward (inverted)
                PORT_ST1_DIR &= ~(1<<PIN_ST1_DIR);
            }
            else if(direction_change_left == 1){
                // Direction forward (inverted)
                PORT_ST1_DIR |= (1<<PIN_ST1_DIR);
            }
            // Stop for a short period
            count1++;
            if(count1 > 3){
                // start ramping up again if set speed != 0
                if(direction_change_left != 2){
                    // Enable stepper
                    PORT_ST1_EN &= ~(1<<PIN_ST1_EN);
                    direction_change_left = 0;
                }
                count1 = 0;
            }
        }
    }
    // Normal ramping
    else{
        // Enable stepper
        PORT_ST1_EN &= ~(1<<PIN_ST1_EN);
        if (OCR0A < freq_target_left) {
            // Prevent overshooting
            if (OCR0A + RAMPING_SPEED > freq_target_left) {
                OCR0A = freq_target_left;  // Directly set it to target to avoid overshooting
            }
            else {
                OCR0A += RAMPING_SPEED;  // Increment by step
            }
        }
        else if (OCR0A > freq_target_left) {
            // Prevent overshooting
            if (OCR0A - RAMPING_SPEED < freq_target_left) {
                OCR0A = freq_target_left;  // Directly set it to target to avoid overshooting
            }
            else {
                OCR0A -= RAMPING_SPEED;  // Decrement by step
            }
        }
    }
    // --- Right stepper --- ///
    // Direction change ramping
    if(direction_change_right){
        // Slow down
        if (OCR2A < 0xFF) {
            // Prevent overshooting
            if (OCR2A + RAMPING_SPEED > 0xFF) {
                OCR2A = 0xFF;  // Directly set it to target to avoid overshooting
            }
            else {
                OCR2A += RAMPING_SPEED;  // Increment by step
            }
        }
        // Change direction or stop
        else{
            // Stepper disable
            PORT_ST2_EN |= (1<<PIN_ST2_EN);
            if(direction_change_right == 1){
                // Backward direction
                PORT_ST2_DIR &= ~(1<<PIN_ST2_DIR);
            }
            else if(direction_change_right == 3){
                // Forward direction
                PORT_ST2_DIR |= (1<<PIN_ST2_DIR);
            }
            count2++;
            if(count2 > 3){
                // start ramping up again if set speed != 0
                if(direction_change_right != 2){
                    // Enable stepper
                    PORT_ST2_EN &= ~(1<<PIN_ST2_EN);
                    direction_change_right = 0;
                }
                count2 = 0;
            }
        }
    }
    // Normal ramping
    else{
        // Enable stepper
        PORT_ST2_EN &= ~(1<<PIN_ST2_EN);
        if (OCR2A < freq_target_right) {
            // Prevent overshooting
            if (OCR2A + RAMPING_SPEED > freq_target_right) {
                OCR2A = freq_target_right;  // Directly set it to target to avoid overshooting
            }
            else {
                OCR2A += RAMPING_SPEED;  // Increment by step
                }
        }
        else if (OCR2A > freq_target_right) {
        // Prevent overshooting
            if (OCR2A - RAMPING_SPEED < freq_target_right) {
                OCR2A = freq_target_right;  // Directly set it to target to avoid overshooting
            }
            else {
                OCR2A -= RAMPING_SPEED;  // Decrement by step
            }
        }
    }
}
// Init
void stepper_init(void){
    // Steppers set to 1/16th step (MS1/2/3 on)
    // Make stepper pins output
    DDR_ST1_STEP |= (1<<PIN_ST1_STEP);
    DDR_ST1_EN |= (1<<PIN_ST1_EN);
    DDR_ST1_DIR |= (1<<PIN_ST1_DIR);
    DDR_ST2_STEP |= (1<<PIN_ST2_STEP);
    DDR_ST2_EN |= (1<<PIN_ST2_EN);
    DDR_ST2_DIR |= (1<<PIN_ST2_DIR);
    // Disable steppers
    PORT_ST1_EN |= (1<<PIN_ST1_EN);
    PORT_ST2_EN |= (1<<PIN_ST2_EN);
    // Set direction forward
    PORT_ST1_DIR |= (1<<PIN_ST1_DIR);
    PORT_ST2_DIR |= (1<<PIN_ST2_DIR);

    // Set frequency to max
    OCR2A = 0xFF;
    OCR0A = 0xFF;
    // Set step signal
    OCR0B = 1;
    OCR2B = 1;
    // Fast PWM mode 7, prescaler 256, non inverting mode, TOV interrupts enable
    TCCR0A = (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
    TCCR0B = (1<<WGM02) | (1<<CS02) | (0<<CS01) | (0<<CS00);
    TCCR2A = (1<<COM2B1) | (1<<WGM21) | (1<<WGM20);
    TCCR2B = (1<<WGM22) | (1<<CS22) | (1<<CS21) | (0<<CS20);
    stepper_control(0, 0);
}
// --- Speed control --- //
void stepper_speed_left(int speed){
    // Determine direction changes
    static int previous_speed;
    if(previous_speed <= 0 && speed > 0){
        direction_change_left = 1; // backward
    }
    else if(previous_speed >= 0 && speed < 0){
        direction_change_left = 3; // forward
    }
    else if(speed == 0){
        direction_change_left = 2; // stop
    }
    previous_speed = speed;
    // Calculate frequency target
    if(speed){
        speed = abs(speed);
        freq_target_left = (STEPPER_MIN_FREQ + (STEPPER_FREQ_RANGE/abs(speed)));
    }
}
void stepper_speed_right(int speed){
    // Determine direction changes
    static int previous_speed;
    if(previous_speed <= 0 && speed > 0){
        direction_change_right = 1; // Backward
    }
    else if(previous_speed >= 0 && speed < 0){
        direction_change_right = 3; // Forward
    }
    else if(speed == 0){
        direction_change_right = 2; // Stop
    }
    previous_speed = speed;
    // Calculate target frequency
    if(speed){
        freq_target_right = (STEPPER_MIN_FREQ + (STEPPER_FREQ_RANGE/abs(speed)));
    }
}
// --- Stepper steering with speed control --- //
void stepper_control(int speed, int steer){
    // Give a steering boost to
    //steer = steer * (( (15.0)/(speed+1) )+1); non linear steering, probably not needed
    // Clamp speed and steer to <-100; 100>
    speed = CLAMP(speed, -100, 100);
    steer = CLAMP(steer, -100, 100);
    // Steering while driving
    if(speed > 0 || speed < 0){
    // No steer, go straight
        if(steer == 0){
            stepper_speed_left(speed);
            stepper_speed_right(speed);
        }
        // Go left
        else if(steer < 0){
            stepper_speed_left(speed-((abs(steer)*speed)/100));
            stepper_speed_right(speed);
        }
        // Go right
        else if(steer > 0){
            stepper_speed_left(speed);
            stepper_speed_right(speed-((steer*speed)/100));
        }
    }
    // Steering in place
    else if(speed == 0){
        stepper_speed_left(steer);
        stepper_speed_right(-steer);
    }
    // Disable step control
    step_count_left = -1;
    step_count_right = -1;
}
// --- Stepper control with step counting --- //
void stepper_step_control(int speed, int steer, unsigned int steps){
    // Enable interrupts for counting
    TIMSK0 = (1<<TOIE0);
    TIMSK2 = (1<<TOIE2);
    // Set speed and direction
    stepper_control(speed, steer);
    // Set steps, fastest stepper determines end of counting
    step_count_left = steps;
    step_count_right = steps;
}
