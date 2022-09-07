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

#include "uart_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Device parameters 
#define M8Q_I2C_7_BIT_ADDR 0x42  // M8Q I2C address (default) 
#define M8Q_I2C_8_BIT_ADDR 0x84  // M8Q I2C address (default) shifted to accomodate R/W bit 

// Communication 
#define M8Q_DATA_LEN 30    // Max length of data string recieved from the module 

#define M8Q_READ_DS_ADDR 0xFD    // Register address to start reading data size 

#define M8Q_INVALID_NMEA 0xff   // NMEA invalid data stream return value 
#define M8Q_VALID_NMEA   0x24   // 0x24 == '$' --> start of valid NMEA message 

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


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    M8Q_NMEA_READ_INVALID, 
    M8Q_NMEA_READ_VALID
} m8q_nmea_read_status_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef m8q_nmea_read_status_t NMEA_VALID; 

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
 * @param i2c : pointer to the I2C port used 
 * @param data : pointer to array that will store a single NMEA message 
 * @return NMEA_VALID : valid read indicator 
 */
NMEA_VALID m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data); 


/**
 * @brief Read the NMEA data stream size 
 * 
 * @details Read registers 0xFD and 0xFE to get the size of the NMEA data stream. If this 
 *          value is not zero then there is data available to be read. This function can be 
 *          used as an indication that data is available to be read. 
 *          
 *          This function uses a "DDC Random Read Access" method to specify the data size 
 *          registers. 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param data_size : pointer to store the size of the NMEA data stream 
 */
void m8q_read_nmea_ds(
    I2C_TypeDef *i2c, 
    uint16_t *data_size); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param i2c 
 * @param data_check 
 */
void m8q_check_nmea_stream(
    I2C_TypeDef *i2c, 
    uint8_t *data_check); 

//=======================================================================================


//=======================================================================================
// Write 



//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
