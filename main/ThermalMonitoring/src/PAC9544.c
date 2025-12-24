#include "PAC9544.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "EMC2101.h"

static const char * TAG = "PAC9544";

static i2c_master_dev_handle_t PAC9544_dev_handle;
static i2c_master_bus_handle_t i2c_bus_handle;

/**
 * @brief Initialize the EMC2101 sensor.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t PAC9544_init() {

    esp_err_t ret = ESP_OK;

    i2c_device_config_t PCA9544_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PAC9544_ADDR,
        .scl_speed_hz = I2C_BUS_SPEED_HZ,
    };

    // Get the I2C bus handle
    ret = i2c_bitforge_get_master_bus_handle(&i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get I2C bus handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // Add a longer delay to allow the I2C bus to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ret = i2c_master_bus_add_device(i2c_bus_handle, &PCA9544_dev_cfg, &PAC9544_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "init: Failed to add device");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    return ret;
}

esp_err_t PAC9544_selectChannel(uint8_t channel) {
#ifdef DEBUG_PAC9544
    ESP_LOGI(TAG, "PCA9544 selecting channel: %d", channel);
#endif
    uint8_t controlRegister = 0x4; // The Control register of the Multiplexer
    controlRegister |= channel; // Bitwise OR controlRegister & channel
    esp_err_t err = ESP_OK;

#ifdef DEBUG_PAC9544
    ESP_LOGI(TAG, "PCA9544: Sending following byte to address 0x00: %d", controlRegister);
#endif
    err = i2c_bitforge_register_write_byte(PAC9544_dev_handle, 0x00, controlRegister);
    if (err != ESP_OK) {
#ifdef DEBUG_PAC9544
        ESP_LOGI(TAG, "PCA9544 cannot selected channel %d", channel);
#endif
        err = ESP_FAIL;
    }
    return err;
}

esp_err_t PAC9544_get_selected_channel(uint8_t *channel)
{
    uint8_t status;
    esp_err_t err = i2c_bitforge_register_read(PAC9544_dev_handle, 0x00, &status, 1);
    if (err != ESP_OK) {
        return err;
    }

    *channel = (status & 0x0F) - 0x4; //Compensate B3 with 0x4
    return ESP_OK;
}