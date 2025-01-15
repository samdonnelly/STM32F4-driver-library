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
// Macros 

#define SIK_MAX_AT_CMD_SIZE 20 

//=======================================================================================


//=======================================================================================
// AT/RT command definitions 

// "AT" can be replaced by "RT" for use on a connected remote radio with the exception of 
// "ATO". 

const char 
sik_at_enter_cmd[] = "+++",         // Enter AT/RT command mode 
sik_ato_cmd[] = "ATO",              // ATO - exit AT command mode 
sik_xti_cmd[] = "%cTI",             // xTI - Show radio version 
sik_xti2_cmd[] = "%cTI2",           // xTI2 - Show board type 
sik_xti3_cmd[] = "%cTI3",           // xTI3 - Show board frequency 
sik_xti4_cmd[] = "%cTI4",           // xTI4 - Show board version 
sik_xti5_cmd[] = "%cTI5",           // xTI5 - Show all user settable EEPROM parameters 
sik_xti6_cmd[] = "%cTI6",           // xTI6 - Display TDM timing report 
sik_xti7_cmd[] = "%cTI7",           // xTI7 - Display RSSI signal report 
sik_xtsn_cmd[] = "%cTS%u?",         // xTSn? - Display radio parameter number ‘n’ 
sik_xtsnx_cmd[] = "%cTS%u=%lu",     // xTSn=X - Set radio parameter number ‘n’ to ‘X’ 
sik_xtz_cmd[] = "%cTZ",             // xTZ - Reboot the radio 
sik_xtw_cmd[] = "%cT&W",            // xT&W - Write current parameters to EEPROM 
sik_xtf_cmd[] = "%cT&F",            // xT&F - Reset all parameters to factory default 
sik_xttrssi_cmd[] = "%cT&T=RSSI",   // xT&T=RSSI - Enable RSSI debug reporting 
sik_xtttdm_cmd[] = "%cT&T=TDM",     // xT&T=TDM - Enable TDM debug reporting 
sik_xtt_cmd[] = "%cT&T";            // xT&T - Disable debug reporting 

//=======================================================================================


//=======================================================================================
// Global data 

// Driver data record 
typedef struct sik_driver_data_s 
{
    USART_TypeDef *uart; 
    char at_cmd_buff[SIK_MAX_AT_CMD_SIZE]; 
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
    memset((void *)sik_driver_data.at_cmd_buff, CLEAR, SIK_MAX_AT_CMD_SIZE); 
    sik_driver_data.at_mode = CLEAR_BIT; 
}

//=======================================================================================


//=======================================================================================
// Read and write functions 

// Read data 
void sik_read_data(char *read_data)
{
    while ((read_data != NULL) && uart_data_ready(sik_driver_data.uart))
    {
        // Add a UART function for reading until no more data is seen (i.e. not uing a 
        // terminating character). 
    }
}


// Send data 
void sik_send_data(const char *send_data)
{
    if (send_data != NULL)
    {
        uart_sendstring(sik_driver_data.uart, send_data); 
    }
}

//=======================================================================================


//=======================================================================================
// AT Command Functions 

// AT command mode: enter or exit 
void sik_at_mode(sik_at_mode_t mode)
{
    switch (mode)
    {
        case SIK_AT_ENTER: 
            sik_send_data(sik_at_enter_cmd); 
            sik_driver_data.at_mode = SET_BIT; 
            break; 

        case SIK_AT_EXIT: 
            sik_send_data(sik_ato_cmd); 
            sik_driver_data.at_mode = CLEAR_BIT; 
        
        default:
            break;
    }
}


// AT command mode: send command 
void sik_at_send_cmd(
    sik_at_rt_t device, 
    const char *cmd)
{
    if (cmd != NULL)
    {
        // snprintf was not used here because the compiler gave a warning that "cmd" 
        // is not a string literal. 
        memcpy((void *)sik_driver_data.at_cmd_buff, (const void *)cmd, sizeof(cmd)); 
        sik_driver_data.at_cmd_buff[BYTE_0] = (char)device; 
        sik_send_data(sik_driver_data.at_cmd_buff); 
    }
}


// AT command mode: get parameter 
void sik_at_get_param(
    sik_at_rt_t device, 
    sik_at_param_number_t param)
{
    // Copy the "xTSn?" command to the buffer but replace 'x' and 'n' with the device 
    // type and parameter number, respectfully. 

    snprintf(sik_driver_data.at_cmd_buff, 
             SIK_MAX_AT_CMD_SIZE, 
             sik_xtsn_cmd, 
             (char)device, 
             (uint8_t)param); 

    sik_send_data(sik_driver_data.at_cmd_buff); 
}


// AT command mode: set parameter 
void sik_at_set_param(
    sik_at_rt_t device, 
    sik_at_param_number_t param, 
    uint32_t value)
{
    // Copy the "xTSn=X" command to the buffer but replace 'x', 'n' and 'X' with the 
    // device type, parameter number and parameter value, respectfully. 

    snprintf(sik_driver_data.at_cmd_buff, 
             SIK_MAX_AT_CMD_SIZE, 
             sik_xtsnx_cmd, 
             (char)device, 
             (uint8_t)param, 
             value); 
    
    sik_send_data(sik_driver_data.at_cmd_buff); 
}

//=======================================================================================
