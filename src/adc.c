/**
 * @file adc.c
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <gpio.h>
#include <stdint.h>
#include <rcc.h>
#include <unistd.h>
#include <adc.h>

/** @brief The ADC register map. */
struct adc_reg_map {
	volatile uint32_t SR;      /**<  status register  */
    volatile uint32_t CR1;      /**<  Control Register 1 */
    volatile uint32_t CR2;      /**<  Control Register 2 */
    volatile uint32_t SMPR1;     /**<  sample time register 1 */
    volatile uint32_t SMPR2;     /**<  sample time register 2 */
	volatile uint32_t JOFR1;     /**< injected channel data offset register 1  */
	volatile uint32_t JOFR2;     /**<  injected channel data offset register 2 */
	volatile uint32_t JOFR3;     /**<  injected channel data offset register 3 */
	volatile uint32_t JOFR4;     /**<  injected channel data offset register 4 */
	volatile uint32_t HTR;     /**<  watchdog higher threshold register */
	volatile uint32_t LTR;     /**<  watchdog lower threshold register */
	volatile uint32_t SQR1;     /**<  regular sequence register 1 */
	volatile uint32_t SQR2;     /**<  regular sequence register 2 */
	volatile uint32_t SQR3;     /**<  regular sequence register 3 */
	volatile uint32_t JSQR;     /**<  injected sequence register */
	volatile uint32_t JDR1;     /**<  injected data register 1 */
	volatile uint32_t JDR2;     /**<  injected data register 2 */
	volatile uint32_t JDR3;     /**<  injected data register 3 */
	volatile uint32_t JDR4;     /**<  injected data register 4 */
	volatile uint32_t DR;     /**<  regular data register */
	volatile uint32_t CCR;     /**<  common control register */
};

/** @brief Base Address of ADC1 */
#define ADC1_BASE   (struct adc_reg_map *) 0x40012000


void adc_init(){
	// set rcc
	struct rcc_reg_map *rcc = RCC_BASE;
	rcc->apb2_enr |= ADC_CLKEN;

	// GPIO Pins(D14: I2C1_SDA, D15: I2C1_SCL) TODO:
    // gpio_init(GPIO_B, 8, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT4);   /* PB_8(D15), SCL */

	// set adc
	struct rcc_reg_map *rcc = ADC1_BASE;


	return;
}

uint16_t adc_read_chan(uint8_t chan){
	return -1;
}
