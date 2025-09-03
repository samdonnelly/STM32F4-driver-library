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

#ifndef _M8Q_DRIVER_H_
#define _M8Q_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes

#include "stm32f411xe.h"
#include "gpio_driver.h"

//=======================================================================================


//=======================================================================================
// Enums 

// M8Q driver status 
typedef enum {
    M8Q_OK                 = 0x00000000,   // No problems with the M8Q operation 
    M8Q_INVALID_PTR        = 0x00000001,   // Invalid pointer provided to function 
    M8Q_INVALID_CONFIG     = 0x00000002,   // Invalid configuration message 
    M8Q_WRITE_FAULT        = 0x00000004,   // A problem occurred while writing via I2C 
    M8Q_READ_FAULT         = 0x00000008,   // A problem occurred while reading via I2C 
    M8Q_NO_DATA_AVAILABLE  = 0x00000010,   // The data stream is empty or does not have the needed info 
    M8Q_DATA_BUFF_OVERFLOW = 0x00000020,   // Device data buffer (stream size) exceeds driver threshold 
    M8Q_UNKNOWN_DATA       = 0x00000040    // Unknown message stream data 
} m8q_status_t;


// M8Q navigation status 
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

typedef uint32_t M8Q_STATUS; 

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
// Device interface 

/**
 * @brief Read and store relevant message data 
 * 
 * @details Reads the data stream and stores message data if the driver has a data record 
 *          for the message. Messages with a data record currently include: 
 *           - NMEA messages: 
 *             - Standard: None 
 *             - PUBX : POSITION, TIME 
 *           - UBX messages: None 
 *          
 *          This function first reads the size of the data stream. If it's not zero and 
 *          the stream isn't greater than the max buffer size then the data stream is read 
 *          in its entirety. The messages read get either stored in the driver or 
 *          discarded, and if they're stored, their data can be accessed through the 
 *          getter functions. If the data stream size is greater than the max buffer size 
 *          then the stream gets cleared without storing data and an overflow status is 
 *          returned. If there is no data then a no data status is returned. 
 * 
 * @see m8q_init 
 * 
 * @return M8Q_STATUS : status of read attempt 
 */
M8Q_STATUS m8q_read_data(void); 


/**
 * @brief Read and return the data stream contents 
 * 
 * @details Reads the raw data stream in its entirety and stores it in 'data_buff' for 
 *          use by the application. If the data stream size is greater than 'buff_size' 
 *          then the stream gets cleared without storing it in 'data_buff' and an 
 *          overflow status is returned. If there is no data then a no data status is 
 *          returned. 
 * 
 * @param data_buff : buffer to store the data stream 
 * @param buff_size : size of data_buff 
 * @return M8Q_STATUS : status of the read attempt 
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
M8Q_STATUS m8q_read_ds_size(uint16_t *data_size); 


/**
 * @brief ACK/NAK message counter status 
 * 
 * @details Returns the ACK and NAK message counter status in the form of a 16-bit integer 
 *          where the least significant byte is the ACK counter and the most significant 
 *          byte is the NAK counter. This function can be used to check for confirmation 
 *          that the device has received CFG messages sent to it. CFG messages can be sent 
 *          using m8q_send_msg and responses can be checked for using m8q_read_data, after 
 *          which this function can then be called. This function will not update without 
 *          calling m8q_read_data. 
 *          
 *          Note that when CFG messages are sent during initialization, m8q_init 
 *          automatically performs this check. 
 * 
 * @see m8q_read_data 
 * @see m8q_send_msg 
 * 
 * @return uint16_t : ACK/NAK counter statuses 
 */
uint16_t m8q_get_ack_status(void); 


/**
 * @brief Write a message to the device 
 * 
 * @details Sends a single user / application level message to the device. The message is 
 *          first checked for formatting before being sent, and if the format is invalid 
 *          then no message is sent and the function returns an invalid config status. 
 *          
 *          This driver will only send messages to the device that are formatted a 
 *          specific way and all messages get defined at the application level (none are 
 *          stored in the driver). NMEA messages are ASCII formatted and UBX message are 
 *          not, however all messages passed to the driver must be ASCII formatted to 
 *          make definition easier to read by the user. An example user defined NMEA 
 *          message that disables GGA messages is as follows: 
 *          
 *          "$PUBX,40,GGA,0,0,0,0,0,0*" 
 *          
 *          Note that this message string is the exact NMEA message format but it's 
 *          missing the checksum and end sequence. The checksum and end sequence do not 
 *          need to be added to user defined messages as they're added by the driver after 
 *          the message format is checked. An example of a user defined UBX CFG message 
 *          that enables the POSITION PUBX NMEA message is as follows: 
 *          
 *          "B562,06,01,0800,F1,00,01,00,00,00,00,00*" 
 *          
 *          A UBX message does not contain commas to separate message fields, an asterisks 
 *          at the end of (or anywhere within) the message, and it's not in ASCII 
 *          representation. This is all done to make writing them easier for the user and 
 *          processing easier for the driver. The driver will take this message and check 
 *          it's format before eventually converting it to the format it needs to be in 
 *          to be sent to the device. Note that the above user message contains all UBX 
 *          message fields except the checksum at the end which will be added by the 
 *          driver. 
 *          
 *          See the device interface manual for more details on message formats and fields. 
 * 
 * @param write_msg : buffer that contains the message to be sent to the device 
 * @param max_msg_size : size of the message being sent 
 * @return M8Q_STATUS : status of the write attempt 
 */
