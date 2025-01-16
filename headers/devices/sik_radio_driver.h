/**
 * @file sik_radio_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK telemetry radio firmware driver interface 
 * 
 * @details Works for the generic SiK telemetry radio as well as the RFD900 and its 
 *          variants. These devices are designed (but not required) to work with the 
 *          MAVLink protocol. This driver doesn't do any MAVLink message formatting so 
 *          the application using this should also use the MAVLink library as needed. 
 * 
 * @version 0.1
 * @date 2024-12-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _SIK_RADIO_DRIVER_H_ 
#define _SIK_RADIO_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "tools.h" 
#include "uart_comm.h" 

//=======================================================================================


//=======================================================================================
// AT/RT command declarations 

/**
 * @brief AT command mode: command list 
 * 
 * @details The listed commands are defined with format specifiers in place of 'x', 'n' 
 *          and 'X' so they can easily be populated using tools like snprintf. 
 *          - 'x' --> %c format, use sik_at_rt_t to select between 'A' and 'R' which is 
 *                    used to specify the local or remote device, respectfully. 
 *          - 'n' --> %u format, use sik_at_param_number_t to select the parameter index. 
 *          - 'X' --> %lu format, this can be replaced with an unsigned 32-bit value. 
 *          
 *          NOTE: the exception to these format specifiers are the first two commands 
 *                for entering and exiting AT command mode. These commands are always 
 *                the same. 
 * 
 * @see sik_at_rt_t 
 * @see sik_at_param_number_t 
 */
extern const char 
sik_at_enter_cmd[],   // "+++" - Enter AT/RT command mode 
sik_ato_cmd[],        // "ATO" - exit AT command mode 
sik_xti_cmd[],        // "xTI" - Show radio version 
sik_xti2_cmd[],       // "xTI2" - Show board type 
sik_xti3_cmd[],       // "xTI3" - Show board frequency 
sik_xti4_cmd[],       // "xTI4" - Show board version 
sik_xti5_cmd[],       // "xTI5" - Show all user settable EEPROM parameters 
sik_xti6_cmd[],       // "xTI6" - Display TDM timing report 
sik_xti7_cmd[],       // "xTI7" - Display RSSI signal report 
sik_xtsn_cmd[],       // "xTSn?" - Display radio parameter number ‘n’ 
sik_xtsnx_cmd[],      // "xTSn=X" - Set radio parameter number ‘n’ to ‘X’ 
sik_xtz_cmd[],        // "xTZ" - Reboot the radio 
sik_xtw_cmd[],        // "xT&W" - Write current parameters to EEPROM 
sik_xtf_cmd[],        // "xT&F" - Reset all parameters to factory default 
sik_xttrssi_cmd[],    // "xT&T=RSSI" - Enable RSSI debug reporting 
sik_xtttdm_cmd[],     // "xT&T=TDM" - Enable TDM debug reporting 
sik_xtt_cmd[];        // "xT&T" - Disable debug reporting 

//=======================================================================================


//=======================================================================================
// Enums 

// SiK driver statuses 
typedef enum {
    SIK_OK, 
    SIK_INVALID_PTR, 
    SIK_NO_DATA, 
    SIK_READ_FAULT 
} sik_status_t; 


// AT command mode: enter or exit 
typedef enum {
    SIK_AT_ENTER, 
    SIK_AT_EXIT 
} sik_at_mode_t; 


// AT command mode: AT/RT device selection 
typedef enum {
    SIK_AT_DEVICE = 65,   // 65 == 'A' 
    SIK_RT_DEVICE = 82    // 82 == 'R' 
} sik_at_rt_t; 


