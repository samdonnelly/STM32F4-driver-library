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
#include "gpio_driver.h"
#include "timers.h" 

// Standard Libraries 
#include <stdio.h> 
#include <stdlib.h>

//=======================================================================================


//=======================================================================================
// Macros 

// Device configuration 
#define M8Q_USER_CONFIG 0   // When set to non-zero then user config code becomes available 
#define M8Q_CONFIG_MSG  75  // Config message max length 

// Device parameters 
#define M8Q_I2C_8_BIT_ADDR 0x84  // M8Q I2C address (default - 0x42 << 1) 

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
#define M8Q_PUBX_ID_OFST  6      // Starting position of PUBX message ID in message string 


// NMEA messages 
#define M8Q_NMEA_RATE_ARGS     7    // Number of data fields in RATE 

#define M8Q_NMEA_CONFIG_ARGS   5    // Number of data fields in CONFIG 

#define M8Q_NMEA_POS_ARGS      19   // Number of data fields in POSITION 
#define M8Q_NMEA_POS_ID        48   // "0" == 48 --> Message ID for POSITION 

#define M8Q_NMEA_SV_ARGS       7    // Number of data fields in SVSTATUS 
#define M8Q_NMEA_SV_ID         51   // "3" == 51 --> Message ID for SVSTATUS 

#define M8Q_NMEA_TIME_ARGS     10   // Number of data fields in TIME 
#define M8Q_NMEA_TIME_ID       52   // "4" == 52 --> Message ID for TIME 

#define M8Q_NMEA_PUBX_ARG_OFST 9    // First data field offset for PUBX messages 
#define M8Q_NMEA_STRD_ARG_OFST 7    // First data field offset for standard messages 


// NMEA data fields 
#define M8Q_COO_DATA_LEN  5  // 


// UBX message format 
#define M8Q_UBX_START        181   // 181 == 0xB5 --> Start of received UBX message 
#define M8Q_UBX_SYNC1        0x42  // 0x42 == 'B' --> Start of user config UBX message 
#define M8Q_UBX_MSG_FMT_LEN  4     // Message format length: CLASS + ID + LENGTH 
#define M8Q_UBX_LENGTH_LEN   2     // LENGTH field length 
#define M8Q_UBX_HEADER_LEN   6     // Number of bytes before the payload 
#define M8Q_UBX_CS_LEN       2     // Number of bytes in a UBC message checksum 

#define M8Q_UBX_SYNC1_OFST   0   // 
#define M8Q_UBX_SYNC2_OFST   1   // 
#define M8Q_UBX_CLASS_OFST   2   // 
#define M8Q_UBX_ID_OFST      3   // 
#define M8Q_UBX_LENGTH_OFST  4   // LENGTH field offset from start of UBX message frame 

// UBX messages 
#define M8Q_UBX_ACK_CLASS 0x05   // 
#define M8Q_UBX_ACK_ID    0x01   // 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q valid read indicator 
 * 
 * @details Used to define a valid or invalid message read in the m8q_read function. The enum 
 *          returns the result indicating the type of message read, if any. 
 * 
 * @see m8q_read
 * 
 */
typedef enum {
    M8Q_READ_INVALID, 
    M8Q_READ_NMEA, 
    M8Q_READ_UBX
} m8q_read_status_t; 


/**
 * @brief M8Q UBX message conversion status 
 * 
 * @details Indicates whether a UBX message string was successfully converted into a format 
 *          readable by the receiver. Message strings come from the receiver config file or 
 *          user input during user config operation. 
 * 
 */
typedef enum {
    M8Q_UBX_MSG_CONV_FAIL, 
    M8Q_UBX_MSG_CONV_SUCC
} m8q_ubx_msg_convert_status_t; 


/**
 * @brief M8Q NMEA POSITION message data fields 
 * 
 * @details 
 * 
 */
typedef enum {
    M8Q_POS_TIME, 
    M8Q_POS_LAT, 
    M8Q_POS_NS, 
    M8Q_POS_LON, 
    M8Q_POS_EW, 
    M8Q_POS_ALTREF, 
    M8Q_POS_NAVSTAT, 
    M8Q_POS_HACC, 
    M8Q_POS_VACC, 
    M8Q_POS_SOG, 
    M8Q_POS_COG, 
    M8Q_POS_VVEL, 
    M8Q_POS_DIFFAGE, 
    M8Q_POS_HDOP, 
    M8Q_POS_VDOP, 
    M8Q_POS_TDOP, 
    M8Q_POS_NUMSVS, 
    M8Q_POS_RES, 
    M8Q_POS_DR 
} m8q_pos_fields_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    M8Q_TIME_TIME, 
    M8Q_TIME_DATE, 
    M8Q_TIME_UTCTOW, 
    M8Q_TIME_UTCWK, 
    M8Q_TIME_LEAPSEC, 
    M8Q_TIME_CLKBIAS, 
    M8Q_TIME_CLKDRIFT, 
    M8Q_TIME_TPGRAN 
} m8q_time_fields_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef m8q_read_status_t M8Q_READ_STAT; 
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
 * @param config_msgs 
 */
void m8q_init(
    I2C_TypeDef *i2c, 
    uint8_t msg_num, 
    uint8_t msg_index, 
    uint8_t *config_msgs); 

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
 * @return M8Q_READ_STAT : valid read indicator 
 */
M8Q_READ_STAT m8q_read(
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
// Getters 

/**
 * @brief 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t m8q_get_tx_ready(void); 


/**
 * @brief M8Q latitude getter 
 * 
 * @details 
 * 
 * @param deg_min 
 * @param min_frac 
 */
void m8q_get_lat(uint16_t *deg_min, uint32_t *min_frac); 


/**
 * @brief M8Q North/South getter 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t m8q_get_NS(void); 


/**
 * @brief M8Q longitude getter 
 * 
 * @details 
 * 
 * @param deg_min 
 * @param min_frac 
 */
void m8q_get_long(uint16_t *deg_min, uint32_t *min_frac); 


/**
 * @brief M8Q East/West getter 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t m8q_get_EW(void); 


/**
 * @brief M8Q navigation status getter 
 * 
 * @details 
 * 
 * @return uint16_t 
 */
uint16_t m8q_get_navstat(void); 


/**
 * @brief M8Q time getter 
 * 
 * @details 
 * 
 * @param time 
 */
void m8q_get_time(uint8_t *utc_time); 


/**
 * @brief M8Q date getter 
 * 
 * @details 
 * 
 * @param date 
 */
void m8q_get_date(uint8_t *utc_date); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param pin_state 
 */
void m8q_set_low_power(gpio_pin_state_t pin_state); 

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
