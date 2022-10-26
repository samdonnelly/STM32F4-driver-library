/**
 * @file gpio_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPIO functions
 * 
 * @version 0.1
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

#include "gpio_driver.h"

//=======================================================================================


//=======================================================================================
// Initialization 

// GPIO communication initialization 
void gpio_port_init(void)
{
    // TODO create a clocks driver to enable these and other clocks (RCC) 
    
    // Enable GPIOA clock - RCC_AHB1ENR register, bit 0 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

    // Enable GPIOB clock - RCC_AHB1ENR register, bit 1 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // Enable GPIOC clock - RCC_AHB1ENR register, bit 2 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_2);
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
    gpio->MODER   = (moder   != NONE) ? (gpio->MODER   |  (moder   << (pin_num*SHIFT_2)))
                                      : (gpio->MODER   & ~(moder   << (pin_num*SHIFT_2)));

    gpio->OTYPER  = (otyper  != NONE) ? (gpio->OTYPER  |  (otyper  <<  pin_num))
                                      : (gpio->OTYPER  & ~(otyper  <<  pin_num));

    gpio->OSPEEDR = (ospeedr != NONE) ? (gpio->OSPEEDR |  (ospeedr << (pin_num*SHIFT_2)))
                                      : (gpio->OSPEEDR & ~(ospeedr << (pin_num*SHIFT_2)));

    gpio->PUPDR   = (pupdr   != NONE) ? (gpio->PUPDR   |  (pupdr   << (pin_num*SHIFT_2)))
                                      : (gpio->PUPDR   & ~(pupdr   << (pin_num*SHIFT_2)));
}

//=======================================================================================


//=======================================================================================
// Write functions 

// TODO see if the ODR register can be used instead of the BSRR register 

// GPIOA write 
void gpio_write(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num,
    gpio_pin_state_t pin_state)
{
    switch (pin_state)
    {
        case GPIO_LOW:  // Set pin low 
            gpio->BSRR |= (((uint32_t)pin_num) << SHIFT_16);   // Set the reset bit 
            gpio->BSRR &= ~((uint32_t)pin_num);                // Clear the set bit 
            break;
        
        case GPIO_HIGH: // Set pin high 
            gpio->BSRR &= ~(((uint32_t)pin_num) << SHIFT_16);  // Clear the reset bit 
            gpio->BSRR |=    (uint32_t)pin_num;                // Set the set bit 
            break;
        
        default:  // Nothing done if 0 or 1 is not specified 
            break;
    }
}

//=======================================================================================


//=======================================================================================
// Read functions 

// GPIOA read 
uint8_t gpio_read(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num)
{
    // Local variables 
    uint16_t gpio_input; 
    uint8_t gpio_state = 0; 

    // Read the GPIO pin 
    gpio_input = (gpio->IDR) & pin_num;
    if (gpio_input) gpio_state = 1; 

    return gpio_state; 
}

//=======================================================================================
