/**
 * @file main.c
 *
 * @brief some helper functions and the main funtion
 *
 * @date 04/04/2024
 *
 * @author Yuhong Yao (yuhongy), Yiying Li (yiyingl4)
 */

#include <FreeRTOS.h>
#include <task.h>
#include <adc.h>
#include <stdio.h>
#include <stdlib.h>
#include <uart.h>

#define ADC_REF_VOLTAGE (3.3f)
#define ADC_MAX_VAL (1023.0f)
#define LIGHT_SENSOR_CHAN (0)
#define TEMP_SENOR_CHAN (1)
#define SAMPLE (10)

void vLIGHTReadTask(void *pvParameters) {
    (void)pvParameters;
     
    while(1) {
        // uint16_t adcValue = adc_read_chan(LIGHT_SENSOR_CHAN);
        // printf("Light ADC Value: %u\n", adcValue);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTEMPTask(void *pvParameters) {
    (void)pvParameters;
    float total = 0;
    float temp[SAMPLE] = {0};
    int index = 0;
    float offsetC = 15.3f;

    while(1) {
        uint16_t temp_adc = adc_read_chan(TEMP_SENOR_CHAN);
        float vout = ((float)temp_adc / ADC_MAX_VAL) * ADC_REF_VOLTAGE;
        float tempC = (vout * 100.0f) - offsetC;

        // rolling average for temp reading
        total -= temp[index];
        temp[index] = tempC;
        total += temp[index];
        index++;
        if (index >= SAMPLE) {
            index = 0;
        }
        float avgTempC = total / SAMPLE;
        float avgTempF = (avgTempC * 9.0f / 5.0f) + 32.0f;

        int tempF_int = (int)(avgTempF * 100);
        int tempC_int = (int)(avgTempC * 100);
        
        // printf("Raw ADC: %u\n", temp_adc);

        printf("Avg Temp: %d.%02d°C or %d.%02d°F\n",
               tempC_int / 100, abs(tempC_int % 100),
               tempF_int / 100, abs(tempF_int % 100));

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

int main( void ) {
    uart_init(115200);
    adc_init();
    xTaskCreate(
        vLIGHTReadTask,
        "LIGHTRead",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    // temperature sensor
    xTaskCreate(
        vTEMPTask,
        "TEMPRead",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskStartScheduler();
    
    // Infinite loop
    for(;;) {}  
    return 0;
}
