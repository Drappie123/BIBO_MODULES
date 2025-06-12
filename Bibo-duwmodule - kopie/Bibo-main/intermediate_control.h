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
char wall_detected(void);
char check_path_end(void);
char donk_detection(char *mem_left, char *mem_righ);
void path_steering(void);
char person_steering(int fol_dist);
void set_first_turn(char *turn);
void set_package_count(int *pac_lim);
void set_follow_distance(int *dist);
char person_detected(int fol_dist);
char step_count_end(void);
void sensor_turn(char direction);

#endif // INTERMEDIATE_CONTROL_H_INCLUDED
