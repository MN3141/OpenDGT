#include "dgt_port.h"

#include "main.h"
#include "opendgtlib.h"
#include "opendgtlib_port.h"

extern I2C_HandleTypeDef hi2c1; /* Master, to the clock */
extern I2C_HandleTypeDef hi2c2; /* Slave, receives the acks */

/* A frame's length is a single byte, so this can never overflow. */
static uint8_t dgt_rx_frame[256];

dgt_i2c_status_t dgt_port_i2c_write(uint8_t addr7, const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
  /* The HAL takes 8-bit (shifted) addresses and a non-const buffer it only reads */
  if (HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(addr7 << 1), (uint8_t *)data, len, timeout_ms) == HAL_OK)
    return DGT_I2C_OK;

  /* HAL only checks ARLO in IT mode, here it shows up as a timeout */
  if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_ARLO))
  {
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ARLO);
    return DGT_I2C_ARB_LOST;
  }
  if (HAL_I2C_GetError(&hi2c1) & HAL_I2C_ERROR_AF)
    return DGT_I2C_NACK;
  return DGT_I2C_ERROR;
}

void dgt_port_delay_ms(uint32_t ms)
{
  HAL_Delay(ms);
}

uint32_t dgt_port_get_tick_ms(void)
{
  return HAL_GetTick();
}

void DGT_Port_Init(void)
{
  /* Listen before sending anything, the clock answers as soon as a frame ends */
  if (HAL_I2C_EnableListen_IT(&hi2c2) != HAL_OK)
    Error_Handler();
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
  UNUSED(AddrMatchCode);

  if (hi2c == &hi2c2 && TransferDirection == I2C_DIRECTION_TRANSMIT)
    HAL_I2C_Slave_Seq_Receive_IT(hi2c, dgt_rx_frame, sizeof(dgt_rx_frame), I2C_LAST_FRAME);
}

/* The frame length is not known up front, so the receive buffer is never filled
   and the HAL ends each frame at the stop condition, reporting it as an
   incomplete transfer. That also ends the listen mode, so restart it here. */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c != &hi2c2)
    return;

  DGT3000_OnFrameReceived(dgt_rx_frame, hi2c->XferSize - hi2c->XferCount);

  HAL_I2C_EnableListen_IT(hi2c);
}
