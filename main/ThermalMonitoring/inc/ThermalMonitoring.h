#ifndef THERMAL_H
#define THERMAL_H

#include "global_state.h"
#include "esp_err.h"

// Debug for Thermal readouts
// #define DEBUG_THERMALMONITORING // Uncomment this line to enable debug logging

esp_err_t Thermal_init(GlobalState * GLOBAL_STATE);
esp_err_t Thermal_setFanSpeedPercent(float percent);
uint16_t Thermal_getFanSpeed(void);
float Thermal_getAsicChipTemp(GlobalState * GLOBAL_STATE);


#endif // THERMAL_H