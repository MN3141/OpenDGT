#include "opendgtlib.h"
#include "hal.h"

int Init_MCU(){
    Board_Init();
    GPIO_Init();
    USB_Init();
}
