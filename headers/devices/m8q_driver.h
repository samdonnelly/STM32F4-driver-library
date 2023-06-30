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
#include "gpio_driver.h"
#include "uart_comm.h"

// Standard Libraries 
#include <stdio.h> 

//=======================================================================================


//=======================================================================================
// Macros 

// Device configuration 
#define M8Q_USER_CONFIG 0                // Sets the code to user config mode 

// Device parameters 
#define M8Q_I2C_8_BIT_ADDR 0x84          // Receiver I2C address (default : 0x42 << 1) 

// M8Q registers 
#define M8Q_REG_0XFD 0xFD                // Available data bytes (high byte) register 
#define M8Q_REG_0XFE 0xFE                // Available data bytes (low byte) register 
#define M8Q_REG_0XFF 0xFF                // Data stream register 

// M8Q messages 
#define M8Q_NO_DATA     0xff             // Return value for an invalid NMEA data stream 
#define M8Q_MSG_MAX_LEN 150              // Message buffer that can hold any message
#define M8Q_PYL_MAX_LEN 100              // Message payload buffer to store any apyload length 

// NMEA message format 
#define M8Q_NMEA_MSG_MAX_LEN 150         // NMEA message buffer that can hold any received message
#define M8Q_NMEA_START       0x24        // 0x24 == '$' --> start of NMEA message 
#define M8Q_NMEA_END_PAY     0x2A        // 0x2A == '*' --> indicates end of NMEA message payload 
#define M8Q_NMEA_END_MSG     6           // Length of string to append to NMEA message after payload 
#define M8Q_NMEA_CS_LEN      2           // Number of characters in NMEA message checksum 
#define M8Q_PUBX_ID_OFST     6           // Starting position of PUBX message ID in message string 

// NMEA messages 
#define M8Q_NMEA_RATE_ARGS     7         // Number of data fields in RATE 
#define M8Q_NMEA_CONFIG_ARGS   5         // Number of data fields in CONFIG 
#define M8Q_NMEA_POS_ARGS      19        // Number of data fields in POSITION 
#define M8Q_NMEA_POS_ID        48        // "0" == 48 --> Message ID for POSITION 
#define M8Q_NMEA_SV_ARGS       7         // Number of data fields in SVSTATUS 
#define M8Q_NMEA_SV_ID         51        // "3" == 51 --> Message ID for SVSTATUS 
#define M8Q_NMEA_TIME_ARGS     10        // Number of data fields in TIME 
#define M8Q_NMEA_TIME_ID       52        // "4" == 52 --> Message ID for TIME 
#define M8Q_NMEA_PUBX_ARG_OFST 9         // First data field offset for PUBX messages 
#define M8Q_NMEA_STRD_ARG_OFST 7         // First data field offset for standard messages 

// NMEA data fields 
#define M8Q_COO_LEN 6                    // Length of integer and fractional parts of coordinates 
#define M8Q_LAT_LEN 10                   // Latitude message length 
#define M8Q_LON_LEN 11                   // Longitude message length 

// UBX message format 
#define M8Q_UBX_START       0xB5         // 0xB5 --> Start of received UBX message 
#define M8Q_UBX_SYNC1       0x42         // 0x42 == 'B' --> Start of user config UBX message 
#define M8Q_UBX_MSG_FMT_LEN 4            // Message format length: CLASS + ID + LENGTH 
#define M8Q_UBX_LENGTH_LEN  2            // LENGTH field length 
#define M8Q_UBX_HEADER_LEN  6            // Number of bytes before the payload 
#define M8Q_UBX_CS_LEN      2            // Number of bytes in a UBC message checksum 

#define M8Q_UBX_SYNC1_OFST  0            // First sync character offset 
#define M8Q_UBX_SYNC2_OFST  1            // Second sync character offset 
#define M8Q_UBX_CLASS_OFST  2            // Class character offset 
#define M8Q_UBX_ID_OFST     3            // Message ID character offset 
#define M8Q_UBX_LENGTH_OFST 4            // LENGTH field offset from start of UBX message frame 

// UBX messages 
#define M8Q_UBX_ACK_CLASS 0x05           // Class of ACK message 
#define M8Q_UBX_ACK_ID    0x01           // ID of ACK message 
#define M8Q_TIME_CHAR_LEN 9              // Number of characters for the time in NMEA messages 
#define M8Q_DATE_CHAR_LEN 6              // Number of characters for the date in NMEA messages 

