#include "neopixel.h"
#include <string.h>
#include "bsp.h"
#include "usb.h"

/* defines */
#define BITS_PER_LED 24 /* 3 8bit colors (i.e. 3B = 24b) for each LED */
#define RESET_PULSE_PERIODS 35 /* 1 period = 1.25 us */

#define FIRST_HALF_OF_BUFFER 0
#define SECOND_HALF_OF_BUFFER 1

/* Ada protocol */
static const uint8_t magic[] = {'A','d','a'};
#define MAGICSIZE  sizeof(magic)
#define HEADERSIZE (MAGICSIZE + 3)

/* typedefs */
typedef enum{STATE_INITIALIZATION, STATE_RESET_PULSE, STATE_LED_DATA_TRANSFER, STATE_WAIT_FOR_THE_LAST, STATE_IDLE} State;

/* variables */
static volatile State curr_state = STATE_IDLE;
static volatile uint8_t next_led;
static volatile uint32_t ping_pong_buffer[2*BITS_PER_LED]; /* each bit needs whole word for setting the PWM, place for two leds color data */
static volatile uint8_t reset_pulse_counter;
static volatile uint8_t new_buffer_prepared = 0;

static uint8_t led_colors_1[3*NEOPIXEL_NUM_OF_LEDS];
static uint8_t led_colors_2[3*NEOPIXEL_NUM_OF_LEDS];
static uint8_t * ptr_led_colors = (uint8_t *) &led_colors_1;

static uint32_t led_colors_remainder[3*NEOPIXEL_NUM_OF_LEDS];

/* lookup tables for fast gamma correction of individual color channels */
/* generated from generate_luts.py script */
static const uint32_t r_gamma_lut[256] = {
     0,   3,  21,  66, 148, 276, 460, 709, 1031, 1434, 1926, 2515, 3209, 4015, 4941, 5994,
   7181, 8510, 9987, 11620, 13415, 15378, 17518, 19840, 22351, 25057, 27966, 31083, 34415, 37968, 41749, 45763,
   50018, 54518, 59271, 64283, 69559, 75105, 80928, 87033, 93427, 100115, 107103, 114398, 122004, 129927, 138175, 146751,
   155662, 164913, 174511, 184460, 194767, 205437, 216476, 227888, 239681, 251858, 264427, 277391, 290757, 304530, 318716, 333319,
   348346, 363801, 379691, 396019, 412793, 430016, 447694, 465833, 484438, 503514, 523065, 543099, 563619, 584630, 606139, 628150,
   650668, 673698, 697246, 721317, 745915, 771046, 796715, 822927, 849687, 877000, 904870, 933304, 962306, 991881, 1022034, 1052770,
   1084094, 1116010, 1148525, 1181642, 1215367, 1249704, 1284659, 1320236, 1356441, 1393277, 1430750, 1468866, 1507627, 1547040, 1587110, 1627840,
   1669237, 1711304, 1754046, 1797469, 1841577, 1886374, 1931867, 1978058, 2024953, 2072557, 2120875, 2169910, 2219669, 2270155, 2321374, 2373329,
   2426026, 2479469, 2533663, 2588613, 2644323, 2700798, 2758043, 2816061, 2874859, 2934439, 2994808, 3055969, 3117928, 3180688, 3244254, 3308632,
   3373825, 3439838, 3506675, 3574342, 3642842, 3712181, 3782362, 3853390, 3925271, 3998007, 4071604, 4146067, 4221399, 4297606, 4374691, 4452660,
   4531516, 4611264, 4691909, 4773455, 4855907, 4939268, 5023544, 5108739, 5194856, 5281902, 5369880, 5458794, 5548649, 5639449, 5731199, 5823903,
   5917565, 6012190, 6107783, 6204347, 6301887, 6400407, 6499912, 6600406, 6701893, 6804378, 6907865, 7012358, 7117862, 7224381, 7331919, 7440481,
   7550071, 7660693, 7772351, 7885051, 7998795, 8113589, 8229437, 8346342, 8464310, 8583344, 8703450, 8824630, 8946889, 9070232, 9194663, 9320185,
   9446804, 9574524, 9703348, 9833281, 9964328, 10096491, 10229777, 10364188, 10499729, 10636404, 10774218, 10913175, 11053278, 11194532, 11336942, 11480511,
   11625243, 11771143, 11918215, 12066463, 12215891, 12366504, 12518305, 12671299, 12825489, 12980881, 13137478, 13295284, 13454303, 13614540, 13775998, 13938682,
   14102596, 14267744, 14434130, 14601758, 14770633, 14940758, 15112137, 15284775, 15458676, 15633844, 15810282, 15987995, 16166988, 16347263, 16528826, 16711680};
