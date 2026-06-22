#include "opendgtlib.h"
#include "hal.h"

/* Wake up message for the DGT3000 clock*/
char ping[] = {80, 32, 5, 13, 70};

int Board_Init()
{
    int init_status = MCU_Init();
    USB_Init();
    I2C_Init(0);

    return init_status;
}

int Wake_DGT3000()
{

    int buffSize = sizeof(ping);
    int status = I2C_TX(I2C_INTERFACE_0, ping, buffSize, DGT3000_WAKE_PORT);

    return status;
}