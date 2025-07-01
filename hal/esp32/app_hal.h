#ifndef APP_HAL_H
#define APP_HAL_H

#define I2C_SDA 33      // Touch screen SDA pin
#define I2C_SCL 32      // Touch screen SCL pin
#define TP_RST 25       // Touch screen reset pin
#define TP_INT 21       // Touch screen interrupt pin
#define BACKLIGHT 27    // Backlight pin
#define LED_RED 4       // Red LED pin
#define LED_BLUE 17     // Blue LED pin
#define LED_GREEN 16    // Green LED pin

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This function runs once and typically includes:
 * - Setting up display drivers.
 * - Configuring LVGL display and input devices
 */
void hal_setup(void);

/**
 * This function is continuously executed and typically includes:
 * - Updating LVGL's internal state & UI.
 */
void hal_loop(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_HAL_H*/
