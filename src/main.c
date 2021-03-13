/**
 * @file main.c
 * @author Tomas Michalek (mi.tom@seznam.cz)
 * @brief Ambilight system for background illumination of a PC monitor
 * @version 0.1
 * @date 2020-09-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "bsp.h"
#include "neopixel.h"

int main(void)
{
	bsp_initialize();

	/* initial test of color settings */
	uint8_t max_intensity = 255;
	for (size_t num_of_blinks = 0; num_of_blinks < 1; num_of_blinks++)
	{
		for (size_t intensity = 0; intensity < max_intensity; intensity++)
		{
			for (uint8_t i = 0; i < NEOPIXEL_NUM_OF_LEDS; i++)
			{
				neopixel_set_color(i, intensity, intensity, intensity);
			}
			neopixel_update(NEOPIXEL_BLOCKING);
			bsp_delayms(1);
		}
		for (size_t intensity = max_intensity-1; intensity > 0; intensity--)
		{
			for (uint8_t i = 0; i < NEOPIXEL_NUM_OF_LEDS; i++)
			{
				neopixel_set_color(i, intensity, intensity, intensity);
			}
			neopixel_update(NEOPIXEL_BLOCKING);
			bsp_delayms(1);
		}
	}

	/* obtain fresh color data from PC and update the LEDs */
	while (1)
	{
		while(!neopixel_update(NEOPIXEL_NO_BLOCKING));
		usb_poll();
	}
	return 0;
}