// AT command mode: parameter index 
typedef enum {
    SIK_AT_PARAM_S0,    // FORMAT 
    SIK_AT_PARAM_S1,    // SERIAL_SPEED 
    SIK_AT_PARAM_S2,    // AIR_SPEED 
    SIK_AT_PARAM_S3,    // NETID 
    SIK_AT_PARAM_S4,    // TXPOWER 
    SIK_AT_PARAM_S5,    // ECC 
    SIK_AT_PARAM_S6,    // MAVLINK 
    SIK_AT_PARAM_S7,    // OPPRESEND 
    SIK_AT_PARAM_S8,    // MIN_FREQ 
    SIK_AT_PARAM_S9,    // MAX_FREQ 
    SIK_AT_PARAM_S10,   // NUM_CHANNELS 
    SIK_AT_PARAM_S11,   // DUTY_CYCLE 
    SIK_AT_PARAM_S12,   // LBT_RSSI 
    SIK_AT_PARAM_S13,   // MANCHESTER 
    SIK_AT_PARAM_S14,   // RTSCTS 
    SIK_AT_PARAM_S15    // MAX_WINDOW 
} sik_at_param_number_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef sik_status_t SIK_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief Sik radio driver initialization 
 * 
 * @param uart : UART port to communicate with 
 * @return SIK_STATUS : driver status 
 */
SIK_STATUS sik_init(USART_TypeDef *uart); 

//=======================================================================================


//=======================================================================================
// Read and write 

/**
 * @brief Read data 
 * 
 * @details Checks if there is UART data available in the RX buffer and proceeds to read 
 *          the data and store it in the provided buffer. This function must be polled 
 *          or called via an interrupt to catch the data when it arrives. It's the users 
 *          responsibility to provide a buffer large enough to store the expected data. 
 *          If the end of the buffer is reached before all data has been read then the 
 *          remaining data will be lost. 
 *          
 *          Note that this function is not recommended. A more efficient and reliable 
 *          method for getting the device data is to use DMA to transfer RX data to a 
 *          buffer which can then be used at your conveinence. 
 *          
 *          SiK telemetry radios are designed (but not required) to work with the MAVLink 
 *          protocol. This driver is intended to exchange date with the device only 
 *          meaning no MAVLink formatting is handled here. Once MAVLink data is received, 
 *          the application should use the MAVLink library to decode messages. 
 * 
 * @param read_data : buffer to store the incoming data 
 * @return SIK_STATUS : driver status 
 */
SIK_STATUS sik_read_data(uint8_t *read_data); 


/**
 * @brief Send data 
 * 
 * @details The provided data string will be sent to the device via UART. In normal 
 *          operation (i.e. not in AT command mode), the device will try to relay the 
 *          data to a remote module. Note that this does not check whether the module 
 *          has an established connection or not. 
 *          
 *          If using this function to send AT commands, then the user must format the 
 *          provided command strings (declared above) on their own. Otherwise, use the 
 *          provided AT command functions below. 
 *          
 *          SiK telemetry radios are designed (but not required) to work with the MAVLink 
 *          protocol. This driver is intended to exchange date with the device only 
 *          meaning no MAVLink formatting is handled here. If using the MAVLink protocol, 
 *          the application should encode the data using the MAVLink library before 
 *          sending the message buffer here. 
 * 
 * @param send_data : data string to send 
 * @return SIK_STATUS : driver status 
 */
SIK_STATUS sik_send_data(const char *send_data); 

//=======================================================================================


//=======================================================================================
// AT/RT command functions 

/**
 * @brief AT command mode: enter or exit 
 * 
 * @details Will send the enter or exit commands for AT command mode depending on the 
 *          provided mode. The device must enter AT command mode before it can process 
 *          any AT/RT requests. According to the SiK documentation, there is a 1 second 
 *          delay between sending the enter command ("+++") and getting the "OK" response 
 *          which indicates that the device is now in AT command mode. 
 *          
 *          NOTE: This function does not wait to check for the "OK" response from the 
 *                device if trying to enter AT command mode. 
 * 
 * @param mode : choice or whether to enter or exit AT command mode 
 */
