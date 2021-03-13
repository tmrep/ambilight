/**
 * @file neopixel.h
 * @author Tomas Michalek (mi.tom@seznam.cz)
 * @brief Module for controlling the LED strip (WS2812B alias Neopixel)
 * @version 0.1
 * @date 2020-09-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef __NEOPIXEL_H
#define __NEOPIXEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* defines */
#define NEOPIXEL_NUM_OF_LEDS 85
#define NEOPIXEL_NO_BLOCKING 0
#define NEOPIXEL_BLOCKING 1

/**
 * @brief Sets the color of an individual LED using RGB color representation (just in an internal buffer).
 * 
 * @param led_num Ordinal number of the LED in a strip
 * @param r red component of the RGB color representation
 * @param g green component of the RGB color representation
 * @param b blue component of the RGB color representation
 * @return uint8_t 1 in case of success, 0 otherwise (not so many LEDs)
 */
uint8_t neopixel_set_color(uint8_t led_num, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Updates the illuminated colors to the current state of the internal buffer.
 * 
 * @param block true if the function should return only after the whole LED strip update procedure ends, 
 * false if the function should only trigger the update, do not block and return immediatelly
 * @return uint8_t 1 in case of success, 0 otherwise (already updating)
 */
uint8_t neopixel_update(uint8_t block);

/**
 * @brief Central place for parsing the events handled by neopixel module. 
 * Should be called from the respective DMA and ISR routines.
 * 
 */
void neopixel_updating_state_machine(void); 

/**
 * @brief Update the whole internal buffer representing the colors of the LED strip.
 * 
 * @param buf New content of the buffer
 * @param len Size of the buffer
 */
void neopixel_update_buffer(uint8_t * buf, uint16_t len);

/**
 * @brief Indicates the presence of an Ambilight device to the PC by sending a special signature.
 * 
 */
void neopixel_send_heartbeat(void);

#ifdef __cplusplus
}
#endif

#endif /* __NEOPIXEL_H */
