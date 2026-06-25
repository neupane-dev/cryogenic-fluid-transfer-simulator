#ifndef CRYO_DEFS_H
#define CRYO_DEFS_H

#include <stdint.h>
#include <stdbool.h>

//Flight configuration thresholds
#define MAX_SAFE_PRESSURE_PSI  300.0f
#define CRITICAL_BOIL_OFF_TEMP_K 90.0f

typedef enum
{
    STATE_IDLE = 0,
    STATE_PRE_CHILL,
    STATE_PRESSURE_EQUALIZATION,
    STATE_TRANSFERRING,
    STATE_PURGING,
    STATE_EMERGENCY_FAULT,
} CryoState_t;

typedef struct {
    float tank_pressure;
    float line_temperature;
    uint32_t isolation_valve_pos; // 0 = Closed, 100 = Fully Open
    uint8_t vent_valve_state; // 0 = closed, 1 = Open
    CryoState_t current_state;
} CryoSystem_t;


#endif