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
 * @param data_buff_limit 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_init(
    I2C_TypeDef *i2c, 
    const char *config_msgs, 
    uint8_t msg_num, 
    uint8_t max_msg_size, 
    uint16_t data_buff_limit); 


/**
 * @brief Low power mode pin initialization 
 * 
 * @details 
 * 
 * @param gpio 
 * @param pwr_save_pin 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_pwr_pin_init(
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
M8Q_STATUS m8q_txr_pin_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t tx_ready_pin); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Read the data stream size 
 * 
 * @details 
 * 
 * @param data_size 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_ds_size(
    uint16_t *data_size); 


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
 * @brief Get TX-Ready status 
 * 
 * @details 
 * 
 * @return GPIO_STATE 
 */
GPIO_STATE m8q_get_tx_ready(void); 


/**
 * @brief Enter low power mode 
 * 
 * @details 
 */
void m8q_set_low_pwr(void); 


/**
 * @brief Exit low power mode 
 * 
 * @details 
 */
void m8q_clear_low_pwr(void); 


/**
 * @brief Get latitude coordinate 
 * 
 * @details 
 * 
 * @return double 
 */
double m8q_get_position_lat(void); 


/**
 * @brief Get latitude coordinate string 
 * 
 * @details 
 * 
 * @param lat_str 
 * @param lat_str_len 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_get_position_lat_str(
    uint8_t *lat_str, 
    uint8_t lat_str_len); 


/**
 * @brief Get North/South hemisphere 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t m8q_get_position_NS(void); 


/**
 * @brief Get longitude coordinate 
 * 
 * @details 
 * 
 * @return double 
 */
double m8q_get_position_lon(void); 


/**
 * @brief Get longitude coordinate string 
 * 
 * @details 
 * 
 * @param lon_str 
 * @param lon_str_len 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_get_position_lon_str(
    uint8_t *lon_str, 
    uint8_t lon_str_len); 


/**
 * @brief Get East/West hemisphere 
 * 
 * @details 
 * 
 * @return uint8_t 
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
