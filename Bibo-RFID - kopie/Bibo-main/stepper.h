#ifndef STEPPER_H_INCLUDED
#define STEPPER_H_INCLUDED
/// --- Defines --- ///
// --- Left stepper --- //
#define DDR_ST1_STEP        DDRG
#define PORT_ST1_STEP       PORTG
#define PIN_ST1_STEP        PG5

#define DDR_ST1_DIR         DDRE
#define PORT_ST1_DIR        PORTE
#define PIN_ST1_DIR         PE3

#define DDR_ST1_EN          DDRH
#define PORT_ST1_EN         PORTH
#define PIN_ST1_EN          PH3
// --- Right stepper --- //
#define DDR_ST2_STEP        DDRH
#define PORT_ST2_STEP       PORTH
#define PIN_ST2_STEP        PH6

#define DDR_ST2_DIR         DDRB
#define PORT_ST2_DIR        PORTB
#define PIN_ST2_DIR         PB4

#define DDR_ST2_EN          DDRB
#define PORT_ST2_EN         PORTB
#define PIN_ST2_EN          PB5
// --- Stepper frequency --- //
// Minimum frequency determines max speed
#define STEPPER_MIN_FREQ       0x02
#define STEPPER_FREQ_RANGE      (0xFF - STEPPER_MIN_FREQ)

/// --- Functions --- ///
void stepper_init(void);
void stepper_speed_left(int speed);
void stepper_speed_right(int speed);
void stepper_control(int speed, int steer);
void stepper_step_control(int speed, int steer, unsigned int steps);
/// --- Variables --- ///
extern volatile int step_count_left;
extern volatile int step_count_right;
extern volatile int freq_target_left;
extern volatile int freq_target_right;
extern volatile char direction_change_left;
extern volatile char direction_change_right;

#endif // STEPPER_H_INCLUDED
