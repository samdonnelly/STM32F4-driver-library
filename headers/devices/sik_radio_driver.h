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

#include "tools.h" 
#include "uart_comm.h" 

//=======================================================================================


//=======================================================================================
// AT/RT command declarations 

extern const char 
// Enter AT/RT command mode 
sik_at_enter_cmd[], 
// ATO - exit AT command mode 
sik_ato_cmd[], 
// ATI - Show radio version 
sik_ati_cmd[], 
sik_rti_cmd[], 
// ATI2 - Show board type 
sik_ati2_cmd[], 
sik_rti2_cmd[], 
// ATI3 - Show board frequency 
sik_ati3_cmd[], 
sik_rti3_cmd[], 
// ATI4 - Show board version 
sik_ati4_cmd[], 
sik_rti4_cmd[], 
// ATI5 - Show all user settable EEPROM parameters 
sik_ati5_cmd[], 
sik_rti5_cmd[], 
// ATI6 - Display TDM timing report 
sik_ati6_cmd[], 
sik_rti6_cmd[], 
// ATI7 - Display RSSI signal report 
sik_ati7_cmd[], 
sik_rti7_cmd[], 
// ATSn? - Display radio parameter number ‘n’ 
sik_atsn_cmd[], 
sik_rtsn_cmd[], 
// ATSn=X - Set radio parameter number ‘n’ to ‘X’ 
sik_atsnx_cmd[], 
sik_rtsnx_cmd[], 
// ATZ - Reboot the radio 
sik_atz_cmd[], 
sik_rtz_cmd[], 
// AT&W - Write current parameters to EEPROM 
sik_atw_cmd[], 
sik_rtw_cmd[], 
// AT&F - Reset all parameters to factory default 
sik_atf_cmd[], 
sik_rtf_cmd[], 
// AT&T=RSSI - Enable RSSI debug reporting 
sik_attrssi_cmd[], 
sik_rttrssi_cmd[], 
// AT&T=TDM - Enable TDM debug reporting 
sik_atttdm_cmd[], 
sik_rtttdm_cmd[], 
// AT&T - Disable debug reporting 
sik_att_cmd[], 
sik_rtt_cmd[]; 

// /**
//  * @brief AT command mode command list 
//  * 
//  * @details 'x' is is to be replaced by 'A' or 'R' depending on if it's an AT or RT 
//  *          command. sik_at_rt_t below can be used to specify the first character. 
//  * 
//  * @see sik_at_rt_t 
//  */
// extern const char 
// sik_xt_enter_cmd[],   // Enter AT/RT command mode 
// sik_xto_cmd[],        // ATO - exit AT command mode 
// sik_xti_cmd[],        // ATI - Show radio version 
// sik_xti2_cmd[],       // ATI2 - Show board type 
// sik_xti3_cmd[],       // ATI3 - Show board frequency 
// sik_xti4_cmd[],       // ATI4 - Show board version 
// sik_xti5_cmd[],       // ATI5 - Show all user settable EEPROM parameters 
// sik_xti6_cmd[],       // ATI6 - Display TDM timing report 
// sik_xti7_cmd[],       // ATI7 - Display RSSI signal report 
// sik_xtsn_cmd[],       // ATSn? - Display radio parameter number ‘n’ 
// sik_xtsnx_cmd[],      // ATSn=X - Set radio parameter number ‘n’ to ‘X’ 
// sik_xtz_cmd[],        // ATZ - Reboot the radio 
// sik_xtw_cmd[],        // AT&W - Write current parameters to EEPROM 
// sik_xtf_cmd[],        // AT&F - Reset all parameters to factory default 
// sik_xttrssi_cmd[],    // AT&T=RSSI - Enable RSSI debug reporting 
// sik_xtttdm_cmd[],     // AT&T=TDM - Enable TDM debug reporting 
// sik_xtt_cmd[];        // AT&T - Disable debug reporting 

//=======================================================================================


//=======================================================================================
// Enums 

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
// Initialization 

/**
 * @brief Sik radio driver initialization 
 * 
 * @param uart : UART port to communicate with 
 */
void sik_init(USART_TypeDef *uart); 

//=======================================================================================


//=======================================================================================
// Read and write 

/**
 * @brief Send data 
 * 
 * @param data : data string to send 
 */
void sik_send_data(const char *data); 

//=======================================================================================


//=======================================================================================
// AT/RT command functions 

/**
 * @brief AT command mode: send string 
 * 
 * @param cmd 
 */
void sik_at_send(const char *cmd); 


/**
 * @brief AT command mode: enter or exit 
 * 
 * @param mode 
 */
void sik_at_mode(sik_at_mode_t mode); 


/**
 * @brief AT command mode: send command 
 * 
 * @param device 
 * @param cmd 
 */
void sik_at_send_cmd(
    sik_at_rt_t device, 
    char *cmd); 


/**
 * @brief AT command mode: get parameter 
 * 
 * @details 
 * 
 * @param param : parameter number 
 */
void sik_at_get_param(sik_at_param_number_t param); 


/**
 * @brief AT command mode: Set parameter 
 * 
 * @details 
 * 
 * @param param : parameter number 
 * @param value : value to set parameter to 
 */
void sik_at_set_param(
    sik_at_param_number_t param, 
    uint32_t value); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _SIK_RADIO_DRIVER_H_ 
