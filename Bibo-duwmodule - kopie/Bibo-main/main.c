/*
Programming for AGV "Bibo"
Contains state machine for AGV control
 */
// -- Standard libraries -- //
#include <avr/io.h>                 // Needed for AVR programming
// -- Custom libraries -- //
#include "TM1637.h"                 // Display
#include "HC-SR04.h"                // Ultrasonic sensors
#include "stepper.h"                // motor control
#include "config.h"                 // defines with settings
#include "basic_io.h"               // various in- and outputs
#include "music.h"                  // Buzzer control
#include "intermediate_control.h"   // Control functions from other libraries
#include "communication.h"          // communication between master and slave

//OPERATOR (MASTER)
/// --- limit_switches---///
void init_limit_switches() {
    // Set PD0 and PD1 (pins 25, 26) as input
    DDRD &= ~((1 << PA3) | (1 << PA4));

    // Optional: Enable pull-up resistors
    PORTD |= (1 << PA3) | (1 << PA4);
}

uint8_t light_limit_switch_25() {
    // Returns 0 if pressed (connected to GND), 1 if not pressed
    return (PINA & (1 << PA3)) ? 1 : 0;
}

uint8_t heavy_limit_switch_26() {
    return (PINA & (1 << PA2)) ? 1 : 0;
}


int main(void)
{
    /// --- Variables --- ///
    // State machine states
    enum available_states {
        STARTING_STATE,
        // Navigeren + tellen
        forward,
        //new code
        einde_opdracht,
        weight_detection,
        light_weight,
        heavy_weight,
        s_turn,
        backward_driving
    };

    enum available_states current_state =       STARTING_STATE;
    int starting_button=        0;
    // First turn 0 is left, 1 is right

    /// --- Init --- ///
    init();

/// ===== Main loop ===== ///
    while(1){
        // Statemachine Switch
        switch(current_state){
        /// --- PACKAGE DETECTED --- ///
        /*
        case package_detected:
            // Stop
            task_manager(stop, standard_speed, standard_acceleration);
            // Reset GP timer
            gp_timer(-1);

            if(RFID){// DEZE GAAT QUENTIN DOEN
                // Add 1 to package count
                package_metal++;
            }
            else{
                package_non_metal++;
            }
            // Turn LEDs on
            led_control(1);
            // Make display extra bright
            display_brightness(MAX_BRIGHTNESS);
            // Play buzzer sound
            while(play_beep()){
                display_metal_and_non_metal(package_metal, package_non_metal);
            }
            // Remain stopped for set time
            while(!gp_timer(DETECTION_STOP_TIME)){
                display_metal_and_non_metal(package_metal, package_non_metal);
            }
            //THIS is for the change_demo_area. at package 5 for non metal
            if (package_non_metal==change_demo_package)
            {
                current_state = einde_opdracht;
            }
            break;*/
        case weight_detection:
            play_beep();
            task_manager(forward_slow, standard_speed, standard_acceleration);
            if (light_limit_switch_25() == 0) {
                while(!gp_timer(DETECTION_STOP_TIME)){
                    task_manager(forward_slow, standard_speed, standard_acceleration);
            }
                if ((light_limit_switch_25() == 0) &! (heavy_limit_switch_26() == 0)){
                    current_state = light_weight;}
                else{
                    current_state = heavy_weight;}}


            break;
        case heavy_weight:
            while(!gp_timer(DETECTION_STOP_TIME)){
                    display_heavy();}
            task_manager(stop, standard_speed, standard_acceleration);//Might need to be in timer, idk what the stop function does
            current_state = backward_driving;


            break;
        case light_weight:
            display_light();
            task_manager(stop, standard_speed, standard_acceleration);
            current_state = s_turn;
            break;
        case einde_opdracht: //einde bereikt
            task_manager(stop, standard_speed, standard_acceleration);
            display_end();
            play_beep();
            break;
        case s_turn:
            //maak S-bocht
            //ja idk man

            //als S-bocht klaar, AGV weer op stand 1
            current_state = forward;//Deze kunnen we ook gwn programmering in s bocht zelf want verder wordt deze state niet gebruikt
            break;
        case backward_driving:
            task_manager(backward_fast, standard_speed, standard_acceleration);
            current_state = einde_opdracht;
            break;
        case forward:
            task_manager(forward_fast, standard_speed, standard_acceleration);//does AGV return an ack here?
            if (light_limit_switch_25() == 0) {
                current_state = weight_detection;
            }

        default:
            if(starting_button==1){
            task_manager(forward_fast, standard_speed, standard_acceleration);//does AGV return an ack here?
                if (light_limit_switch_25() == 0) {
                        current_state = weight_detection;
            }}
            break;


        //display_statemachine(current_state);
    } // end of Main loop //

    return 0;
}} // end of Main //
