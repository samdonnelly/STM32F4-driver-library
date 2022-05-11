/**
 * @file i2c_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C initialization, read and write 
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
// Function Prototypes 

//==============================================================
// Note: These functions are for internal driver use only and 
//       are therefore not included in the header file. 
//==============================================================

/**
 * @brief I2C1 wait for ADDR bit to set
 * 
 * @details This bit is set once the address has been successfully sent in master mode or
 *          successfully matched in slave mode. This event must occur before proceeding 
 *          to transfer data. 
 */
void i2c1_addr_wait(void);


/**
 * @brief I2C1 clear acknowledge bit 
 * 
 * @details This function is used to clear the acknowledge bit which sends a NACK pulse to 
 *          the slave device. The NACK pulse is sent after the last byte of data has been 
 *          recieved from the slave. Once the slave sees the pulse it releases control of 
 *          the bus which allows the master to send a stop or restart condition. 
 * 
 */
void i2c1_clear_ack(void);


/**
 * @brief I2C1 set acknowledge bit 
 * 
 * @details Setting the acknowledge bit is used to tell a slave device that data has been 
 *          recieved so the slave can proceed to send the next byte of data. This function 
 *          is called immediately after the data register is read. The acknowledge bit must
 *          also be set before generating a start condition. 
 * 
 */
void i2c1_set_ack(void);


/**
 * @brief I2C1 wait for RxNE bit to set 
 * 
 * @details The RxNE bit indicates that there is data in the data register to be read from 
 *          the slave. This functions waits for the bit to set before proceeding to read 
 *          the data register. Once the data register is read then this bit clears until 
 *          more data is available. 
 * 
 */
void i2c1_rxne_wait(void);


/**
 * @brief I2C1 wait for TxE bit to set 
 * 
 * @details The TxE bit is set when the data register is empty during transmission. It is 
 *          cleared when the data register is written to or when a start or stop condition 
 *          is generated. The bit won't set if a NACK pulse is received from the slave. 
 *          This function is called to wait for the bit to set before writing to the data 
 *          register. 
 * 
 */
void i2c1_txe_wait(void);


/**
 * @brief I2C1 wait for BTF bit to set
 * 
 * @details This function is called at the end of a data transmission to a slave device. 
 *          The BTF bit indicates if the byte transfer is in progress or complete. When 
 *          all bytes have been written to the slave this function is called and it waits 
 *          for BTF to set to indicate that the last byte has been transferred at which 
 *          point the write sequence in the code ends and a stop condition can be 
 *          generated.
 * 
 */
void i2c1_btf_wait(void);

//=======================================================================================


//=======================================================================================
// Initiate I2C 

//==============================================================
// I2C Setup Steps 
//  1. Enable the I2C clock and the GPIO clock
//      a) Enable I2CX clock
//      b) Enable GPIOX clock
//  2. Configure the I2C pins for alternative functions
//      a) Select alternate function in MODER register
//      b) Select Open Drain Output
//      c) Select High SPEED for the PINs 
//      d) Select Pill-up for both the Pins 
//      e) Configure the Alternate Function in AFR Register
//  3. Reset the I2C
//  4. Ensure PE is disabled before setting up the I2C
//  5. Program the peripheral input clock in I2C_CR2 register
//  6. Configure the clock control register based on the run mode
//      a) Choose Sm or Fm mode 
//      b) Calculated clock control register based on PCLK1 & SCL frquency 
//  7. Configure the rise time register 
//  8. Program the I2C_CR1 register to enable the peripheral 
//  9. Set the START but in the I2C_CR1 register to generate a 
//     Start condition 
//==============================================================

