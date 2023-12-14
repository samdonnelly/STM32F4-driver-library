/**
 * @file i2c_comm.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C communication driver interface - mock 
 * 
 * @version 0.1
 * @date 2023-13-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _I2C_COMM_MOCK_H_
#define _I2C_COMM_MOCK_H_

//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "gpio_driver.h" 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief I2C read and write bit offset 
 * 
 * @details Used in conjunction with an I2C module address to indicate a read or write 
 *          operation at bit 0. 
 */
typedef enum {
    I2C_W_OFFSET, 
    I2C_R_OFFSET
} i2c_rw_offset_t; 


/**
 * @brief I2C operation status 
 */
typedef enum {
    I2C_OK, 
    I2C_TIMEOUT 
} i2c_status_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef i2c_status_t I2C_STATUS; 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief I2C start condition generation 
 * 
 * @details This must be called by the master to begin an I2C read or write 
 *          transmisssion. The controller is in slave mode when idle but becomes 
 *          the master when the start condition is generated. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to I2C port 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_start(
    I2C_TypeDef *i2c);


/**
 * @brief I2C stop condition condition 
 * 
 * @details This must be called in order to end an I2C read or write transmission. 
 *          Once the stop condition is sent then the controller releases the bus and 
 *          reverts back to slave mode. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_stop(
    I2C_TypeDef *i2c);


/**
 * @brief I2C clear ADDR bit
 * 
 * @details This bit must be cleared before data can start to be sent on the bus. The 
 *          function gets called after the slave has acknowledged the address sent by 
 *          the master. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_clear_addr(
    I2C_TypeDef *i2c);


/**
 * @brief I2C write address 
 * 
 * @details Sends the slave address to the bus to initialize communication with a certain 
 *          device. This function is called after the start condition has been generated. 
 *          Only 7-bit addresses are supported. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port 
 * @param i2c_address : 7-bit address of slave device on the bus 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_write_addr(
    I2C_TypeDef *i2c, 
    uint8_t i2c_address);


/**
 * @brief I2C write data
 * 
 * @details Write data to a slave device. This function is called after the ADDR bit has 
 *          been cleared. The function takes a pointer to the data that will be send over 
 *          the bus and the size of the data so it knows how many bytes to send. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port 
 * @param data : pointer to data to be sent over the bus 
 * @param data_size : integer indicating the number of bytes to be sent 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_write(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t data_size);


/**
 * @brief I2C read data
 * 
 * @details Read data from a slave device. This function is called after the ADDR bit has 
 *          been cleared. The function takes a pointer where it will store the recieved 
 *          data and the size of the data so it knows how many bytes to read. 
 * 
 * @see i2c_status_t
 *          
 * @param i2c : pointer to the I2C port 
 * @param data : pointer that data is placed into 
 * @param data_size : integer indicating the number of bytes to be receieved 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_read(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size);


/**
 * @brief I2C read data until a termination character is seen 
 * 
 * @details This function is for reading data of unknown or varying length. It reads 
 *          data in the same way as i2c_read but takes a termination character
 *          and the number of bytes left to read after the termination character as 
 *          arguments. Once the termination character is seen then the transaction will 
 *          be stopped based on how many bytes remain to be read. There must be at least one 
 *          byte left to read. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port used 
 * @param data : pointer to the buffer that stores read data 
 * @param term_char : termination character to indicate when to stop reading 
 * @param bytes_remain : number of bytes left to read after the termination character 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_read_to_term(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t term_char, 
    uint16_t bytes_remain);


/**
 * @brief I2C read data of a certain length that is defined within the message 
 * 
 * @details This function was created for the M8Q driver. The M8Q message protocol contains 
 *          the message length in one of it's message formats (UBX). There are different 
 *          messages of different lengths within this format and there is no easy indicator 
 *          for the end of the data portion of the message. This function reads the portion 
 *          of the message that specifies the length so the code knows how many bytes to read 
 *          from the device. 
 *          
 *          NOTE: Reading the length in the message is currently only supported 
 *                for little endian format. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port used 
 * @param address : I2C address of the device 
 * @param data : pointer to buffer that stores the read data  
 * @param len_location : where the message length is located in the message (bytes) 
 * @param len_bytes : number of bytes used to define the length within the message 
 * @param add_bytes : additional bytes to read at the end of the message (if needed) 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_read_to_len(
    I2C_TypeDef *i2c, 
    uint8_t address, 
    uint8_t *data, 
    uint8_t len_location, 
    uint8_t len_bytes, 
    uint8_t add_bytes); 

//=======================================================================================


//=======================================================================================
// Mocks 

// Mock initialization 
void i2c_mock_init(
    uint8_t timeout_status); 


// Get write data 
void i2c_mock_get_write_data(
    char *data_buff, 
    uint8_t *data_size); 

//=======================================================================================

#endif  // _I2C_COMM_MOCK_H_ 