static const uint32_t g_gamma_lut[256] = {
     0,   3,  21,  66, 148, 276, 460, 709, 1031, 1434, 1926, 2515, 3209, 4015, 4941, 5994,
   7181, 8510, 9987, 11620, 13415, 15378, 17518, 19840, 22351, 25057, 27966, 31083, 34415, 37968, 41749, 45763,
   50018, 54518, 59271, 64283, 69559, 75105, 80928, 87033, 93427, 100115, 107103, 114398, 122004, 129927, 138175, 146751,
   155662, 164913, 174511, 184460, 194767, 205437, 216476, 227888, 239681, 251858, 264427, 277391, 290757, 304530, 318716, 333319,
   348346, 363801, 379691, 396019, 412793, 430016, 447694, 465833, 484438, 503514, 523065, 543099, 563619, 584630, 606139, 628150,
   650668, 673698, 697246, 721317, 745915, 771046, 796715, 822927, 849687, 877000, 904870, 933304, 962306, 991881, 1022034, 1052770,
   1084094, 1116010, 1148525, 1181642, 1215367, 1249704, 1284659, 1320236, 1356441, 1393277, 1430750, 1468866, 1507627, 1547040, 1587110, 1627840,
   1669237, 1711304, 1754046, 1797469, 1841577, 1886374, 1931867, 1978058, 2024953, 2072557, 2120875, 2169910, 2219669, 2270155, 2321374, 2373329,
   2426026, 2479469, 2533663, 2588613, 2644323, 2700798, 2758043, 2816061, 2874859, 2934439, 2994808, 3055969, 3117928, 3180688, 3244254, 3308632,
   3373825, 3439838, 3506675, 3574342, 3642842, 3712181, 3782362, 3853390, 3925271, 3998007, 4071604, 4146067, 4221399, 4297606, 4374691, 4452660,
   4531516, 4611264, 4691909, 4773455, 4855907, 4939268, 5023544, 5108739, 5194856, 5281902, 5369880, 5458794, 5548649, 5639449, 5731199, 5823903,
   5917565, 6012190, 6107783, 6204347, 6301887, 6400407, 6499912, 6600406, 6701893, 6804378, 6907865, 7012358, 7117862, 7224381, 7331919, 7440481,
   7550071, 7660693, 7772351, 7885051, 7998795, 8113589, 8229437, 8346342, 8464310, 8583344, 8703450, 8824630, 8946889, 9070232, 9194663, 9320185,
   9446804, 9574524, 9703348, 9833281, 9964328, 10096491, 10229777, 10364188, 10499729, 10636404, 10774218, 10913175, 11053278, 11194532, 11336942, 11480511,
   11625243, 11771143, 11918215, 12066463, 12215891, 12366504, 12518305, 12671299, 12825489, 12980881, 13137478, 13295284, 13454303, 13614540, 13775998, 13938682,
   14102596, 14267744, 14434130, 14601758, 14770633, 14940758, 15112137, 15284775, 15458676, 15633844, 15810282, 15987995, 16166988, 16347263, 16528826, 16711680};
