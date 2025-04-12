/**
 * @file pca9685_servo_controller.c
 * @brief ESP32-based PCA9685 servo control via I2C using FreeRTOS.
 *
 * This code initializes the I2C master and configures the PCA9685 PWM driver
 * to control multiple servo motors with different motion patterns.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "esp_err.h"

// I2C configuration
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_FREQ_HZ 100000

// PCA9685 configuration
#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define SERVO_FREQ 50

// PWM bounds for servo
#define SERVO_MIN 100
#define SERVO_MAX 475

static const char *TAG = "PCA9685_SERVO";

/**
 * @struct ServoConfig
 * @brief Configuration parameters for a single servo channel.
 */
typedef struct
{
    uint8_t channelID;
    uint32_t max;
    uint32_t min;
    uint32_t mid;
} ServoConfig;

// Leg servo configurations
static const ServoConfig legs[] = {
    {1, SERVO_MAX, SERVO_MIN - 30, 185},
    {4, SERVO_MAX + 30, SERVO_MIN, 300},
    {5, SERVO_MAX, SERVO_MIN - 30, 200},
    {7, SERVO_MAX, SERVO_MIN, 200},
    {15, SERVO_MAX + 30, SERVO_MIN, 310},
    {11, SERVO_MAX + 30, SERVO_MIN, 220},
};

// Mid servo configurations
static const ServoConfig mids[] = {
    {2, 300, 0, 220},
    {3, 270, 0, 220},
    {6, 270, 0, 220},
    {8, 270, 0, 120},
    {10, 250, 0, 280},
    {12, 110, 0, 180},
};

static const uint32_t numLegs = sizeof(legs) / sizeof(ServoConfig);
static const uint32_t numMids = sizeof(mids) / sizeof(ServoConfig);

/**
 * @brief Initialize I2C as master.
 */
void i2cMasterInit(void)
{
    const i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}

/**
 * @brief Write a single byte to PCA9685 register.
 */
void pca9685WriteByte(uint8_t reg, uint8_t data)
{
    const uint8_t buf[] = {reg, data};
    ESP_ERROR_CHECK(i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_ADDR, buf, sizeof(buf), 1000 / portTICK_PERIOD_MS));
}

/**
 * @brief Set PWM value for a channel.
 */
void pca9685SetPWM(uint8_t channel, uint16_t on, uint16_t off)
{
    const uint8_t buf[] = {
        0x06 + (4 * channel),
        on & 0xFF,
        (on >> 8) & 0xFF,
        off & 0xFF,
        (off >> 8) & 0xFF};
    ESP_ERROR_CHECK(i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_ADDR, buf, sizeof(buf), 1000 / portTICK_PERIOD_MS));
}

/**
 * @brief Initialize PCA9685 module.
 */
void pca9685Init(void)
{
    pca9685WriteByte(PCA9685_MODE1, 0x10); // Sleep
    uint8_t prescale = (uint8_t)(25000000 / (4096 * SERVO_FREQ) - 1);
    pca9685WriteByte(PCA9685_PRESCALE, prescale);
    pca9685WriteByte(PCA9685_MODE1, 0xA0); // Restart and normal mode
}

// --- Leg control helpers ---

void legUp(int id)
{
    pca9685SetPWM(legs[id].channelID, 0, legs[id].max);
}

void legDown(int id)
{
    pca9685SetPWM(legs[id].channelID, 0, legs[id].min);
}

void legMid(int id)
{
    pca9685SetPWM(legs[id].channelID, 0, legs[id].mid);
}

void midMid(int id)
{
    pca9685SetPWM(mids[id].channelID, 0, mids[id].mid);
}

void midMax(int id)
{
    pca9685SetPWM(mids[id].channelID, 0, mids[id].max);
}

/**
 * @brief Main app entry point. Drives servo animations in loop.
 */
void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_LOGI(TAG, "Initializing I2C and PCA9685...");
    i2cMasterInit();
    pca9685Init();

    const uint32_t setupDelay = 1500;

    while (1)
    {
        for (int i = 0; i < numMids; i++)
            midMax(i);

        for (int i = 0; i < numLegs; i++)
        {
            if (i % 2)
            {
                legUp(i);
                midMax(i);
            }
            else
            {
                legMid(i);
                midMid(i);
            }
        }

        vTaskDelay(setupDelay / portTICK_PERIOD_MS);

        for (int i = 0; i < numLegs; i++)
            legMid(i);
        vTaskDelay(setupDelay / portTICK_PERIOD_MS);

        for (int i = 0; i < numLegs; i++)
        {
            if (i % 2)
            {
                legMid(i);
                midMid(i);
            }
            else
            {
                legUp(i);
                midMax(i);
            }
        }

        vTaskDelay(setupDelay / portTICK_PERIOD_MS);
        for (int i = 0; i < numLegs; i++)
            legMid(i);
        vTaskDelay(setupDelay / portTICK_PERIOD_MS);
    }
}
