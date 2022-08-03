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
#include "uart_comm.h"

// Libraries 
#if HC05_AT_CMD_MODE
#include "string.h"
#include <stdio.h>
#endif  // HC05_AT_CMD_MODE

//=======================================================================================


//=======================================================================================
// Initialization 

// TODO initialize GPIOs 

//=======================================================================================

// TODO change the UART buad rate for AT command mode 
// TODO add GPIO output and input (voltaile pin read) control 

//=======================================================================================
// AT Command Mode 

#if HC05_AT_CMD_MODE

// Test 2 - UART string format and send verification 
// Test 3 - UART string receive verification and command response verification 

// This code will be used once to configure the module as needed then only the data mode 
// will be used. 

// TODO add HC-05 standard configs 

// Send AT commands and record responses 
void hc05_at_command(
    AT_CMD command, 
    AT_OPR operation, 
    char *param, 
    char *response)
{
    // Local variables 

    // Command to send 
    switch (command)
    {
        case HC05_AT_TEST:  // 1. Test command 
            strcpy(response, "AT\r\n"); 
            break; 
        
        case HC05_AT_RESET:  // 2. Reset 
            strcpy(response, "AT+RESET\r\n");
            break; 
        
        case HC05_AT_FIRMWARE:  // 3. Get firmware version
            strcpy(response, "AT+VERSION?\r\n");
            break; 
        
        case HC05_AT_DEFAULT:  // 4. Restore default 
            strcpy(response, "AT+ORGL\r\n");
            break; 
        
        case HC05_AT_ADDRESS:  // 5. Get module address 
            strcpy(response, "AT+ADDR?\r\n");
            break; 
        
        case HC05_AT_MOD_NAME:  // 6. Set/check module name 
            if (operation == HC05_SET) sprintf(response, "AT+NAME=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+NAME?\r\n"); 
            break; 
        
        case HC05_AT_DEV_NAME:  // 7. Get the Bluetooth device name 
            sprintf(response, "AT+RNAME?%s\r\n", param); 
            break; 
        
        case HC05_AT_MODE:  // 8. Set/check module mode 
            if (operation == HC05_SET) sprintf(response, "AT+ROLE=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+ROLE?\r\n"); 
            break; 
        
        case HC05_AT_CLASS:  // 9. Set/check device class 
            if (operation == HC05_SET) sprintf(response, "AT+CLASS=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+CLASS?\r\n"); 
            break; 
        
        case HC05_AT_GIAC:  // 10. Set/check GIAC (General Inquire Access Code) 
            if (operation == HC05_SET) sprintf(response, "AT+IAC=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+IAC?\r\n");
            break; 
        
        case HC05_AT_QUERY:  // 11. Set/check query access patterns 
            if (operation == HC05_SET) sprintf(response, "AT+INQM=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+INQM?\r\n");
            break; 
        
        case HC05_AT_PIN:  // 12. Set/check pin code 
            if (operation == HC05_SET) sprintf(response, "AT+PSWD=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+PSWD?\r\n");
            break; 
        
        case HC05_AT_SERIAL:  // 13. Set/check serial parameter 
            if (operation == HC05_SET) sprintf(response, "AT+UART=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+UART?\r\n");
            break; 
        
        case HC05_AT_CONNECT:  // 14. Set/check connect mode 
            if (operation == HC05_SET) sprintf(response, "AT+CMODE=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+CMODE?\r\n");
            break; 
        
        case HC05_AT_FIXED:  // 15. Set/check fixed address 
            if (operation == HC05_SET) sprintf(response, "AT+BIND=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+BIND?\r\n");
            break; 
        
        case HC05_AT_LED:  // 16. Set/check LED IO 
            if (operation == HC05_SET) sprintf(response, "AT+POLAR=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+POLAR?\r\n");
            break; 
        
        case HC05_AT_PIO:  // 17. Set PIO output 
            sprintf(response, "AT+PIO=%s\r\n", param); 
            break; 
        
        case HC05_AT_SCAN:  // 18. Set/check scan parameter 
            if (operation == HC05_SET) sprintf(response, "AT+IPSCAN=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+IPSCAN?\r\n");
            break; 
        
        case HC05_AT_SNIFF:  // 19. Set/check SNIFF parameter 
            if (operation == HC05_SET) sprintf(response, "AT+SNIFF=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+SNIFF?\r\n");
            break; 
        
        case HC05_AT_SECURITY:  // 20. Set/check security mode 
            if (operation == HC05_SET) sprintf(response, "AT+SENM=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(response, "AT+SENM?\r\n");
            break; 
        
        case HC05_AT_DELETE:  // 21. Delete authentication device 
            sprintf(response, "AT+PMSAD=%s\r\n", param); 
            break; 
        
        case HC05_AT_DELETE_ALL:  // 22. Delete all authentication device 
            strcpy(response, "AT+RMAAD\r\n");
            break; 
        
        case HC05_AT_SEARCH:  // 23. Search authentication device 
            sprintf(response, "AT+FSAD=%s\r\n", param); 
            break; 
        
        case HC05_AT_COUNT:  // 24. Get authentication device count 
            strcpy(response, "AT+ADCN?\r\n");
            break; 
        
        case HC05_AT_RECENT:  // 25. Most recently used authenticated device 
            strcpy(response, "AT+MRAD?\r\n");
            break; 
        
        case HC05_AT_STATE:  // 26. Get the module working state 
            strcpy(response, "AT+STATE?\r\n");
            break; 
        
        case HC05_AT_SPP:  // 27. Initialze the SPP profile lib 
            strcpy(response, "AT+INIT\r\n");
            break; 
        
        case HC05_AT_INQUIRY:  // 28. Inquiry Bluetooth device 
            strcpy(response, "AT+INQ\r\n");
            break; 
        
        case HC05_AT_CANCEL:  // 29. Cancel inquiry Bluetooth device 
            strcpy(response, "AT+INQC\r\n");
            break; 
        
        case HC05_AT_MATCH:  // 30. Equipment matching 
            sprintf(response, "AT+PAIR=%s\r\n", param); 
            break; 
        
        case HC05_AT_CONN_DEV:  // 31. Connect device 
            sprintf(response, "AT+LINK=%s\r\n", param); 
            break; 
        
        case HC05_AT_DISCONNECT:  // 32. Disconnect 
            strcpy(response, "AT+DISC\r\n");
            break; 
        
        case HC05_AT_SAVING:  // 33. Energy saving mode 
            sprintf(response, "AT+ENSNIFF=%s\r\n", param); 
            break; 
        
        case HC05_AT_EXERT:  // 34. Exerts energy saving mode 
            sprintf(response, "AT+EXSNIFF=%s\r\n", param); 
            break; 
        
        default:
            strcpy(response, "Invalid command\r\n");
            break; 
    }
}

#endif  // HC05_AT_CMD_MODE

//=======================================================================================
