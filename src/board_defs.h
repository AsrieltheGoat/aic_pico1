/*
 * AIC Reader Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_AIC_PICO

#define I2C_PORT_LIST { { i2c1, 27, 26 } }
#define I2C_FREQ 400*1000

#define RGB_PIN 16
#define DEFAULT_RGB_ORDER GRB
#define LED_DEF { 25, 22, 13, 15 }


#else

#endif
