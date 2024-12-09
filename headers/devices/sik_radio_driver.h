/**
 * @file sik_radio_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK telemetry radio firmware driver interface 
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

#ifndef _SIK_RADIO_DRIVER_H_ 
#define _SIK_RADIO_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include <stdint.h> 
#include "tools.h" 
#include "uart_comm.h" 

//=======================================================================================


//=======================================================================================
// AT/RT command declarations 

extern const char 
// Enter AT/RT command mode 
sik_at_enter_cmd[], 
// - ATO - exit AT command mode 
sik_atio_cmd[], 
// Show radio version 
sik_ati_cmd[], 
sik_rti_cmd[], 
// Show board type 
sik_ati2_cmd[], 
sik_rti2_cmd[], 
// Show board frequency 
sik_ati3_cmd[], 
sik_rti3_cmd[], 
// Show board version 
sik_ati4_cmd[], 
sik_rti4_cmd[], 
// Show all user settable EEPROM parameters 
sik_ati5_cmd[], 
sik_rti5_cmd[], 
// Display TDM timing report 
sik_ati6_cmd[], 
sik_rti6_cmd[], 
// Display RSSI signal report 
sik_ati7_cmd[], 
sik_rti7_cmd[], 
// Display radio parameter number ‘n’ 
sik_atsn_cmd[], 
sik_rtsn_cmd[], 
// Set radio parameter number ‘n’ to ‘X’ 
sik_atsnx_cmd[], 
sik_rtsnx_cmd[], 
// Reboot the radio 
sik_atz_cmd[], 
sik_rtz_cmd[], 
// Write current parameters to EEPROM 
sik_atw_cmd[], 
sik_rtw_cmd[], 
// Reset all parameters to factory default 
sik_atf_cmd[], 
sik_rtf_cmd[], 
// Enable RSSI debug reporting 
sik_attrssi_cmd[], 
sik_rttrssi_cmd[], 
// Enable TDM debug reporting 
sik_atttdm_cmd[], 
sik_rtttdm_cmd[], 
// Disable debug reporting 
sik_att_cmd[], 
sik_rtt_cmd[]; 

//=======================================================================================


//=======================================================================================
// Enums 

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

#ifdef __cplusplus
}
#endif

#endif   // _SIK_RADIO_DRIVER_H_ 
