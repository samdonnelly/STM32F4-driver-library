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
    // For the TODO's listed below, create a configuration (maybe 
    // in the header) that the user can select to configure I2C1 
    // how they'd like easily without needing to figure out all the 
    // individual parameters to pass. 
    //==============================================================

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
    //      a) Set the mode
    //      b) If in Fm mode then choose the duty cycle
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
    // TODO add the ability to select which pins to use

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
    // TODO add the ability to configure the I2C with different frquencies. 
    I2C1->CR2 |= (I2C_APB1_42MHZ << SHIFT_0);


    // 6. Configure the clock control register 
    // TODO make Sm or Fm mode configurable when calling the init function 

    // a) Set the mode
    I2C1->CCR &= ~(CLEAR_BIT << SHIFT_15);  // Sm mode
    // I2C1->CCR |= (SET_BIT << SHIFT_15);  // Fm mode

    // b) If in Fm mode then choose the duty cycle
    // TODO Add the duty cycle selection here

    // c) Set the clock control register (CCR) - master mode
    I2C1->CCR |= (I2C_CCR_SM_42_100 << SHIFT_0);


    // 7. Configure the rise time register
    // TODO configure this based on Sm or Fm mode
    I2C1->TRISE |= (I2C_TRISE_1000_42 << SHIFT_0); 


    // 8. Program the I2C_CR1 register to enable the peripheral
    I2C1->CR1 |= (SET_BIT << SHIFT_0);
}

//=======================================================================================


//=======================================================================================
// Working with Registers 

// Generate start condition 
void i2c1_start(void)
{
    I2C1->CR1 |= (SET_BIT << SHIFT_8);           // Set start generation bit 
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_0)));  // Wait for start bit to set 
}

// Generate a stop condition by setting the stop generation bit 
void i2c1_stop(void)
{
    I2C1->CR1 |= (SET_BIT << SHIFT_9);
}

// Read SR1 and SR2 to clear ADDR
void i2c1_clear_addr(void)
{
    uint16_t read_clear = (I2C1->SR1) | (I2C1->SR2);
}

// Wait for ADDR to set
void i2c1_addr_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_1)));
}

// Clear the ACK bit to send a NACK pulse to slave device 
void i2c1_clear_ack(void)
{
    I2C1->CR1 &= ~(SET_BIT << SHIFT_10);
}

// Set the ACK bit to tell the slave data has been receieved
void i2c1_set_ack(void)
{
    I2C1->CR1 |= (SET_BIT << SHIFT_10);
}

// Wait for RxNE bit to set indicating data is ready 
void i2c1_rxne_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_6)));
}

// Wait for TxE bit to set 
void i2c1_txe_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_7)));
}

// Wait for BTF to set
void i2c1_btf_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_2)));
}

//=======================================================================================


//=======================================================================================
// Write I2C 

// Send address 
void i2c1_write_address(uint8_t i2c1_address)
{
    I2C1->DR = i2c1_address;  // Send slave address 
    i2c1_addr_wait();         // Wait for ADDR to set
}

// Send data to a device using I2C 1 in master mode 
void i2c1_write_master_mode(uint8_t *data, uint8_t data_size)
{
    for (uint8_t i = 0; i < data_size; i++)
    {
        i2c1_txe_wait();   // Wait for TxE bit to set 
        I2C1->DR = *data;  // Send data 
        data++;            // Increment memory 
    }

    // Wait for BTF to set
    i2c1_btf_wait();
}

//=======================================================================================


//=======================================================================================
// Read I2C 

// Read data from a device using I2C 1 
void i2c1_read_master_mode(uint8_t *data, uint8_t data_size)
{
    // Check the amount of data to be receieved 
    switch(data_size)
    {
        case I2C_0_BYTE:  // No transmission 
            // No data specified. Return. 
            return;

        case I2C_1_BYTE:  // One-byte transmission 
            // Set ACK to zero to send a NACK pulse to slave device 
            i2c1_clear_ack();

            // Read SR1 and SR2 to clear ADDR
            i2c1_clear_addr();

            // Generate stop condition by setting the stop bit
            i2c1_stop();

            // Wait for RxNE bit to set indicating data is ready 
            i2c1_rxne_wait();

            // Read the data regsiter
            *data = I2C1->DR;

            break;

        default:  // Greater than one-byte transmission 
            // Normal reading 
            for (uint8_t i = 0; i < (data_size - I2C_2_BYTE); i++)
            {
                i2c1_rxne_wait();  // Wait for RxNE bit to set indicating data is ready 
                *data = I2C1->DR;  // Read data
                i2c1_set_ack();    // Set the ACK bit 
                data++;            // Increment memeory location 
            }

            // Read the second last data byte 
            i2c1_rxne_wait();
            *data = I2C1->DR;

            // Set ACK to zero to send a NACK pulse to slave device 
            i2c1_clear_ack();

            // Generate stop condition by setting the stop bit
            i2c1_stop();

            // Read the last data byte
            data++;
            i2c1_rxne_wait();
            *data = I2C1->DR;

            break;
    }
}

//=======================================================================================
