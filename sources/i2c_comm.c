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

// TODO add the ability to customize the init with different frquencies. 

// Initialize I2C 1 in master mode 
void i2c1_init_master_mode(void)
{
    //==============================================================
    // Pin information 
    //  PB8: SCL - I2C1
    //  PB9: SDA - I2C1 
    //==============================================================

    //==============================================================
    // I2C Mater Mode Setup Steps 
    //  1. Enable the I2C clock and the GPIO clock. 
    //  2. Configure the I2C pins for alternative functions. 
    //      a) Select alternate function in MODER register. 
    //      b) Select Open Drain Output. 
    //      c) Select High SPEED for the PINs 
    //      d) Select Pill-up for both the Pins 
    //      e) Configure the Alternate Function in AFR Register. 
    //  3. Reset the I2C. 
    //  4. Ensure PE is disabled before setting up the I2C
    //  5. Program the peripheral input clock in I2C_CR2 register
    //  6. Configure the clock control register 
    //      a) Set to fast mode (Fm)
    //      b) Set Fm mode duty cycle to 16/9
    //      c) Set the clock control register (CCR) - master mode
    //  7. Configure the rise time register 
    //  8. Program the I2C_CR1 register to enable the peripheral 
    //  9. Set the START but in the I2C_CR1 register to generate a 
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
    I2C1->CR1 |=  (SET_BIT << SHIFT_15);  // Enable reset bit 
    I2C1->CR1 &= ~(SET_BIT << SHIFT_15);  // Disable reset bit


    // 4. Ensure PE is disabled before setting up the I2C
    I2C1->CR1 |= (CLEAR_BIT << SHIFT_0);


    // 5. Program the peripheral input clock in I2C_CR2 register
    I2C1->CR2 |= (I2C_APB1_42MHZ << SHIFT_0);


    // 6. Configure the clock control register 

    // a) Set to fast mode (Fm)
    I2C1->CCR |= (SET_BIT << SHIFT_15);

    // b) Set Fm mode duty cycle to 16/9
    I2C1->CCR |= (SET_BIT << SHIFT_14);

    // c) Set the clock control register (CCR) - master mode
    I2C1->CCR |= (I2C_CCR_FM_169_42_400 << SHIFT_0);


    // 7. Configure the rise time register
    I2C1->TRISE |= (I2C_TRISE_300_42 << SHIFT_0); 


    // 8. Program the I2C_CR1 register to enable the peripheral
    I2C1->CR1 |= (SET_BIT << SHIFT_0);
}

//=======================================================================================


//=======================================================================================
// Write I2C 

// Send data to a device using I2C 1 in master mode 
// void i2c1_write_master_mode(uint8_t *data, uint8_t data_size, uint8_t slave_address)
void i2c1_write_master_mode(uint8_t data, uint8_t data_size, uint8_t slave_address)
{
    // Note: this function runs in a blocking mode

    // Create start condition to initiate master mode 
    I2C1->CR1 |= (SET_BIT << SHIFT_8);           // Set start generation bit 
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_0)));  // Wait for start bit to set 

    // Send slave address 
    I2C1->DR = slave_address;                         // Send slave address 
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_1)));       // Wait for ADDR to set
    uint16_t read_clear = (I2C1->SR1) | (I2C1->SR2);  // Read SR1 and SR2 to clear ADDR

    // Send data 
    // for (uint8_t i = 0; i < data_size; i++)
    // {
    //     while(!(I2C1->SR1 & (SET_BIT << SHIFT_7)));  // Wait for TxE bit to set 
    //     I2C1->DR = *data;                            // Send data 
    //     data++;                                      // Move to next memory location 
    // }
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_7)));  // Wait for TxE bit to set 
    I2C1->DR = data;                            // Send data 
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_2)));  // Wait for byte transfer to finish 

    // Create stop condition 
    I2C1->CR1 |= (SET_BIT << SHIFT_9);  // Set the stop generation bit 
}

//=======================================================================================


//=======================================================================================
// Read I2C 

// Read data from a device using I2C 1 
void i2c1_read_master_mode(void)
{
    // 
}

//=======================================================================================
