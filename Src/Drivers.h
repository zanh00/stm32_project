/*
 * Drivers.h
 *
 *  Created on: Jun 23, 2021
 *      Author: Žan Hertiš
 */

#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "Definitions.h"

extern void gpio_INIT(void);
extern void ledON(void);
extern void tim_INIT(void);
extern void PWM_INIT(void);
extern uint8_t  button_pushed(void);
extern void usart_transmit (uint8_t *tx_buffer, uint8_t dolzina);
extern void usart_receive(uint8_t *rx_buffer, uint16_t dolzina);


#endif /* DRIVERS_H_ */
