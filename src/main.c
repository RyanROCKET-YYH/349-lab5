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
#include <atcmd.h>
#include <gpio.h>
#include <unistd.h>
#include <string.h>
#include <lcd_driver.h>
#include <keypad_driver.h>
#include <servo.h>
#include <stdbool.h>
#include <i2c.h>


#define ADC_REF_VOLTAGE (3.3f)
#define ADC_MAX_VAL (1023.0f)
#define LIGHT_SENSOR_CHAN (0)
#define TEMP_SENOR_CHAN (1)
#define SAMPLE (10)

/** @brief define MAX_PASSCODE_LENGTH */
#define MAX_PASSCODE_LENGTH 12
/** @brief define LOCKED_POSITION */
#define LOCKED_POSITION 0
/** @brief define UNLOCKED_POSITION */
#define UNLOCKED_POSITION 180

/** @brief define parser */
atcmd_parser_t parser;
/** @brief define passcode */
int g_passcode = 349;

/**
 * @brief  handle the AT+Resume command
 * 
*/
uint8_t handleResume(void *args, const char *cmdArgs) {
    (void)args;
    (void)cmdArgs;
    isInCommandMode = false;
    printf("Exit command mode\n");
    return 1; // sucess
}

/**
 * @brief  handle the AT+Hello=<> command
 *
*/
uint8_t handleHello(void *args, const char *cmdArgs) {
    (void)args;
    if (cmdArgs != NULL) {
        printf("Hello, %s!\n", cmdArgs);
    } else {
        printf("Hello!\n"); //if <name> not provided
    }
    return 1; // success
}

/**
 * @brief  handle the AT+Passcpde? command
 *
*/
uint8_t handlePasscode(void *args, const char *cmdArgs) {
    (void)args;
    (void)cmdArgs;
    printf("Current passcode: %d\n", g_passcode);
    return 1; // success
}

/**
 * @brief  handle the AT+Passcpde=<> command
 *
*/
uint8_t handlePasscodeChange(void *args, const char *cmdArgs) {
    (void)args; 

    if (cmdArgs != NULL && strlen(cmdArgs) <= 12) {
        int new_passcode = atoi(cmdArgs);
        g_passcode = new_passcode;
        printf("Passcode changed successfully.\n");
        return 1; // Success
    } else if (cmdArgs == NULL) {
        printf("No passcode provided.\n");
    } else {
        printf("Invalid passcode (string up to 12 digits).\n");
    }
    return 0; // Fail
}

/**
 * @brief  handle the commands:
 * AT+RESUME
 * AT+HELLO=<>
 * AT+PASSCODE?
 * AT+PASSCODE=<>
*/
const atcmd_t commands[] = {
    {"RESUME", handleResume, NULL},
    {"HELLO", handleHello, NULL},
    {"PASSCODE?", handlePasscode, NULL},
    {"PASSCODE", handlePasscodeChange, NULL}
};


