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

// TODO combine all GPIOs into common functins 

//=======================================================================================
// Initialization 

// GPIOA init
// TODO skip steps that are not needed when initializing an input 
void gpioa_init(
    uint16_t pin_num,
    uint32_t moder,
    uint32_t otyper,
    uint32_t ospeedr,
    uint32_t pupdr)
{
    GPIOA->MODER   = (moder   != NONE) ? (GPIOA->MODER   |  (moder   << (pin_num*SHIFT_2)))
                                       : (GPIOA->MODER   & ~(moder   << (pin_num*SHIFT_2)));

    GPIOA->OTYPER  = (otyper  != NONE) ? (GPIOA->OTYPER  |  (otyper  <<  pin_num))
                                       : (GPIOA->OTYPER  & ~(otyper  <<  pin_num));

    GPIOA->OSPEEDR = (ospeedr != NONE) ? (GPIOA->OSPEEDR |  (ospeedr << (pin_num*SHIFT_2)))
                                       : (GPIOA->OSPEEDR & ~(ospeedr << (pin_num*SHIFT_2)));

    GPIOA->PUPDR   = (pupdr   != NONE) ? (GPIOA->PUPDR   |  (pupdr   << (pin_num*SHIFT_2)))
                                       : (GPIOA->PUPDR   & ~(pupdr   << (pin_num*SHIFT_2)));
}


// GPIOB init
void gpiob_init(
    uint16_t pin_num,
    uint32_t moder,
    uint32_t otyper,
    uint32_t ospeedr,
    uint32_t pupdr)
{
    GPIOB->MODER   = (moder   != NONE) ? (GPIOB->MODER   |  (moder   << (pin_num*SHIFT_2)))
                                       : (GPIOB->MODER   & ~(moder   << (pin_num*SHIFT_2)));

    GPIOB->OTYPER  = (otyper  != NONE) ? (GPIOB->OTYPER  |  (otyper  <<  pin_num))
                                       : (GPIOB->OTYPER  & ~(otyper  <<  pin_num));

    GPIOB->OSPEEDR = (ospeedr != NONE) ? (GPIOB->OSPEEDR |  (ospeedr << (pin_num*SHIFT_2)))
                                       : (GPIOB->OSPEEDR & ~(ospeedr << (pin_num*SHIFT_2)));

    GPIOB->PUPDR   = (pupdr   != NONE) ? (GPIOB->PUPDR   |  (pupdr   << (pin_num*SHIFT_2)))
                                       : (GPIOB->PUPDR   & ~(pupdr   << (pin_num*SHIFT_2)));
}

//=======================================================================================


//=======================================================================================
// Write functions 

// TODO see if the ODR register can be used instead of the BSRR register 

// GPIOA write 
// TODO why not just pass pin_num as uint32_t? 
void gpioa_write(
    uint16_t pin_num,
    uint8_t  pin_state)
{
    switch (pin_state)
    {
        case GPIO_LOW:  // Set pin low 
            GPIOA->BSRR |= (((uint32_t)pin_num) << SHIFT_16);   // Set the reset bit 
            GPIOA->BSRR &= ~((uint32_t)pin_num);                // Clear the set bit 
            break;
        
        case GPIO_HIGH: // Set pin high 
            GPIOA->BSRR &= ~(((uint32_t)pin_num) << SHIFT_16);  // Clear the reset bit 
            GPIOA->BSRR |=    (uint32_t)pin_num;                // Set the set bit 
            break;
        
        default:  // Nothing done if 0 or 1 is not specified 
            break;
    }
}


// GPIOB write 
void gpiob_write(
    uint16_t pin_num,
    uint8_t  pin_state)
{
    switch (pin_state)
    {
        case GPIO_LOW:  // Set pin low 
            GPIOB->BSRR |= (((uint32_t)pin_num) << SHIFT_16);   // Set the reset bit 
            GPIOB->BSRR &= ~((uint32_t)pin_num);                // Clear the set bit 
            break;
        
        case GPIO_HIGH: // Set pin high 
            GPIOB->BSRR &= ~(((uint32_t)pin_num) << SHIFT_16);  // Clear the reset bit 
            GPIOB->BSRR |=    (uint32_t)pin_num;                // Set the set bit 
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
    uint8_t gpio, 
    uint16_t pin_num)
{
    // TODO figure out how to pass the register to the function and call it via pointer 
    //      here to reduce the amount of code 

    // Local variables 
    uint16_t gpio_input; 
    uint8_t gpio_state = 0; 

    switch (gpio)
    {
        case GPIO_A:
            gpio_input = (GPIOA->IDR) & pin_num; 
            break; 
        
        case GPIO_B:
            gpio_input = (GPIOB->IDR) & pin_num;  
            break; 
        
        case GPIO_C:
            gpio_input = (GPIOC->IDR) & pin_num; 
            break;
        
        default:
            gpio_input = 0; 
            break; 
    }

    if (gpio_input) gpio_state = 1; 

    return gpio_state; 
}

//=======================================================================================
