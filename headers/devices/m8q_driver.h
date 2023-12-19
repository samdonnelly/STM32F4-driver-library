/**
 * @file m8q_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS driver interface 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Notes 

// This driver currently only support writing of UBX messages to the device. It does not 
// support reading UBX messages. 

//=======================================================================================

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
#define M8Q_NMEA_TIME_ARGS     8         // Number of data fields in TIME 
#define M8Q_NMEA_TIME_ID       52        // "4" == 52 --> Message ID for TIME 
#define M8Q_NMEA_PUBX_ARG_OFST 9         // First data field offset for PUBX messages 
#define M8Q_NMEA_STRD_ARG_OFST 7         // First data field offset for standard messages 

// NMEA POSITION coordinate calculation 
#define M8Q_LAT_LEN 10                   // Latitude message length 
#define M8Q_LON_LEN 11                   // Longitude message length 
#define M8Q_LAT_DEG_INT_LEN 2            // Number of digits for integer portion of latitude 
#define M8Q_LON_DEG_INT_LEN 3            // Number of digits for integer portion of longitude 
#define M8Q_MIN_DIGIT_INDEX 6            // Index of scaled minute portion of coordinate 
#define M8Q_MIN_FRAC_LEN 5               // Length of fractional part of minutes 
#define M8Q_DIR_SOUTH 0x53               // Value of South from the North/South indicator 
#define M8Q_DIR_WEST 0x57                // Value of West from the East/West indicator 
#define M8Q_MIN_TO_DEG 60                // Used to convert coordinate minutes to degrees 
#define M8Q_COO_LEN 6                    // Length of integer and fractional parts of coordinates 

// UBX message format 
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
 * @brief M8Q driver status 
 */
typedef enum {
    M8Q_OK,                  // No problems with the M8Q operation 
    M8Q_INVALID_PTR,         // Invalid pointer provided to function 
    M8Q_INVALID_CONFIG,      // Invalid configuration message 
    M8Q_WRITE_FAULT,         // 
    M8Q_UBX_MSG_CONV_FAIL,   // UBX message failed to convert to receiver format 
    M8Q_UBX_MSG_CONV_SUCC    // UBX message successfully converted to receiver format 
} m8q_status_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t M8Q_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization - dev 

/**
 * @brief 
 * 
 * @details 
 *          The NMEA configuration messages will be checked for a start character, a valid 
 *          address, the correct number of message fields and valid field characters. Aside 
 *          from character validity, the fields are not checked for formatting. This is left 
 *          to the user to ensure it's done correctly. 
 * 
 * @param i2c 
 * @param config_msgs 
 * @param msg_num 
 * @param max_msg_size 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_init_dev(
    I2C_TypeDef *i2c, 
    const char *config_msgs, 
    uint8_t msg_num, 
    uint8_t max_msg_size); 


/**
 * @brief Low power mode pin initialization 
 * 
 * @details 
 * 
 * @param gpio 
 * @param pwr_save_pin 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_pwr_pin_init_dev(
    GPIO_TypeDef *gpio, 
    pin_selector_t pwr_save_pin); 


/**
 * @brief TX ready pin initialization 
 * 
 * @details 
 * 
 * @param gpio 
 * @param tx_ready_pin 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_txr_pin_init_dev(
    GPIO_TypeDef *gpio, 
    pin_selector_t tx_ready_pin); 

//=======================================================================================


//=======================================================================================
// User functions - dev 

/**
 * @brief Read one message at a time 
 * 
 * @details 
 * 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_msg_dev(void); 


/**
 * @brief Read the whole data stream 
 * 
 * @details 
 * 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_stream_dev(void); 


// Write configuration (singular) 


/**
 * @brief Get TX-Ready status 
 * 
 * @details 
 * 
 * @return GPIO_STATE 
 */
GPIO_STATE m8q_get_tx_ready_dev(void); 


// Clear driver fault code 

// Get driver fault code 


/**
 * @brief Enter low power mode 
 * 
 * @details 
 */
void m8q_set_low_pwr_dev(void); 


/**
 * @brief Exit low power mode 
 * 
 * @details 
 */
void m8q_clear_low_pwr_dev(void); 


// Get latitude 

// Get North/South 

// Get longitude 

// Get East/West 

// Get navigation status 

// Get time 

// Get date 

//=======================================================================================




// Old Code 




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
 * @return M8Q_STATUS : valid read indicator 
 */
M8Q_STATUS m8q_read(void); 


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
 * @brief Latitude coordinate getter 
 * 
 * @details Takes the NMEA standard formatted latitude read from the device and converts 
 *          it to a coordinate in degrees. This can be used to perform navigation 
 *          calculations. 
 * 
 * @return double : latitude coordinate (degrees) 
 */
double m8q_get_lat(void); 


/**
 * @brief Latitude string getter 
 * 
 * @details Copies the NMEA standard formatted latitude string read from the receiver. The 
 *          integer portion of the latitude (degrees and integer minutes) is stored in deg_min 
 *          and the fractional part (fractional minutes) is stored in min_frac. This allows 
 *          for the application to use the coordinate string as needed. For more information 
 *          on latitude formatting see the M8Q protocol specification documentation. 
 * 
 * @param deg_min : pointer to buffer to store degrees and integer minutes 
 * @param min_frac : pointer to buffer to store fractional minutes 
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
 * @brief Longitude coordinate getter 
 * 
 * @details Takes the NMEA standard formatted longitude read from the device and converts 
 *          it to a coordinate in degrees. This can be used to perform navigation 
 *          calculations. 
 * 
 * @return double : longitude coordinate (degrees) 
 */
double m8q_get_long(void); 


/**
 * @brief Longitude string getter 
 * 
 * @details Copies the NMEA standard formatted longitude string read from the receiver. The 
 *          integer portion of the longitude (degrees and integer minutes) is stored in deg_min 
 *          and the fractional part (fractional minutes) is stored in min_frac. This allows 
 *          for the application to use the coordinate string as needed. For more information 
 *          on longitude formatting see the M8Q protocol specification documentation. 
 * 
 * @param deg_min : pointer to buffer to store degrees and integer minutes 
 * @param min_frac : pointer to buffer to store fractional minutes 
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
