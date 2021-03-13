/**
 * @file bsp.h
 * @author Tomas Michalek (mi.tom@seznam.cz)
 * @brief Board support package (BSP) for the Ambilight application
 * @version 0.1
 * @date 2020-09-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include "usb.h"

// Neopixel dataline
#define NEOPIXEL_PORT GPIOA
#define NEOPIXEL_PIN GPIO0
// indicator LED
#define LED_PORT GPIOC
#define LED_PIN GPIO13

/**
 * @brief Initialize the used hardware peripherals
 * 
 */
void bsp_initialize(void);

/**
 * @brief Simple bussy-wait implementation of time delay
 * 
 * @param ms Time delay in milliseconds
 */
void bsp_delayms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */
