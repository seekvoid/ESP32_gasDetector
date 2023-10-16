#include "driver/adc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include <string.h>
#include <stdbool.h>

#define ADC_MQ2 ADC1_CHANNEL_7
#define ADC_MQ7 ADC1_CHANNEL_6
#define ADC_MQ9 ADC1_CHANNEL_4
#define ADC_MQ135 ADC1_CHANNEL_5
#define ADC_ATTEN ADC_ATTEN_DB_11
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define ADC_VREF 5000 // mV

#define MQ2_LEVEL 0.5 // 0.5%, 5000ppm
#define MQ7_LEVEL 0.15 // CO 174ppm
#define MQ9_LEVEL 0.5 // 0.5%, 5000ppm
#define MQ135_LEVEL 0.2 // 0.2%, 2000ppm

#define GPIO_OUTPUT_PIN 22

#define tag "gasDetector"

void app_main(void)
{
    // screen variable init
    SSD1306_t dev;

    // Initialize the ADC and set the attenuation
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_MQ2, ADC_ATTEN);
    char tmp[16];
    float mq[4] = {0.0, 0.0, 0.0, 0.0};
    bool warn = false;
    unsigned short cnt = 0;

    // Configure gpio22 as output mode
    esp_rom_gpio_pad_select_gpio(GPIO_OUTPUT_PIN);
    gpio_set_direction(GPIO_OUTPUT_PIN, GPIO_MODE_OUTPUT);

    // screen SPI init
    ESP_LOGI(tag, "INTERFACE is SPI");
    ESP_LOGI(tag, "CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
    ESP_LOGI(tag, "CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
    ESP_LOGI(tag, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
    ESP_LOGI(tag, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
    ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);

    // screen init
    ESP_LOGI(tag, "Panel is 128x64");
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);

    // for(int i = 0; i < 60; i++){
    //     // display i on ssd1306
    //     ssd1306_clear_screen(&dev, false);
    //     ssd1306_display_text(&dev, 0, "Heating", 7, false);
    //     memset(tmp, 0, sizeof(tmp)); // Fill the array with zeros
    //     sprintf(tmp, "remain: %.2d", (int)(60-i));
    //     ssd1306_display_text(&dev, 4, tmp, 10, false);
 
    //     // delay 1s
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }


    // Get the raw ADC value
    while (1)
    {
        // get gas detector analog input
        mq[0] = adc1_get_raw(ADC_MQ2) / 4095.0;
        mq[1] = adc1_get_raw(ADC_MQ7) / 4095.0;
        mq[2] = adc1_get_raw(ADC_MQ9) / 4095.0;
        mq[3] = adc1_get_raw(ADC_MQ135) / 4095.0;

        if (mq[0] > MQ2_LEVEL || mq[1] > MQ7_LEVEL || mq[2] > MQ9_LEVEL || mq[3] > MQ135_LEVEL)
        {
            gpio_set_level(GPIO_OUTPUT_PIN, 1);
            warn = true;
        }
        else
        {
            gpio_set_level(GPIO_OUTPUT_PIN, 0);
            warn = false;
        }

        // Calculate the voltage value in mV
        // int voltage = (raw * ADC_EXAMPLE_VREF) / 4095;

        // Print the voltage value
        // printf("%d: %.2f %.2f %.2f %.2f\n", count++, mq[0], mq[1], mq[2], mq[3]);

        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "GasDetector", 11, false);
        memset(tmp, 0, sizeof(tmp)); // Fill the array with zeros
        sprintf(tmp, "KeRan1: %.5d", (int)(mq[0]*10000));
        ssd1306_display_text(&dev, 1, tmp, 16, false);
        memset(tmp, 0, sizeof(tmp)); // Fill the array with zeros
        sprintf(tmp, "CO: %.4d", (int)(mq[1]*1000));
        ssd1306_display_text(&dev, 2, tmp, 16, false);
        memset(tmp, 0, sizeof(tmp)); // Fill the array with zeros
        sprintf(tmp, "KeRan2: %.5d", (int)(mq[2]*10000));
        ssd1306_display_text(&dev, 3, tmp, 16, false);
        memset(tmp, 0, sizeof(tmp)); // Fill the array with zeros
        sprintf(tmp, "Quality: %.4d", (int)(mq[3]*1000));
        ssd1306_display_text(&dev, 4, tmp, 16, false);
        ssd1306_display_text(&dev, 5, "unit: ppm", 9, false);
        memset(tmp, 0, sizeof(tmp)); // Fill the array with zeros
        if (cnt<99){
            cnt++;
        }
        sprintf(tmp, "boot: %.2ds", cnt);
        ssd1306_display_text(&dev, 6, tmp, 9, false);
        if (warn){
            ssd1306_display_text(&dev, 7, "Warning!!!", 10, false);
        }

        // Wait for 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
}