M8Q_STATUS m8q_send_msg(
    const char *write_msg, 
    uint8_t max_msg_size); 


/**
 * @brief Get TX ready status 
 * 
 * @details Read the state of the TX ready pin. If it's high then it means there is 
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

//=======================================================================================


//=======================================================================================
// POSITION (PUBX,00) message

/**
 * @brief Get the floating point latitude coordinate (degrees) 
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
 * @return float : latitude (degrees) 
 */
float m8q_get_position_lat(void); 


/**
 * @brief Get the scaled integer latitude coordinate (degrees*10^7) 
 * 
 * @details Get the latitude represented as a scaled integer using the data from the 
 *          POSITION PUBX NMEA message (absolute value of latitude and N/S indicator). 
 *          Latitude ranges from +/-90 deg so the returned value is within 
 *          +/-900000000 degrees*10^7. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return int32_t : scaled integer latitude coordinates (degrees*10^7) 
 */
int32_t m8q_get_position_latI(void); 


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
 * @brief Get the floating point longitude coordinate 
 * 
 * @details Get the longitude read from the POSITION PUBX NMEA message. POSITION returns 
 *          an absolute value of longitude so the EW byte is used to put the returned 
 *          value in the range of +/-180deg. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return float : longitude (degrees) 
 */
float m8q_get_position_lon(void); 


/**
 * @brief Get the scaled integer longitude coordinate 
 * 
 * @details Get the longitude represented as a scaled integer using the data from the 
 *          POSITION PUBX NMEA message (absolute value of longitude and E/W indicator). 
 *          Longitude ranges from +/-180 deg so the returned value is within 
 *          +/-1800000000 degrees*10^7. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return int32_t : scaled integer longitude coordinates (degrees*10^7) 
 */
int32_t m8q_get_position_lonI(void); 


/**
 * @brief Get longitude coordinate string 
 * 
 * @details Get the longitude string read from the POSITION PUBX NMEA message. This 
 *          string is exactly what is read from the device. See the device interface 
 *          manual for formatting of data in the POSITION message. If the buffer used 
 *          to store this string is too small then an overflow status will be returned. 
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
 * @brief Get the floating point WGS84 altitude (m) 
 * 
 * @details Get the altitude above the user datum ellipsoid represented as a floating 
 *          point value using the data from the POSITION PUBX NMEA message. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return float : WGS84 altitude (m) 
 */
float m8q_get_position_altref(void); 


/**
 * @brief Get the integer WGS84 altitude (mm) 
 * 
 * @details Get the altitude above the user datum ellipsoid represented as an integer 
 *          value using the data from the POSITION PUBX NMEA message. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return int32_t : WGS84 altitude (mm) 
 */
int32_t m8q_get_position_altrefI(void); 


/**
 * @brief Get navigation status 
 * 
 * @details Returns the navStat field of the POSITION PUBX NMEA message in the form of a 
 *          16-bit integer. m8q_navstats_t contains all the possible navigation status 
 *          codes that can be returned. A valid position lock is indicated by codes G2, 
 *          G3, D2 and D3, which means the status must be one of these in order to be  
 *          receiving reliable coordinate information. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_navstats_t 
 * 
 * @return uint16_t : navigation status code 
 */
uint16_t m8q_get_position_navstat(void); 


/**
 * @brief Get acceptable navigation status 
 * 
 * @details Takes the navigation status from m8q_get_position_navstat and returns true 
 *          if there is a valid position lock (i.e. statuses G2, G3, D2, D3). Returns 
 *          false otherwise. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_navstats_t 
 * @see m8q_get_position_navstat 
 * 
 * @return uint8_t : valid position lock status 
 */
uint8_t m8q_get_position_navstat_lock(void); 


