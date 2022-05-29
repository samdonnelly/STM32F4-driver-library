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
