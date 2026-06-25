#ifndef CRYO_HAL_H
#define CRYO_HAL_H

#include <stdint.h>

// External variable configuration for unit testing injection
extern float HAL_Test_Override_Pressure;

//Driver interfaces
float HAL_read_pressure_sensor_SPI(void);
void HAL_write_cryo_valve_PWM(uint32_t duty_cycle);
void HAL_write_vent_valve_GPIO(uint8_t state);

#endif