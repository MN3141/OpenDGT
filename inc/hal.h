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

#define I2C_INTERFACES 3

int MCU_Init();
void MCU_Delay(int miliseconds);

int GPIO_Init(int gpioPort, int gpioPin);
int GPIO_Set(int gpioPort, int gpioPin, int value);

int USB_Init();
int USB_TX(char *buffer, int bufferSize);

int I2C_Init(int i2c_interface);
int I2C_TX(int i2c_interface, char *buffer, int bufferSize, int addr);

#endif