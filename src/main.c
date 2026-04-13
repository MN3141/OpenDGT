/* This shall be the stub for the user application*/

#include "opendgtlib.h"
#include "hal.h"

int received = 0;
void USB_RX_Callback()
{
    received = 1;
}

int main()
{
    Board_Init();
    GPIO_Init(PORT_A, 9);
    GPIO_Init(PORT_B, 12);

    char buff[] = {"Hello STM\n"};
    int buffSize = sizeof(buff);

    while (1)
    {
        if (received)
        {
            received = 0;
            GPIO_Set(PORT_B, 12, PIN_LOW);
            MCU_Delay(1000);
            GPIO_Set(PORT_B, 12, PIN_HIGH);
            MCU_Delay(1000);
        }
        else
        {
            GPIO_Set(PORT_A, 9, PIN_HIGH);
            USB_TX(buff, buffSize);
            MCU_Delay(1000);
            GPIO_Set(PORT_A, 9, PIN_LOW);
            MCU_Delay(1000);
        }
    }

    return 0;
}