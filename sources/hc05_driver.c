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

// This mode is designed to be used along with serial terminal input/output with 
// the help of uart2 

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
        case HC05_AT_TEST:  // 1. Test command 
            strcpy(cmd_str, "AT\r\n"); 
            break; 
        
        case HC05_AT_RESET:  // 2. Reset 
            strcpy(cmd_str, "AT+RESET\r\n");
            break; 
        
        case HC05_AT_FIRMWARE:  // 3. Get firmware version
            strcpy(cmd_str, "AT+VERSION?\r\n");
            break; 
        
        case HC05_AT_DEFAULT:  // 4. Restore default 
            strcpy(cmd_str, "AT+ORGL\r\n");
            break; 
        
        case HC05_AT_ADDRESS:  // 5. Get module address 
            strcpy(cmd_str, "AT+ADDR?\r\n");
            break; 
        
        case HC05_AT_MOD_NAME:  // 6. Set/check module name 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+NAME=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+NAME?\r\n"); 
            break; 
        
        case HC05_AT_DEV_NAME:  // 7. Get the Bluetooth device name 
            // TODO Get the module address first? 
            strcpy(cmd_str, "AT+\r\n");
            break; 
        
        case HC05_AT_MODE:  // 8. Set/check module mode 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+ROLE=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+ROLE?\r\n"); 
            break; 
        
        case HC05_AT_CLASS:  // 9. Set/check device class 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+CLASS=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+CLASS?\r\n"); 
            break; 
        
        case HC05_AT_GIAC:  // 10. Set/check GIAC (General Inquire Access Code) 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+IAC=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+IAC?\r\n");
            break; 
        
        case HC05_AT_QUERY:  // 11. Set/check query access patterns 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+INQM=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+INQM?\r\n");
            break; 
        
        case HC05_AT_PIN:  // 12. Set/check pin code 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+PSWD=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+PSWD?\r\n");
            break; 
        
        case HC05_AT_SERIAL:  // 13. Set/check serial parameter 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+UART=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+UART?\r\n");
            break; 
        
        case HC05_AT_CONNECT:  // 14. Set/check connect mode 
            strcpy(cmd_str, "AT+\r\n");
            break; 
        
        case HC05_AT_FIXED:  // 15. Set/check fixed address 
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
