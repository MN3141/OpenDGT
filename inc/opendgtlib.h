#ifndef OPENDGTLIB_H
#define OPENDGTLIB_H

#include <stdint.h>

/* Hardware agnostic DGT3000 driver. The target provides the functions in
 * opendgtlib_port.h, and passes each frame its I2C slave receives to
 * DGT3000_OnFrameReceived(): the clock answers every command with a frame sent
 * to DGT_OWN_ADDR7 or broadcast (general call). */

#define DGT_OWN_ADDR7 0x10 /* The slave I2C must listen on this address */

/* Same meaning as the dgtpi return codes */
#define DGT_OK 0
#define DGT_ERR_NEGATIVE_ACK -1 /* Clock answered, but refused the command */
#define DGT_ERR_I2C -2          /* Bus stuck or timed out */
#define DGT_ERR_RETRY -3        /* Collision, or no ack received */
#define DGT_ERR_CLOCK_OFF -7    /* Address not acked, clock probably off */

#define DGT_DISPLAY_CHARS 11

/* Call when a frame received on the slave I2C ends (stop condition), with the
   bytes after the address byte. Safe to call from interrupt context. */
void DGT3000_OnFrameReceived(const uint8_t *frame, uint16_t len);

void DGT3000_WakeClock(void);

/* Get the clock into mode 25, switching to central control if needed. */
int DGT3000_Configure(void);

/* Clear the display, then show text (up to 11 characters, padded with spaces). */
int DGT3000_Display(const char *text, uint8_t beep, uint8_t left_dots, uint8_t right_dots);

/* Single attempts, without retries */
int DGT3000_EndDisplay(void);
int DGT3000_SetDisplay(const char *text, uint8_t beep, uint8_t left_dots, uint8_t right_dots);
int DGT3000_SetMode25(void);
int DGT3000_SetCCMode(void);

/* Host link: called by the target when data arrives over USB. */
void USB_RX_Callback(void);

#endif /* OPENDGTLIB_H */
