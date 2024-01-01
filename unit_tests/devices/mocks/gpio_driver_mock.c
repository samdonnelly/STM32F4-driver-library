/**
 * @file gpio_driver_mock.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Mock GPIO driver implementation - for unit testing 
 * 
 * @version 0.1
 * @date 2023-13-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "gpio_driver.h"

// Unit testing 
#include "gpio_driver_mock.h"

//=======================================================================================


//=======================================================================================
// Driver functions 

// GPIO communication initialization 
void gpio_port_init(void)
{    
    // 
}


// GPIO pin initialization 
void gpio_pin_init(
    GPIO_TypeDef  *gpio, 
    pin_selector_t pin_num,
    gpio_moder_t   moder,
    gpio_otyper_t  otyper,
    gpio_ospeedr_t ospeedr,
    gpio_pupdr_t   pupdr)
{
    // 
}


// GPIO Alternate Function Register (AFR) configuration 
void gpio_afr(
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    bit_setter_t setpoint)
{
    // 
}


// GPIOA write 
void gpio_write(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num,
    gpio_pin_state_t pin_state)
{
    // 
}


// GPIO read 
GPIO_STATE gpio_read(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num)
{
    return GPIO_LOW; 
}


// GPIO port read 
GPIOX_DR gpio_port_read(
    GPIO_TypeDef *gpio)
{
    return CLEAR; 
}

//=======================================================================================


//=======================================================================================
// Mock functions 
//=======================================================================================
