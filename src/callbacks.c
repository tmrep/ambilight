/**
 * @file callbacks.c
 * @author Tomas Michalek (mi.tom@seznam.cz)
 * @brief This file groups implementation of the used callback functions.
 * @version 0.1
 * @date 2020-09-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <stdint.h>
#include "neopixel.h"

/**
 * @brief Callback processing data received over USB virtual COM port.
 * 
 * @param frame_buf received raw bytes
 * @param len number of received bytes
 */
void usb_rx_callback(uint8_t * frame_buf, uint16_t len) {
    neopixel_update_buffer(frame_buf, len);
}