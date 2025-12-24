#include "i2c_bitforge.h"
#include "esp_log.h"
#include <stdio.h>

#include "EMC2302.h"

#define I2C_MASTER_SCL_IO 48 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 47 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "EMC2302.c";

static i2c_master_dev_handle_t emc2302_dev_handle;

/**
 * @brief Read a sequence of I2C bytes
 */
static esp_err_t register_read(uint8_t reg_addr, uint8_t * data, size_t len)
{
    return i2c_bitforge_register_read(emc2302_dev_handle, reg_addr, data, len);
}

/**
 * @brief Write a byte to a I2C register
 */
static esp_err_t register_write_byte(uint8_t reg_addr, uint8_t data)
{
    esp_err_t err;
    err = i2c_bitforge_register_write_byte(emc2302_dev_handle, reg_addr, data);
    return err;

}

// run this first. sets up the PWM polarity register
esp_err_t EMC2302_init(bool invertPolarity)
{

    esp_err_t err;
    ESP_LOGI(TAG, "initializing EMC2302");

    if (i2c_bitforge_add_device(EMC2302_I2CADDR_DEFAULT, &emc2302_dev_handle, TAG) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device");
        return ESP_FAIL;
    }

    // set polarity of ch1 and ch2
    err = i2c_bitforge_register_write_byte(emc2302_dev_handle, EMC2302_PWM_POLARITY, (invertPolarity) ? 0x03 : 0x00);
    if (err != ESP_OK) {
        return false;
    }

    // set output type to push pull of ch1 and ch2
    err = i2c_bitforge_register_write_byte(emc2302_dev_handle, EMC2302_PWM_OUTPUT, 0x03);
    if (err != ESP_OK) {
        return false;
    }

    // set base frequency of ch1 and ch2 to 19.53kHz
    err = i2c_bitforge_register_write_byte(emc2302_dev_handle, EMC2302_PWM_BASEF123, (0x01 << 0) | (0x01 << 3));
    if (err != ESP_OK) {
        return false;
    }

    // Fan configuration for Noctua fan
    // bits 7-5: 000 = no validation count
    // bits 4-3: 01 = 5 edge samples (better for low RPM fans)
    // bits 2-0: 000 = no edges skipped
    uint8_t fan_config = (0b01 << 3);
    
    err = i2c_bitforge_register_write_byte(emc2302_dev_handle, EMC2302_FAN1_CONFIG1, fan_config);
    if (err != ESP_OK) {
        return false;
    }

    err = i2c_bitforge_register_write_byte(emc2302_dev_handle, EMC2302_FAN2_CONFIG1, fan_config);
    if (err != ESP_OK) {
        return false;
    }

    return ESP_OK;

}

// Sets the fan speed to a given percent
void EMC2302_set_fan_speed(uint8_t devicenum, float percent)
{
    uint8_t speed;
	uint8_t FAN_SETTING_REG = EMC2302_FAN1_SETTING + (devicenum * 0x10);

    speed = (uint8_t) (255.0 * (1.0f-percent));
    
    //ESP_LOGI(TAG, "setting fan speed to %.2f%% (0x%02x)", percent * 100.0, speed);

    ESP_ERROR_CHECK(register_write_byte(FAN_SETTING_REG, speed));
}

float EMC2302_get_external_temp(void)
{
    // We don't have temperature on this chip, so fake it
    return 0;
}

uint8_t EMC2302_get_internal_temp(void)
{
    // We don't have temperature on this chip, so fake it
    return 0;
}

uint16_t EMC2302_get_fan_speed(uint8_t devicenum)
{
    uint8_t msb, lsb;
    uint16_t tach_reading;
    uint16_t rpm;
    esp_err_t err;

    if (devicenum >= 2) {
        ESP_LOGE(TAG, "Index out of range!");
    }
    
    if(devicenum == 0) {
        err = register_read(EMC2302_TACH1_LSB, &lsb, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read fan LSB");
            return 0;
        }

        err = register_read(EMC2302_TACH1_MSB, &msb, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read fan MSB");
            return 0;
        }
    }
    else if (devicenum == 1) {
        err = register_read(EMC2302_TACH2_LSB, &lsb, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read fan LSB");
            return 0;
        }

        err = register_read(EMC2302_TACH2_MSB, &msb, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read fan MSB");
            return 0;
        }
    }


    tach_reading = (msb << 8) | lsb;
    
    ESP_LOGI(TAG, "Raw %d tach reading - MSB: 0x%02x, LSB: 0x%02x, Combined: 0x%04x", 
             devicenum, msb, lsb, tach_reading);
    
    // If tach reading is 0 or 0xFFFF, fan is likely stopped
    if (tach_reading == 0 || tach_reading == 0xFFFF) {
        ESP_LOGI(TAG, "Invalid tach reading, fan likely stopped");
        return 0;
    }

    // For Noctua fans with period measurement
    // RPM = (60 * fan_clock) / (edges_per_rev * poles * tach_period)
    // Where fan_clock = 32768 Hz / 256 (prescaler for 19.53kHz base freq)
    // edges_per_rev = 5 (from CONFIG1)
    // poles = 2 (1 pole pair)
    // Simplified: RPM = (60 * 128) / (5 * 2 * tach_period)
    // Further simplified: RPM = 768 / tach_period
    rpm = (768 * 65536) / tach_reading;
    
    ESP_LOGI(TAG, "Fan RPM %d: %u (tach: %u)", devicenum, (unsigned int)rpm, tach_reading);
    return rpm;
}
