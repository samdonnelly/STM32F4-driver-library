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

// Drivers 
#include "hc05_driver.h"

// Libraries 
#if HC05_AT_CMD_MODE
#include "string.h"
#include <stdio.h>
#endif  // HC05_AT_CMD_MODE

//=======================================================================================


//=======================================================================================
// Initialization 



//=======================================================================================


//=======================================================================================
// AT Command Mode 

#if HC05_AT_CMD_MODE

// TODO add HC-05 standard configs 

// 
void hc05_at_command(
    AT_CMD command, 
    AT_OPR operation, 
    char *param, 
    char *response)
{
    // Local variables 
    char cmd_str[HC05_AT_CMD_LEN]; 

    // Command to send 
    // TODO test if each string works 
    switch (command)
    {
        case HC05_AT_TEST:
            strcpy(cmd_str, "AT\r\n"); 
            break; 
        case HC05_AT_RESET:
            strcpy(cmd_str, "AT+RESET\r\n");
            break; 
        case HC05_AT_FIRMWARE:
            strcpy(cmd_str, "AT+VERSION?\r\n");
            break; 
        case HC05_AT_DEFAULT:
            strcpy(cmd_str, "AT+ORGL\r\n");
            break; 
        case HC05_AT_ADDRESS:
            strcpy(cmd_str, "AT+ADDR?\r\n");
            break; 
        case HC05_AT_MOD_NAME:
            if (operation == HC05_SET) sprintf(cmd_str, "AT+NAME=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+NAME?\r\n"); 
            break; 
        case HC05_AT_DEV_NAME:
            // TODO Get the module address first? 
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_MODE:
             if (operation == HC05_SET) sprintf(cmd_str, "AT+ROLE=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+ROLE?\r\n"); 
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_CLASS:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_GIAC:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_QUERY:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_PIN:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SERIAL:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_CONNECT:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_FIXED:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_LED:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_PIO:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SCAN:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SHIFF:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SECURITY:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_DELETE:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_DELETE_ALL:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SEARCH:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_COUNT:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_RECENT:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_STATE:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SPP:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_INQUIRY:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_CANCEL:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_MATCH:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_CONN_DEV:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_DISCONNECT:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_SAVING:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        case HC05_AT_EXERT:
            strcpy(cmd_str, "AT+\r\n");
            break; 
        default:
            break; 
    }
}

#endif  // HC05_AT_CMD_MODE

//=======================================================================================
