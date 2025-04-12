#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/i2c.h"
#include "esp_log.h"

// PCA9685 I2C address and parameters
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_FREQ_HZ 100000
#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE

#define C_SERVO_MIN 100
#define C_SERVO_MAX 475
#define SERVO_FREQ 50

static const char *TAG = "PCA9685_SERVO";

typedef struct {
    uint8_t channelID;
    uint32_t servoMaxValue;
    uint32_t servoMinValue;
    uint32_t servoHalfValue;
} TServoParam;

TServoParam legs[] =
{
    {
        .channelID = 1,
        .servoMaxValue = C_SERVO_MAX, 
        .servoMinValue = C_SERVO_MIN - 30, 
        .servoHalfValue = 185
    },

    {
        .channelID = 4,
        .servoMaxValue = C_SERVO_MAX + 30, 
        .servoMinValue = C_SERVO_MIN, 
        .servoHalfValue = 300
    },

    {
        .channelID = 5,
        .servoMaxValue = C_SERVO_MAX, 
        .servoMinValue = C_SERVO_MIN - 30, 
        .servoHalfValue = 200
    },

    {
        .channelID = 7,
        .servoMaxValue = C_SERVO_MAX, 
        .servoMinValue = C_SERVO_MIN, 
        .servoHalfValue = 200
    },

    {
        .channelID = 15,
        .servoMaxValue = C_SERVO_MAX + 30, 
        .servoMinValue = C_SERVO_MIN, 
        .servoHalfValue = 310
    },

    {
        .channelID = 11,
        .servoMaxValue = C_SERVO_MAX + 30, 
        .servoMinValue = C_SERVO_MIN, 
        .servoHalfValue = 220
    },
};

const TServoParam mids[] = {
    {
        .channelID = 2,
        .servoMaxValue = 300, 
        .servoMinValue = 0, 
        .servoHalfValue = 220
    },

    {
        .channelID = 3,
        .servoMaxValue = 270, 
        .servoMinValue = 0, 
        .servoHalfValue = 220
    },

    {
        .channelID = 6,
        .servoMaxValue = 270, 
        .servoMinValue = 0, 
        .servoHalfValue = 220
    },

    {
        .channelID = 8,
        .servoMaxValue = 270, 
        .servoMinValue = 0, 
        .servoHalfValue = 120
    },

    {
        .channelID = 10,
        .servoMaxValue = 250, 
        .servoMinValue = 0, 
        .servoHalfValue = 280
    },

    {
        .channelID = 12,
        .servoMaxValue = 110, 
        .servoMinValue = 0, 
        .servoHalfValue = 180
    }
};

const uint32_t sizeServo = sizeof(legs) / sizeof(legs[0]);
const uint32_t sizeServoMid = sizeof(mids) / sizeof(mids[0]);

void i2c_master_init()
{
    const i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t retval = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (retval == ESP_OK)
    {
        retval = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
        if (retval != ESP_OK)
        {
            ESP_LOGE(TAG, "on install");
        }
    }
    else
    {
        ESP_LOGE(TAG, "on config");
    }
}

void pca9685_write_byte(uint8_t reg, uint8_t data)
{
    const uint8_t write_buf[] = {reg, data};

    esp_err_t retval = i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_ADDR,
                                                  write_buf, sizeof(write_buf),
                                                  1000 / portTICK_PERIOD_MS);
    if (retval != ESP_OK)
    {
        ESP_LOGE(TAG, "on write");
    }
}

void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
{
    const uint8_t write_buf[] = {
        0x06 + (4 * channel), // LEDn_ON_L register address
        on & 0xFF,            // Low byte of ON value
        (on >> 8) & 0xFF,     // High byte of ON value
        off & 0xFF,           // Low byte of OFF value
        (off >> 8) & 0xFF     // High byte of OFF value
    };

    esp_err_t retval = i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_ADDR,
                                                  write_buf, sizeof(write_buf),
                                                  1000 / portTICK_PERIOD_MS);
    if (retval != ESP_OK)
    {
        ESP_LOGE(TAG, "on write multiple");
    }
}

void pca9685_init()
{
    pca9685_write_byte(PCA9685_MODE1, 0x10);                          // Put PCA9685 into sleep mode
    uint8_t prescale = (uint8_t)(25000000 / (4096 * SERVO_FREQ) - 1); // Set PWM frequency to 50 Hz
    pca9685_write_byte(PCA9685_PRESCALE, prescale);
    pca9685_write_byte(PCA9685_MODE1, 0xA0); // Restart and set to normal mode
}

void legUp(int id)
{
    pca9685_set_pwm(legs[id].channelID, 0, legs[id].servoMaxValue);
}

void legDown(int id)
{
    pca9685_set_pwm(legs[id].channelID, 0, legs[id].servoMinValue);
}

void legHalf(int id)
{
    pca9685_set_pwm(legs[id].channelID, 0, legs[id].servoHalfValue);
}

void legMiddle(int id)
{
    pca9685_set_pwm(mids[id].channelID, 0, mids[id].servoHalfValue);
}

void legInc(int id)
{
    pca9685_set_pwm(mids[id].channelID, 0, mids[id].servoMaxValue);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing I2C...");
    i2c_master_init();

    ESP_LOGI(TAG, "Initializing PCA9685...");
    pca9685_init();

    uint32_t setupDelay = 1500;
    while (1)
    {
        for (int i = 0; i < sizeServoMid; i++)
        {
            legInc(i);
        }

        for (int i = 0; i < sizeServo; i++)
        {
            if (i % 2)
            {
                legUp(i);
                legInc(i);
            }
            else
            {
                legHalf(i);
                legMiddle(i);
            }
        }

        vTaskDelay(setupDelay / portTICK_PERIOD_MS);
        for (int i = 0; i < sizeServo; i++)
        {
            legHalf(i);
        }

        vTaskDelay(setupDelay / portTICK_PERIOD_MS);

        for (int i = 0; i < sizeServo; i++)
        {

            if (i % 2)
            {
                legHalf(i);
                legMiddle(i);
            }
            else
            {
                legUp(i);
                legInc(i);
            }
        }

        vTaskDelay(setupDelay / portTICK_PERIOD_MS);
        for (int i = 0; i < sizeServo; i++)
        {
            legHalf(i);
        }

        vTaskDelay(setupDelay / portTICK_PERIOD_MS);
    }
}
