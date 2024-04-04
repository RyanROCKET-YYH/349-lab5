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

/**
 *
 * @brief The task of ADC read
 * 
 */
void vADCReadTask(void *pvParameters) {
    (void)pvParameters;
    adc_init(); 
    while(1) {
        uint16_t adcValue = adc_read_chan(0);
        printf("ADC Value: %u\n", adcValue);

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

    xTaskCreate(
        vADCReadTask,
        "ADCRead",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskStartScheduler();
    
    // Infinite loop
    for(;;) {}  
    return 0;
}