// I2C1 Initialization 
void i2c1_init(
    uint8_t sda_pin,
    uint8_t scl_pin,
    uint8_t run_mode,
    uint8_t apb1_freq,
    uint8_t fm_duty_cycle,
    uint8_t ccr_reg,
    uint8_t trise_reg)
{
    //==============================================================
    // Pin information for I2C1
    //  PB6: SCL
    //  PB7: SDA
    //  PB8: SCL
    //  PB9: SDA
    //==============================================================

    // 1. Enable the I2C clock and the GPIO clock. 

    // a) Enable I2C1 clock - RCC_APB1ENR register, bit 21
    RCC->APB1ENR |= (SET_BIT << SHIFT_21);

    // b) Enable GPIOB clock - RCC_AHB1ENR register, bit 1
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);


    // 2. Configure the I2C pins for alternative functions.

    // a) Select alternate function in MODER register. 
    GPIOB->MODER |= (SET_2 << (SHIFT_12 + 2*scl_pin));
    GPIOB->MODER |= (SET_2 << (SHIFT_14 + 2*sda_pin));

    // b) Select Open Drain Output - used for lines with multiple devices
    GPIOB->OTYPER |= (SET_BIT << (SHIFT_6 + scl_pin));
    GPIOB->OTYPER |= (SET_BIT << (SHIFT_7 + sda_pin));

    // c) Select High SPEED for the pins 
    GPIOB->OSPEEDR |= (SET_3 << (SHIFT_12 + 2*scl_pin));
    GPIOB->OSPEEDR |= (SET_3 << (SHIFT_14 + 2*sda_pin));

    // d) Select pull-up for both the pins 
    GPIOB->PUPDR |= (SET_BIT << (SHIFT_12 + 2*scl_pin));
    GPIOB->PUPDR |= (SET_BIT << (SHIFT_14 + 2*sda_pin));

    // e) Configure the Alternate Function in AFR Register. 
    switch(scl_pin)
    {
        case I2C1_SCL_PB6:
            GPIOB->AFR[0] |= (SET_4 << SHIFT_24);
            break;
        case I2C1_SCL_PB8:
            GPIOB->AFR[1] |= (SET_4 << SHIFT_0);
            break;
        default:
            break;
    }

    switch(sda_pin)
    {
        case I2C1_SDA_PB7:
            GPIOB->AFR[0] |= (SET_4 << SHIFT_28);
            break;
        case I2C1_SDA_PB9:
            GPIOB->AFR[1] |= (SET_4 << SHIFT_4);
            break;
        default:
            break;
    }

    // 3. Reset the I2C - enable then disable reset bit 
    I2C1->CR1 |=  (SET_BIT << SHIFT_15);
    I2C1->CR1 &= ~(SET_BIT << SHIFT_15);


    // 4. Ensure PE is disabled before setting up the I2C
    I2C1->CR1 |= (CLEAR_BIT << SHIFT_0);


    // 5. Program the peripheral input clock in I2C_CR2 register
    I2C1->CR2 |= (apb1_freq << SHIFT_0);


    // 6. Configure the clock control register 

    // a) Choose Sm or Fm mode 
    switch(run_mode)
    {
        case I2C_SM_MODE:
            I2C1->CCR &= ~(SET_BIT << SHIFT_15);
            break;
        
        case I2C_FM_MODE:
            I2C1->CCR |= (SET_BIT << SHIFT_15);
            
            // Only applies in Fm mode - chooses duty cycle 
            switch(fm_duty_cycle)
            {
                case I2C_FM_DUTY_2:
                    I2C1->CCR &= ~(SET_BIT << SHIFT_14);
                    break;
                case I2C_FM_DUTY_169:
                    I2C1->CCR |= (SET_BIT << SHIFT_14);
                    break;
                default:
                    break;
            }
            break;
        
        default:
            break;
    }

    // b) Calculated clock control register based on PCLK1 & SCL frquency 
    I2C1->CCR |= (ccr_reg << SHIFT_0);


    // 7. Configure the rise time register
    I2C1->TRISE |= (trise_reg << SHIFT_0);


    // 8. Program the I2C_CR1 register to enable the peripheral
    I2C1->CR1 |= (SET_BIT << SHIFT_0);
}

