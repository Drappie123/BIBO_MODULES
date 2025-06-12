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
        wait,
        // Navigeren + tellen
        package_detected,
        forward,
        //new code
        einde_opdracht,
        u_turn,
        backward_driving
    };

    enum available_states current_state = wait;
    int package_metal=          0;
    int package_non_metal=      0;
    int change_demo_package=    4;
    char donk_mem_left = 0;
    char donk_mem_right = 0;

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
        //display_end();
        // Statemachine Switch
        switch(current_state){
        case wait: // Should this be starting_state?
            if(donk_left() || donk_right()){
                display_fol();
            }
            else{
                display_pac();
            }
            //display_go();
            //USART1_sendCommand(0xFF, 0xAA, 0x11, 0xDD);
            //_delay_ms(5000);
            if(starting_button()){
            task_manager(forward_fast, standard_speed, standard_acceleration);
            current_state = forward;
            display_end();
            }//does AGV return an ack here?
            break;
        /// --- PACKAGE DETECTED --- ///
        case package_detected:
            // AGV should be stopped
            // Reset GP timer
            gp_timer(-1);
            if(rfid_tag_detected(rfid_right)){
                // Add 1 to package count
                package_metal++;
            }
            else{
                if(rfid_tag_detected(rfid_left)){
                // Add 1 to package count
                package_metal++;
                }
                else{
                package_non_metal++;
            }
            }
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
                // Stop AGV
                task_manager(stop, standard_speed, standard_acceleration);
                // Play sound and display end
                display_end();
                while(play_beep());
                current_state = einde_opdracht;
            }
            else{
                // Start moving
                task_manager(forward_fast, standard_speed, standard_acceleration);
                // Transition
                current_state = forward;
            }
            break;
        case einde_opdracht: //einde bereikt
            // AGV is stopped
            display_end();
            break;
        case backward_driving:
            task_manager(backward_fast, standard_speed, standard_acceleration);
            current_state = einde_opdracht;
            break;
        case forward:
            display_metal_and_non_metal(package_metal, package_non_metal);
            //task_manager(forward_fast, standard_speed, standard_acceleration);//does AGV return an ack here?
            //while driving look for packages
            //_delay_ms(3000);
            if (donk_left() || donk_right()){
                // Stop
                task_manager(stop, standard_speed, standard_acceleration);
                // Transition
                display_cfg();
                current_state = package_detected;
            }
            //listing for an ACK to know when to make u_turn, this only happens once
            if(USART1_receiveByte()==0x01){
                task_manager(turn_right, standard_speed, standard_acceleration);
                current_state = u_turn;
            }
            break;
        case u_turn:
            display_turn();
            if(USART1_receiveByte()==0x01){
                task_manager(forward_fast, standard_speed, standard_acceleration);
                current_state = forward;
            }
            break;
        }


        //display_statemachine(current_state);
    } // end of Main loop //

    return 0;
} // end of Main //
