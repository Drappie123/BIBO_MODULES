#ifndef BASIC_IO_H_INCLUDED
#define BASIC_IO_H_INCLUDED
/// --- Defines --- ///
// --- E18-D80NK sensors --- ///
#define DDR_DONK1       DDRL
#define REG_DONK1       PINL
#define PORT_DONK1      PORTL
#define PIN_DONK1       PL3

#define DDR_DONK2       DDRL
#define REG_DONK2       PINL
#define PORT_DONK2      PORTL
#define PIN_DONK2       PL2
// --- Buttons --- //
#define DDR_PLUS        DDRD
#define REG_PLUS        PIND
#define PORT_PLUS       PORTD
#define PIN_PLUS        PD2

#define DDR_MINUS       DDRF
#define REG_MINUS       PIND
#define PORT_MINUS      PORTD
#define PIN_MINUS       PD1

#define DDR_ENTER       DDRD
#define REG_ENTER       PIND
#define PORT_ENTER      PORTD
#define PIN_ENTER       PD3

#define PIN_STOP         PD0
#define REG_STOP         PIND
#define PORT_STOP        PORTD
#define DDR_STOP         DDRD
// --- LED --- ///
#define PIN_LED         PD7
#define PORT_LED        PORTD
#define DDR_LED         DDRD

/// --- Functions --- ///
void basic_io_init(void);
void donk_init(void);
void init_stop_button(void);
char stop_pressed(void);
char donk_left(void);
char donk_right(void);
void start_button_init(void);
uint8_t starting_button(void);

#endif // BASIC_IO_H_INCLUDED
