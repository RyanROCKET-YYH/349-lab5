/**
 * @file main.c
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <FreeRTOS.h>
#include <task.h>
#include <adc.h>
#include <stdio.h>
#include <uart.h>


void vADCReadTask(void *pvParameters) {
    (void)pvParameters;
    adc_init(); 
    while(1) {
        uint16_t adcValue = adc_read_chan(0);
        printf("ADC Value: %u\n", adcValue);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

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
