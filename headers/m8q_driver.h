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

// M8Q registers 
#define M8Q_READ_DS_ADDR 0xFD    // Register address to start reading data size 

// NMEA message format 
#define M8Q_INVALID_NMEA 0xff   // NMEA invalid data stream return value 
#define M8Q_VALID_NMEA   0x24   // 0x24 == '$' --> start of valid NMEA message 
#define M8Q_END_NMEA     0x2A   // 0x2A == '*' --> indicates end of NMEA payload 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q read and write bit offset 
 * 
 * @details Used in conjunction with the module I2C address to indicate a read or write 
 *          operation when communicating over I2C. 
 * 
 */
typedef enum {
    M8Q_W_OFFSET, 
    M8Q_R_OFFSET
} m8q_rw_offset_t; 


/**
 * @brief M8Q valid read indicator 
 * 
 * @details Used to define a valid or invalid read of NMEA message data in the m8q_read_nmea
 *          function. 
 * 
 * @see m8q_read_nmea
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
// Initialization 

/**
 * @brief M8Q initialization 
 * 
 * @details To be implemented. 
 * 
 */
void m8q_init(void); 

//=======================================================================================


//=======================================================================================
// Message identification 
//=======================================================================================


//=======================================================================================
// NMEA Read 

/**
 * @brief Read an NMEA message from the M8Q 
 * 
 * @details Checks for a valid data stream using m8q_check_nmea_stream, and if valid then 
 *          proceeds to read a single NMEA message. The function returns an indication 
 *          of whether the read was valid or not. If there was no data (or unknown data) the 
 *          function return will indicate an invalid read, and if there is data then it will 
 *          indicate a valid read and the data buffer passed to the function will be filled. 
 *          This function has to be called for each NMEA message available. 
 * 
 * @see m8q_check_nmea_stream
 * @see m8q_nmea_read_status_t
 * 
 * @param i2c : pointer to the I2C port used 
 * @param data : pointer to array that will store a single NMEA message 
 * @return NMEA_VALID : valid read indicator 
 */
NMEA_VALID m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data); 


/**
 * @brief Read the size of the available NMEA data 
 * 
 * @details Reads registers 0xFD and 0xFE to get the number of available NMEA message bytes. 
 *          If this value is zero then there is no available data to be read. A non-zero value 
 *          will indicate the total NMEA message bytes available, however it does not indicate 
 *          the number of messages contained within the data size. This function can be 
 *          used as an indication that data is available to be read. 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param data_size : pointer to single-integer buffer to store the NMEA data stream size 
 */
void m8q_read_nmea_ds(
    I2C_TypeDef *i2c, 
    uint16_t *data_size); 


/**
 * @brief Check the NMEA data stream 
 * 
 * @details Reads the data stream register (0xFF) and stores the result in the argument 
 *          data_check. This function can be used to check for a valid data stream. If the 
 *          returned result if 0xff then there is no data to be read and the steam is not 
 *          valid. If '$' (36d) is returned then there a valid data steam (NMEA message) 
 *          waiting to be read. 
 * 
 * @param i2c : pointer to I2C port used 
 * @param data_check : pointer to single-integer buffer to store register 0xFF value 
 */
void m8q_check_nmea_stream(
    I2C_TypeDef *i2c, 
    uint8_t *data_check); 

//=======================================================================================


//=======================================================================================
// NMEA Write 
//=======================================================================================


//=======================================================================================
// PUBX read 
//=======================================================================================


//=======================================================================================
// PUBX write 
//=======================================================================================


//=======================================================================================
// Message processing 

// How to parse received NMEA messages 

// How to format outgoing NMEA messages 

// How to take user input for configuration NMEA messages 

// How to parse received UBX messages 

// How to format outgoing UBX messages 

// How to take user input to specify a UBX message 

//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