static const uint32_t b_gamma_lut[256] = {
     0,   3,  21,  66, 148, 276, 460, 709, 1031, 1434, 1926, 2515, 3209, 4015, 4941, 5994,
   7181, 8510, 9987, 11620, 13415, 15378, 17518, 19840, 22351, 25057, 27966, 31083, 34415, 37968, 41749, 45763,
   50018, 54518, 59271, 64283, 69559, 75105, 80928, 87033, 93427, 100115, 107103, 114398, 122004, 129927, 138175, 146751,
   155662, 164913, 174511, 184460, 194767, 205437, 216476, 227888, 239681, 251858, 264427, 277391, 290757, 304530, 318716, 333319,
   348346, 363801, 379691, 396019, 412793, 430016, 447694, 465833, 484438, 503514, 523065, 543099, 563619, 584630, 606139, 628150,
   650668, 673698, 697246, 721317, 745915, 771046, 796715, 822927, 849687, 877000, 904870, 933304, 962306, 991881, 1022034, 1052770,
   1084094, 1116010, 1148525, 1181642, 1215367, 1249704, 1284659, 1320236, 1356441, 1393277, 1430750, 1468866, 1507627, 1547040, 1587110, 1627840,
   1669237, 1711304, 1754046, 1797469, 1841577, 1886374, 1931867, 1978058, 2024953, 2072557, 2120875, 2169910, 2219669, 2270155, 2321374, 2373329,
   2426026, 2479469, 2533663, 2588613, 2644323, 2700798, 2758043, 2816061, 2874859, 2934439, 2994808, 3055969, 3117928, 3180688, 3244254, 3308632,
   3373825, 3439838, 3506675, 3574342, 3642842, 3712181, 3782362, 3853390, 3925271, 3998007, 4071604, 4146067, 4221399, 4297606, 4374691, 4452660,
   4531516, 4611264, 4691909, 4773455, 4855907, 4939268, 5023544, 5108739, 5194856, 5281902, 5369880, 5458794, 5548649, 5639449, 5731199, 5823903,
   5917565, 6012190, 6107783, 6204347, 6301887, 6400407, 6499912, 6600406, 6701893, 6804378, 6907865, 7012358, 7117862, 7224381, 7331919, 7440481,
   7550071, 7660693, 7772351, 7885051, 7998795, 8113589, 8229437, 8346342, 8464310, 8583344, 8703450, 8824630, 8946889, 9070232, 9194663, 9320185,
   9446804, 9574524, 9703348, 9833281, 9964328, 10096491, 10229777, 10364188, 10499729, 10636404, 10774218, 10913175, 11053278, 11194532, 11336942, 11480511,
   11625243, 11771143, 11918215, 12066463, 12215891, 12366504, 12518305, 12671299, 12825489, 12980881, 13137478, 13295284, 13454303, 13614540, 13775998, 13938682,
   14102596, 14267744, 14434130, 14601758, 14770633, 14940758, 15112137, 15284775, 15458676, 15633844, 15810282, 15987995, 16166988, 16347263, 16528826, 16711680};

/* function prototypes */
static inline void fill_buffer(uint8_t led_id, uint8_t offset);
static inline void clear_buffer(uint8_t offset);

/* function definitions */
uint8_t neopixel_set_color(uint8_t led_num, uint8_t r, uint8_t g, uint8_t b) {
    if (led_num<NEOPIXEL_NUM_OF_LEDS) { /* TBD: change for assertion */
        ptr_led_colors[led_num*3+0] = r;
        ptr_led_colors[led_num*3+1] = g;
        ptr_led_colors[led_num*3+2] = b;
        led_colors_remainder[led_num*3+0] = 0;
        led_colors_remainder[led_num*3+1] = 0;
        led_colors_remainder[led_num*3+2] = 0;
        return 1;
	} else {
        return 0;
	}
}

uint8_t neopixel_update(uint8_t block) {
    if (curr_state != STATE_IDLE) {
        return 0;
    }
	curr_state = STATE_INITIALIZATION;
    neopixel_updating_state_machine();
    if (block) {
        while(curr_state != STATE_IDLE);
    }
    return 1;
}

/**
 * @brief Central state machine holding the context of the LED strip update process.
 * 
 * The communication with daisy-chained Neopixel (WS2812B) LEDs is in detail described in the
 * respective datasheet. Briefly, the color to be illuminated is represented in 8b RGB color 
 * model (or more precisely GRB). The communication takes place over a single signal line. 
 * Transmittion of each bit happens within a constant time period when a line is pulled high and 
 * then low respectivelly for varying time intervals encoding the bit value. All the bits have to
 * be transmitted in succession with a defined idle periods (here called reset pulse) at the
 * beginning and end of the whole transfer.
 * 
 * Here, the timer with output compare feature (for PWM) is used to generate the pulses whose length 
 * is updated by feeding the respective register from a DMA stream. For memory efficiency, a ping-pong
 * like buffer is used as a source of timer compare values, which is effectively updated when half of
 * the prepared data was already transmitted. 
 */
