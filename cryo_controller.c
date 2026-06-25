#include "cryo_controller.h"
#include "cryo_hal.h"
#include <stdio.h>

bool CRYO_check_safety_invariants(CryoSystem_t *sys)
{
    sys->tank_pressure = HAL_read_pressure_sensor_SPI();

    if(sys->tank_pressure >= MAX_SAFE_PRESSURE_PSI)
    {
        sys->current_state = STATE_EMERGENCY_FAULT;

        //Immediately close the cryo valve and open the vent valve
        HAL_write_cryo_valve_PWM(0);
        HAL_write_vent_valve_GPIO(1);
        return false;
    }
    return true;
}
void CRYO_run_control_logic(CryoSystem_t *sys)
{
    if(!CRYO_check_safety_invariants(sys))
    {
        printf("[CRITICAL ALERT] Safety Invariant Broken! Forced State to EMERGENCY_FAULT.\n");
        return;
    }

    switch (sys->current_state)
    {
        case STATE_IDLE:
            printf("State: IDLE. Awaiting Transfer Authorization Command...\n");
            HAL_write_cryo_valve_PWM(0);
            HAL_write_vent_valve_GPIO(0);
            sys->current_state = STATE_PRE_CHILL;
            break;
        case STATE_PRE_CHILL:
            printf("State: PRE_CHILL. Reducing line temperatures with bleed lines.\n");
            HAL_write_cryo_valve_PWM(5); //5% trickle path
            HAL_write_vent_valve_GPIO(1);
            sys->current_state = STATE_PRESSURE_EQUALIZATION;
            break;
        case STATE_PRESSURE_EQUALIZATION:
            printf("State: PRESSURE_EQUALIZATION. Matching target spacecraft ullage.\n");
            HAL_write_cryo_valve_PWM(15);
            HAL_write_vent_valve_GPIO(0);
            sys->current_state = STATE_TRANSFERRING;
            break;
        case STATE_TRANSFERRING:
            printf("State: TRANSFERRING. Full-flow active cryogenic mass delivery.\n");
            HAL_write_cryo_valve_PWM(100); // Valve fully open
            break;
        case STATE_PURGING:
            printf("State: PURGING. Clearing line residuals with Helium push.\n");
            HAL_write_cryo_valve_PWM(0);
            HAL_write_vent_valve_GPIO(1);
            sys->current_state = STATE_IDLE;
            break;
        case STATE_EMERGENCY_FAULT:
        default:
            // Fail-safe latch state configuration
            HAL_write_cryo_valve_PWM(0);
            HAL_write_vent_valve_GPIO(1);
            break;

    }
}