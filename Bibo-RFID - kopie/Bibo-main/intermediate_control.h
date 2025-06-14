#ifndef INTERMEDIATE_CONTROL_H_INCLUDED
#define INTERMEDIATE_CONTROL_H_INCLUDED
/// --- Defines --- ///
// Timer cycle is 0.1s
#define GP_TIMER_TOP            25000
// Increments for setting follow distance
#define FOLLOW_DISTANCE_STEP    10
/// --- Functions --- ///
void init(void);
void itermediate_control_init(void);
void init_gp_timer(void);
char gp_timer(int set_value);
void init_gp_timer2(void);
char gp_timer2(int set_value);
char donk_detection(char *mem_left, char *mem_righ);

#endif // INTERMEDIATE_CONTROL_H_INCLUDED
