#include "opendgtlib.h"

/* Wake up message for the DGT3000 clock*/
char ping[] = {80, 32, 5, 13, 70};
void USB_RX_Callback(void)
{
    /* TODO: handle received USB data */
}
