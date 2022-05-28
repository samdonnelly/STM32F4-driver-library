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

// 
void gpiob_init(
    uint16_t pin_num,
    uint32_t moder,
    uint32_t otyper,
    uint32_t ospeedr,
    uint32_t pupdr)
{
    // 
    if (moder   != NONE) { };
    if (otyper  != NONE) { };
    if (ospeedr != NONE) { };
    if (pupdr   != NONE) { };
}


// 
void gpiob_moder(
    uint16_t pin_num,
    uint32_t moder)
{
    // 
}


// 
void gpiob_otyper(
    uint16_t pin_num,
    uint32_t otyper)
{
    // 
}


// 
void gpiob_ospeedr(
    uint16_t pin_num,
    uint32_t ospeedr)
{
    // 
}


// 
void gpiob_pupdr(
    uint16_t pin_num,
    uint32_t pupdr)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Write functions 

// GPIOA write 
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
            GPIOA->BSRR |= (uint32_t)pin_num;                   // Set the set bit 
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
            GPIOB->BSRR |= (uint32_t)pin_num;                   // Set the set bit 
            break;
        
        default:  // Nothing done if 0 or 1 is not specified 
            break;
    }
}


// GPIOC write 
void gpioc_write(
    uint16_t pin_num,
    uint8_t  pin_state)
{
    switch (pin_state)
    {
        case GPIO_LOW:  // Set pin low 
            GPIOC->BSRR |= (((uint32_t)pin_num) << SHIFT_16);   // Set the reset bit 
            GPIOC->BSRR &= ~((uint32_t)pin_num);                // Clear the set bit 
            break;
        
        case GPIO_HIGH: // Set pin high 
            GPIOC->BSRR &= ~(((uint32_t)pin_num) << SHIFT_16);  // Clear the reset bit 
            GPIOC->BSRR |= (uint32_t)pin_num;                   // Set the set bit 
            break;
        
        default:  // Nothing done if 0 or 1 is not specified 
            break;
    }
}


//=======================================================================================
