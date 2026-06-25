#include <stdio.h>
#include "cryo_defs.h"
#include "cryo_controller.h"


int main(void)
{
    //Instantiate the spacecraft state structure
    CryoSystem_t spaceium_station_tank = {
        .tank_pressure = 120.0f,
        .line_temperature = 293.0f,
        .isolation_valve_pos = 0,
        .vent_valve_state = 0,
        .current_state = STATE_IDLE
    };

    printf("--- Initializing Spaceium Cryogenic Controller Testbench ---\n");
    
    // Core mission executive execution cycle
    while(spaceium_station_tank.current_state != STATE_EMERGENCY_FAULT)
    {
        CRYO_run_control_logic(&spaceium_station_tank);

        // Simulating a minor execution break in the flight controller cycle
        // In bare metal/RTOS this would be an explicit vTaskDelay or Timer interval
        static int iterations = 0;
        if (++iterations >= 4) break;
    }
    
    
    return 0;
}