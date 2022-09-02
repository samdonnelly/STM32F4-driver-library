/**
 * @file m8q_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS module driver 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_DRIVER_H_
#define _M8Q_DRIVER_H_

//=======================================================================================
// Includes

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Device parameters 
#define M8Q_I2C_7_BIT_ADDR 0x42  // M8Q I2C address (default) 
#define M8Q_I2C_8_BIT_ADDR 0x84  // M8Q I2C address (default) shifted to accomodate R/W bit 

// Communication 
#define M8Q_DATA_LEN 30    // Max length of data string recieved from the module 

#define M8Q_READ_ADDR_DATA_SIZE 0xFD    // Register address to start reading data size 
#define M8Q_READ_ADDR_DATA_STREAM 0xFF  // Register address where the data stream is read 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q read and write bit offset 
 * 
 * @details 
 * 
 */
typedef enum {
    M8Q_W_OFFSET, 
    M8Q_R_OFFSET
} m8q_rw_offset_t; 

//=======================================================================================


//=======================================================================================
// Read 

/**
 * @brief Read an NMEA message from the M8Q 
 * 
 * @details Reads the number of bytes available then proceeds to read a NMEA message from 
 *          the M8Q. This utilizes the "DDC Random Read Access" method for U-blox devices. 
 *          "DDC Current Address Read Access" method is not used because the length of 
 *          NMEA messages changes. 
 *          
 *          *******
 * 
 * @param i2c : 
 * @param data : 
 */
void m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data); 

//=======================================================================================


//=======================================================================================
// Write 



//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
