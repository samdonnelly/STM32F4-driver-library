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

/**
 * @brief I2C1 wait for ADDR bit to set
 * 
 * @details This bit is set once the address has been successfully sent in master mode or
 *          successfully matched in slave mode. This event must occur before proceeding 
 *          to transfer data. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_addr_wait(
    I2C_TypeDef *i2c);


/**
 * @brief I2C1 clear acknowledge bit 
 * 
 * @details This function is used to clear the acknowledge bit which sends a NACK pulse to 
 *          the slave device. The NACK pulse is sent after the last byte of data has been 
 *          recieved from the slave. Once the slave sees the pulse it releases control of 
 *          the bus which allows the master to send a stop or restart condition. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_clear_ack(
    I2C_TypeDef *i2c);


/**
 * @brief I2C1 set acknowledge bit 
 * 
 * @details Setting the acknowledge bit is used to tell a slave device that data has been 
 *          recieved so the slave can proceed to send the next byte of data. This function 
 *          is called immediately after the data register is read. The acknowledge bit must
 *          also be set before generating a start condition. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_set_ack(
    I2C_TypeDef *i2c);


/**
 * @brief I2C1 wait for RxNE bit to set 
 * 
 * @details The RxNE bit indicates that there is data in the data register to be read from 
 *          the slave. This functions waits for the bit to set before proceeding to read 
 *          the data register. Once the data register is read then this bit clears until 
 *          more data is available. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_rxne_wait(
    I2C_TypeDef *i2c);


/**
 * @brief I2C1 wait for TxE bit to set 
 * 
 * @details The TxE bit is set when the data register is empty during transmission. It is 
 *          cleared when the data register is written to or when a start or stop condition 
 *          is generated. The bit won't set if a NACK pulse is received from the slave. 
 *          This function is called to wait for the bit to set before writing to the data 
 *          register. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_txe_wait(
    I2C_TypeDef *i2c);


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
 * @param i2c : pointer to I2C port 
 */
void i2c_btf_wait(
    I2C_TypeDef *i2c);

//=======================================================================================


//=======================================================================================
// Initiate I2C 

// I2C1 Initialization 
void i2c1_init(
    I2C_TypeDef *i2c, 
    i2c1_sda_pin_t sda_pin,
    i2c1_scl_pin_t scl_pin,
    i2c_run_mode_t run_mode,
    i2c_apb1_freq_t apb1_freq,
    i2c_ccr_setpoint_t ccr_reg,
    i2c_trise_setpoint_t trise_reg)
{
    //==============================================================
    // Pin information for I2C1
    //  PB6: SCL
    //  PB7: SDA
    //  PB8: SCL
    //  PB9: SDA
    // 
    // Pin information for I2C2
    //  PB3:  SDA
    //  PB9:  SDA
    //  PB10: SCL
    // 
    // Pin information for I2C3
    //  PA8: SCL
    //  PB4: SDA
    //  PB8: SDA
    //  PC9: SDA
    //==============================================================

    // Enable I2C1 clock - RCC_APB1ENR register, bit 21
    RCC->APB1ENR |= (SET_BIT << SHIFT_21);

    // Enable GPIOB clock - RCC_AHB1ENR register, bit 1
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // Configure the I2C pins for alternative functions.
    // Select alternate function in MODER register. 
    GPIOB->MODER |= (SET_2 << (SHIFT_12 + 2*scl_pin));
    GPIOB->MODER |= (SET_2 << (SHIFT_14 + 2*sda_pin));

    // Select Open Drain Output - used for lines with multiple devices
    GPIOB->OTYPER |= (SET_BIT << (SHIFT_6 + scl_pin));
    GPIOB->OTYPER |= (SET_BIT << (SHIFT_7 + sda_pin));

    // Select High SPEED for the pins 
    GPIOB->OSPEEDR |= (SET_3 << (SHIFT_12 + 2*scl_pin));
    GPIOB->OSPEEDR |= (SET_3 << (SHIFT_14 + 2*sda_pin));

    // Select pull-up for both the pins 
    GPIOB->PUPDR |= (SET_BIT << (SHIFT_12 + 2*scl_pin));
    GPIOB->PUPDR |= (SET_BIT << (SHIFT_14 + 2*sda_pin));

    // Configure the Alternate Function in AFR Register. 
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

    // Reset the I2C - enable then disable reset bit 
    i2c->CR1 |=  (SET_BIT << SHIFT_15);
    i2c->CR1 &= ~(SET_BIT << SHIFT_15);

    // Ensure PE is disabled before setting up the I2C
    i2c->CR1 |= (CLEAR_BIT << SHIFT_0);

    // Program the peripheral input clock in I2C_CR2 register
    i2c->CR2 |= (apb1_freq << SHIFT_0);

    // Configure the clock control register 
    // Choose Sm or Fm mode 
    switch(run_mode)
    {
        case I2C_MODE_SM:
            i2c->CCR &= ~(SET_BIT << SHIFT_15);
            break;
        
        case I2C_MODE_FM_2:
            i2c->CCR |= (SET_BIT << SHIFT_15);
            i2c->CCR &= ~(SET_BIT << SHIFT_14);  // Set duty cycle to 2 
            break;

        case I2C_MODE_FM_169: 
            i2c->CCR |= (SET_BIT << SHIFT_15);
            i2c->CCR |= (SET_BIT << SHIFT_14);  // Set duty cycle to 16/9 
            break;
        
        default:
            break;
    }

    // Calculated clock control register based on PCLK1 & SCL frquency 
    i2c->CCR |= (ccr_reg << SHIFT_0);

    // Configure the rise time register
    i2c->TRISE |= (trise_reg << SHIFT_0);

    // Program the I2C_CR1 register to enable the peripheral
    i2c->CR1 |= (SET_BIT << SHIFT_0);
}