// NAVSTAT states - read as a two byte character string from the device 
#define M8Q_NAVSTAT_NF 0x4E46            // No Fix 
#define M8Q_NAVSTAT_DR 0x4452            // Dead reckoning only solution 
#define M8Q_NAVSTAT_G2 0x4732            // Stand alone 2D solution 
#define M8Q_NAVSTAT_G3 0x4733            // Stand alone 3D solution 
#define M8Q_NAVSTAT_D2 0x4432            // Differential 2D solution 
#define M8Q_NAVSTAT_D3 0x4433            // Differential 3D solution 
#define M8Q_NAVSTAT_RK 0x524B            // Combined GPS and DR solution 
#define M8Q_NAVSTAT_TT 0x5454            // Time only solution 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q valid read indicator 
 * 
 * @details Used to define a valid or invalid message read in the m8q_read function. m8q_read 
 *          returns the result indicating the type of message read, if any. 
 * 
 * @see m8q_read
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
 *          user input during user config mode. 
 */
typedef enum {
    M8Q_UBX_MSG_CONV_FAIL, 
    M8Q_UBX_MSG_CONV_SUCC
} m8q_ubx_msg_convert_status_t; 


/**
 * @brief M8Q NMEA POSITION message data fields 
 * 
 * @details List of all data fields in the POSITION message. This enum allows for indexing 
 *          of the POSITION fields for retreival data in getters. 
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
 * @brief M8Q NMEA TIME message data fields 
 * 
 * @details List of all data fields in the TIME message. This enum allows for indexing 
 *          of the TIME fields for retreival data in getters. 
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


/**
 * @brief M8Q driver status codes 
 * 
 * @details 
 *          
 *          Old comments: 
 *          - Codes used to indicate errors during NMEA message processing. These codes help 
 *            with debugging. 
 *          - Codes used to indicate errors during UBX message processing. These codes help 
 *            with debugging. 
 */
typedef enum {
    M8Q_FAULT_NONE,           // No fault 
    M8Q_FAULT_NO_DATA,        // No data available 
    M8Q_FAULT_NMEA_ID,        // Unsupported PUBX message ID 
    M8Q_FAULT_NMEA_FORM,      // Invalid formatting of PUBX message 
    M8Q_FAULT_NMEA_INVALID,   // Only PUBX messages are supported 
    M8Q_FAULT_UBX_SIZE,       // Payload length doesn't match size 
    M8Q_FAULT_UBX_FORM,       // Invalid payload format 
    M8Q_FAULT_UBX_LEN,        // Invalid payload length format 
    M8Q_FAULT_UBX_CONVERT,    // Message conversion failed. Check format 
    M8Q_FAULT_UBX_ID,         // Invalid ID format 
    M8Q_FAULT_UBX_NA,         // Unknown message type 
    M8Q_FAULT_UBX_NAK,        // Message not acknowledged 
    M8Q_FAULT_UBX_RESP        // Response message sent - only used during user config mode 
} m8q_status_codes_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint16_t CHECKSUM; 

// Status 
typedef m8q_read_status_t M8Q_READ_STAT; 
typedef m8q_ubx_msg_convert_status_t UBX_MSG_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief M8Q initialization 
 * 
 * @details Initializes the receiver configuration and it's peripherals. The communication 
 *          ports and peripheral pins passed as arguments are saved in the receivers data 
 *          record for use throughout the driver. GPIO pins are initialized for power save 
 *          mode and TX-ready operations. All the messages sepecified in the programs 
 *          m8q_config file are sent to the receiver. If there are any errors sending config 
 *          messages the return value of the function will indicate the message that caused 
 *          an error and which error occured. M8Q_MSG_ERROR_CODE is a 16-bit value with the 
 *          following breakdown: <br> 
 *            - High byte: NMEA or UBX error code <br> 
 *            - Low byte: Message index based on m8q_config file <br> 
 * 
 * @see m8q_nmea_error_code_t
 * @see m8q_ubx_error_code_t
 * 
 * @param i2c : pointer to I2C port used for receiver communication 
 * @param gpio : pointer to GPIO port used for receiver peripherals 
 * @param pwr_save_pin : pin used for low power mode trigger (peripheral) 
 * @param tx_ready_pin : pin used for tx_ready indication (peripheral) 
 * @param msg_num : number of configuration messages to send 
 * @param msg_max_size : maximum config message size (see config file) 
 * @param config_msgs : pointer to buffer that storages the config messages 
 */
