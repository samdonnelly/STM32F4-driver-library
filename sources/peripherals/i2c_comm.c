/**
 * @file i2c_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C driver 
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
// Enums 

/**
 * @brief Data buffer increment 
 */
typedef enum {
    I2C_BUFF_NO_INCREMENT, 
    I2C_BUFF_INCREMENT
} i2c_buff_increment_t; 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief I2C wait for ADDR bit to set
 * 
 * @details This bit is set once the address has been successfully sent in master mode or
 *          successfully matched in slave mode. This event must occur before proceeding 
 *          to transfer data. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to I2C port 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_addr_wait(I2C_TypeDef *i2c);


/**
 * @brief I2C clear acknowledge bit 
 * 
 * @details This function is used to clear the acknowledge bit which sends a NACK pulse to 
 *          the slave device. The NACK pulse is sent after the last byte of data has been 
 *          recieved from the slave. Once the slave sees the pulse it releases control of 
 *          the bus which allows the master to send a stop or restart condition. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_clear_ack(I2C_TypeDef *i2c);


/**
 * @brief I2C set acknowledge bit 
 * 
 * @details Setting the acknowledge bit is used to tell a slave device that data has been 
 *          recieved so the slave can proceed to send the next byte of data. This function 
 *          is called immediately after the data register is read. The acknowledge bit must
 *          also be set before generating a start condition. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_set_ack(I2C_TypeDef *i2c);


/**
 * @brief I2C wait for RxNE bit to set 
 * 
 * @details The RxNE bit indicates that there is data in the data register to be read from 
 *          the slave. This functions waits for the bit to set before proceeding to read 
 *          the data register. Once the data register is read then this bit clears until 
 *          more data is available. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to I2C port 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_rxne_wait(I2C_TypeDef *i2c);


/**
 * @brief I2C wait for TxE bit to set 
 * 
 * @details The TxE bit is set when the data register is empty during transmission. It is 
 *          cleared when the data register is written to or when a start or stop condition 
 *          is generated. The bit won't set if a NACK pulse is received from the slave. 
 *          This function is called to wait for the bit to set before writing to the data 
 *          register. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to I2C port 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_txe_wait(I2C_TypeDef *i2c);


/**
 * @brief I2C wait for BTF bit to set
 * 
 * @details This function is called at the end of a data transmission to a slave device. 
 *          The BTF bit indicates if the byte transfer is in progress or complete. When 
 *          all bytes have been written to the slave this function is called and it waits 
 *          for BTF to set to indicate that the last byte has been transferred at which 
 *          point the write sequence in the code ends and a stop condition can be 
 *          generated. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to I2C port 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_btf_wait(I2C_TypeDef *i2c);


/**
 * @brief Reads I2C data - called by i2c_read and i2c_clear 
 * 
 * @see i2c_read 
 * @see i2c_clear 
 * 
 * @param i2c : I2C port to use 
 * @param data : data buffer to store read data 
 * @param data_size : size of data to read 
 * @param increment : data buffer increment 
 * @return I2C_STATUS : read operation status 
 */
I2C_STATUS i2c_get(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size, 
    uint8_t increment); 

//=======================================================================================


//=======================================================================================
// Initiate I2C 

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

// I2C initialization 
void i2c_init(
    I2C_TypeDef *i2c, 
    pin_selector_t sda_pin, 
    GPIO_TypeDef *sda_gpio, 
    pin_selector_t scl_pin, 
    GPIO_TypeDef *scl_gpio, 
    i2c_run_mode_t run_mode,
    i2c_apb1_freq_t apb1_freq,
    i2c_ccr_setpoint_t ccr_reg,
    i2c_trise_setpoint_t trise_reg)
{
    // Enable the I2C clock 
    RCC->APB1ENR |= (SET_BIT << (SHIFT_21 + (uint8_t)((uint32_t)(i2c - I2C1) >> SHIFT_10))); 

    //==================================================
    // Configure pins for alternate functions 

    // SCL pin 
    gpio_pin_init(scl_gpio, scl_pin, MODER_AF, OTYPER_OD, OSPEEDR_HIGH, PUPDR_PU); 
    gpio_afr(scl_gpio, scl_pin, SET_4); 

    // SDA pin 
    gpio_pin_init(sda_gpio, sda_pin, MODER_AF, OTYPER_OD, OSPEEDR_HIGH, PUPDR_PU); 
    gpio_afr(sda_gpio, sda_pin, SET_4); 

    //==================================================

    //==================================================
    // Configure the I2C 

    // Software reset the I2C - enable then disable SWRST bit 
    i2c->CR1 |=  (SET_BIT << SHIFT_15);
    i2c->CR1 &= ~(SET_BIT << SHIFT_15);

    // Ensure PE is disabled (PE = 0) before setting up the I2C
    i2c->CR1 |= (CLEAR_BIT << SHIFT_0);

    // Set the peripheral input clock frequency 
    i2c->CR2 |= (apb1_freq << SHIFT_0);

    // Choose Sm (standard) or Fm (fast) mode 
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
            i2c->CCR |= (SET_BIT << SHIFT_14);   // Set duty cycle to 16/9 
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

    //==================================================
}

