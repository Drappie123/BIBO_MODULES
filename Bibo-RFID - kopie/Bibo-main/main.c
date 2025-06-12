/*
Programming for AGV "Bibo"
Contains state machine for AGV control
 */
// -- Standard libraries -- //
#include <avr/io.h>                 // Needed for AVR programming
#include <util/delay.h>             // used for RFID scanner
// -- Custom libraries -- //
#include "TM1637.h"                 // Display
#include "HC-SR04.h"                // Ultrasonic sensors
#include "stepper.h"                // motor control
#include "config.h"                 // defines with settings
#include "basic_io.h"               // various in- and outputs
#include "music.h"                  // Buzzer control
#include "intermediate_control.h"   // Control functions from other libraries
#include "communication.h"          // communication between master and slave
#include "rfid_mfrc522.h"           // RFID


//OPERATOR (MASTER)


int main(void)
{
    /// --- Variables --- ///
    // State machine states
    enum available_states {
        STARTING_STATE,
        // Navigeren + tellen
        package_detected,
        forward,
        //new code
        einde_opdracht,
        s_turn,
        backward_driving
    };

    enum available_states current_state =       STARTING_STATE;
    int starting_button=        0;
    int package_metal=          0;
    int package_non_metal=      0;
    int change_demo_package=    4;
    int package_detected_donk=  0;

    /// --- Init --- ///
    // Deze twee moet je in de main (of op de plek waar je de functie roept als dat niet in main is)
    spi_device rfid_left = {&PORTB, PB0, &DDRB};
    spi_device rfid_right = {&PORTL, PL0, &DDRL};
    init();
    rfid_init(rfid_left);
    rfid_init(rfid_right);

/// ===== Main loop ===== ///
    while(1){
        // Statemachine Switch
        switch(current_state){
        /// --- PACKAGE DETECTED --- ///

        case package_detected:
            // Stop
            task_manager(stop, standard_speed, standard_acceleration);
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
            else{
                current_state = forward;
            }
            break;
        case einde_opdracht: //einde bereikt
            task_manager(stop, standard_speed, standard_acceleration);
            display_end();
            play_beep();
            break;
        case backward_driving:
            task_manager(backward_fast, standard_speed, standard_acceleration);
            current_state = einde_opdracht;
            break;
        case forward:
            task_manager(forward_fast, standard_speed, standard_acceleration);//does AGV return an ack here?
            //while driving look for packages
            if (package_detected_donk){
                current_state = package_detected;
            }
            //listing for an ACK to know when to make u_turn, this only happens once
            if(USART1_receiveByte()==0x01){
                display_turn();
                task_manager(turn_right, standard_speed, standard_acceleration);
            }
        default:
            if(starting_button==1){
            task_manager(forward_fast, standard_speed, standard_acceleration);}//does AGV return an ack here?
            break;


        //display_statemachine(current_state);
    } // end of Main loop //

    return 0;
}} // end of Main //
