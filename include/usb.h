#ifndef __USB_H
#define __USB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void usb_init(void);
void usb_poll(void);
void usb_tx(const void * buffer, uint16_t len);
__attribute__((weak)) void usb_rx_callback(uint8_t * frame_buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __USB_H */
