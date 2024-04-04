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
#include <uart.h>

// /**
//  *
//  * @brief The task of ADC read of temperature sensor
//  * 
//  */
// void vADCReadTask(void *pvParameters) {
//     (void)pvParameters;
//     adc_init(); 
//     while(1) {
//         uint16_t adcValue = adc_read_chan(0);
//         printf("ADC Value: %u\n", adcValue);

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

/**
 *
 * @brief The task of ADC read of LM34
 * 
 */
// void vLM34ADCReadTask(void *pvParameters) {
//     (void)pvParameters;
//     adc_init(); 
//     while(1) {
//         uint16_t adcValue = adc_read_chan(1);
//         printf("ADC: %u\n", adcValue);

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

void vLM34ADCReadTask(void *pvParameters) {
    (void)pvParameters;
    adc_init();

    const int readingsCount = 10; // Number of readings for the moving average
    uint16_t adcValues[readingsCount];
    adcValues[readingsCount] = 0;
    int readIndex = 0;
    long sum = 0;
    int numReadings = 0;

    // TODO: the temperature is incorrect. Maybe it's because the process of converting the values wrong
    while(1) {
        uint16_t adcValue = adc_read_chan(1); // Read ADC value from LM34 channel
        
        // Subtract the last reading:
        sum -= adcValues[readIndex];
        // Read from the sensor:
        adcValues[readIndex] = adcValue;
        // Add the reading to the sum:
        sum += adcValues[readIndex];
        // Advance to the next position in the array:
        readIndex = (readIndex + 1) % readingsCount;
        
        // For the first few iterations until array is filled
        if(numReadings < readingsCount) {
            numReadings++;
        }

        long averageAdcValue = sum / numReadings;

        // Convert ADC value to voltage
        float voltage = (averageAdcValue / 1024.0) * 3.3;

        // Convert voltage to Fahrenheit
        float temperatureF = voltage / 0.01; // LM34: 10mV/°F

        // Convert Fahrenheit to Celsius
        float temperatureC = (5.0 / 9.0) * (temperatureF - 32);

        printf("Temperature: %ld°F, %ld°C\n", (long)(temperatureF * 100), (long)(temperatureC * 100));

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


/**
 *
 * @brief The main function
 * 
 */
int main( void ) {
    uart_init(115200);

    // temperature sensor
    xTaskCreate(
        vLM34ADCReadTask,
        "LM34ADCRead",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskStartScheduler();
    
    // Infinite loop
    for(;;) {}  
    return 0;
}
