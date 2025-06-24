#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/// === Configuration === ///
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
#define INITIAL_FORWARD_TIME    80
#define SECOND_FORWARD_TIME     55
// --- Package counting --- //
#define PACKAGE_COUNT_MIN       1
#define PACKAGE_COUNT_MAX       99
// Stop time is in addition to buzzer time
#define DETECTION_STOP_TIME     10
// --- Turn movement control --- //
#define TURNING_SPEED           8 // Rec: 8 Max:
#define TURNING_RATIO           81
#define TURN_FORWARD_STEPS      3000
#define TURN_COUNTED_RATIO      40
#define TURN_WALL_DISTANCE      12
/// --- Following mode --- ///
// --- Range settings --- ///
#define FOLLOW_DISTANCE_MIN     10
#define FOLLOW_DISTANCE_MAX     100
// Margin is the allowed error between left and right distances
#define FOLLOW_DISTANCE_MARGIN  15
#define FOLLOW_DETECTION_MARGIN 30
// AGV stops when the actual distance is greater than limit + following distance
#define FOLLOW_DISTANCE_LIMIT   50
// --- Movement settings --- //
#define FOLLOW_SPEED_MULTIPLIER 1
#define FOLLOW_STEERING         60
/// --- General settings --- ///
// Sound loudness
#define VOLUME                  20
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