/**
 * @brief  handle the Blinky LED task
 *
*/
void vBlinkyTask(void *pvParameters) {
    (void)pvParameters;
   
    gpio_init(GPIO_A, 5, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);

    for(;;) {
        if (gpio_read(GPIO_A, 5)) {
            gpio_clr(GPIO_A, 5);
        } else {
            gpio_set(GPIO_A, 5);
        }

        // wait for 1000 millisec
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief  handle the UART echo task
 *
*/
static void vUARTEchoTask(void *pvParameters) {
    (void)pvParameters;
    char buffer[100];
    // char testCmd2[] = "AT+RESUME";

    for (;;) {
        // only work when command mode
        if (isInCommandMode){
            write(STDOUT_FILENO, "> ", 2);
            // Attempt to read data from UART
            memset(buffer, 0, sizeof(buffer)); // clear buffer
            ssize_t numBytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            // check if data was read
            if (numBytesRead > 0) {
                buffer[numBytesRead] = '\0'; // Null-terminate the received string

                // Trim newline and carriage return from the end of the command
                for (int i = 0; i < numBytesRead; ++i) {
                    if (buffer[i] == '\n' || buffer[i] == '\r') {
                        buffer[i] = '\0';
                        break; // Stop at the first newline/carriage return character
                    }
                }
                // Now pass the trimmed and null-terminated command string to atcmd_parse
                atcmd_parse(&parser, buffer);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief  handle the keypad and servo task
 *
*/
void vKeypadServoLCDTask(void *pvParameters) {
    (void)pvParameters;
    char passcode[MAX_PASSCODE_LENGTH] = {0};
    char prompt[] = "Enter password:";
    uint8_t index = 0;
    uint8_t is_locked = 1;
    lcd_driver_init();

    // SERVO 1 (A0)
    gpio_init(GPIO_A, 10, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    // SERVO 2 (A1)
    // gpio_init(GPIO_A, 1, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    servo_enable(0, 1);
    servo_set(0,LOCKED_POSITION); // initialized to lock state
    lcd_clear();
    lcd_print(prompt);

    for (;;) {
        // when it is command mode, it doesn't read keypad
        if (!isInCommandMode){
            char key = keypad_read();
            if (key != '\0') {
                if ((key >= '0' && key <= '9') && index < MAX_PASSCODE_LENGTH) {
                    passcode[index++] = key;
                    passcode[index] = '\0';
                    
                    lcd_set_cursor(1, 0);
                    lcd_print("                "); // Clear the second line by printing spaces
                    lcd_set_cursor(1, 0);
                    lcd_print(passcode);
                } else if (key == '#') {
                    passcode[index] = '\0';
                    int password = atoi(passcode);
                    if (password == g_passcode) {
                        is_locked = !is_locked;
                        servo_set(0, is_locked ? LOCKED_POSITION : UNLOCKED_POSITION);
                        lcd_clear();
                        lcd_print(is_locked ? "Locking!" : "Unlocking!");
                        const char* message = is_locked ? "Locking!\n" : "Unlocking!\n";
                        write(STDOUT_FILENO, message, strlen(message));
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        lcd_clear();
                        lcd_print(prompt);
                    } else {
                        lcd_clear();
                        lcd_print("Incorrect Passco");
                        lcd_set_cursor(1,0);
                        lcd_print("de...");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        lcd_clear();
                        lcd_print(prompt);
                    }
                    
                    memset(passcode, 0, sizeof(passcode)); // Clear angle string
                    index = 0;
                }
            } 
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Polling delay
    }
}


/**
 * @brief  handle the escape task // 8.4
 *
*/
void escapeSequenceTask(void *pvParameters) {
    (void)pvParameters;
    char byte;
    while (1) {
        if (!isInCommandMode) {
            if (uart_get_byte(&byte) == 0) { 
                // Directly pass each byte to atcmd_detect_escape
                if (atcmd_detect_escape(NULL, byte)) {
                    isInCommandMode = 1;
                    printf("Entering Command Mode.\n");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vFlashDetectTask(void *pvParameters) {
    (void)pvParameters;
    //led 
    gpio_init(GPIO_A, 4, MODE_GP_OUTPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
    int flashCount = 0;
    uint16_t last_light_adc = adc_read_chan(LIGHT_SENSOR_CHAN);
    const uint16_t flashThreshold = 150;
    bool flashDetected = false;

    for(;;) {
        uint16_t new_light_adc = adc_read_chan(LIGHT_SENSOR_CHAN);
        if (((new_light_adc - last_light_adc) >= flashThreshold) && !flashDetected) {
            flashDetected = true;
        }
        // Once a flash is detected, look for an increase back to normal (decrease in ADC reading)
        else if (((last_light_adc - new_light_adc) >= flashThreshold) && flashDetected) {
            flashCount++;
            flashDetected = false;
            if (flashCount == 5) {
                if (gpio_read(GPIO_A, 4)) {
                    gpio_clr(GPIO_A, 4);
                } else {
                    gpio_set(GPIO_A, 4);
                }
                flashCount = 0;
            }
        }
        // if (!isInCommandMode) {
        //     uint16_t adcValue = adc_read_chan(LIGHT_SENSOR_CHAN);
        //     printf("Light ADC Value: %u\n", adcValue);
        // }
        last_light_adc = new_light_adc;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTEMPTask(void *pvParameters) {
    (void)pvParameters;
    float total = 0;
    float temp[SAMPLE] = {0};
    int index = 0;
    float offsetC = 24.3f;

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
        if (!isInCommandMode) {
            printf("Avg Temp: %d.%02d°C or %d.%02d°F\n",
                tempC_int / 100, abs(tempC_int % 100),
                tempF_int / 100, abs(tempF_int % 100));
        }
       
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}

int main( void ) {
    uart_init(115200);
    adc_init();
    keypad_init();
    i2c_master_init(80);
    atcmd_parser_init(&parser, commands, (sizeof(commands) / sizeof(commands[0])));

    xTaskCreate(
        vBlinkyTask,
        "BlinkyTask",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    // Create the UART echo task
    xTaskCreate(
        vUARTEchoTask,
        "UARTEcho",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL); 

    // servo keypad
    xTaskCreate(
        vKeypadServoLCDTask, 
        "KeypadServoLCD", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL);

    xTaskCreate(
        escapeSequenceTask, 
        "ENTERCommand", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL);

    xTaskCreate(
        vFlashDetectTask,
        "FlashDetect",
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
