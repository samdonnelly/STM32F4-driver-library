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

// This driver currently does not support the interpretation of received UBX messages 
// aside from checking for ACK/NAK messages in response to UBX CFG messages. 

//=======================================================================================

#ifndef _M8Q_DRIVER_H_
#define _M8Q_DRIVER_H_

//=======================================================================================
// Includes

#include "stm32f411xe.h"
#include "gpio_driver.h"

//=======================================================================================


//=======================================================================================
// Enums 

// M8Q driver status 
typedef enum {
    M8Q_OK,                   // No problems with the M8Q operation 
    M8Q_INVALID_PTR,          // Invalid pointer provided to function 
    M8Q_INVALID_CONFIG,       // Invalid configuration message 
    M8Q_WRITE_FAULT,          // A problem occurred while writing via I2C 
    M8Q_READ_FAULT,           // A problem occurred while reading via I2C 
    M8Q_NO_DATA_AVAILABLE,    // The data stream is empty or does not have the needed info 
    M8Q_DATA_BUFF_OVERFLOW,   // Device data buffer (stream size) exceeds driver threshold 
    M8Q_UNKNOWN_DATA          // Unknown message stream data 
} m8q_status_t; 


// M8Q navigation statuses 
typedef enum {
    M8Q_NAVSTAT_NF = 0x4E46,   // No Fix 
    M8Q_NAVSTAT_DR = 0x4452,   // Dead reckoning only solution 
    M8Q_NAVSTAT_G2 = 0x4732,   // Stand alone 2D solution 
    M8Q_NAVSTAT_G3 = 0x4733,   // Stand alone 3D solution 
    M8Q_NAVSTAT_D2 = 0x4432,   // Differential 2D solution 
    M8Q_NAVSTAT_D3 = 0x4433,   // Differential 3D solution 
    M8Q_NAVSTAT_RK = 0x524B,   // Combined GPS and DR solution 
    M8Q_NAVSTAT_TT = 0x5454    // Time only solution 
} m8q_navstats_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t M8Q_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief M8Q driver initialization 
 * 
 * @details Initializes driver data and sends configuration messages to the device so the 
 *          device can be set up to operate a certain way. This function must be called 
 *          once before using other driver functions. 
 *          
 *          Each configuration message passed to this function gets checked for 
 *          formatting before being sent to the device. If one of the messages is not of 
 *          a correct format then the function will abort and an invalid config status 
 *          will be returned. If a CFG message is sent then the function looks for an ACK 
 *          response, where a NAK response or no response will be considered an invalid 
 *          config. 
 *          
 *          Configuration messages are defined by the application as a jagged array of 
 *          strings, where a char pointer to the start of the first message is passed to 
 *          this function. m8q_send_msg has more information on message definition. If no 
 *          config messages are needed then make the 'msg_num' argument zero. 
 *          
 *          Note that "configuration messages" are not referring to CFG UBX messages 
 *          specifically. It's referring to any message being used to change the settings 
 *          of the device, which is what the messages passed to this function are 
 *          intended to do. 
 *          
 *          The 'data_buff_limit' argument is used to set the max buffer size the driver 
 *          is allowed to allocate when reading the device data stream. If the data 
 *          stream is larger than the max buffer size and a read is attempted, then 
 *          the driver will be forced to flush the data stream without recording any 
 *          data and an overflow status will be indicated. If this argument is set to 
 *          zero then there will be no limit set. 
 * 
 * @see m8q_send_msg 
 * 
 * @param i2c : I2C port used for communicating with the device 
 * @param config_msgs : buffer that contains the configuration messages 
 * @param msg_num : number of configuration messages 
 * @param max_msg_size : max possible config message size in config_msgs 
 * @param data_buff_limit : max buffer size that can be allocated to read data stream 
 * @return M8Q_STATUS : status of the initialization 
 */
M8Q_STATUS m8q_init(
    I2C_TypeDef *i2c, 
    const char *config_msgs, 
    uint8_t msg_num, 
    uint8_t max_msg_size, 
    uint16_t data_buff_limit); 


/**
 * @brief M8Q low power pin initialization 
 * 
 * @details Initializes an output pin to control the state of the device's INT pin 
 *          which is used to put the device into low power mode. This function must be 
 *          called before the INT pin can be controlled. By default, the device is not 
 *          put into low power mode. 
 * 
 * @see m8q_set_low_pwr 
 * @see m8q_clear_low_pwr 
 * 
 * @param gpio : GPIO port of pin being used 
 * @param pwr_save_pin : pin number of pin being used 
 * @return M8Q_STATUS : status of the initialization 
 */
M8Q_STATUS m8q_pwr_pin_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t pwr_save_pin); 


/**
 * @brief M8Q TX ready pin initialization 
 * 
 * @details Initializes an input pin to read the state of the device's TX ready pin. The 
 *          TX ready pin indicates when there is data in the device data stream (i.e. 
 *          there is data available to be read). This function must be called before the 
 *          TX ready status can be read. 
 * 
 * @see m8q_get_tx_ready 
 * 
 * @param gpio : GPIO port of pin being used 
 * @param tx_ready_pin : pin number of pin being used 
 * @return M8Q_STATUS : status of the initialization 
 */
M8Q_STATUS m8q_txr_pin_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t tx_ready_pin); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Read the data stream and sort/store relevant message data 
 * 
 * @details 
 * 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_data(void); 