void m8q_init(
    I2C_TypeDef *i2c, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pwr_save_pin, 
    pin_selector_t tx_ready_pin, 
    uint8_t msg_num, 
    uint8_t msg_max_size, 
    uint8_t *config_msgs); 

//=======================================================================================


//=======================================================================================
// Read and write functions 

/**
 * @brief Read a message from the M8Q 
 * 
 * @details Checks for a valid data stream and if true then proceeds to read the next 
 *          available message from the receiver. The function returns an indication 
 *          of whether the read was valid or not. If there was no data (or unknown data) the 
 *          function return will indicate an invalid read. If the data is valid and the message 
 *          read then the message will be saved and used where needed, such as for getters 
 *          or print outs in user config mode. 
 * 
 * @see m8q_check_nmea_stream
 * @see m8q_nmea_read_status_t
 * 
 * @return M8Q_READ_STAT : valid read indicator 
 */
M8Q_READ_STAT m8q_read(void); 


/**
 * @brief Read the number of available bytes from the M8Q 
 * 
 * @details Reads registers 0xFD and 0xFE to get the number of available NMEA message bytes. 
 *          If this value is zero then there is no available data to be read. A non-zero value 
 *          will indicate the total message bytes available, however it does not indicate 
 *          the number of messages contained within the data size. This function can be 
 *          used as an indication that data is available to be read. 
 * 
 * @param data_size : pointer to single-integer buffer to store the NMEA data stream size 
 */
void m8q_check_data_size(
    uint16_t *data_size); 


/**
 * @brief Read the current value at the data stream register 
 * 
 * @details Reads the data stream register (0xFF) and stores the result in the argument 
 *          data_check. This function can be used to check for a valid data stream. If the 
 *          returned result is 0xff then there is no data to be read and the steam is not 
 *          valid. 
 * 
 * @param data_check : pointer to one-byte buffer to store the data stream value 
 */
void m8q_check_data_stream(
    uint8_t *data_check); 


/**
 * @brief M8Q write 
 * 
 * @details Sends messages to the receiever. This function is primarily used for setting the 
 *          receivers configuration. 
 * 
 * @param data : pointer to data buffer that contains the message 
 * @param data_size : length of message being sent to the receiver 
 */
void m8q_write(
    uint8_t *data, 
    uint8_t data_size); 

//=======================================================================================


//=======================================================================================
// Setters and Getters 

/**
 * @brief M8Q clear device driver fault flag 
 * 
 * @details 
 */
void m8q_clear_status(void); 


/**
 * @brief M8Q get device driver fault code 
 * 
 * @details 
 *          
 *          Status info / fault code: 
 *            --> bit 0: i2c status (see i2c_status_t) 
 *            --> bit 1-12: driver faults (see status getter) 
 *            --> bits 13-15: not used 
 * 
 * @return uint16_t : driver status code for a given device number 
 */
uint16_t m8q_get_status(void); 


/**
 * @brief Power save mode setter 
 * 
 * @details Sets the output of the power save mode pin. If set to high then the receiver will 
 *          enter power save mode where it will not report position information. If set low 
 *          then the receiver will operate as normal. Note that communication with the receiver 
 *          can't be achieved while in power save mode. The pin used for power save mode setting 
 *          is initialized in the m8q_init function. 
 * 
 * @param pin_state : desired output state of the power save mode pin 
 */
void m8q_set_low_power(
    gpio_pin_state_t pin_state); 


/**
 * @brief TX-ready getter 
 * 
 * @details Returns the status of the TX-ready pin which indicates when data is available to 
 *          be read. If the signal is high then data is available. The TX-ready pin is 
 *          initialized in the m8q_init function. 
 * 
 * @return uint8_t : TX-ready pin status 
 */
uint8_t m8q_get_tx_ready(void); 


/**
 * @brief Latitude getter 
 * 
 * @details Reads the current latitude of the receiver. The integer portion of the latitude 
 *          (degrees and minutes) is stored in deg_min and the decimal/fractional part (minutes) 
 *          is stored in min_frac. The latitude is broken up into these two parts because the 
 *          full latitude value is not always needed so being able to use just the fractional 
 *          portion becomes easier. For more information on latitude formatting see the 
 *          M8Q protocol specification documentation. 
 * 
 * @param deg_min : degrees and minutes of the latitude 
 * @param min_frac : fractional portion of the minutes in the latitude 
 */
void m8q_get_lat(
    uint16_t *deg_min, 
    uint32_t *min_frac); 


