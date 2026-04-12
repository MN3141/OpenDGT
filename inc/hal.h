#ifndef HAL
#define HAL

#define PORTS_NUM 6
#define PORT_A 0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3
#define PORT_E 4
#define PORT_H 5

#define PIN_INPUT_MODE 0
#define PIN_OUTPUT_MODE 1

#define PIN_HIGH 1
#define PIN_LOW 0

#define MCU_INIT_OK 0
#define MCU_INI_ERR 1

int MCU_Init();
int GPIO_Init(int gpioPort, int gpioPin);
int GPIO_Set(int gpioPort, int gpioPin, int value);
void MCU_Delay(int miliseconds);
int USB_Init();

#endif