#include "opendgtlib.h"
#include "opendgtlib_port.h"

#include <stddef.h>

#define DGT_CLOCK_ADDR7 0x08
#define DGT_WAKE_ADDR7 0x28
#define DGT_MAX_SYMBOLS 11

/* The CRC covers the 8-bit address byte that goes on the wire */
#define DGT_ADDR8(addr7) ((uint8_t)((addr7) << 1))
#define DGT_BROADCAST_ADDR8 0x00

#define DGT_MSG_ACK 0x01
#define DGT_CMD_DISPLAY 0x06
#define DGT_CMD_END_DISPLAY 0x07
#define DGT_CMD_CHANGE_STATE 0x0B
#define DGT_CMD_SET_CC 0x0F
#define DGT_ACK_POSITIVE 0x08

#define DGT_TX_TIMEOUT_MS 10
#define DGT_ACK_TIMEOUT_MS 50
#define DGT_CONFIGURE_ATTEMPTS 6
#define DGT_DISPLAY_ATTEMPTS 3

#define DGT_COMMAND_DELAY_MS 25
#define DGT_RETRY_DELAY_MS 100

#define DGT_WAKE_PINGS 9
#define DGT_WAKE_TIMEOUT_MS 100
#define DGT_WAKE_DELAY_MS 1000

static const uint8_t crc_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

/* Written from the receive interrupt, polled by DGT_SendCommand() */
static volatile uint8_t dgt_ack_cmd;
static volatile uint8_t dgt_ack_status;

static uint8_t DGT_Crc(uint8_t addr_byte, const uint8_t *frame, uint16_t length)
{
  uint8_t crc = crc_table[addr_byte];

  for (uint16_t i = 0; i < length; i++)
    crc = crc_table[crc ^ frame[i]];

  return crc;
}

/* Ack frame: source, length (counts the address byte), type, command, status,
   ..., crc. The CRC is seeded with the address the frame was sent to: ours,
   or 0x00 for broadcasts (general call), which the display acks use. */
void DGT3000_OnFrameReceived(const uint8_t *frame, uint16_t len)
{
  if (frame == NULL || len < 6 || frame[1] != len + 1 || frame[2] != DGT_MSG_ACK)
    return;

  uint8_t crc = frame[len - 1];
  if (DGT_Crc(DGT_ADDR8(DGT_OWN_ADDR7), frame, len - 1) != crc &&
      DGT_Crc(DGT_BROADCAST_ADDR8, frame, len - 1) != crc)
    return;

  dgt_ack_status = frame[4];
  dgt_ack_cmd = frame[3]; /* Last: DGT_SendCommand() waits on this one */
}

void DGT3000_WakeClock(void)
{
  static const uint8_t ping_frame[] = {0x20, 0x05, 0x0D, 0x46};

  /* The clock does not ack these, the result does not matter */
  for (int i = 0; i < DGT_WAKE_PINGS; i++)
  {
    (void)dgt_port_i2c_write(DGT_WAKE_ADDR7, ping_frame, sizeof(ping_frame), DGT_WAKE_TIMEOUT_MS);
    dgt_port_delay_ms(DGT_WAKE_DELAY_MS);
  }
}

/* Send a frame to the clock and wait for the ack of its command, which the
   clock sends to our slave address. */
static int DGT_SendCommand(const uint8_t *frame, uint16_t length, uint8_t *ack_status)
{
  uint8_t cmd = frame[2];

  dgt_port_delay_ms(DGT_COMMAND_DELAY_MS);

  /* Cleared before sending: the ack can arrive right after the stop condition */
  dgt_ack_cmd = 0;

  switch (dgt_port_i2c_write(DGT_CLOCK_ADDR7, frame, length, DGT_TX_TIMEOUT_MS))
  {
  case DGT_I2C_OK:
    break;
  case DGT_I2C_ARB_LOST:
    return DGT_ERR_RETRY;
  case DGT_I2C_NACK:
    return DGT_ERR_CLOCK_OFF;
  default:
    return DGT_ERR_I2C;
  }

  uint32_t start = dgt_port_get_tick_ms();
  while (dgt_ack_cmd != cmd)
  {
    if (dgt_port_get_tick_ms() - start >= DGT_ACK_TIMEOUT_MS)
      return DGT_ERR_RETRY;
  }

  *ack_status = dgt_ack_status;
  return DGT_OK;
}

/* Clear the display, as dgt3000EndDisplay() does. The clock answers either
   right away to our address (status 5: already empty) or with a broadcast
   once the display is cleared (status 0). */
