/*
Programming for AGV "Bibo"
Contains state machine for RFID module
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

    enum available_states current_state = wait;
    enum available_substates current_substate = entry;

    int package_tag=          0;
    int package_no_tag=      0;
    char donk_mem_left = 0;
    char donk_mem_right = 0;
    char turn_made = 0;
    char tag_seen = 0;

    /// --- Init --- ///
    // Deze twee moet je in de main (of op de plek waar je de functie roept als dat niet in main is)
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
            if(rfid_tag_detected(rfid_left)){
                display_cfg();
            }
            else{
                display_end();
            }
            break;
        /// -- Wait state -- ///
        case wait:
            display_pac();
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
                // Reset donk memory
                //donk_mem_left = 0;
                //donk_mem_right = 0;
                tag_seen = 0;
                break;
            case running:
                // Display packages counted by category
                display_metal_and_non_metal(package_tag, package_no_tag);
                // Check sensors for packages
                if (donk_detection(&donk_mem_left, &donk_mem_right)){
                    while(!gp_timer(4)){
                        if(rfid_check_tag_present(rfid_right) || rfid_check_tag_present(rfid_left)){
                            tag_seen = 1;
                        }
                    }
                    // Transition to detection
                    current_state = package_detected;
                    current_substate = entry;
                }
                // Check for ACK to initiate U-turn
                if (UCSR1A & (1 << RXC1)) {
                    if(USART1_receiveByte()==0x01){
                    // Transition to U-turn
                    current_state = u_turn;
                    current_substate = entry;
                    }
                }
                break;
            }
            break;
        /// -- PACKAGE DETECTED -- ///
        case package_detected:
            switch(current_substate){
            case entry:
                if(rfid_check_tag_present(rfid_right) || rfid_check_tag_present(rfid_left)){
                    tag_seen = 1;
                }
                if(turn_made){
                    current_state = end;
                }
                else{
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
                // Remain stopped for set time
                while(!gp_timer(DETECTION_STOP_TIME)){
                    display_metal_and_non_metal(package_tag, package_no_tag);
                    if(rfid_check_tag_present(rfid_right) || rfid_check_tag_present(rfid_left)){
                    tag_seen = 1;
                    }
                }
                // Count up and play sound
                if(tag_seen){
                   package_tag++;
                   while(play_beep());
                   }
                else{
                    package_no_tag++;
                    while(play_beep_sad());
                }
                tag_seen = 0;
                current_state = forward;
                current_substate = entry;

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
