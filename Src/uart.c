#include "uart.h"

#define GPIOAEN			(1U<<0)
#define UART2EN			(1U<<17)
#define CR1_TE 			(1U<<3)
#define CR1_UE 			(1U<<0)

#define SR_TXE 			(1U<<7)


#define SYS_FREQ		4000000
#define APB1_CLK   		SYS_FREQ

#define UART_BAUDRATE	115200

static void uart2_set_baudrate(uint32_t periph_clk, uint32_t baudrate);
static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate);


void uart2_write(int ch);

int __io_putchar(int ch)
{
	uart2_write(ch);

	return ch;
}


void uart2_tx_init(void)
{
	/*** configure uart gpio pin ***/
	/* enable clock accses */
	RCC->AHB2ENR |= GPIOAEN;

	/* set pa2 mode to af mode */
	GPIOA->MODER &=~(1U<<4);
	GPIOA->MODER |=(1U<<5);

	/* set pa2 af type to tx */
	GPIOA->AFR[0] |=(1U<<8);
	GPIOA->AFR[0] |=(1U<<9);
	GPIOA->AFR[0] |=(1U<<10);
	GPIOA->AFR[0] &=~(1U<<11);

	/*** configure uart ***/
	/* enable clock accses to uart 2 */
	RCC->APB1ENR1 |= UART2EN;

	/* configure baud rate */
	uart2_set_baudrate(APB1_CLK, UART_BAUDRATE);

	/* configure transfer direction */
	USART2->CR1 = CR1_TE;

	/* enable uart module */
	USART2->CR1 |= CR1_UE;
}

void uart2_write(int ch)
{
	/* make sure transmit data is empty */
	while(!(USART2->ISR & SR_TXE)){}

	/* write to transmit data register */
	USART2->TDR = (ch & 0xFFF);
}


static void uart2_set_baudrate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR = compute_uart_bd(periph_clk, baudrate);
}

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate)
{
	return ((periph_clk + (baudrate/2U))/baudrate);
}

