/**
 * @file sik_radio_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK telemetry radio firmware driver 
 * 
 * @details Works for the generic SiK telemetry radio as well as the RFD900 and its 
 *          variants. 
 * 
 * @version 0.1
 * @date 2024-12-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "sik_radio_driver.h" 

//=======================================================================================


//=======================================================================================
// Notes 

// Available commands: 
// - +++ - Enter AT/RT command mode 
// - RT - Remote AT command 
// - ATI - show radio version 
// - ATI2 - show board type 
// - ATI3 - show board frequency 
// - ATI4 - show board version 
// - ATI5 - show all user settable EEPROM parameters 
// - ATI6 - display TDM timing report 
// - ATI7 - display RSSI signal report 
// - ATO - exit AT command mode 
// - ATSn? - display radio parameter number ‘n’ 
// - ATSn=X - set radio parameter number ‘n’ to ‘X’ 
// - ATZ - reboot the radio 
// - AT&W - write current parameters to EEPROM 
// - AT&F - reset all parameters to factory default 
// - AT&T=RSSI - enable RSSI debug reporting 
// - AT&T=TDM - enable TDM debug reporting 
// - AT&T - disable debug reporting 

// In the above commands, "AT" can be replaced by "RT" for use on a connected remote 
// radio with the exception of "ATO". 

// Updating parameters process: 
// - Change parameters with "ATSn=X" 
// - Write parameter to EEPROM with "AT&W" 
//   - Transmit power setting will take effect immediately without needing to write to 
//     the EEPROMs, but you still need to write to the EEPROMs for the parameter to save 
//     between reboots. 
// - Reboot using "ATZ" so new parameters can take effect 

// For two radios to communicate the following must be the same at both ends of the link:
// - Radio firmware version
// - AIR_SPEED 
// - MIN_FREQ 
// - MAX_FREQ 
// - NUM_CHANNELS 
// - NETID 
// - ECC setting 
// - LBT_RSSI setting 
// - MAX_WINDOW setting 

//=======================================================================================


//=======================================================================================
// AT/RT command definitions 

extern const char 
// Enter AT/RT command mode 
sik_at_enter_cmd[] = "+++", 
// - ATO - exit AT command mode 
sik_atio_cmd[] = "ATO", 
// Show radio version 
sik_ati_cmd[] = "ATI", 
sik_rti_cmd[] = "RTI", 
// Show board type 
sik_ati2_cmd[] = "ATI2", 
sik_rti2_cmd[] = "RTI2", 
// Show board frequency 
sik_ati3_cmd[] = "ATI3", 
sik_rti3_cmd[] = "RTI3", 
// Show board version 
sik_ati4_cmd[] = "ATI4", 
sik_rti4_cmd[] = "RTI4", 
// Show all user settable EEPROM parameters 
sik_ati5_cmd[] = "ATI5", 
sik_rti5_cmd[] = "RTI5", 
// Display TDM timing report 
sik_ati6_cmd[] = "ATI6", 
sik_rti6_cmd[] = "RTI6", 
// Display RSSI signal report 
sik_ati7_cmd[] = "ATI7", 
sik_rti7_cmd[] = "RTI7", 
// Display radio parameter number ‘n’ 
sik_atsn_cmd[] = "ATSn?", 
sik_rtsn_cmd[] = "RTSn?", 
// Set radio parameter number ‘n’ to ‘X’ 
sik_atsnx_cmd[] = "ATSn=X", 
sik_rtsnx_cmd[] = "RTSn=X", 
// Reboot the radio 
sik_atz_cmd[] = "ATZ", 
sik_rtz_cmd[] = "RTZ", 
// Write current parameters to EEPROM 
sik_atw_cmd[] = "AT&W", 
sik_rtw_cmd[] = "RT&W", 
// Reset all parameters to factory default 
sik_atf_cmd[] = "AT&F", 
sik_rtf_cmd[] = "RT&F", 
// Enable RSSI debug reporting 
sik_attrssi_cmd[] = "AT&T=RSSI", 
sik_rttrssi_cmd[] = "RT&T=RSSI", 
// Enable TDM debug reporting 
sik_atttdm_cmd[] = "AT&T=TDM", 
sik_rtttdm_cmd[] = "RT&T=TDM", 
// Disable debug reporting 
sik_att_cmd[] = "AT&T", 
sik_rtt_cmd[] = "RT&T"; 

//=======================================================================================


//=======================================================================================
// Global data 

// Driver data record 
typedef struct sik_driver_data_s 
{
    USART_TypeDef *uart; 
    uint8_t at_mode : 1; 
}
sik_driver_data_t; 

// Driver data record instance 
static sik_driver_data_t sik_driver_data; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Initialization 
void sik_init(USART_TypeDef *uart)
{
    if (uart == NULL)
    {
        return; 
    }
    
    sik_driver_data.uart = uart; 
    sik_driver_data.at_mode = CLEAR_BIT; 
}

//=======================================================================================


//=======================================================================================
// Read and write functions 

// Receive data 
// - Manual read? Only used if not using DMA? 
// - Parse a data buffer? Can I send the data to the mavlink library for parsing? 


// Send data 

//=======================================================================================


//=======================================================================================
// AT Command Functions 

// Enter AT/RT command mode 
void sik_at_mode(void)
{
    // There should be some function to check for the "OK" from the radio when requesting 
    // AT command mode which then sets this bit if it's seen. 
    sik_driver_data.at_mode = SET_BIT; 
}


// Send command 
void sik_at_send_cmd(const char *cmd)
{
    if (cmd == NULL)
    {
        return; 
    }

    uart_sendstring(sik_driver_data.uart, cmd); 
}


// 
void sik_at_param_get(sik_at_param_number_t param)
{
    // Format the command 
    char format_cmd[50]; 

    sik_at_send_cmd(format_cmd); 
}


// 
void sik_at_param_set(
    sik_at_param_number_t param, 
    uint32_t value)
{
    // Format the command 
    char format_cmd[50]; 

    sik_at_send_cmd(format_cmd); 
}

//=======================================================================================
