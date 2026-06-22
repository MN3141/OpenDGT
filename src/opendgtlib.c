#include "opendgtlib.h"
#include "hal.h"

int Board_Init()
{
    int init_status = MCU_Init();
    USB_Init();
    I2C_Init(0);

    return init_status;
}