// I2C2 Initialization 
void i2c2_init(
    uint8_t sda_pin,
    uint8_t scl_pin,
    uint8_t run_mode,
    uint8_t apb1_freq,
    uint8_t fm_duty_cycle,
    uint8_t ccr_reg,
    uint8_t trise_reg)
{
    //==============================================================
    // Pin information for I2C2
    //  PB3:  SDA
    //  PB9:  SDA
    //  PB10: SCL
    //==============================================================
}

// I2C3 Initialization 
void i2c3_init(
    uint8_t sda_pin,
    uint8_t scl_pin,
    uint8_t run_mode,
    uint8_t apb1_freq,
    uint8_t fm_duty_cycle,
    uint8_t ccr_reg,
    uint8_t trise_reg)
{
    //==============================================================
    // Pin information for I2C3
    //  PA8: SCL
    //  PB4: SDA
    //  PB8: SDA
    //  PC9: SDA
    //==============================================================
}

//=======================================================================================


//=======================================================================================
// I2C1 register functions

// I2C1 generate start condition 
void i2c1_start(void)
{
    i2c1_set_ack();                              // Enable acknowledgement bit 
    I2C1->CR1 |= (SET_BIT << SHIFT_8);           // Set start generation bit 
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_0)));  // Wait for start bit to set 
}

// I2C1 generate a stop condition by setting the stop generation bit 
void i2c1_stop(void)
{
    I2C1->CR1 |= (SET_BIT << SHIFT_9);
}

// I2C1 read SR1 and SR2 to an unused variable to clear ADDR
void i2c1_clear_addr(void)
{
    uint16_t read_clear = (I2C1->SR1) | (I2C1->SR2);
}

// I2C1 wait for ADDR bit to set
void i2c1_addr_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_1)));
}

// I2C1 clear the ACK bit to send a NACK pulse to slave device 
void i2c1_clear_ack(void)
{
    I2C1->CR1 &= ~(SET_BIT << SHIFT_10);
}

// I2C1 set the ACK bit to tell the slave that data has been receieved
void i2c1_set_ack(void)
{
    I2C1->CR1 |= (SET_BIT << SHIFT_10);
}

// I2C1 wait for RxNE bit to set indicating data is ready 
void i2c1_rxne_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_6)));
}

// I2C1 wait for TxE bit to set 
void i2c1_txe_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_7)));
}

// I2C1 wait for BTF to set
void i2c1_btf_wait(void)
{
    while(!(I2C1->SR1 & (SET_BIT << SHIFT_2)));
}

//=======================================================================================


//=======================================================================================
// Write I2C1 

// I2C1 send address 
void i2c1_write_address(uint8_t i2c1_address)
{
    I2C1->DR = i2c1_address;  // Send slave address 
    i2c1_addr_wait();         // Wait for ADDR to set
}

// I2C1 send data to a device
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
// Read I2C1

// I2C1 read data from a device
void i2c1_read_master_mode(uint8_t *data, uint8_t data_size)
{
    // Check the amount of data to be receieved 
    switch(data_size)
    {
        case I2C_0_BYTE:  // No data specified - no transmission
            break;

        case I2C_1_BYTE:  // One-byte transmission 
            // Clear the ACK bit to send a NACK pulse to the slave
            i2c1_clear_ack();

            // Read SR1 and SR2 to clear ADDR
            i2c1_clear_addr();

            // Generate stop condition
            i2c1_stop();

            // Wait for RxNE bit to set indicating data is ready 
            i2c1_rxne_wait();

            // Read the data regsiter
            *data = I2C1->DR;

            break;

        default:  // Greater than one-byte transmission 
            // Read SR1 and SR2 to clear ADDR
            i2c1_clear_addr();

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

            // Clear the ACK bit to send a NACK pulse to the slave
            i2c1_clear_ack();

            // Generate stop condition
            i2c1_stop();

            // Read the last data byte
            data++;
            i2c1_rxne_wait();
            *data = I2C1->DR;

            break;
    }
}

//=======================================================================================