void neopixel_updating_state_machine(void) {
    switch (curr_state) {
        case STATE_INITIALIZATION: /* make request to send the reset pulse */
            /* prepare for subsequent DMA transfer */
            if(new_buffer_prepared) {
                new_buffer_prepared = 0;
                if(ptr_led_colors == (uint8_t *) &led_colors_1) {
                    ptr_led_colors = (uint8_t *) &led_colors_2;
                } else {
                    ptr_led_colors = (uint8_t *) &led_colors_1;
                }
            }
            next_led = 0;
            fill_buffer(next_led++, FIRST_HALF_OF_BUFFER);
            fill_buffer(next_led++, SECOND_HALF_OF_BUFFER);
            DMA_CCR(DMA1, DMA_CHANNEL5) &= ~DMA_CCR_CIRC; /* disable circular (set normal) mode */
            while(DMA_CCR(DMA1, DMA_CHANNEL5) & DMA_CCR_CIRC)
                ;
            dma_enable_circular_mode(DMA1, DMA_CHANNEL5);
            dma_set_memory_address(DMA1, DMA_CHANNEL5, (uint32_t) &ping_pong_buffer);
            dma_set_number_of_data(DMA1, DMA_CHANNEL5, 2*BITS_PER_LED);
            dma_disable_half_transfer_interrupt(DMA1, DMA_CHANNEL5);
            dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
            dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_HTIF);
            dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);

            dma_enable_half_transfer_interrupt(DMA1, DMA_CHANNEL5);
            dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
            dma_enable_channel(DMA1, DMA_CHANNEL5);           
            reset_pulse_counter = RESET_PULSE_PERIODS;
            timer_set_oc_value(TIM2, TIM_OC1, 0);
            curr_state = STATE_RESET_PULSE;
            timer_enable_irq(TIM2, TIM_DIER_UIE);
            break;
        case STATE_RESET_PULSE:
            if(--reset_pulse_counter == 0) {
                curr_state = STATE_LED_DATA_TRANSFER;
                timer_enable_irq(TIM2, TIM_DIER_CC1DE); /* enable dma request, start the transfer */
                timer_disable_irq(TIM2, TIM_DIER_UIE);
                timer_clear_flag(TIM2, TIM_SR_UIF);
            }
            break;
        case STATE_LED_DATA_TRANSFER: ;
            if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL5, DMA_TCIF)) {
                if (next_led < NEOPIXEL_NUM_OF_LEDS) {
                    fill_buffer(next_led++, SECOND_HALF_OF_BUFFER);
                } else if ((NEOPIXEL_NUM_OF_LEDS % 2) == 0) {
                    timer_disable_irq(TIM2, TIM_DIER_CC1DE); /* disable dma request */
                    while(TIM_DIER(TIM2) & TIM_DIER_CC1DE)
                        ;
                    timer_set_oc_value(TIM2, TIM_OC1, 0);
                    dma_disable_half_transfer_interrupt(DMA1, DMA_CHANNEL5);
                    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
                    dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_HTIF | DMA_TCIF);
                    dma_disable_channel(DMA1, DMA_CHANNEL5);
                    curr_state = STATE_IDLE;
                } else {
                    clear_buffer(SECOND_HALF_OF_BUFFER);
                }
                dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);
            } else if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL5, DMA_HTIF)) {
                if (next_led < NEOPIXEL_NUM_OF_LEDS) {
                    fill_buffer(next_led++, FIRST_HALF_OF_BUFFER);
                } else if((NEOPIXEL_NUM_OF_LEDS % 2) == 1) {
                    timer_disable_irq(TIM2, TIM_DIER_CC1DE); /* disable dma request */
                    while(TIM_DIER(TIM2) & TIM_DIER_CC1DE)
                        ;
                    timer_set_oc_value(TIM2, TIM_OC1, 0);
                    dma_disable_half_transfer_interrupt(DMA1, DMA_CHANNEL5);
                    dma_disable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
                    dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_HTIF);
                    dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);
                    dma_disable_channel(DMA1, DMA_CHANNEL5);
                    curr_state = STATE_IDLE;
                } else {
                    clear_buffer(FIRST_HALF_OF_BUFFER);
                }
                dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_HTIF);
            }
            break;
        case STATE_IDLE:
        default:
            break;
    }
    return;
}

