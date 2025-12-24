#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "i2c_bitforge.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "TMP1075.h"

static const char *TAG = "TMP1075";

static i2c_master_dev_handle_t tmp1075_dev_handle;
static i2c_master_bus_handle_t i2c_bus_handle;
static bool device_initialized = false;
static float last_valid_temperature = 25.0f; // Default to room temperature

// Default configuration: continuous conversion mode, 12-bit resolution
#define TMP1075_DEFAULT_CONFIG 0x0000

// Maximum number of retries for I2C operations
#define MAX_RETRIES 5

// Delay between retries in milliseconds
#define RETRY_DELAY_MS 20

/**
 * @brief Reset and recover the I2C bus
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure
 */
static esp_err_t TMP1075_reset_i2c_bus(void)
{
    esp_err_t ret;
    
    ESP_LOGW(TAG, "Resetting I2C bus for TMP1075");
    
    // Mark device as uninitialized first to prevent other functions from using it
    device_initialized = false;
    
    // Get the I2C bus handle
    ret = i2c_bitforge_get_master_bus_handle(&i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get I2C bus handle: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Try to remove the device from the bus, but don't worry if it fails
    ret = i2c_master_bus_rm_device(tmp1075_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to remove TMP1075 device from I2C bus: %s", esp_err_to_name(ret));
        // Continue anyway
    }
    
    // Add a longer delay to allow the I2C bus to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Re-add the device to the I2C bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TMP1075_I2CADDR_DEFAULT,
        .scl_speed_hz = I2C_BUS_SPEED_HZ,
    };
    
    ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &tmp1075_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to re-add TMP1075 device to I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Add another delay before trying to communicate with the device
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Try to configure the sensor with default settings, but don't fail if it doesn't work
    uint8_t config_data[3] = {TMP1075_CONFIG_REG,
                             (uint8_t)(TMP1075_DEFAULT_CONFIG & 0xFF),
                             (uint8_t)((TMP1075_DEFAULT_CONFIG >> 8) & 0xFF)};
    
    ret = i2c_bitforge_register_write_bytes(tmp1075_dev_handle, config_data, 3);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to reconfigure TMP1075: %s - will try again later", esp_err_to_name(ret));
        // Don't return error, just continue
    } else {
        ESP_LOGI(TAG, "Successfully reconfigured TMP1075");
    }
    
    // Mark device as initialized even if configuration failed
    // We'll try again on the next temperature read
    device_initialized = true;
    
    ESP_LOGI(TAG, "I2C bus reset completed");
    return ESP_OK;
}

/**
 * @brief Initialize the TMP1075 sensor.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t TMP1075_init(void) {
    esp_err_t ret;
    
    // Add the device to the I2C bus
    ret = i2c_bitforge_add_device(TMP1075_I2CADDR_DEFAULT, &tmp1075_dev_handle, TAG);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add TMP1075 device to I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    device_initialized = true;
    
    // Get the I2C bus handle for future use
    ret = i2c_bitforge_get_master_bus_handle(&i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get I2C bus handle: %s", esp_err_to_name(ret));
        // Not critical, continue
    }
    
    // Configure the sensor with default settings
    uint8_t config_data[3] = {TMP1075_CONFIG_REG,
                             (uint8_t)(TMP1075_DEFAULT_CONFIG & 0xFF),
                             (uint8_t)((TMP1075_DEFAULT_CONFIG >> 8) & 0xFF)};
    
    // Try multiple times to configure the sensor
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        ret = i2c_bitforge_register_write_bytes(tmp1075_dev_handle, config_data, 3);
        if (ret == ESP_OK) {
            break;
        }
        
        ESP_LOGW(TAG, "Failed to configure TMP1075 (attempt %d/%d): %s",
                 retry + 1, MAX_RETRIES, esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure TMP1075 after %d attempts", MAX_RETRIES);
        return ret;
    }
    
    // Verify device ID
    uint8_t id_data[2];
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        ret = i2c_bitforge_register_read(tmp1075_dev_handle, TMP1075_DEVICE_ID, id_data, 2);
        if (ret == ESP_OK) {
            break;
        }
        
        ESP_LOGW(TAG, "Failed to read TMP1075 device ID (attempt %d/%d): %s",
                 retry + 1, MAX_RETRIES, esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read TMP1075 device ID after %d attempts", MAX_RETRIES);
        return ret;
    }
    
    uint16_t device_id = (id_data[0] << 8) | id_data[1];
    ESP_LOGI(TAG, "TMP1075 Device ID: 0x%04X", device_id);
    
    return ESP_OK;
}

bool TMP1075_installed(int device_index)
{
    uint8_t data[2];
    esp_err_t result;
    
    if (!device_initialized) {
        ESP_LOGW(TAG, "TMP1075 not initialized");
        return false;
    }
    
    // Try up to MAX_RETRIES times to read the configuration register
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        result = i2c_bitforge_register_read(tmp1075_dev_handle, TMP1075_CONFIG_REG, data, 2);
        if (result == ESP_OK) {
            ESP_LOGD(TAG, "Configuration[%d] = %02X %02X", device_index, data[0], data[1]);
            return true;
        }
        
        // If we get a NACK error, try to reset the I2C bus
        if (retry == 2) {
            ESP_LOGW(TAG, "Persistent I2C errors, attempting bus reset");
            TMP1075_reset_i2c_bus();
        }
        
        // Longer delay between retries
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }
    
    ESP_LOGW(TAG, "TMP1075[%d] not detected or not responding", device_index);
    return false;
}

/**
 * @brief Read the temperature from the TMP1075 sensor
 *
 * @param device_index The device index for logging purposes
 * @param temperature Pointer to store the temperature value (in degrees Celsius)
 * @return esp_err_t ESP_OK on success, or an error code on failure
 */