/**
 * @brief Latitude getter (string format) 
 * 
 * @details 
 * 
 * @param deg_min 
 * @param min_frac 
 */
void m8q_get_lat_str(
    uint8_t *deg_min, 
    uint8_t *min_frac); 


/**
 * @brief North/South getter 
 * 
 * @details Returns the latitude North/South indicator. Note that this return value is in ASCII 
 *          character form. For example, a return value of 78 corresponds to "N" for North. 
 * 
 * @return uint8_t : North/South indicator 
 */
uint8_t m8q_get_NS(void); 


/**
 * @brief Longitude getter 
 * 
 * @details Reads the current longitude of the receiver. The integer portion of the longitude 
 *          (degrees and minutes) is stored in deg_min and the decimal/fractional part (minutes) 
 *          is stored in min_frac. The longitude is broken up into these two parts because the 
 *          full longitude value is not always needed so being able to use just the fractional 
 *          portion becomes easier. For more information on longitude formatting see the 
 *          M8Q protocol specification documentation. 
 * 
 * @param deg_min : degrees and minutes of the longitude 
 * @param min_frac : fractional portion of the minutes in the longitude 
 */
void m8q_get_long(
    uint16_t *deg_min, 
    uint32_t *min_frac); 


/**
 * @brief Longitude getter (string format) 
 * 
 * @details 
 * 
 * @param deg_min 
 * @param min_frac 
 */
void m8q_get_long_str(
    uint8_t *deg_min, 
    uint8_t *min_frac); 


/**
 * @brief East/West getter 
 * 
 * @details Returns the longitude East/West indicator. Note that this return value is in ASCII 
 *          character form. For example, a return value of 69 corresponds to "E" for East. 
 * 
 * @return uint8_t : East/West indicator 
 */
uint8_t m8q_get_EW(void); 


/**
 * @brief Navigation status getter 
 * 
 * @details Returns the navigation status of the receiver. Note that this return value is in 
 *          ASCII character form and consists of two bytes. The 8 most significant bits of the 
 *          16-bit return value holds the first character and the 8 least significant bits holds 
 *          the second character. For example, a return value of 20038 corresponds to "NF" which 
 *          stands for "No Fix". A list of all available statuses are listed below: 
 *            - NF = No Fix (0x4E46) 
 *            - DR = Dead reckoning only solution (0x4452) 
 *            - G2 = Stand alone 2D solution (0x4732) 
 *            - G3 = Stand alone 3D solution (0x4733) 
 *            - D2 = Differential 2D solution (0x4432) 
 *            - D3 = Differential 3D solution (0x4433) 
 *            - RK = Combined GPS + dead reckoning solution (0x524B) 
 *            - TT = Time only solution (0x5454) 
 * 
 * @return uint16_t : Navigation status of the receiver 
 */
uint16_t m8q_get_navstat(void); 


/**
 * @brief Time getter 
 * 
 * @details Gets the current UTC time. Note that the time is returned as a character string 
 *          in the format "hhmmss.ss". 
 * 
 * @param utc_time : pointer to buffer to store the UTC time 
 */
void m8q_get_time(
    uint8_t *utc_time); 


/**
 * @brief Date getter 
 * 
 * @details Gets the current UTC date. Note that the date is returned as a character string 
 *          in the format "ddmmyy". 
 * 
 * @param utc_date : pointer to buffer to store the UTC date 
 */
void m8q_get_date(
    uint8_t *utc_date); 

//=======================================================================================


//=======================================================================================
// User Configuration 

/**
 * @brief User configuration initialization 
 * 
 * @details Initializes user config mode. This function is called once during the setup/init 
 *          procedure. Note that this function is only valid during user config mode which can 
 *          be set by setting M8Q_USER_CONFIG to 1. 
 * 
 * @param i2c : pointer to the I2C port used 
 */
void m8q_user_config_init(
    I2C_TypeDef *i2c); 


/**
 * @brief User configuration 
 * 
 * @details User config mode allows the user to change the settings of the receiver using a 
 *          serial terminal interface. Users input NMEA or UBX messages which get converted 
 *          to proper message strings and then sent to the receiver. Is message formatting is 
 *          not correct then error codes will be displayed on the terminal indicating the 
 *          problem with the input message. This is done to help ensure garbage data is not 
 *          sent to the reciever. Note that this function is only valid during user config mode
 *          which can be set by setting M8Q_USER_CONFIG to 1. 
 */
void m8q_user_config(void); 

//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
