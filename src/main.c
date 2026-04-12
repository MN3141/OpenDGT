/* This shall be the stub for the user application*/

#include "opendgtlib.h"
#include "hal.h"

int main()
{
    Board_Init();
    GPIO_Init(PORT_A,9);
    char buff[] = {"Hello STM"};
    int buffSize = sizeof(buff);

    while (1)
    {
        GPIO_Set(PORT_A,9,PIN_HIGH);
        USB_TX(buff,buffSize);
        MCU_Delay(1000);
        GPIO_Set(PORT_A,9,PIN_LOW);
        MCU_Delay(1000);
    }

    return 0;
}