void sik_at_mode(sik_at_mode_t mode); 


/**
 * @brief AT command mode: send command 
 * 
 * @details Takes an AT command and formats it with the device type (i.e. 'A' or 'R') 
 *          before sending it to the device. This function does not check the contents of 
 *          the provided command so it's up to the user to provide the correct command 
 *          string. It's recommended to use the pre-declared strings above. An 
 *          incorrectly formatted string will have no affect. 
 *          
 *          If the user does not wish to use the pre-declared strings and this drivers 
 *          formatting functions, they can format their own commands and send them via 
 *          sik_send_data instead. 
 *          
 *          NOTE: This function is not meant for commands "+++", "ATO", "xTSn?", and 
 *                "xTSn=X". The first two require no formatting and can be handled by 
 *                sik_at_mode. The second two have additional formatting needed and are 
 *                handled by sik_at_get_param and sik_at_set_param. 
 * 
 * @see sik_send_data 
 * @see sik_at_mode 
 * @see sik_at_get_param 
 * @see sik_at_set_param 
 * 
 * @param device : device type 
 * @param cmd : command - use one of the pre-decalred commands above 
 */
void sik_at_send_cmd(
    sik_at_rt_t device, 
    const char *cmd); 


/**
 * @brief AT command mode: get parameter 
 * 
 * @details This function if exclusively for command "xTSn?" (Display radio parameter 
 *          number ‘n’). It takes the device type and parameter number then formats the 
 *          command string before sending it to the device. This function does not check 
 *          for a response to the parameter request, this must be done by the user. 
 *          
 *          If the user does not wish to use the pre-declared strings and this drivers 
 *          formatting functions, they can format their own commands and send them via 
 *          sik_send_data instead. 
 * 
 * @see sik_send_data 
 * 
 * @param device : device type 
 * @param param : parameter number 
 */
void sik_at_get_param(
    sik_at_rt_t device, 
    sik_at_param_number_t param); 


/**
 * @brief AT command mode: Set parameter 
 * 
 * @details This function if exclusively for command "xTSn=X" (Set radio parameter number 
 *          ‘n’ to ‘X’). It takes the device type, parameter number and desired parameter 
 *          value then formats the command string before sending it to the device. 
 *          Updating parameters in EEPROM is done with the following steps: 
 *          
 *          1. Change all needed parameters with "ATSn=X" (this function) 
 *          2. Write parameter to EEPROM with "AT&W" (use sik_at_send_cmd) 
 *             - Transmit power setting will take effect immediately without needing to 
 *               write to the EEPROMs, but you still need to write to the EEPROMs for the 
 *               parameter to save between reboots. 
 *          3. Reboot using "ATZ" so new parameters can take effect (use sik_at_send_cmd) 
 *          
 *          Some parameters must be the same between local ('A') and remote ('R') devices 
 *          for them to communicate. If one of these parameters must be updated then it's 
 *          recommended to update the remote device first as changing the parameter will 
 *          likely result in a loss of connection. For two radios to communicate, the 
 *          following must be the same at both ends of the link: 
 *          
 *          - Radio firmware version
 *          - AIR_SPEED 
 *          - MIN_FREQ 
 *          - MAX_FREQ 
 *          - NUM_CHANNELS 
 *          - NETID 
 *          - ECC setting 
 *          - LBT_RSSI setting 
 *          - MAX_WINDOW setting 
 *          
 *          If the user does not wish to use the pre-declared strings and this drivers 
 *          formatting functions, they can format their own commands and send them via 
 *          sik_send_data instead. 
 * 
 * @see sik_send_data 
 * @see sik_at_send_cmd 
 * 
 * @param device : device type 
 * @param param : parameter number 
 * @param value : value to set parameter to 
 */
void sik_at_set_param(
    sik_at_rt_t device, 
    sik_at_param_number_t param, 
    uint32_t value); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _SIK_RADIO_DRIVER_H_ 