/**
 * @brief Get speed over ground (SOG) 
 * 
 * @details Get speed over ground (SOG) read from the POSITION PUBX NMEA message. POSITION 
 *          returns a numeric value of SOG in km/h. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return float : speed over ground (km/h) 
 */
float m8q_get_position_sog(void);


/**
 * @brief Get speed over ground (SOG) as a scaled integer 
 * 
 * @details Get the scaled value of speed over ground (SOG) read from the POSITION PUBX 
 *          NMEA message. POSITION returns a numeric value of SOG in km/h but the returned 
 *          value is scaled by 1000 to eliminate decimal places. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return uint32_t : speed over ground (km/h * 1000) 
 */
uint32_t m8q_get_position_sogI(void);


/**
 * @brief Get speed over ground (SOG) string 
 * 
 * @details Get the speed over ground (SOG) string read from the POSITION PUBX NMEA 
 *          message. This string is exactly what is read from the device. See the device 
 *          interface manual for formatting of data in the POSITION message. If the 
 *          buffer used to store this string is too small then an overflow status will 
 *          be returned. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @param sog_str : buffer to store the speed over ground string 
 * @param sog_str_len : length of the sog_str buffer 
 * @return M8Q_STATUS : status of the getter 
 */
M8Q_STATUS m8q_get_position_sog_str(
    uint8_t *sog_str, 
    uint8_t sog_str_len);


/**
 * @brief Get course over ground (COG) 
 * 
 * @details Get course over ground (COG) read from the POSITION PUBX NMEA message. POSITION 
 *          returns a numeric value of COG in degrees. COG is the direction of the SOG 
 *          relative to true North. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return float : course over ground (deg) 
 */
float m8q_get_position_cog(void);


/**
 * @brief Get course over ground (COG) as a scaled integer 
 * 
 * @details Get the scaled value of course over ground (COG) read from the POSITION PUBX 
 *          NMEA message. POSITION returns a numeric value of COG in degrees but the 
 *          returned value is scaled by 100 to eliminate decimal places. COG is the 
 *          direction of the SOG relative to true North. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return uint32_t : course over ground (deg*1000) 
 */
uint32_t m8q_get_position_cogI(void);


/**
 * @brief Get vertical velocity 
 * 
 * @details Get vertical velocity (vVel) read from the POSITION PUBX NMEA message. POSITION 
 *          returns a numeric value of vVel in m/s. This velocity is positive downward 
 *          which follows NED frame convention. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return float : vertical velocity (m/s) 
 */
float m8q_get_position_vvel(void);


/**
 * @brief Get vertical velocity (vVel) as a scaled integer 
 * 
 * @details Get the scaled value of vertical velocity (vVel) read from the POSITION PUBX 
 *          NMEA message. POSITION returns a numeric value of vVel in m/s but the returned 
 *          value is scaled by 1000 to eliminate decimal places which therefore returns 
 *          thr velocity in mm/s. This velocity is positive downward which follows NED 
 *          frame convention. 
 *          
 *          This value is only updated if new POSITION messages are read. 
 * 
 * @see m8q_read_data 
 * 
 * @return int32_t : vertical velocity (mm/s) 
 */
int32_t m8q_get_position_vvelI(void);

//=======================================================================================


//=======================================================================================
// TIME (PUBX,04) message 

/**
 * @brief Get UTC time 
 * 
 * @details Get the UTC time string read from the TIME PUBX NMEA message. This string is 
 *          exactly what is read from the device. See the device interface manual for 
 *          formatting of data in the TIME message. If the buffer used to store this 
 *          string is too small then an overflow status will be returned. 
 *          
 *          This value is only updated if new TIME messages are read. 
 * 
 * @param utc_time : buffer to store the UTC time string 
 * @param utc_time_len : length of the utc_time buffer 
 */
M8Q_STATUS m8q_get_time_utc_time(
    uint8_t *utc_time, 
    uint8_t utc_time_len); 


/**
 * @brief Get UTC date 
 * 
 * @details Get the UTC date string read from the TIME PUBX NMEA message. This string is 
 *          exactly what is read from the device. See the device interface manual for 
 *          formatting of data in the TIME message. If the buffer used to store this 
 *          string is too small then an overflow status will be returned. 
 *          
 *          This value is only updated if new TIME messages are read. 
 * 
 * @param utc_date : buffer to store the UTC date string 
 * @param utc_date_len : length of utc_date buffer 
 */
M8Q_STATUS m8q_get_time_utc_date(
    uint8_t *utc_date, 
    uint8_t utc_date_len); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _M8Q_DRIVER_H_ 
