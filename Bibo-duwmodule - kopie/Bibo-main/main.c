/*
Programming for AGV "Bibo"
Contains state machine for AGV control
 */
// -- Standard libraries -- //
#include <avr/io.h>                 // Needed for AVR programming
// -- Custom libraries -- //
#include "TM1637.h"                 // Display
#include "config.h"                 // defines with settings
#include "basic_io.h"               // various in- and outputs
#include "music.h"                  // Buzzer control
#include "intermediate_control.h"   // Control functions from other libraries
#include "communication.h"          // communication between master and slave

int main(void)
{
    /// --- Variables --- ///
    // State machine states
    enum available_states {
        test,
        wait,
        forward,
        weight_detection,
        light_weight,
        heavy_weight,
        s_turn,
        reverse,
        end
    };
    enum available_sub_states {
        entry,
        running,
        exit
    };

    enum available_states current_state = wait;
    enum available_sub_states current_sub_state = entry;
    // First turn 0 is left, 1 is right

    /// --- Init --- ///
    init();

/// ===== Main loop ===== ///
    while(1){
        // Statemachine Switch
        switch(current_state){
        case test:
            break;
        case wait:
            if(starting_button()){
                while(!gp_timer(TEXT_DISPLAY_TIME)){
                    display_go();
                }
                current_state = forward;
            }
            break;
        case forward:
            switch(current_sub_state){
                case entry:
                    // Send forward driving command
                    task_manager(forward_fast, standard_speed, standard_acceleration);
                    current_sub_state = running;
                    gp_timer(-1);
                    break;
                case running:
                    // After set time, go slower
                    if(gp_timer(50)){
                        // Slow down
                        task_manager(forward_fast, 0x0F, standard_acceleration);
                        current_sub_state = exit;
                    }
                    break;
                case exit:
                    if(light_limit_switch_25()){
                        // Slow down further
                        task_manager(forward_fast, 0x09, standard_acceleration);
                        // Transition
                        current_state = weight_detection;
                        current_sub_state = entry;
                    }
                break;
            }
            break;
        case reverse:
            switch(current_sub_state){
                default:
                case entry:
                    // Send forward driving command
                    task_manager(forward_fast, standard_speed, standard_acceleration);
                    current_sub_state = running;
                    break;
                case running:
                    // Check for ACK at end of path
                    if(USART1_receiveByte()==0x01){
                        // Transition to end
                        current_state = end;
                        current_sub_state = entry;
                    }
                    break;
            }
            break;
        case weight_detection:
            switch(current_sub_state){
                default:
                case entry:
                    if(heavy_limit_switch_26()){
                        // Stop
                        task_manager(stop, standard_speed, standard_acceleration);
                        // Add to heavy count
                        current_sub_state = running;
                    }
                    if(gp_timer(5)){
                        // Stop
                        task_manager(stop, standard_speed, standard_acceleration);
                        // Add to light count
                        current_sub_state = running;
                    }
                    break;
                case running:
                    if(gp_timer(5)){
                        current_state = s_turn;
                        current_sub_state = entry;
                    }
                    break;
            }
            break;
        case s_turn:
            break;
        case end:
            switch(current_sub_state){
                default:
                case entry:
                    // Send command to stop AGV
                    task_manager(stop, standard_speed, standard_acceleration);
                    display_end();
                    // Reset timer and play song
                    gp_timer(-1);
                    while(play_song());
                    current_sub_state = running;
                    break;
                case running:
                    display_end();
                    break;
            }
            break;
        }
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
            //play_beep();
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
            current_state = reverse;


            break;
        case light_weight:
            display_light();
            task_manager(stop, standard_speed, standard_acceleration);
            current_state = s_turn;
            break;
        case end: //einde bereikt
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
        case reverse:
            task_manager(backward_fast, standard_speed, standard_acceleration);
            current_state = end;
            break;
        case forward:
            //task_manager(forward_fast, standard_speed, standard_acceleration);//does AGV return an ack here?
            if (light_limit_switch_25() == 0) {
                current_state = weight_detection;
            }

        default: // Should be starting_state?
            if(starting_button()){
            task_manager(forward_fast, standard_speed, standard_acceleration);//does AGV return an ack here?
            // Transition
            current_state = forward;
            }
            break;


        //display_statemachine(current_state);
    } // end of Main loop //

    return 0;
}} // end of Main //
