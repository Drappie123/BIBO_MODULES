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
    ultrasonic_init();
    display_init();
    stepper_init();
    itermediate_control_init();
    music_init();
    sei();
}
// --- Init ic functions --- //
void itermediate_control_init(void){
    init_gp_timer();
}
// --- Init general purpose timer --- //
void init_gp_timer(void){
    // Mode 4 CTC, prescaler 64
    TCCR1A = 0;
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
    // OCR1A is top in mode 4
    OCR1A = GP_TIMER_TOP;
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
// --- detect path walls --- //
char wall_detected(void){
    static int last_detection = 0;
    int left;
    int right;
    left = get_left_distance();
    right = get_right_distance();
    if(gp_timer(SENSOR_TIMING)){
        // Check for detections within set range
        if(left >= WALL_MIN_RANGE && left <= WALL_MAX_RANGE && right >= WALL_MIN_RANGE && right <= WALL_MAX_RANGE){
            if(last_detection >= PATH_SENSITIVITY){
                last_detection = 0;
                // Reset timer
                gp_timer(-1);
                return 1;
            }
            last_detection++;
        }
        else{
            last_detection = 0;
        }
    }
    return 0;
}
// Check if AGV is still in path
char check_path_end(void){
    static int last_detection = 0;
    if(gp_timer(SENSOR_TIMING)){
        if(get_left_distance() > WALL_MAX_RANGE && get_right_distance() > WALL_MAX_RANGE){
            if(last_detection >= PATH_SENSITIVITY){
                last_detection = 0;
                // Reset timer
                gp_timer(-1);
                return 1;
            }
            last_detection++;
        }
        else{
            last_detection = 0;
        }
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
        *mem_left = donk_left();
        *mem_right = donk_right();
        return 1;
     }
    return 0;
}
// --- Steering and speed control inside path --- //
void path_steering(void){
    static int steering_error = 0;
    int left;
    int right;
    left = get_left_distance();
    right = get_right_distance();
    // - Error calculation - //
    // Calculate error, if both are readings are within range, use both for error calc
    if(left <= ERROR_MAX_DISTANCE && right <= ERROR_MAX_DISTANCE){
        steering_error = (right - left);
    }
    // Use left sensor if it is the only one in range
    else if (left <= ERROR_MAX_DISTANCE){
        steering_error = -2*(left - ERROR_CENTER_DISTANCE);
    }
    // Use right reading if it is the only one in range
    else if(right <= ERROR_MAX_DISTANCE){
        steering_error = 2*(right - ERROR_CENTER_DISTANCE);
    }
    // - Steering - //
    // Go straight if error is within allowed range
    if((steering_error >= -ALLOWED_STEERING_ERROR) && (steering_error <= ALLOWED_STEERING_ERROR)){
        stepper_control(CRUISE_SPEED, 0);
    }
    // Make steering corrections proportional to error
    else{
        stepper_control(CRUISE_SPEED, steering_error*STEERING_MULTIPLIER);
    }
}
// --- Steering and speed control when following person --- //
char person_steering(int fol_dist){
    static int left;
    static int right;
    static int distance;
    static char previous_limit_reached;
    static int error;
    static int speed;
    left = get_left_distance();
    right = get_right_distance();
    error = left - right;
    // Go straight when person is in the middle
    if(error < FOLLOW_DISTANCE_MARGIN && error > -FOLLOW_DISTANCE_MARGIN){
        // Get average distance
        distance = (left + right) / 2;
        // Drive straight
        stepper_control(speed, 0);
        display_number(distance);
    }
    // Go to the right when person is on the right
    else if(error >= FOLLOW_DISTANCE_MARGIN){
        // Slower steering when stationary
        if(speed == 0){
            stepper_control(0, 6);
        }
        else{
            stepper_control(speed, FOLLOW_STEERING);
        }
        distance = right;
        display_rght();
    }
    // Go to the left when person is on the left
    else{
        // Slower steering when stationary
        if(speed == 0){
            stepper_control(0, -6);
        }
        else{
           stepper_control(speed, -FOLLOW_STEERING);
        }
        distance = left;
        display_left();
    }
    // Calculate speed
    speed = CLAMP((distance - fol_dist)*FOLLOW_SPEED_MULTIPLIER, 0, 100);
    // Check if person is still within range
    if(gp_timer(SENSOR_TIMING)){
        // Person needs to be out of range for set time
        if(left > fol_dist + FOLLOW_DISTANCE_LIMIT && right > fol_dist + FOLLOW_DISTANCE_LIMIT){
            if(previous_limit_reached >= PERSON_SENSITIVITY){
                previous_limit_reached = 0;
                // reset timer
                gp_timer(-1);
                return 1;
            }
            previous_limit_reached++;
        }
        else{
            previous_limit_reached = 0;
        }
    }
    return 0;
}

// First turn user entry
void set_first_turn(char *turn){
    if(plus_pressed_once() || minus_pressed_once()){
                    (*turn)++;
                }
                if(*turn > 1){
                    *turn = 0;
                }
}
// Package count limit user entry
void set_package_count(int *pac_lim){
    if(plus_pressed_once() && *pac_lim < PACKAGE_COUNT_MAX){
        (*pac_lim)++;
    }
    if(minus_pressed_once() && *pac_lim > PACKAGE_COUNT_MIN){
        (*pac_lim)--;
    }
}
// Following distance user entry
void set_follow_distance(int *dist){
    if(plus_pressed_once() && *dist < FOLLOW_DISTANCE_MAX){
        (*dist)+= FOLLOW_DISTANCE_STEP;
    }
    if(minus_pressed_once() && *dist > FOLLOW_DISTANCE_MIN){
        (*dist)-= FOLLOW_DISTANCE_STEP;
    }
}

// Detect person based on front distances
char person_detected(int fol_dist){
    static int left;
    static int right;
    static int last_detection = 0;
    left = get_left_distance();
    right = get_right_distance();
    // Person needs to be in range for set time
    if(gp_timer(SENSOR_TIMING)){
        if(left < fol_dist + FOLLOW_DETECTION_MARGIN && right < fol_dist + FOLLOW_DETECTION_MARGIN){
            if(last_detection >= PERSON_SENSITIVITY){
                last_detection = 0;
                // Reset timer
                gp_timer(-1);
                return 1;
            }
            last_detection++;
        }
        else{
            last_detection = 0;
        }
    }
    return 0;
}
// Check if step count has ended before new command
char step_count_end(void){
    if(step_count_left == -1 && step_count_right == -1){
        return 1;
    }
    return 0;
}

void sensor_turn(char direction){
    static int distance;
    static int turn_ratio;
    if(direction){
        distance = get_right_distance();
        turn_ratio = 1.2*CLAMP(distance - TURN_WALL_DISTANCE, -TURNING_RATIO, TURNING_RATIO);
    }
    else{
        distance = get_left_distance();
        turn_ratio = -1.2*CLAMP(distance - TURN_WALL_DISTANCE, -TURNING_RATIO, TURNING_RATIO);
    }
    stepper_control(TURNING_SPEED, turn_ratio);
}
