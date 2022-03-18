/**
 * @file i2c_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2022-03-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Initiate I2C 

// Initialize I2C 1 in master mode 
void i2c1_init_master_mode(void)
{
    //==============================================================
    // I2C Mater Mode 
    //  1. Enable the I2C clock and the GPIO clock. 
    //  2. Configure the I2C pins for alternative functions. 
    //      a) Select alternate function in MODER register. 
    //      b) Select Open Drain Output. 
    //      c) Select High SPEED for the PINs 
    //      d) Select Pill-up for both the Pins 
    //      e) Configure the Alternate Function in AFR Register. 
    //  3. Reset the I2C. 
    //  4. Program the peripheral input clock in I2C_CR2 register
    //  5. Configure the clock control register 
    //  6. Configure the rise time register 
    //  7. Program the I2C_CR1 register to enable the peripheral 
    //  8. Set the START but in the I2C_CR1 register to generate a 
    //     Start condition 
    //==============================================================

    // 1. Enable the I2C clock and the GPIO clock. 

    // Enable I2C clock - RCC_APB1ENR register, bit 21
    RCC->APB1ENR |= (SET_BIT << SHIFT_21);

    // Enable GPIOB clock - RCC_AHB1ENR register, bit 1
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // 2. Configure the I2C pins for alternative functions.

    // a) Select alternate function in MODER register. 
    GPIOB->MODER |= (SET_2 << SHIFT_16);      // pin PB8
    GPIOB->MODER |= (SET_2 << SHIFT_18);      // pin PB9

    // b) Select Open Drain Output - used for lines with multiple devices
    GPIOB->OTYPER |= (SET_BIT << SHIFT_8);    // pin PB8 
    GPIOB->OTYPER |= (SET_BIT << SHIFT_9);    // pin PB9

    // c) Select High SPEED for the PINs 
    GPIOB->OSPEEDR |= (SET_3 << SHIFT_16);    // pin PB8
    GPIOB->OSPEEDR |= (SET_3 << SHIFT_18);    // pin PB9

    // d) Select Pill-up for both the Pins 
    GPIOB->PUPDR |= (SET_BIT << SHIFT_16);    // pin PB8 
    GPIOB->PUPDR |= (SET_BIT << SHIFT_18);    // pin PB9

    // e) Configure the Alternate Function in AFR Register. 
    GPIOB->AFR[1] |= (SET_4 << SHIFT_0);      // pin PB8 
    GPIOB->AFR[1] |= (SET_4 << SHIFT_4);      // pin PB9

    // 3. Reset the I2C. 

    // 4. Program the peripheral input clock in I2C_CR2 register
    I2C1->CR2 |= (I2C_APB1_42MHZ << SHIFT_0);

    // 5. Configure the clock control register 

    // Set to fast mode (fm)
    I2C1->CCR |= (SET_BIT << SHIFT_15);

}


// Initialize I2C 1 in slave mode 
void i2c1_init_slave_mode(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Write I2C 

// Send data to a device using I2C 1 
void i2c1_write(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Read I2C 

// Read data from a device using I2C 1 
void i2c1_read(void)
{
    // 
}

//=======================================================================================
