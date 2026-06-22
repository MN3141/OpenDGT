#ifndef OPENDGTLIB
#define OPENDGTLIB

#define DGT3000_WAKE_PORT 40 << 1 /*address used to wake up the DGT3000 clock*/

int Board_Init();
int Wake_DGT3000();

#endif