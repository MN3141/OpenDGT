#include "opendgtlib.h"
#include "hal.h"

int Board_Init()
{
    int init_status = MCU_Init();
    return init_status;
}
