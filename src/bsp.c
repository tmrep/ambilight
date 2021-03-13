#include "bsp.h"
#include <libopencm3/cm3/systick.h>

void bsp_initialize(void)
{
	// clock settings
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	// systick counter
	systick_set_frequency(1000U, rcc_ahb_frequency);
	systick_counter_enable();

	// gpio init
	// onboard LED
	rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
	gpio_set(LED_PORT, LED_PIN);
	// Neopixel dataline
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_set_mode(NEOPIXEL_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, NEOPIXEL_PIN);

	// DMA init
	rcc_periph_clock_enable(RCC_DMA1);
	nvic_enable_irq(NVIC_DMA1_CHANNEL5_IRQ);
	nvic_set_priority(NVIC_DMA1_CHANNEL5_IRQ, 1);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL5);
	dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_LOW);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_16BIT);
	dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_32BIT);
	dma_set_peripheral_address(DMA1, DMA_CHANNEL5, (uint32_t) &TIM2_CCR1);
	dma_enable_circular_mode(DMA1, DMA_CHANNEL5);
	dma_enable_half_transfer_interrupt(DMA1, DMA_CHANNEL5);
	dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);
	
	// timer 2 (for pwm+dma) init
	rcc_periph_clock_enable(RCC_TIM2);
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_set_priority(NVIC_TIM2_IRQ, 1);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, 0); /* Set timer prescaler. 72MHz/1440 => 50000 counts per second. */
	timer_set_period(TIM2, 90);
	timer_disable_preload(TIM2);
	timer_enable_oc_preload(TIM2, TIM_OC1);
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_value(TIM2, TIM_OC1, 5);
	timer_set_oc_polarity_high(TIM2, TIM_OC1);
	timer_set_dma_on_compare_event(TIM2);
	
	// usb device init
	usb_init();
	/* wait for the initial enumeration */
	for (volatile int i = 0; i < 2000000; i++) {
		usb_poll();
	}

	// timer 3 (for detection of no refreshs)
	rcc_periph_clock_enable(RCC_TIM3);
	nvic_enable_irq(NVIC_TIM3_IRQ);
	nvic_set_priority(NVIC_TIM3_IRQ, 1);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	// timer_set_prescaler(TIM3, 2880); /* 5 sec intervals */
	timer_set_prescaler(TIM3, 576); /* 1 sec intervals */
	timer_set_period(TIM3, 62500);
	timer_enable_irq(TIM3, TIM_DIER_UIE);
	timer_enable_counter(TIM3);

	/* small delay at a start */
	timer_set_oc_value(TIM2, TIM_OC1, 0);
	timer_enable_oc_output(TIM2, TIM_OC1);
	timer_enable_counter(TIM2);
	bsp_delayms(100);
}

void bsp_delayms(uint32_t ms) {
  systick_clear();
  /* add a period to guaranty minimum wait */
  if (ms < 0xFFFFFFFFU)
  {
    ms++;
  }
  while (ms)
  {
    if (systick_get_countflag() != 0U)
    {
      ms--;
    }
  }
}
