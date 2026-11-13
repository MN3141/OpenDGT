#ifndef OPENDGTLIB_PORT_H
#define OPENDGTLIB_PORT_H

#include <stdint.h>

/* Functions every target must implement for opendgtlib (link-time binding).
 * A missing one is a link error. For host tests, link a mock instead.
 *
 * The target must also run an I2C slave on DGT_OWN_ADDR7 with general call
 * enabled, and pass each received frame to DGT3000_OnFrameReceived().
 *
 * All addresses here are 7-bit. */

/* Result of a platform I2C write */
typedef enum
{
  DGT_I2C_OK = 0,
  DGT_I2C_NACK,     /* Address not acked, clock probably off */
  DGT_I2C_ARB_LOST, /* Collision with the clock talking on the bus */
  DGT_I2C_ERROR     /* Bus stuck, timeout or anything else */
} dgt_i2c_status_t;

/* Blocking master write of len bytes to addr7. */
dgt_i2c_status_t dgt_port_i2c_write(uint8_t addr7, const uint8_t *data, uint16_t len, uint32_t timeout_ms);

void dgt_port_delay_ms(uint32_t ms);

/* Free running millisecond counter, wrapping is fine. */
uint32_t dgt_port_get_tick_ms(void);

#endif /* OPENDGTLIB_PORT_H */
