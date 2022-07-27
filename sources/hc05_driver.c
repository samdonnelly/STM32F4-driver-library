/**
 * @file hc05_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC-05 bluetooth driver 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

#include "hc05_driver.h"

//=======================================================================================


//=======================================================================================
// Initialization 



//=======================================================================================


//=======================================================================================
// AT Command Mode 

#if HC05_AT_CMD_MODE

// 
void hc05_at_command(AT_CMD command, char *response)
{
    // Local variables 
    // TODO use strcpy to put command string into this buffer 
    char cmd_str[HC05_AT_CMD_LEN]; 

    // Command to send 
    switch (command)
    {
        case HC05_AT_TEST:
            break; 
        case HC05_AT_RESET:
            break; 
        case HC05_AT_FIRMWARE:
            break; 
        case HC05_AT_DEFAULT:
            break; 
        case HC05_AT_ADDRESS:
            break; 
        case HC05_AT_MOD_NAME:
            break; 
        case HC05_AT_DEV_NAME:
            break; 
        case HC05_AT_MODE:
            break; 
        case HC05_AT_CLASS:
            break; 
        case HC05_AT_GIAC:
            break; 
        case HC05_AT_QUERY:
            break; 
        case HC05_AT_PIN:
            break; 
        case HC05_AT_SERIAL:
            break; 
        case HC05_AT_CONNECT:
            break; 
        case HC05_AT_FIXED:
            break; 
        case HC05_AT_LED:
            break; 
        case HC05_AT_PIO:
            break; 
        case HC05_AT_SCAN:
            break; 
        case HC05_AT_SHIFF:
            break; 
        case HC05_AT_SECURITY:
            break; 
        case HC05_AT_DELETE:
            break; 
        case HC05_AT_DELETE_ALL:
            break; 
        case HC05_AT_SEARCH:
            break; 
        case HC05_AT_COUNT:
            break; 
        case HC05_AT_RECENT:
            break; 
        case HC05_AT_STATE:
            break; 
        case HC05_AT_SPP:
            break; 
        case HC05_AT_INQUIRY:
            break; 
        case HC05_AT_CANCEL:
            break; 
        case HC05_AT_MATCH:
            break; 
        case HC05_AT_CONN_DEV:
            break; 
        case HC05_AT_DISCONNECT:
            break; 
        case HC05_AT_SAVING:
            break; 
        case HC05_AT_EXERT:
            break; 
        default:
            break; 
    }
}

#endif  // HC05_AT_CMD_MODE

//=======================================================================================
