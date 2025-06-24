#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/// === Configuration === ///
#define STARTING_STATE wait
/// --- Package mode --- ///
// --- Wall range settings --- //
#define WALL_MAX_RANGE          25
#define WALL_MIN_RANGE          1
#define ALLOWED_STEERING_ERROR  1
// --- Path movement control --- //
#define CRUISE_SPEED            10 // Rec: 10
#define STEERING_MULTIPLIER     7
#define ERROR_MAX_DISTANCE      14
#define ERROR_CENTER_DISTANCE   10
// --- Package counting --- //
#define PACKAGE_COUNT_MIN       1
#define PACKAGE_COUNT_MAX       99
// Stop time is in addition to buzzer time
#define DETECTION_STOP_TIME     10
#define SCAN_TIME               20
// --- Turn movement control --- //
#define TURNING_SPEED           8 // Rec: 8 Max:
#define TURNING_RATIO           81
#define TURN_FORWARD_STEPS      3000
#define TURN_COUNTED_RATIO      40
#define TURN_WALL_DISTANCE      12
/// --- General settings --- ///
// Buzzer loudness
#define VOLUME                  15
// Number of detection needed, 1 lowest sensitivity
#define PATH_SENSITIVITY        4
#define PERSON_SENSITIVITY      2
// --- Stepper control --- //
#define RAMPING_SPEED           18
// --- Timing --- //
#define TEXT_DISPLAY_TIME       5
#define SENSOR_TIMING           1
// --- Buttons --- //
#define DEBOUNCE_TIME           10
// --- Display brightness --- //
#define NORMAL_BRIGHTNESS       2
#define MAX_BRIGHTNESS          7
#define standard_speed          0xFF
#define standard_acceleration   0xFF
/// === Utility === ///
#define CLAMP(value, min, max) ((value) < (min) ? (min) : (value) > (max) ? (max) : (value))


#endif // CONFIG_H_INCLUDED