/**
 * @brief Read and return the data stream contents 
 * 
 * @details 
 * 
 * @param data_buff 
 * @param buff_size 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_ds(
    uint8_t *data_buff, 
    uint16_t buff_size); 


/**
 * @brief Read data stream size 
 * 
 * @details Reads and formats the size of the device data stream and stores the value in 
 *          'data_size'. If there is no data available then a no data status will be 
 *          returned. 
 * 
 * @param data_size : buffer to store the data stream size (16-bit value) 
 * @return M8Q_STATUS : status of the read operation 
 */
M8Q_STATUS m8q_read_ds_size(
    uint16_t *data_size); 


/**
 * @brief Return the ACK/NAK message counter status 
 * 
 * @details 
 * 
 * @return uint16_t 
 */
uint16_t m8q_get_ack_status(void); 


/**
 * @brief Write a message to the device 
 * 
 * @details 
 * 
 * @param write_msg 
 * @param max_msg_size 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_send_msg(
    const char *write_msg, 
    uint8_t max_msg_size); 


/**
 * @brief Get TX ready status 
 * 
 * @details Read that state of the TX ready pin. If it's high then it means there is 
 *          data in the device data stream that can be read. 
 * 
 * @return GPIO_STATE : state of the TX ready pin 
 */
GPIO_STATE m8q_get_tx_ready(void); 


/**
 * @brief Enter low power mode 
 * 
 * @details Sets the device INT pin low to put the device into low power mode. 
 * 
 * @see m8q_pwr_pin_init 
 */
void m8q_set_low_pwr(void); 


/**
 * @brief Exit low power mode 
 * 
 * @details Sets the device INT pin high to take the device out of low power mode. 
 * 
 * @see m8q_pwr_pin_init 
 */
void m8q_clear_low_pwr(void); 


/**
 * @brief Get latitude coordinate 
 * 
 * @details Get the converted value of latitude read from the POSITION PUBX NMEA message. 
 *          POSITION returns an absolute value of latitude and the NS byte is used to 
 *          determine the hemisphere. This function uses these two values to calculate 
 *          latitude in the range of +/- 90deg. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return double : latitude 
 */
double m8q_get_position_lat(void); 


/**
 * @brief Get latitude coordinate string 
 * 
 * @details Get the latitude string read from the POSITION PUBX NMEA message. This string 
 *          is exactly what is read from the device. See the device interface manual for 
 *          formatting of data in the POSITION message. If the buffer used to store this 
 *          string is too small then an overflow status will be returned. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @param lat_str : buffer to store the latitude string 
 * @param lat_str_len : length of the lat_str buffer 
 * @return M8Q_STATUS : status of the getter 
 */
M8Q_STATUS m8q_get_position_lat_str(
    uint8_t *lat_str, 
    uint8_t lat_str_len); 


/**
 * @brief Get North/South hemisphere 
 * 
 * @details Returns the NS byte of the POSITION PUBX NMEA message. This indicates which 
 *          hemisphere the device is in, where the return value can be either: 
 *          - "N" (78) --> Northern hemisphere 
 *          - "S" (83) --> Southern hemisphere 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @return uint8_t : NS byte 
 */
uint8_t m8q_get_position_NS(void); 


/**
 * @brief Get longitude coordinate 
 * 
 * @details Get the converted value of longitude read from the POSITION PUBX NMEA message. 
 *          POSITION returns an absolute value of longitude and the EW byte is used to 
 *          determine the hemisphere. This function uses these two values to calculate 
 *          longitude in the range of +/- 180deg. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return double : longitude 
 */
double m8q_get_position_lon(void); 


/**
 * @brief Get longitude coordinate string 
 * 
 * @details Get the longitude string read from the POSITION PUBX NMEA message. This 
 *          string is exactly what is read from the device. See the device interface 
 *          manual for formatting of data in the POSITION message. In order for this 
 *          value to be updated, m8q_read_data must be called when new data is available. 
 *          If the buffer used to store this string is too small then an overflow status 
 *          will be returned. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @param lon_str : buffer to store the longitude string 
 * @param lon_str_len : length of the lon_str buffer 
 * @return M8Q_STATUS : status of the getter 
 */
M8Q_STATUS m8q_get_position_lon_str(
    uint8_t *lon_str, 
    uint8_t lon_str_len); 


/**
 * @brief Get East/West hemisphere 
 * 
 * @details Returns the EW byte of the POSITION PUBX NMEA message. This indicates which 
 *          hemisphere the device is in, where the return value can be either: 
 *          - "E" (69) --> Eastern hemisphere 
 *          - "W" (87) --> Western hemisphere 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @return uint8_t : EW byte 
 */
uint8_t m8q_get_position_EW(void); 


/**
 * @brief Get navigation status 
 * 
 * @details 
 * 
 * @return uint16_t 
 */
uint16_t m8q_get_position_navstat(void); 


/**
 * @brief Get acceptable navigation status 
 * 
 * @details 
 *          Returns true for valid position lock, false otherwise. 
 * 
 * @return uint8_t 
 */
uint8_t m8q_get_position_navstat_lock(void); 


/**
 * @brief Get UTC time 
 * 
 * @details 
 * 
 * @param utc_time 
 * @param utc_time_len 
 */
M8Q_STATUS m8q_get_time_utc_time(
    uint8_t *utc_time, 
    uint8_t utc_time_len); 


/**
 * @brief Get UTC date 
 * 
 * @details 
 * 
 * @param utc_date 
 * @param utc_date_len 
 */
M8Q_STATUS m8q_get_time_utc_date(
    uint8_t *utc_date, 
    uint8_t utc_date_len); 

//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