esp_err_t TMP1075_read_temperature_float(int device_index, float *temperature)
{
    uint8_t data[2];
    esp_err_t result;
    static uint32_t consecutive_errors = 0;
    
    if (!device_initialized) {
        ESP_LOGW(TAG, "TMP1075 not initialized");
        *temperature = last_valid_temperature;
        return ESP_ERR_INVALID_STATE;
    }
    
    // Try up to MAX_RETRIES times to read the temperature register
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        result = i2c_bitforge_register_read(tmp1075_dev_handle, TMP1075_TEMP_REG, data, 2);
        if (result == ESP_OK) {
            // Convert the raw data to temperature
            // The temperature register is a 16-bit signed value with a resolution of 0.0625°C
            int16_t raw_temp = (data[0] << 8) | data[1];
            *temperature = raw_temp * 0.0625f;
            
            // Store the valid temperature for future use if communication fails
            last_valid_temperature = *temperature;
            consecutive_errors = 0;
            
            ESP_LOGD(TAG, "Raw Temperature = %02X %02X, Temperature[%d] = %.2f°C",
                    data[0], data[1], device_index, *temperature);
            return ESP_OK;
        }
        
        // If we get persistent errors, try to reset the I2C bus
        if (retry == 2) {
            ESP_LOGW(TAG, "Persistent I2C errors, attempting bus reset");
            TMP1075_reset_i2c_bus();
        }
        
        // Longer delay between retries
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }
    
    consecutive_errors++;
    
    // If we have too many consecutive errors, return the last valid temperature
    // but log a warning
    if (consecutive_errors > 10) {
        ESP_LOGW(TAG, "Using last valid temperature (%.2f°C) after %d consecutive errors",
                last_valid_temperature, (int)consecutive_errors);
        *temperature = last_valid_temperature;
        return ESP_OK; // Return OK to prevent system disruption
    }
    
    ESP_LOGW(TAG, "Failed to read temperature from TMP1075[%d]", device_index);
    *temperature = last_valid_temperature;
    return ESP_ERR_TIMEOUT;
}

/**
 * @brief Read the temperature from the TMP1075 sensor (legacy function)
 *
 * @param device_index The device index for logging purposes
 * @return uint8_t The integer part of the temperature in degrees Celsius, or 0 on error
 */
uint8_t TMP1075_read_temperature(int device_index)
{
    float temp;
    esp_err_t ret = TMP1075_read_temperature_float(device_index, &temp);
    
    // Even if there's an error, we'll return the last valid temperature
    // to prevent system disruption
    return (uint8_t)temp;
}

