#ifndef MUSIC_H_INCLUDED
#define MUSIC_H_INCLUDED
/// --- Defines --- ///
// Buzzer
#define DDR_BUZZ        DDRE
#define PORT_BUZZ       PORTE
#define PIN_BUZZ        PE5
// --- Notes --- ///
#define NOTE_1  262+100
#define NOTE_2  294+100
#define NOTE_3  330+100
#define NOTE_4  349+100
#define NOTE_5  392+100
#define NOTE_6  440+100
#define NOTE_7  494+100
#define NOTE_8  523+100
/// --- Functions --- ///
void music_init(void);
void buzzer_enable(char input);
int hz_to_icr_value(int hz);
void play_note(int note, int beats, int state, int *note_num);
char play_beep();
char play_beep_sad();
char play_song();

#endif // MUSIC_H_INCLUDED
