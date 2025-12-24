#ifndef ADC_H_
#define ADC_H_

#include "global_state.h"

typedef enum _Channel
{
  V_BUCK_OUTPUT = 1U,
  V_TEMP_10K_A2 = 3U,
  V_TEMP_10K_A1 = 4U,
  V_0V8_REF = 5U,
  V_1V2_REF = 6U
} ADC_CHANNEL;


void ADC_init(GlobalState * GLOBAL_STATE);
uint16_t ADC_get_vcore(void);
uint16_t ADC_read(ADC_CHANNEL Channel, GlobalState * GLOBAL_STATE);
float ADC_get_temperature(ADC_CHANNEL Channel, DeviceModel Device);

#endif /* ADC_H_ */