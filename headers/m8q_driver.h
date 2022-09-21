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

// Device configuration 
#define M8Q_USER_CONFIG 1   // When set to non-zero then user config code becomes available 
#define M8Q_CONFIG_MSG  75  // Config message max length 

// Device parameters 
#define M8Q_I2C_7_BIT_ADDR 0x42  // M8Q I2C address (default) 
#define M8Q_I2C_8_BIT_ADDR 0x84  // M8Q I2C address (default) shifted to accomodate R/W bit 

// M8Q registers 
#define M8Q_REG_0XFD 0xFD    // Available bytes (high byte) register 
#define M8Q_REG_0XFE 0xFE    // Available bytes (low byte) register 
#define M8Q_REG_0XFF 0xFF    // Data stream register 

// M8Q messages 
#define M8Q_NO_DATA  0xff   // NMEA invalid data stream return value 

// NMEA message format 
#define M8Q_NMEA_START    0x24   // 0x24 == '$' --> start of NMEA message 
#define M8Q_NMEA_END_PAY  0x2A   // 0x2A == '*' --> indicates end of NMEA message payload 
#define M8Q_NMEA_END_MSG  6      // Length of string to append to NMEA message after payload 
#define M8Q_NMEA_CS_LEN   2      // Number of characters in NMEA message checksum 

// NMEA messages 
#define M8Q_NMEA_RATE          40   // Message ID for NMEA RATE 
#define M8Q_NMEA_RATE_ARGS     7    // Number of arguments in RATE message 
#define M8Q_NMEA_CONFIG        41   // Message ID for NMEA CONFIG 
#define M8Q_NMEA_CONFIG_ARGS   5    // Number of arguments in CONFIG message 

// UBX message format 
#define M8Q_UBX_SYNC1 0x42      // 0x42 == 'B' --> Start of UBX message (0xB562) 
#define M8Q_UBX_PAY_CHAR_LEN 4  // Terminal input characters to specify payload length 
#define M8Q_UBX_CS_CALC_LEN  4  // Length to add to the payload length to calculate checksum 
#define M8Q_UBX_HEADER_LEN   6  // Number of bytes before the payload 
#define M8Q_UBX_CS_LEN       2  // Number of bytes in a UBC message checksum 

// UBX messages 
#define M8Q_UBX_CFG_CFG_LEN  12  // CFG message payload byte length 
#define M8Q_UBX_CFG_DAT_LEN  44  // DAT message payload byte length 
#define M8Q_UBX_CFG_HNR_LEN  4   // HNR message payload byte length 
#define M8Q_UBX_CFG_MSG_LEN  2   // MSG message payload byte length 
#define M8Q_UBX_CFG_NAV5_LEN 36  // NAV5 message payload byte length 
#define M8Q_UBX_CFG_NMEA_LEN 4   // NMEA message payload byte length 
#define M8Q_UBX_CFG_ODO_LEN  20  // ODO message payload byte length 
#define M8Q_UBX_CFG_PM2_LEN  44  // PM2 message payload byte length 
#define M8Q_UBX_CFG_PMS_LEN  8   // PMS message payload byte length 
#define M8Q_UBX_CFG_PRT_LEN  1   // PRT message payload byte length 
#define M8Q_UBX_CFG_RATE_LEN 6   // RATE message payload byte length 
#define M8Q_UBX_CFG_RST_LEN  4   // RST message payload byte length 
#define M8Q_UBX_CFG_RXM_LEN  2   // RXM message payload byte length 

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


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    M8Q_UBX_MSG_CONV_FAIL, 
    M8Q_UBX_MSG_CONV_SUCC
} m8q_ubx_msg_convert_status_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef m8q_nmea_read_status_t NMEA_VALID; 
typedef uint16_t CHECKSUM; 
typedef m8q_ubx_msg_convert_status_t UBX_MSG_STATUS; 

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
// Read functions 

/**
 * @brief Read a message from the M8Q 
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
NMEA_VALID m8q_read(
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
void m8q_check_data_size(
    I2C_TypeDef *i2c, 
    uint16_t *data_size); 


/**
 * @brief Read the current value at the data stream register 
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
void m8q_check_data_stream(
    I2C_TypeDef *i2c, 
    uint8_t *data_check); 

//=======================================================================================


//=======================================================================================
// Write functions 

/**
 * @brief M8Q write 
 * 
 * @details Writes an NMEA message to the receiver using a specified I2C interface 
 * 
 * @param i2c : pointer to I2C port used 
 * @param data : pointer to data buffer that contains the NMEA message 
 */
void m8q_write(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t data_size); 

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


//=======================================================================================
// Getters 
//=======================================================================================


//=======================================================================================
// User Configuration 

/**
 * @brief M8Q user configuration 
 * 
 * @details This function allows the user to change the settings of the receiver. This is 
 *          done by taking user inputs from the serial terminal that specify the desired 
 *          settings. This input is redirected to the receiver in the form of an NMEA or 
 *          UBX message depending on what is input. Other message types are not supported. 
 * 
 * @param i2c : pointer to the I2C port used 
 */
void m8q_user_config(
    I2C_TypeDef *i2c); 


/**
 * @brief M8Q NMEA config user interface 
 * 
 * @details Prints a prompt to the serial terminal to guide the user during configuration 
 *          of the receiver. 
 * 
 */
void m8q_nmea_config_ui(void); 

//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
