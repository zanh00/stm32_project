/*
 * Drivers.c
 *
 *  Created on: Jun 23, 2021
 *      Author: Žan Hertiš
 */
#include "Drivers.h"

void gpio_INIT(){
	///////////////////Clock Enable /////////////////////////////////
	//RCC->AHB2ENR |= 1<<0; //Enable clock GPIOA
	//RCC_AHB2ENR_POINTER |= 1<<0;
	//RCC->AHB2ENR |= 1<<2; // Enable clock GPIOC
	*((uint32_t*)(0x40021000+0x4c)) |= 1<<2 | 1<<0; //GPIOC in GPIOA enable clock

	///////////////////PA5 built in led /////////////////////////////
	GPIOA->MODER &= ~(0x3 << (10));
	GPIOA->MODER |= (0x1<<10); //output
	GPIOA->OTYPER &= ~(1<<5); //push pull
	////////////////////////////////////////////////////////////

	//////////////PA0 in PA1 led//////////////////////////
	GPIOA->MODER &= ~(0x3 << 0); // izbris
	GPIOA->MODER |= (0x2 << 0); //alternate function mode PA0
	GPIOA->OTYPER &= ~(1<<0); //push pull PA0
	GPIOA->AFRL |= 0x1<<0; //AF1 na PA0

	GPIOA->MODER &= ~(0x3 << 2); //izbris
	GPIOA->MODER |= (0x2 << 2); //alternate function mode PA1
	GPIOA->OTYPER |= (1<<1); //open drain PA1
	GPIOA->AFRL |= 0x1<<4; //AF1 na PA1
	////////////PC13 tipka/////////////////////
	GPIOC->MODER &= ~(0x3 << (26)); //input
	GPIOC->PUPDR &= ~(0x3 << (26)); //no pull up no pull down
	//////////////////////////////////////////////

}

void tim_INIT(){
	//RCC->APB1ENR1 |= 1<<0;
	*((uint32_t*)(0x40021000+0x58)) |= 1<<0; //Timer 2 clock enable
	//*((uint32_t*)(0x40021000+0x58)) |= 1<<3; // 32 butni Timer 5 clock enable
	*((uint32_t*)(0x40021000+0x58)) |= 1<<1; // 16-bitni Timer 3
	///////////////////TIM5//////////////////////////////////
	TIM3->CR1 &= ~(1<<7); //ARPE: TIMx_ARR register is not buffered
	TIM3->CR1 &= ~(0x3<<5); //CMS: 00: Edge-aligned mode.
	TIM3->CR1 &= ~(1<<4); //DIR(direction) 0-up counter

	TIM3->PSC = 4000-1; //presclaer = 100
	//TIM3->ARR = 0xffffffff-1; // auto reload register = največja 32 bitna vrednost

	TIM3->CR1 |= 1<<0; // Counter enable (1)



}

void PWM_INIT(){
	//CC1S- capture-compare; 00 - channel 1 output
	TIM2->CCMR1 &= ~(1<<0);
	TIM2->CCMR1 &= ~(1<<1);

	//CC2S - capture-compare; 00 - channel 2 output
	TIM2->CCMR1 &= ~(1<<8);
	TIM2->CCMR1 &= ~(1<<9);

	TIM2->CCMR1 |= (0x3<<5);// OC1M 0110 PWM 1 mode upcounter - channel 1
	TIM2->CCMR1 |= (0x3<<13);// OC2M 0110 PWM 1 mode upcounter - channel 2

	TIM2->CCMR1 |= 1<<3; // OC1PE Preload register on TIMx_CCR1 enabled (preload enable)
	TIM2->CCMR1 |= 1<<11; //OC2PE: Output compare 2 preload enable

	TIM2->CCER |= 1<<0; // CC1E Capture/Compare 1 output enable.
	TIM2->CCER |= 1<<4; // CC2E: Capture/Compare 2 output enable.

	TIM2->CR1 &= ~(0x3<<5); //CMS: 00: Edge-aligned mode.
	TIM2->CR1 &= ~(1<<4); //DIR(direction) 0-up counter

	TIM2->CR1 &= ~(1<<7); //ARPE: TIMx_ARR register is not buffered
	TIM2->CR1 |= 1<<0; // Counter enable (1)

	TIM2->PSC = 100; //presclaer = 100
	TIM2->ARR = 400; // auto reload register = 400





}

void ledON(void){
	GPIOA->ODR |= (1<<5); //led ON
}

uint8_t button_pushed (void){
	return !((GPIOC->IDR) & (1<<13));
}

void USART2_INIT(void)
{
	*((uint32_t*)(0x40021000+0x58)) |= 1<<17; // USART2 clock enable

	//PA2->Tx
	//PA3->Rx

	GPIOA->MODER &= ~(0x3 << 4); // izbris
	GPIOA->MODER |= (0x2 << 4); //alternate function mode PA2

	GPIOA->MODER &= ~(0x3 << 6); // izbris
	GPIOA->MODER |= (0x2 << 6); //alternate function mode PA3

	GPIOA->OTYPER |= 1<<3; // Rx-> open drain

	GPIOA->AFRL |= 0x7<<8; // AF7 za PA2
	GPIOA->AFRL |= 0x7<<12; // AF7 za PA3

	USART2->CR1 &= ~(1<<0); //disable USART2
	USART2->CR1 |= 0x3<<2; // enable Tx and Rx

	/*
	 * Baud Rate:
	 * USARTDIV = 4000000/115200 = 34,72 -> 35 = 0x23
	 *
	 * */

	USART2->BRR = 0x23;

	////////////////////// Interupt nastavitve //////////////////

	USART2->CR1 |= 1<<5; // enable receive interupt: RXNEIE
	NVIC->ISER[1] |= 1<< (38-32); // 38 linija interupta

	////////////////////////////////////////////////////////////
	USART2->CR1 |= 1<<0; // enable usart2

}


void usart_transmit (uint8_t *tx_buffer, uint8_t dolzina)
{
	uint8_t poslji;
	uint8_t st_poslanih_znakov = 0;

	while (1)
	{
		poslji = *tx_buffer++;
		if(poslji==0 || st_poslanih_znakov == dolzina) break;

				while(!(USART2->ISR & (1<<7))); // TXE -> čakamo da pošlje
				USART2->TDR = poslji & 0xff;
				st_poslanih_znakov++;

	}

	while(!(USART2->ISR & (1<<6))); // TC - transmission complete
}

void usart_receive(uint8_t *rx_buffer, uint16_t dolzina){
	uint8_t prejmi;
	uint8_t i = 0;

	while(1){

		if(i == dolzina || prejmi == '\r') break;
		while(!(USART2->ISR & (1<<5))); // RXNE
		prejmi = USART2->RDR & 0xff;
		rx_buffer[i] = prejmi;
		i++;
	}
	prejmi = 0;
}



