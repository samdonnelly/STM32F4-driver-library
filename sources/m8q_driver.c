/**
 * @file m8q_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS driver 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

// Device drivers 
#include "m8q_driver.h"

// Libraries 

//=======================================================================================


//=======================================================================================
// Read 

// Read an NMEA message from the M8Q 
void m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{
    // Local variables
    uint16_t data_size = 0; 
    uint8_t reg_0xFD = 0; 
    uint8_t reg_0xFE = 0; 
    uint8_t reg_0xFF = 0; 
    uint8_t address = M8Q_READ_ADDR_DATA_SIZE;  

    // Generate a start condition 
    i2c_start(i2c); 

    // Write the slave address with write access 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_W_OFFSET); 
    i2c_clear_addr(i2c); 

    // Send the register address to start reading at 
    i2c_write_master_mode(i2c, &address, I2C_1_BYTE); 

    // Generate another start condition 
    i2c_start(i2c); 

    // Send the device address again with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET);  
    i2c_clear_addr(i2c); 

    //===================================================
    // Read data 

    // Read the data size (high byte - 0xFD) 
    i2c_rxne_wait(i2c); 
    reg_0xFD = i2c->DR; 
    i2c_set_ack(i2c); 

    // Read the data size (low byte - 0xFE) 
    i2c_rxne_wait(i2c);  
    reg_0xFE = i2c->DR; 
    i2c_clear_ack(i2c);

    // Generate stop condition
    i2c_stop(i2c);

    // Read the data stream (0xFF) 
    i2c_rxne_wait(i2c);
    reg_0xFF = i2c->DR;

    //===================================================

    //===================================================
    // Display the results 

    uart_sendstring(USART2, "Register 0xFD: "); 
    uart_send_integer(USART2, (int16_t)reg_0xFD); 
    uart_sendstring(USART2, "    "); 

    uart_sendstring(USART2, "Register 0xFE: "); 
    uart_send_integer(USART2, (int16_t)reg_0xFE); 
    uart_sendstring(USART2, "    "); 

    data_size = (uint16_t)((reg_0xFD << SHIFT_8) | reg_0xFE); 
    uart_sendstring(USART2, "Data size: "); 
    uart_send_integer(USART2, (int16_t)data_size); 
    uart_sendstring(USART2, "    "); 

    uart_sendstring(USART2, "Register 0xFF: "); 
    uart_sendchar(USART2, reg_0xFF); 
    uart_sendstring(USART2, "  \r"); 

    //===================================================

    // uart_send_new_line(USART2); 
}

//=======================================================================================
