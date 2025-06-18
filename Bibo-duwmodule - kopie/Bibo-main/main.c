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
    // Store the first package detection
    char first_package_detected = 0;
    // Store number of detected packages
    int light_packages = 0;
    int heavy_packages = 0;
    int forward_timer = INITIAL_FORWARD_TIME;

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
                    // Show package count
                    display_metal_and_non_metal(heavy_packages, light_packages);
                    // After set time, go slower
                    if(gp_timer(forward_timer)){
                        // Slow down
                        task_manager(forward_slow, 0x0F, standard_acceleration);
                        current_sub_state = exit;
                    }
                    // Check limit switch in case it comes before the slowdown
                    if(light_limit_switch_25()){
                        // Transition
                        current_state = weight_detection;
                        current_sub_state = entry;
                    }
                    break;
                case exit:
                    // Show package count
                    display_metal_and_non_metal(heavy_packages, light_packages);
                    // Check limit switch for package
                    if(light_limit_switch_25()){
                        // Slow down further
                        task_manager(forward_slow, 0x09, standard_acceleration);
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
                    task_manager(backward_fast, standard_speed, standard_acceleration);
                    current_sub_state = running;
                    break;
                case running:
                    // Show package count
                    display_metal_and_non_metal(heavy_packages, light_packages);
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
                case entry:
                    // Go extra slow
                    task_manager(forward_slow, 0x09, standard_acceleration);
                    current_sub_state = running;
                    break;
                case running:
                    // Show package count
                    display_metal_and_non_metal(heavy_packages, light_packages);
                    // Check switches
                    if(heavy_limit_switch_26()){
                        // Stop
                        task_manager(stop, standard_speed, standard_acceleration);
                        heavy_packages++;
                        current_sub_state = exit;
                    }
                    if(gp_timer(5)){
                        // Stop
                        task_manager(stop, standard_speed, standard_acceleration);
                        light_packages++;
                        current_sub_state = exit;
                    }
                    break;
                case exit:
                    // Show package count
                    display_metal_and_non_metal(heavy_packages, light_packages);
                    // Transition after set time
                    if(gp_timer(5)){
                        if(first_package_detected){
                            current_state = reverse;
                            current_sub_state = entry;
                        }
                        else{
                            first_package_detected = 1;
                            current_state = s_turn;
                            current_sub_state = entry;
                        }
                    }
                    break;
            }
            break;
        case s_turn:
            switch(current_sub_state){
                default:
                case entry:
                    task_manager(stop, standard_speed, standard_acceleration);
                    current_sub_state = running;
                    break;
                case running:
                    display_turn();
                    // Check for ACK at end s-turn
                    if(USART1_receiveByte()==0x01){
                        // Set forward timer to shorter interval
                        forward_timer = SECOND_FORWARD_TIME;
                        // Transition to weight detection
                        current_state = forward;
                        current_sub_state = entry;
                    }
                    break;
            }
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
        } // end of statemachine switch
    } // end of Main loop //

    return 0;
} // end of Main //