int DGT3000_EndDisplay(void)
{
  static const uint8_t end_display_frame[] = {0x20, 0x05, DGT_CMD_END_DISPLAY, 0x70};
  uint8_t status = 0;

  int e = DGT_SendCommand(end_display_frame, sizeof(end_display_frame), &status);
  if (e != DGT_OK)
    return e;

  status &= 0x07;
  return (status == 0x05 || status == 0x00) ? DGT_OK : DGT_ERR_NEGATIVE_ACK;
}

/* Show text as dgt3000SetDisplay() does. The ack is a broadcast. */
int DGT3000_SetDisplay(const char *text, uint8_t beep, uint8_t left_dots, uint8_t right_dots)
{
  /* source, length, command, 11 characters, 0xFF, beep, 0x03, left dots,
     right dots, crc */
  uint8_t display_frame[] = {0x20, 0x15, DGT_CMD_DISPLAY,
                             ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                             0xFF, 0x00, 0x03, 0x00, 0x00, 0x00};
  uint8_t status = 0;

  for (int i = 0; text != NULL && i < DGT_DISPLAY_CHARS && text[i] != '\0'; i++)
    display_frame[3 + i] = (uint8_t)text[i];

  display_frame[15] = beep;
  display_frame[17] = left_dots;
  display_frame[18] = right_dots;
  display_frame[19] = DGT_Crc(DGT_ADDR8(DGT_CLOCK_ADDR7), display_frame, sizeof(display_frame) - 1);

  int e = DGT_SendCommand(display_frame, sizeof(display_frame), &status);
  if (e != DGT_OK)
    return e;

  /* Refused: a message is already being displayed */
  return ((status & 0xF3) == 0x23) ? DGT_ERR_NEGATIVE_ACK : DGT_OK;
}

/* Clear the display and then show the text, three attempts each, as
   dgt3000Display() does, with a pause before each retry. */
int DGT3000_Display(const char *text, uint8_t beep, uint8_t left_dots, uint8_t right_dots)
{
  int e = DGT_ERR_I2C;

  for (int i = 0; i < DGT_DISPLAY_ATTEMPTS; i++)
  {
    if (i > 0)
      dgt_port_delay_ms(DGT_RETRY_DELAY_MS);
    e = DGT3000_EndDisplay();
    if (e == DGT_OK)
      break;
  }
  if (e != DGT_OK)
    return e;

  for (int i = 0; i < DGT_DISPLAY_ATTEMPTS; i++)
  {
    if (i > 0)
      dgt_port_delay_ms(DGT_RETRY_DELAY_MS);
    e = DGT3000_SetDisplay(text, beep, left_dots, right_dots);
    if (e == DGT_OK)
      break;
  }
  return e;
}

/* Returns DGT_ERR_NEGATIVE_ACK if the clock is not in central control. */
int DGT3000_SetMode25(void)
{
  static const uint8_t mode25_frame[] = {0x20, 0x06, DGT_CMD_CHANGE_STATE, 0x39, 0xB9};
  uint8_t status = 0;

  int e = DGT_SendCommand(mode25_frame, sizeof(mode25_frame), &status);
  if (e != DGT_OK)
    return e;

  return (status == DGT_ACK_POSITIVE) ? DGT_OK : DGT_ERR_NEGATIVE_ACK;
}

/* Returns DGT_ERR_NEGATIVE_ACK if the clock refuses, e.g. because it is running. */
int DGT3000_SetCCMode(void)
{
  static const uint8_t central_control_frame[] = {0x20, 0x05, DGT_CMD_SET_CC, 0x48};
  uint8_t status = 0;

  int e = DGT_SendCommand(central_control_frame, sizeof(central_control_frame), &status);
  if (e != DGT_OK)
    return e;

  return (status & DGT_ACK_POSITIVE) ? DGT_OK : DGT_ERR_NEGATIVE_ACK;
}

/* Get the clock into mode 25, as dgt3000Configure() does: mode 25 is tried
   first, central control is only needed when the clock refuses it. */
int DGT3000_Configure(void)
{
  int e = DGT_ERR_I2C;

  for (int i = 0; i < DGT_CONFIGURE_ATTEMPTS; i++)
  {
    if (i > 0)
      dgt_port_delay_ms(DGT_RETRY_DELAY_MS);
    e = DGT3000_SetMode25();
    if (e == DGT_OK)
      return DGT_OK;

    if (e == DGT_ERR_NEGATIVE_ACK)
      (void)DGT3000_SetCCMode();
    else if (e == DGT_ERR_CLOCK_OFF)
      DGT3000_WakeClock();
  }

  return e;
}