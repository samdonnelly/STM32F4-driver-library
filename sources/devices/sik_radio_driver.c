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

// Available AT commands: 
// - +++ - Enter AT/RT command mode 
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
// 

// Driver data record 
typedef struct sik_driver_data_s 
{
    uint8_t at_mode : 1; 
}
sik_driver_data_t; 

// Driver data record instance 
static sik_driver_data_t sik_driver_data; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Initialization 
void sik_init(void)
{
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

//=======================================================================================
