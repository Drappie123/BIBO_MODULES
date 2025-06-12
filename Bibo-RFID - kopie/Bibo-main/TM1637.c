/*
 * Used to control a 7-segment display with a TM1637 IC
 */
#include <avr/io.h>
#include "TM1637.h"
#include "config.h"

// 7-segment digit patterns
const unsigned char digits[] = {
    0b00111111,     //"0"
    0b00000110,     //"1"
    0b01011011,     //"2"
    0b01001111,     //"3"
    0b01100110,     //"4"
    0b01101101,     //"5"
    0b01111101,     //"6"
    0b00000111,     //"7"
    0b01111111,     //"8"
    0b01101111,     //"9"
    0b00000000      // BLANK
};
// 7-segment letter patterns
const unsigned char letters[] = {
    0b01000000,     //"-" 0
    0b01110111,     //"A" 1
    0b01111100,     //"B" 2
    0b00111001,     //"C" 3
    0b01011110,     //"D" 4
    0b01111001,     //"E" 5
    0b01110001,     //"F" 6
    0b00111101,     //"G" 7
    0b01110100,     //"H" 8
    0b00000110,     //"I" 9
    0b00001110,     //"J" 10
    0b00111000,     //"L" 11
    0b00110111,     //"N" 12
    0b00111111,     //"O" 13
    0b01110011,     //"P" 14
    0b00110001,     //"r" 15
    0b01101101,     //"S" 16
    0b01111000,     //"t" 17
    0b00111110,      //"U" 18
    0b00000000      // BLANK 19
};

// Start Condition
void display_start(void){
    DDR_DIO |= (1<<PIN_DIO);
    PORT_DIO |= (1<<PIN_DIO);
    PORT_CLK |= (1<<PIN_CLK);
    PORT_DIO &= ~(1<<PIN_DIO);
}

// Stop Condition
void display_stop(void){
    DDR_DIO |= (1<<PIN_DIO);
    PORT_CLK &= ~(1<<PIN_CLK);
    PORT_CLK |= (1<<PIN_CLK);
    PORT_DIO |= (1<<PIN_DIO);
}

// Write One Byte (LSB first)
void display_sendbyte(unsigned char data){
    DDR_DIO |= (1<<PIN_DIO);
    for(int i = 0; i < 8; i++){
        PORT_CLK &= ~(1<<PIN_CLK);
        if(data & 0x01){
            PORT_DIO |= (1<<PIN_DIO);
        }
        else{
            PORT_DIO &= ~(1<<PIN_DIO);
        }
        PORT_CLK |= (1<<PIN_CLK);
        data >>= 1;
    }
    // Let IC ACK without polling
    PORT_CLK &= ~(1<<PIN_CLK);
    PORT_CLK |= (1<<PIN_CLK);
}

// Set Brightness
void display_brightness(unsigned char brightness){
    display_start();
    display_sendbyte(0x88 | (brightness & 0x07));
    display_stop();
}

// Display 4 digit integer
void display_number(int number){
    display_start();
    display_sendbyte(0x40);
    display_stop();
    // Write each digit separate
    for (int i = 0; i < 4; i++) {
        display_start();
        display_sendbyte(0xC0 + (3 - i));
        display_sendbyte(digits[number%10]);
        number /= 10;
        display_stop();
    }
}
void display_metal_and_non_metal(int metal, int non_metal){
    // Ensure values are within range 0–99
    if (metal < 0) metal = 0;
    if (metal > 99) metal = 99;
    if (non_metal < 0) non_metal = 0;
    if (non_metal > 99) non_metal = 99;

    // Start communication
    display_start();
    display_sendbyte(0x40); // Set auto-increment mode
    display_stop();

    // Combine into one 4-digit number: MMNN
    int display_digits[4];
    display_digits[0] = metal / 10;        // Tens place of metal
    display_digits[1] = metal % 10;        // Units place of metal
    display_digits[2] = non_metal / 10;    // Tens place of non_metal
    display_digits[3] = non_metal % 10;    // Units place of non_metal

    // Send digits to display
    for (int i = 0; i < 4; i++) {
        display_start();
        display_sendbyte(0xC0 + i); // Address for digit i
        display_sendbyte(digits[display_digits[i]]);
        display_stop();
    }
}
// Write a single letter
void display_letter(unsigned int letter, unsigned char position){
    display_start();
    // Send command before writing
    display_sendbyte(0xC0 + position);
    // Send letter
    display_sendbyte(letters[letter]);
    display_stop();
}



// **Initialize TM1637**
void display_init(void){
    DDR_DIO |= (1<<PIN_DIO);
    DDR_CLK |= (1<<PIN_CLK);
    PORT_DIO |= (1<<PIN_DIO);
    PORT_CLK |= (1<<PIN_CLK);
    display_brightness(NORMAL_BRIGHTNESS); // 7 is max brightness
}
// Display package or following mode
void display_mode(char mode){
    if(mode){
        display_fol();
    }
    else{
        display_pac();
    }
}
// Display left or right first turn direction
void display_direction(char dir){
    if(dir){
        display_rght();
    }
    else{
        display_left();
    }
}
// --- Various things to display --- //
void display_cfg(void){
    display_letter(19, 0);
    display_letter(3, 1);
    display_letter(6, 2);
    display_letter(7, 3);
}

void display_heavy(void){
    display_number(4);
    display_letter(1, 1);
    display_letter(5, 2);
    display_letter(8, 3);
}

void display_light(void){
    display_letter(17, 0);
    display_letter(8, 1);
    display_letter(7, 2);
    display_letter(11, 3);
}

void display_fol(void){
    display_letter(19, 0);
    display_letter(6, 1);
    display_letter(13, 2);
    display_letter(11, 3);
}

void display_cnt(void){
    display_letter(19, 0);
    display_letter(3, 1);
    display_letter(12, 2);
    display_letter(17, 3);
}

void display_turn(void){
    display_letter(17, 0);
    display_letter(18, 1);
    display_letter(15, 2);
    display_letter(12, 3);
}

void display_dist(void){
    display_letter(4, 0);
    display_letter(9, 1);
    display_letter(16, 2);
    display_letter(17, 3);
}

void display_pac(void){
    display_letter(19, 0);
    display_letter(14, 1);
    display_letter(1, 2);
    display_letter(3, 3);
}

void display_left(void){
    display_letter(11, 0);
    display_letter(5, 1);
    display_letter(6, 2);
    display_letter(17, 3);
}

void display_rght(void){
    display_letter(15, 0);
    display_letter(7, 1);
    display_letter(8, 2);
    display_letter(17, 3);
}

void display_go(void){
    display_letter(19, 0);
    display_letter(19, 1);
    display_letter(7, 2);
    display_letter(13, 3);
}

void display_end(void){
    display_letter(19, 0);
    display_letter(5, 1);
    display_letter(12, 2);
    display_letter(4, 3);
}

void display_srch(void){
    display_letter(16, 0);
    display_letter(15, 1);
    display_letter(3, 2);
    display_letter(8, 3);
}

void display_txt_stop(void){
    display_letter(16, 0);
    display_letter(17, 1);
    display_letter(13, 2);
    display_letter(14, 3);
}