//=======================================================================================


//=======================================================================================
// I2C1 register functions

// I2C1 generate start condition 
void i2c_start(I2C_TypeDef *i2c)
{
    i2c_set_ack(i2c);                           // Enable acknowledgement bit 
    i2c->CR1 |= (SET_BIT << SHIFT_8);           // Set start generation bit 
    // TODO add a timeout here with a status return for fail or success 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_0)));  // Wait for start bit to set 
}

// I2C1 generate a stop condition by setting the stop generation bit 
void i2c_stop(I2C_TypeDef *i2c)
{
    i2c->CR1 |= (SET_BIT << SHIFT_9);
}

// Read the SR1 and SR2 registers to clear ADDR
void i2c_clear_addr(I2C_TypeDef *i2c)
{
    dummy_read(((i2c->SR1) | (i2c->SR2))); 
}

// I2C1 wait for ADDR bit to set
void i2c_addr_wait(I2C_TypeDef *i2c)
{
    // TODO add a timeout here with a status return for fail or success 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_1)));
}

// I2C1 clear the ACK bit to send a NACK pulse to slave device 
void i2c_clear_ack(I2C_TypeDef *i2c)
{
    i2c->CR1 &= ~(SET_BIT << SHIFT_10);
}

// I2C1 set the ACK bit to tell the slave that data has been receieved
void i2c_set_ack(I2C_TypeDef *i2c)
{
    i2c->CR1 |= (SET_BIT << SHIFT_10);
}

// I2C1 wait for RxNE bit to set indicating data is ready 
void i2c_rxne_wait(I2C_TypeDef *i2c)
{
    while(!(i2c->SR1 & (SET_BIT << SHIFT_6)));
}

// I2C1 wait for TxE bit to set 
void i2c_txe_wait(I2C_TypeDef *i2c)
{
    // TODO add a timeout here with a status return for fail or success 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_7)));
}

// I2C1 wait for BTF to set
void i2c_btf_wait(I2C_TypeDef *i2c)
{
    while(!(i2c->SR1 & (SET_BIT << SHIFT_2)));
}

//=======================================================================================


//=======================================================================================
// Write I2C1 

