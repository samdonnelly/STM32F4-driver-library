/**
 * @file ws2812_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 (Neopixel) driver 
 * 
 * @version 0.1
 * @date 2023-03-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "ws2812_driver.h"

//=======================================================================================


//=======================================================================================
// Notes 
// - You can set all the psudo_pwm data to zero and every 4th bit starting at the first 
//   bit to one during initialization then only update every 4th bit starting at the 
//   second bit for the update function. 
// - You could set multiple LED colours with one function call by including the LED 
//   index in the data pointer array passed to the driver. 
// - During driver testing, LED8 on device one was always lighting up as white until I 
//   added a second device. 
//=======================================================================================


//=======================================================================================
// Function Prototypes 
//=======================================================================================


//=======================================================================================
// Global variables 

// Device data record 
typedef struct ws2812_driver_data_s
{
    // Linked list tracking 
    struct ws2812_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripherals 
    TIM_TypeDef *timer; 
    GPIO_TypeDef *gpio; 

    // Pin information 
    pin_selector_t pin_num; 
    gpio_pin_num_t pin_code; 

    // Data 
    uint32_t colour_data[WS2812_LED_NUM]; 
    uint8_t psudo_pwm[WS2812_LED_NUM * WS2812_BITS_PER_LED * WS2812_COUNTS_PER_BIT]; 
}
ws2812_driver_data_t; 


// Device driver first data pointer 
static ws2812_driver_data_t *ws2812_driver_data_ptr; 

//=======================================================================================


//=======================================================================================
// Initialization 

// WS2812 initialization 
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin)
{
    // Create data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)create_linked_list_entry(
            device_num, 
            (void *)&ws2812_driver_data_ptr, 
            sizeof(ws2812_driver_data_t)); 

    // Initialize data record 
    driver_data_ptr->timer = timer; 
    driver_data_ptr->gpio = gpio; 
    driver_data_ptr->pin_num = pin; 
    driver_data_ptr->pin_code = SET_BIT << pin; 
    memset((void *)driver_data_ptr->colour_data, CLEAR, sizeof(driver_data_ptr->colour_data)); 
    memset((void *)driver_data_ptr->psudo_pwm, CLEAR, sizeof(driver_data_ptr->psudo_pwm)); 

    // Initialize GPIO output pin 
    gpio_pin_init(driver_data_ptr->gpio, 
                  driver_data_ptr->pin_num, 
                  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
    gpio_write(driver_data_ptr->gpio, driver_data_ptr->pin_num, GPIO_LOW); 
}

//=======================================================================================


//=======================================================================================
// Write 

// Update data 
void ws2812_update(
    device_number_t device_num)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) return; 

    // Local variables 
    uint8_t led_index; 
    uint8_t colour_index; 
    uint16_t psudo_pwm_index = CLEAR; 

    // Update data 
    for (led_index = CLEAR; led_index < WS2812_LED_NUM; led_index++)
    {
        for (colour_index = WS2812_BITS_PER_LED; colour_index > 0; colour_index--)
        {
            driver_data_ptr->psudo_pwm[psudo_pwm_index++] = SET_BIT; 
            driver_data_ptr->psudo_pwm[psudo_pwm_index++] = (uint8_t)
                ((driver_data_ptr->colour_data[led_index] >> (colour_index - 1)) & 0x01); 
            psudo_pwm_index += 2; 
        }
    }
}


// Write to device 
void ws2812_send(
    device_number_t device_num)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) return; 

    // Local variables 
    uint32_t time_count; 
    uint32_t count_check; 
    uint16_t psudo_pwm_index; 
    uint16_t psudo_pwm_size = sizeof(driver_data_ptr->psudo_pwm); 

    // Get the starting timer count 
    time_count = tim_cnt_read(driver_data_ptr->timer); 
    
    // Send data when there is a timer count 
    for (psudo_pwm_index = CLEAR; psudo_pwm_index < psudo_pwm_size; psudo_pwm_index++)
    {
        // Check for a timer count 
        count_check = tim_cnt_read(driver_data_ptr->timer); 
        
        if (time_count != count_check)
        {
            gpio_write(
                driver_data_ptr->gpio, 
                driver_data_ptr->pin_code, 
                driver_data_ptr->psudo_pwm[psudo_pwm_index]); 
            
            time_count = count_check; 
        }
    }
}


// Write to device 
void ws2812_write(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    const uint8_t *colour_data, 
    uint8_t led_num)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) return; 

    // Set the colour data 
    ws2812_colour_set(device_num, colour_data, led_num); 

    // Update the write data 
    ws2812_update(device_num); 

    // Send the write data 
    ws2812_send(device_num); 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Colour set 
void ws2812_colour_set(
    device_number_t device_num, 
    const uint8_t *colour_data, 
    uint8_t led_num)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL ||     // No data record 
        colour_data == NULL ||         // No colour data 
        led_num > WS2812_LED_NUM)      // LED doesn't exist 
    {
        return; 
    }

    // Set data 
    driver_data_ptr->colour_data[led_num] = 
        (*colour_data << SHIFT_16) + (*(colour_data + 1) << SHIFT_8) + *(colour_data + 2); 
}

//=======================================================================================
