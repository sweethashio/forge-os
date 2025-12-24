#ifndef TMP1075_H_
#define TMP1075_H_

#include "esp_err.h"

#define TMP1075_I2CADDR_DEFAULT 0x49  ///< TMP1075 i2c address
#define TMP1075_TEMP_REG 0x00         ///< Temperature register
#define TMP1075_CONFIG_REG 0x01       ///< Configuration register
#define TMP1075_LOW_LIMIT 0x02        ///< Low limit register
#define TMP1075_HIGH_LIMIT 0x03       ///< High limit register
#define TMP1075_DEVICE_ID 0x0F        ///< Device ID register

/**
 * @brief Initialize the TMP1075 sensor
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure
 */
esp_err_t TMP1075_init(void);

/**
 * @brief Check if the TMP1075 sensor is installed and responding
 *
 * @param device_index The device index for logging purposes
 * @return bool true if the sensor is installed and responding, false otherwise
 */
bool TMP1075_installed(int device_index);

/**
 * @brief Read the temperature from the TMP1075 sensor
 *
 * @param device_index The device index for logging purposes
 * @return uint8_t The integer part of the temperature in degrees Celsius, or 0 on error
 */
uint8_t TMP1075_read_temperature(int device_index);

/**
 * @brief Read the temperature from the TMP1075 sensor with floating-point precision
 *
 * @param device_index The device index for logging purposes
 * @param temperature Pointer to store the temperature value (in degrees Celsius)
 * @return esp_err_t ESP_OK on success, or an error code on failure
 */
esp_err_t TMP1075_read_temperature_float(int device_index, float *temperature);

#endif /* TMP1075_H_ */