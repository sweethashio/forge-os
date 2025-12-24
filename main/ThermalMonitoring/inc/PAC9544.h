#ifndef PAC9544_H_
#define PAC9544_H_

#include "esp_log.h"
#include "i2c_bitforge.h"

#define PAC9544_ADDR         0x70

// Debug flag for PAC9544 module
// #define DEBUG_PAC9544  // Uncomment this line to enable debug logging

esp_err_t PAC9544_init();
esp_err_t PAC9544_selectChannel(uint8_t channel);
esp_err_t PAC9544_get_selected_channel(uint8_t *channel);

#endif /* PAC9544_H_ */
