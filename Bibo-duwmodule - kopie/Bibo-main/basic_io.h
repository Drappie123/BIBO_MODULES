#ifndef BASIC_IO_H_INCLUDED
#define BASIC_IO_H_INCLUDED
/// --- Defines --- ///
#define PIN_STOP         PD0
#define REG_STOP         PIND
#define PORT_STOP        PORTD
#define DDR_STOP         DDRD
/// --- Functions --- ///
void basic_io_init(void);
void init_stop_button(void);
char stop_pressed(void);
void init_limit_switches(void);
uint8_t light_limit_switch_25();
uint8_t heavy_limit_switch_26();
uint8_t starting_button();

#endif // BASIC_IO_H_INCLUDED
