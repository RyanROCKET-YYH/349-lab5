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

/** @brief ADON: A/DConverterON/OFF */
#define ADC1_CR2_ADON  (1 << 0)

/** @brief higher bit of RES */
#define ADC1_CR1_RES  (1 << 25)

/** @brief lower bit of RES */
#define ADC1_CR1_RES  (1 << 24)

/** @brief Start conversion of regular channels */
#define ADC1_CR2_SWSTART  (1 << 30)

/** @brief Continuous conversion */
#define ADC1_CR2_CONT  (1 << 1)


void adc_init(){
	// set rcc
	struct rcc_reg_map *rcc = RCC_BASE;
	rcc->apb2_enr |= ADC_CLKEN;

	// GPIO Pins(PA_1, A1)(ADC1/1) 
	gpio_init(GPIO_A, 1, MODE_ANALOG_INPUT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);

	// set adc
	struct adc_reg_map *adc = ADC1_BASE;

	// turn on adc power
	adc->CR2 |= ADC1_CR2_ADON;
	// single conversion mode
	adc->CR2 &= ~(ADC1_CR2_CONT); // enable single conversion mode
	adc->CR2 |= ADC1_CR2_SWSTART;

	// set 10 bit resolution for the ADC
	adc->CR1 &= ~(ADC1_CR1_RES); // 0
	adc->CR1 |= ADC1_CR1_RES; // 1


	return;
}

uint16_t adc_read_chan(uint8_t chan){
	return -1;
}