//=======================================================================================


//=======================================================================================
// I2C register functions

// I2C generate start condition 
I2C_STATUS i2c_start(I2C_TypeDef *i2c)
{
    // Local variables 
    uint16_t timeout = I2C_TIMEOUT_COUNT; 

    // Enable the acknowledgement bit and set the start generation bit 
    i2c_set_ack(i2c); 
    i2c->CR1 |= (SET_BIT << SHIFT_8); 

    // Wait for the start bit to set - abort if operation times out 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_0)) && --timeout); 

    if (timeout) 
    {
        return I2C_OK; 
    }
    
    return I2C_TIMEOUT; 
}


// I2C generate a stop condition by setting the stop generation bit 
void i2c_stop(I2C_TypeDef *i2c)
{
    i2c->CR1 |= (SET_BIT << SHIFT_9);
}


// Read the SR1 and SR2 registers to clear ADDR
void i2c_clear_addr(I2C_TypeDef *i2c)
{
    dummy_read(((i2c->SR1) | (i2c->SR2))); 
}


// I2C wait for ADDR bit to set
I2C_STATUS i2c_addr_wait(I2C_TypeDef *i2c)
{
    // Local variables 
    uint16_t timeout = I2C_TIMEOUT_COUNT; 

    // Wait for the ADDR bit to set - abort if operation times out 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_1)) && --timeout); 

    if (timeout)
    {
        return I2C_OK; 
    }

    return I2C_TIMEOUT; 
}


// I2C clear the ACK bit to send a NACK pulse to slave device 
void i2c_clear_ack(I2C_TypeDef *i2c)
{
    i2c->CR1 &= ~(SET_BIT << SHIFT_10);
}


// I2C set the ACK bit to tell the slave that data has been receieved
void i2c_set_ack(I2C_TypeDef *i2c)
{
    i2c->CR1 |= (SET_BIT << SHIFT_10);
}


// I2C wait for RxNE bit to set indicating data is ready 
I2C_STATUS i2c_rxne_wait(I2C_TypeDef *i2c)
{
    // Local variables 
    uint16_t timeout = I2C_TIMEOUT_COUNT; 

    // Wait for the RXNE bit to set - abort if operation times out 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_6)) && --timeout); 

    if (timeout)
    {
        return I2C_OK; 
    }

    return I2C_TIMEOUT; 
}


// I2C wait for TxE bit to set 
I2C_STATUS i2c_txe_wait(I2C_TypeDef *i2c)
{
    // Local variables 
    uint16_t timeout = I2C_TIMEOUT_COUNT; 

    // Wait for the TXE bit to set - abort if operation times out 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_7)) && --timeout); 

    if (timeout)
    {
        return I2C_OK; 
    }

    return I2C_TIMEOUT; 
}


// I2C wait for BTF to set
I2C_STATUS i2c_btf_wait(I2C_TypeDef *i2c)
{
    // Local variables 
    uint16_t timeout = I2C_TIMEOUT_COUNT; 

    // Wait for the BTF bit to set - abort if operation times out 
    while(!(i2c->SR1 & (SET_BIT << SHIFT_2)) && --timeout); 

    if (timeout)
    {
        return I2C_OK; 
    }

    return I2C_TIMEOUT; 
}

//=======================================================================================


//=======================================================================================
// Write I2C 

