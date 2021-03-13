/**
 * @file interrupts.c
 * @author Tomas Michalek (mi.tom@seznam.cz)
 * @brief This file groups implementation of the utilized interrupt service routines (ISRs).
 * @version 0.1
 * @date 2020-09-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <libopencm3/cm3/nvic.h>
#include "neopixel.h"

/**
 * @brief DMA half transfer / transfer complete interrupt used for updating the content of a ping-pong buffer
 * 
 */
void dma1_channel5_isr(void)	
{
	neopixel_updating_state_machine();
}

/**
 * @brief timer output compare interrupt used for triggering the DMA requests
 * 
 */
void tim2_isr(void)
{
	neopixel_updating_state_machine();
}

/**
 * @brief timer overflow interrupt for signalizing the presence of the device in case of prolonged 
 * communication inactivity.
 * 
 */
void tim3_isr(void)
{
	neopixel_send_heartbeat();
}