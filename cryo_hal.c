#include "cryo_hal.h"
#include <stdio.h>

// Global test vectors (initialized to nominal values)
float HAL_Test_Override_Pressure = 150.0f;

float HAL_read_pressure_sensor_SPI(void)
{
    // In hardware this would be reading from the actual SPI
    return HAL_Test_Override_Pressure;
}
void HAL_write_cryo_valve_PWM(uint32_t duty_cycle)
{
    // In hardware this modifies the compare register of hardware timer
    printf("[HAL] Cryo valve PWM adjusted to: %u%%\n",duty_cycle);
}
void HAL_write_vent_valve_GPIO(uint8_t state)
{
    //In hardware this flips a bit in an atomic GPIO ODR register
    printf("[HAL] vent valve gpio forced: %s\n",state? "HIGH (OPEN)":"LOW(CLOSED)");
}