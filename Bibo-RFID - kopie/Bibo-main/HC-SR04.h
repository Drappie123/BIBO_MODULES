#ifndef HC_SR04_H_INCLUDED
#define HC_SR04_H_INCLUDED
/// --- Defines --- ///
// --- Timer 4 --- //
// Trigger is linked to timer OCR
// Connect both triggers of the primary sensor set to OC4B
#define DDR_TRIG4        DDRH
#define PORT_TRIG4       PORTH
#define PIN_TRIG4        PH4
// Echo is linked to timer ICR
// Connect the left sensor of each set to ICP4
#define DDR_ECHO4        DDRL
#define PIN_ECHO4        PL0
// --- Timer 5 --- ///
// Trigger is linked to timer OCR
// Connect both triggers of the secondary sensor set to OC5B
#define DDR_TRIG5        DDRL
#define PORT_TRIG5       PORTL
#define PIN_TRIG5        PL4
// Echo is linked to timer ICR
// Connect the right sensor of each set to ICP5
#define DDR_ECHO5        DDRL
#define PIN_ECHO5        PL1
/// --- Global variables --- ///
// Needed because they are updated inside ISR
extern volatile unsigned int left_distance;
extern volatile unsigned int right_distance;
/// --- Functions --- ///
void ultrasonic_init(void);
void ultrasonic_sync(void);
void ultrasonic_select(char selection);
int get_left_distance(void);
int get_right_distance(void);
#endif // HC_SR04_H_INCLUDED
