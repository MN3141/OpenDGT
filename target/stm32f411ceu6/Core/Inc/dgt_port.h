#ifndef DGT_PORT_H
#define DGT_PORT_H

/* STM32 HAL port of opendgtlib: I2C1 is the master towards the clock, I2C2 the
   slave that receives the clock's acks (OwnAddress1 = DGT_OWN_ADDR7 << 1,
   general call enabled). */

/* Starts listening on I2C2. Call after MX_I2C2_Init(), before any DGT3000_* call. */
void DGT_Port_Init(void);

#endif /* DGT_PORT_H */