static inline void fill_buffer(uint8_t led_id, uint8_t offset) {
    uint8_t r, g, b;
    uint32_t r_goal, g_goal, b_goal;
    offset = offset*BITS_PER_LED;

    r_goal = r_gamma_lut[ptr_led_colors[led_id*3+0]];
    g_goal = g_gamma_lut[ptr_led_colors[led_id*3+1]];
    b_goal = b_gamma_lut[ptr_led_colors[led_id*3+2]];

    // // no gamma correction
    // r_goal = ptr_led_colors[led_id*3+0] << 16;
    // g_goal = ptr_led_colors[led_id*3+1] << 16;
    // b_goal = ptr_led_colors[led_id*3+2] << 16;

// original
    r = (uint8_t)((r_goal+led_colors_remainder[led_id*3+0])>>16);
    g = (uint8_t)((g_goal+led_colors_remainder[led_id*3+1])>>16);
    b = (uint8_t)((b_goal+led_colors_remainder[led_id*3+2])>>16);

    // r = (uint8_t)((r_goal)>>16);
    // g = (uint8_t)((g_goal)>>16);
    // b = (uint8_t)((b_goal)>>16);

    // r = (uint8_t)((r_goal+led_colors_remainder[0])>>16);
    // g = (uint8_t)((g_goal+led_colors_remainder[1])>>16);
    // b = (uint8_t)((b_goal+led_colors_remainder[2])>>16);

// // original
//     led_colors_remainder[led_id*3+0] += r_goal-(((uint32_t) r)<<16);
//     led_colors_remainder[led_id*3+1] += g_goal-(((uint32_t) g)<<16);
//     led_colors_remainder[led_id*3+2] += b_goal-(((uint32_t) b)<<16);

    led_colors_remainder[led_id*3+0] = 0;
    led_colors_remainder[led_id*3+1] = 0;
    led_colors_remainder[led_id*3+2] = 0;

    uint8_t i;
    for (i=0; i<8; i++) {
        ping_pong_buffer[offset+i] = (g & (1 << (7-i)) ? 60 : 30); /* g */
        ping_pong_buffer[offset+i+8] = (r & (1 << (7-i)) ? 60 : 30); /* r */
        ping_pong_buffer[offset+i+16] = (b & (1 << (7-i)) ? 60 : 30); /* b */
    }
}

static inline void clear_buffer(uint8_t offset) { /* so that we do not risk sending nonzero values before the stream is stopped */
    offset = offset*BITS_PER_LED;

    uint8_t i;
    for (i=0; i<8; i++) {
        ping_pong_buffer[offset+i] = 0; /* g */
        ping_pong_buffer[offset+i+8] = 0; /* r */
        ping_pong_buffer[offset+i+16] = 0; /* b */
    }
}

void neopixel_update_buffer(uint8_t * buf, uint16_t len) {
    if(len == 3*NEOPIXEL_NUM_OF_LEDS+HEADERSIZE) {
        if(ptr_led_colors == (uint8_t *) &led_colors_1) {
            memcpy(led_colors_2,buf+HEADERSIZE,len-HEADERSIZE);
        } else {
            memcpy(led_colors_1,buf+HEADERSIZE,len-HEADERSIZE);
        }
        memset(led_colors_remainder,0,len);
        new_buffer_prepared = 1;
        timer_set_counter(TIM3, 0); /* reset the couter detecting prolonged absence of communication */
    }
}

void neopixel_send_heartbeat(void) {
    timer_clear_flag(TIM3, TIM_DIER_UIE);
    uint8_t buffer[] = "Ada\n";
    usb_tx(buffer, 4);
}