// I2C send address 
I2C_STATUS i2c_write_addr(
    I2C_TypeDef *i2c, 
    uint8_t i2c_address)
{
    if (i2c == NULL)
    {
        return I2C_NULL_PTR; 
    }
    
    I2C_STATUS i2c_status = I2C_OK; 

    i2c->DR = i2c_address;                // Send slave address 
    i2c_status |= i2c_addr_wait(i2c);     // Wait for ADDR to set 

    return i2c_status; 
}


// I2C send data to a device 
I2C_STATUS i2c_write(
    I2C_TypeDef *i2c, 
    const uint8_t *data, 
    uint8_t data_size)
{
    if ((i2c == NULL) || (data == NULL))
    {
        return I2C_NULL_PTR; 
    }

    I2C_STATUS i2c_status = I2C_OK; 

    for (uint8_t i = CLEAR; (i < data_size), (data != NULL); i++)
    {
        i2c_status |= i2c_txe_wait(i2c);    // Wait for TxE bit to set 
        i2c->DR = *data++;                  // Send data 
    }

    // Wait for BTF to set
    i2c_status |= i2c_btf_wait(i2c); 

    return i2c_status; 
}

//=======================================================================================


//=======================================================================================
// Read I2C 

// I2C read data from a device
I2C_STATUS i2c_read(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size)
{
    return i2c_get(i2c, data, data_size, I2C_BUFF_INCREMENT); 
}


// Clear I2C data 
I2C_STATUS i2c_clear(
    I2C_TypeDef *i2c, 
    uint16_t data_size)
{
    uint8_t data_buff = CLEAR; 
    return i2c_get(i2c, &data_buff, data_size, I2C_BUFF_NO_INCREMENT); 
}


// Get I2C data 
I2C_STATUS i2c_get(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size, 
    uint8_t increment)
{
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 

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
            i2c_status |= i2c_rxne_wait(i2c);

            // Read the data regsiter
            *data = i2c->DR;

            break;

        default:  // Greater than one-byte transmission 
            // Read SR1 and SR2 to clear ADDR
            i2c_clear_addr(i2c);

            // Normal reading 
            for (uint16_t i = 0; i < (data_size - BYTE_2); i++)
            {
                i2c_status |= i2c_rxne_wait(i2c);  // Indicates when data is ready 
                *data = i2c->DR; 
                data += increment; 
                i2c_set_ack(i2c); 
            }

            // Read the second last data byte 
            i2c_status |= i2c_rxne_wait(i2c);
            *data = i2c->DR;

            // Clear the ACK bit to send a NACK pulse to the slave
            i2c_clear_ack(i2c);

            // Generate stop condition
            i2c_stop(i2c);

            // Read the last data byte
            data += increment; 
            i2c_status |= i2c_rxne_wait(i2c);
            *data = i2c->DR;

            break;
    }

    return i2c_status; 
}


// I2C read data until a termination character is seen 
I2C_STATUS i2c_read_to_term(
    I2C_TypeDef *i2c, 
    uint8_t *data,  
    uint8_t term_char, 
    uint16_t bytes_remain)
{
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 

    // Read the data until the termination character is seen 
    do
    {
        i2c_status |= i2c_rxne_wait(i2c); 
        *data = i2c->DR; 
        i2c_set_ack(i2c); 
    } 
    while (*data++ != term_char); 

    // Read the remaining bytes and terminate the data 
    i2c_status |= i2c_read(i2c, data, bytes_remain); 
    data += bytes_remain; 
    *data = 0; 

    return i2c_status; 
}


// I2C read data of a certain length that is defined within the message 
I2C_STATUS i2c_read_to_len(
    I2C_TypeDef *i2c, 
    uint8_t address, 
    uint8_t *data, 
    uint8_t len_location, 
    uint8_t len_bytes, 
    uint8_t add_bytes)
{
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 
    uint16_t msg_length = 0; 

    // Read up to and including the part of the message that specifies the length 
    i2c_status |= i2c_read(i2c, data, len_location + len_bytes); 

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
    i2c_status |= i2c_write_addr(i2c, address); 
    i2c_clear_addr(i2c); 
    i2c_status |= i2c_read(i2c, data, msg_length); 

    return i2c_status; 
}

//=======================================================================================
