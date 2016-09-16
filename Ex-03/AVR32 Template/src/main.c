/**
 * AVR32 UC3 template for TTK4147
 */

// include header files for all drivers that have been imported from AVR Software Framework (ASF).
#include <asf.h>
#include <board.h>
#include <gpio.h>
#include <sysclk.h>
#include "busy_delay.h"

// defines for USB UART
#define CONFIG_USART_IF (AVR32_USART2)

// defines for BRTT interface
#define TEST_A AVR32_PIN_PA31
#define RESPONSE_A AVR32_PIN_PA30
#define TEST_B AVR32_PIN_PA29
#define RESPONSE_B AVR32_PIN_PA28
#define TEST_C AVR32_PIN_PA27
#define RESPONSE_C AVR32_PIN_PB00

// declare interrupts
__attribute__((__interrupt__)) 
static void interrupt_J3(void);

static void init(void)
{
	// board init
	sysclk_init();
	board_init();
	busy_delay_init(BOARD_OSC0_HZ);
	
	// interrupts init
	cpu_irq_disable();
    INTC_init_interrupts();
	INTC_register_interrupt(&interrupt_J3,   AVR32_GPIO_IRQ_3, AVR32_INTC_INT1);
	cpu_irq_enable();

	//  stdio init
	stdio_usb_init(&CONFIG_USART_IF);

	// Specify that stdout and stdin should not be buffered.

#if defined(__GNUC__) && defined(__AVR32__)
	setbuf(stdout, NULL);
	setbuf(stdin,  NULL);
#endif
}

/*********************************************************************
User decelerations
*********************************************************************/

static inline void testInterrupt(uint testPin,uint responsePin){
	if (gpio_get_pin_interrupt_flag(testPin)){
		gpio_set_pin_low(responsePin);
		busy_delay_us(5);
		gpio_set_pin_high(responsePin);
		gpio_clear_pin_interrupt_flag(testPin);
	}
}

volatile uint8_t A = 0, B = 0, C = 0;

static inline void testYolo(uint testPin, uint responsePin){
	if (gpio_pin_is_low(testPin)){
		gpio_set_pin_low(responsePin);
		busy_delay_us(5);
		gpio_set_pin_high(responsePin);
	}		
}

/*********************************************************************
Interrupt routines
*********************************************************************/
__attribute__((__interrupt__)) 
static void interrupt_J3(void) { 
	/*
	testInterrupt(TEST_A,RESPONSE_A);
	testInterrupt(TEST_B,RESPONSE_B);
	testInterrupt(TEST_C,RESPONSE_C);
	*/
	if (gpio_get_pin_interrupt_flag(TEST_A)){
		A=1;
		gpio_clear_pin_interrupt_flag(TEST_A);
	}	
	else if (gpio_get_pin_interrupt_flag(TEST_B)){
		B=1;
		gpio_clear_pin_interrupt_flag(TEST_B);
	}
	else if (gpio_get_pin_interrupt_flag(TEST_C)){
		C=1;
		gpio_clear_pin_interrupt_flag(TEST_C);
	}
}

static void initInterrupts(void){
	gpio_enable_pin_interrupt(TEST_A,GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(TEST_B,GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(TEST_C,GPIO_FALLING_EDGE);
}

static void initPins(void){
	gpio_configure_pin(TEST_A,GPIO_DIR_INPUT);
	gpio_configure_pin(TEST_A,GPIO_PULL_DOWN);
	gpio_configure_pin(TEST_B,GPIO_DIR_INPUT);
	gpio_configure_pin(TEST_B,GPIO_PULL_DOWN);
	gpio_configure_pin(TEST_C,GPIO_DIR_INPUT);
	gpio_configure_pin(TEST_C,GPIO_PULL_DOWN);
	
	gpio_configure_pin(RESPONSE_A,GPIO_DIR_OUTPUT);
	gpio_configure_pin(RESPONSE_B,GPIO_DIR_OUTPUT);
	gpio_configure_pin(RESPONSE_C,GPIO_DIR_OUTPUT);
	gpio_set_pin_high(RESPONSE_A);
	gpio_set_pin_high(RESPONSE_B);
	gpio_set_pin_high(RESPONSE_C);
}





/*********************************************************************
Functions
*********************************************************************/
int main (void)
{
	// initialize
	init();
	initPins();
	initInterrupts();
	
	//// BUSY WAIT
	/*
	while (1) {
		testYolo(TEST_A,RESPONSE_A);
		//testYolo(TEST_B,RESPONSE_B);
		//testYolo(TEST_C,RESPONSE_C);
	}
	*/
	
	//BIGWHILE
	while(1){
		if (A){
			gpio_set_pin_low(RESPONSE_A);
			busy_delay_us(5);
			gpio_set_pin_high(RESPONSE_A);
			A=0;
		}
		else if (B){
			gpio_set_pin_low(RESPONSE_B);
			busy_delay_us(5);
			gpio_set_pin_high(RESPONSE_B);
			B=0;
		}
		else if (C){
			gpio_set_pin_low(RESPONSE_C);
			busy_delay_us(5);
			gpio_set_pin_high(RESPONSE_C);
			C=0;
		}
	}
	
	//INTERRUPT
	/*
	while(1)
	{
		gpio_toggle_pin(LED0_GPIO);
		busy_delay_ms(500);
	}
	*/
	
}
