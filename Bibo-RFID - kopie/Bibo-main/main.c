/*
Programming for AGV "Bibo"
Contains state machine for AGV control
 */
// -- Standard libraries -- //
#include <avr/io.h>                 // Needed for AVR programming
#include <util/delay.h>             // used for RFID scanner
// -- Custom libraries -- //
#include "TM1637.h"                 // Display
#include "config.h"                 // defines with settings
#include "basic_io.h"               // various in- and outputs
#include "music.h"                  // Buzzer control
#include "intermediate_control.h"   // Control functions from other libraries
#include "communication.h"          // communication between master and slave
#include "rfid_mfrc522.h"           // RFID


//OPERATOR (MASTER)
void start_button_init(void){
    // Set PD1 startbutton
    DDRD &= ~((1 << PD1));

    // Optional: Enable pull-up resistors startbutton
    PORTD |= (1 << PD1);
}
//starting button
uint8_t starting_button() {
    return !(PIND & (1 << PD1));
}


int main(void)
{
    /// --- Variables --- ///
    // State machine states
    enum available_states {
        test,
        wait,
        package_detected,
        forward,
        end,
        u_turn,

    };
    // State machine substates
    enum available_substates {
        entry,
        running
    };

    enum available_states current_state = test;
    enum available_substates current_substate = entry;

    int package_tag=          0;
    int package_no_tag=      0;
    char donk_mem_left = 0;
    char donk_mem_right = 0;
    int scan_time_left = 0;
    int scan_time_right = 0;
    char turn_made = 0;

    /// --- Init --- ///
    // Deze twee moet je in de main (of op de plek waar je de functie roept als dat niet in main is)
    start_button_init();
    spi_device rfid_left = {&PORTB, PB0, &DDRB};
    spi_device rfid_right = {&PORTL, PL0, &DDRL};
    init();
    USART1_init(9600);
    USART0_init(9600);
    rfid_init(rfid_left);
    rfid_init(rfid_right);

/// ===== Main loop ===== ///
    while(1){
        /// === Statemachine === ///
        switch(current_state){
        /// -- Test state -- ///
        case test:
            if(donk_detection(&donk_mem_left, &donk_mem_right)){
                display_fol();
            }
            else{
                display_pac();
            }
            break;
        /// -- Wait state -- ///
        case wait:
            if(starting_button()){
                while(!gp_timer(TEXT_DISPLAY_TIME)){
                    display_go();
                }
                current_state = forward;
            }
            break;
        /// -- Forward driving -- ///
        case forward:
            switch(current_substate){
            case entry:
                // Send forward driving command
                task_manager(forward_fast, standard_speed, standard_acceleration);
                current_substate = running;
                break;
            case running:
                // Display packages counted by category
                display_metal_and_non_metal(package_tag, package_no_tag);
                // Check sensors for packages
                if (donk_detection(&donk_mem_left, &donk_mem_right)){
                    // Transition to detection
                    current_state = package_detected;
                    current_substate = entry;
                }
                // Scan RFID tags
                if(scan_time_left){
                    if(rfid_check_tag_present(rfid_left)){
                        // Correct count
                        package_no_tag--;
                        package_tag++;
                        // Stop scanning
                        scan_time_left = 0;
                    }
                }
                if(scan_time_right){
                    if(rfid_check_tag_present(rfid_right)){
                        // Correct count
                        package_no_tag--;
                        package_tag++;
                        // Stop scanning
                        scan_time_right = 0;
                    }
                }
                // Reduce scan time
                if(gp_timer2(1)){
                    if(scan_time_left){
                        scan_time_left--;
                    }
                    if(scan_time_right){
                        scan_time_right--;
                    }
                }
                // Check for ACK to initiate U-turn
                if(USART1_receiveByte()==0x01){
                    // Transition to U-turn
                    current_state = u_turn;
                    current_substate = entry;
                }
                break;
            }
            break;
        /// -- PACKAGE DETECTED -- ///
        case package_detected:
            switch(current_substate){
            case entry:
                if(turn_made){
                    current_state = end;
                }
                else{
                    package_no_tag++;
                    // Remember to scan RFID tags while driving
                    if(donk_mem_left){
                        scan_time_left = SCAN_TIME;
                    }
                    else if(donk_mem_right){
                        scan_time_right = SCAN_TIME;
                    }
                    // Send command to stop AGV
                    task_manager(stop, standard_speed, standard_acceleration);
                    current_substate = running;
                }
                break;
            case running:
                // Display package count
                display_metal_and_non_metal(package_tag, package_no_tag);
                // Make display extra bright
                display_brightness(MAX_BRIGHTNESS);
                // Play buzzer sound
                while(play_beep()){
                    display_metal_and_non_metal(package_tag, package_no_tag);
                }
                // Remain stopped for set time
                while(!gp_timer(DETECTION_STOP_TIME)){
                    display_metal_and_non_metal(package_tag, package_no_tag);
                }

                break;
            }
            break;
        /// -- U-turn -- ///
        case u_turn:
            switch(current_substate){
            case entry:
                // Send command to initiate turn
                task_manager(turn_right, standard_speed, standard_acceleration);
                turn_made = 1;
                current_substate = running;
                break;
            case running:
                display_turn();
                if(USART1_receiveByte()==0x01){
                    current_state = forward;
                    current_substate = entry;
                }
                break;
            }
            break;
        /// -- End state -- ///
        case end: //einde bereikt
            switch(current_substate){
            case entry:
                // Send command to stop AGV
                task_manager(stop, standard_speed, standard_acceleration);
                display_end();
                // Reset timer and play song
                gp_timer(-1);
                while(play_song());
                current_substate = running;
                break;
            case running:
                display_end();
                break;
            }
            break;
        }


        //display_statemachine(current_state);
    } // end of Main loop //
    return 0;
} // end of Main //