// I2C1 send address 
void i2c_write_address(
    I2C_TypeDef *i2c, 
    uint8_t i2c1_address)
{
    i2c->DR = i2c1_address;  // Send slave address 
    i2c_addr_wait(i2c);      // Wait for ADDR to set
}

// I2C1 send data to a device
void i2c_write_master_mode(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t data_size)
{
    for (uint8_t i = 0; i < data_size; i++)
    {
        i2c_txe_wait(i2c);  // Wait for TxE bit to set 
        i2c->DR = *data;    // Send data 
        data++;             // Increment memory 
    }

    // Wait for BTF to set
    i2c_btf_wait(i2c);
}

//=======================================================================================


//=======================================================================================
// Read I2C1

// I2C1 read data from a device
void i2c_read_master_mode(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size)
{
    // Check the amount of data to be received 
    switch(data_size)
    {
        case BYTE_0:  // No data specified - no transmission
            break;

        case BYTE_1:  // One-byte transmission 
            // Clear the ACK bit to send a NACK pulse to the slave
            i2c_clear_ack(i2c);

            // Read SR1 and SR2 to clear ADDR
            i2c_clear_addr(i2c);

            // Generate stop condition
            i2c_stop(i2c);

            // Wait for RxNE bit to set indicating data is ready 
            i2c_rxne_wait(i2c);

            // Read the data regsiter
            *data = i2c->DR;

            break;

        default:  // Greater than one-byte transmission 
            // Read SR1 and SR2 to clear ADDR
            i2c_clear_addr(i2c);

            // Normal reading 
            for (uint8_t i = 0; i < (data_size - BYTE_2); i++)
            {
                i2c_rxne_wait(i2c);  // Wait for RxNE bit to set indicating data is ready 
                *data = i2c->DR;     // Read data
                i2c_set_ack(i2c);    // Set the ACK bit 
                data++;              // Increment memeory location 
            }

            // Read the second last data byte 
            i2c_rxne_wait(i2c);
            *data = i2c->DR;

            // Clear the ACK bit to send a NACK pulse to the slave
            i2c_clear_ack(i2c);

            // Generate stop condition
            i2c_stop(i2c);

            // Read the last data byte
            data++;
            i2c_rxne_wait(i2c);
            *data = i2c->DR;

            break;
    }
}


// I2C read data until a termination character is seen 
void i2c_read_to_term(
    I2C_TypeDef *i2c, 
    uint8_t *data,  
    uint8_t term_char, 
    uint16_t bytes_remain)
{
    // Read the data until the termination character is seen 
    do
    {
        i2c_rxne_wait(i2c); 
        *data = i2c->DR; 
        i2c_set_ack(i2c); 
    } 
    while (*data++ != term_char); 

    // Read the remaining bytes and terminate the data 
    i2c_read_master_mode(i2c, data, bytes_remain); 
    data += bytes_remain; 
    *data = 0; 
}


// I2C read data of a certain length that is defined within the message 
void i2c_read_to_len(
    I2C_TypeDef *i2c, 
    uint8_t address, 
    uint8_t *data, 
    uint8_t len_location, 
    uint8_t len_bytes, 
    uint8_t add_bytes)
{
    // Local variables 
    uint16_t msg_length = 0; 

    // Read up to and including the part of the message that specifies the length 
    i2c_read_master_mode(i2c, data, len_location + len_bytes); 

    // Extract the length and correct it using the 'add_bytes' argument 
    data += len_location; 
    
    if (len_bytes == BYTE_1)
    {
        msg_length = (uint16_t)(*data++) + add_bytes; 
    }

    else if (len_bytes == BYTE_2)
    {
        msg_length  = (uint16_t)(*data++); 
        msg_length |= (uint16_t)(*data++ << SHIFT_8); 
        msg_length += add_bytes; 
    }
    
    // Read the rest of the message 
    i2c_start(i2c); 
    i2c_write_address(i2c, address); 
    i2c_clear_addr(i2c); 
    i2c_read_master_mode(i2c, data, msg_length); 

    // TODO Add a termination/NULL character? You will need to offset the 'data' address for 
    // when you return from the I2C read function. 
}

//=======================================================================================
