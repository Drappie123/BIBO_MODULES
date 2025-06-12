/*
 * Functions for making beeps with a buzzer, allows programming of simple songs
 */
#include <avr/io.h>
#include "music.h"
#include "intermediate_control.h"

void music_init(void){
    // Start as high input to prevent noise from voltage fluctuations
    // Set to output in separate function
    DDR_BUZZ &= ~(1<<PIN_BUZZ);
    PORT_BUZZ |= (1<<PIN_BUZZ);
    // OC3C Mode 14, Fast PWM, prescaler 1
    // COM bits are set in separate function
    TCCR3A = (1<<WGM31);
    TCCR3B = (1<<CS30) | (1<<WGM33) | (1<<WGM32);
    OCR3C = 65535*0.02;
}
// Set buzzer to output and enable COM bits for PWM
void buzzer_enable(char input){
    if(input){
        DDR_BUZZ |= (1<<PIN_BUZZ);
        TCCR3A |= (1<<COM3C1) | (1<<COM3C0);
    }
    else{
        DDR_BUZZ &= ~(1<<PIN_BUZZ);
        TCCR3A &= ~((1<<COM3C1) | (1<<COM3C0));
    }
}

int hz_to_icr_value(int hz){
    return (16000000/hz);
}

void play_note(int note, int beats, int state, int *note_num){
    // Use GP timer to time beats
    if(!gp_timer(beats)){
        // Set frequency and volume
        int ICR_value = (hz_to_icr_value(note));
        int OCR_value = 65535*0.01;
        ICR3 =  ICR_value;
        OCR3C = OCR_value*state;
    }
    // Next note
    else{
        (*note_num)++;
    }
}
// Play multiple notes with switch
char play_beep(){
    static int note_number = 0;
    if(!note_number){
        buzzer_enable(1);
    }
    switch(note_number){
    case 0:
        play_note(NOTE_5, 1, 10, &note_number);
        break;
    case 1:
        play_note(NOTE_3, 1, 0, &note_number);
        break;
    case 2:
        play_note(NOTE_1, 1, 10, &note_number);
        break;
    case 3:
        play_note(NOTE_8, 1, 10, &note_number);
        break;
    case 4:
        play_note(NOTE_8, 1, 0, &note_number);
        break;
    case 5:
        buzzer_enable(0);
        note_number = 0;
        return 0;
        break;
    }
    return 1;
}
// Slight variation on beep
char play_beep_sad(){
    static int note_number = 0;
    if(!note_number){
        buzzer_enable(1);
    }
    switch(note_number){
    case 0:
        play_note(NOTE_3, 3, 1, &note_number);
        break;
    case 1:
        play_note(NOTE_3, 2, 0, &note_number);
        break;
    case 2:
        play_note(NOTE_2, 3, 1, &note_number);
        break;
    case 3:
        play_note(NOTE_1, 7, 1, &note_number);
        break;
    case 4:
        play_note(NOTE_8, 30, 0, &note_number);
        break;
    case 5:
        buzzer_enable(0);
        note_number = 0;
        return 0;
        break;
    }
    return 1;
}
// Sounds like old nokia
char play_song(){
    static int note_number = 0;
    if(!note_number){
        buzzer_enable(1);
    }
    switch(note_number){
    case 0:
        play_note(NOTE_8, 1, 1, &note_number);
        break;
    case 1:
        play_note(NOTE_7, 1, 1, &note_number);
        break;
    case 2:
        play_note(NOTE_3, 2, 1, &note_number);
        break;
    case 3:
        play_note(NOTE_4, 2, 1, &note_number);
        break;
    case 4:
        play_note(NOTE_7, 1, 1, &note_number);
        break;
    case 5:
        play_note(NOTE_6, 1, 1, &note_number);
        break;
    case 6:
        play_note(NOTE_1, 2, 1, &note_number);
        break;
    case 7:
        play_note(NOTE_2, 2, 1, &note_number);
        break;
    case 8:
        play_note(NOTE_6, 1, 1, &note_number);
        break;
    case 9:
        play_note(NOTE_5, 1, 1, &note_number);
        break;
    case 10:
        play_note(NOTE_1, 2, 1, &note_number);
        break;
    case 11:
        play_note(NOTE_2, 2, 1, &note_number);
        break;
    case 12:
        play_note(NOTE_3, 5, 1, &note_number);
        break;
    case 13:
        play_note(NOTE_1, 5, 0, &note_number);
        break;
    case 14:
        buzzer_enable(0);
        note_number = 0;
        return 0;
        break;
    }
    return 1;
}
