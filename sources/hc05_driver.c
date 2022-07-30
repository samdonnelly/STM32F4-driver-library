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
            sprintf(cmd_str, "AT+RNAME?%s\r\n", param); 
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
            if (operation == HC05_SET) sprintf(cmd_str, "AT+CMODE=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+CMODE?\r\n");
            break; 
        
        case HC05_AT_FIXED:  // 15. Set/check fixed address 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+BIND=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+BIND?\r\n");
            break; 
        
        case HC05_AT_LED:  // 16. Set/check LED IO 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+POLAR=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+POLAR?\r\n");
            break; 
        
        case HC05_AT_PIO:  // 17. Set PIO output 
            sprintf(cmd_str, "AT+PIO=%s\r\n", param); 
            break; 
        
        case HC05_AT_SCAN:  // 18. Set/check scan parameter 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+IPSCAN=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+IPSCAN?\r\n");
            break; 
        
        case HC05_AT_SNIFF:  // 19. Set/check SNIFF parameter 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+SNIFF=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+SNIFF?\r\n");
            break; 
        
        case HC05_AT_SECURITY:  // 20. Set/check security mode 
            if (operation == HC05_SET) sprintf(cmd_str, "AT+SENM=%s\r\n", param); 
            else if (operation == HC05_CHECK) strcpy(cmd_str, "AT+SENM?\r\n");
            break; 
        
        case HC05_AT_DELETE:  // 21. Delete authentication device 
            sprintf(cmd_str, "AT+PMSAD=%s\r\n", param); 
            break; 
        
        case HC05_AT_DELETE_ALL:  // 22. Delete all authentication device 
            strcpy(cmd_str, "AT+RMAAD\r\n");
            break; 
        
        case HC05_AT_SEARCH:  // 23. Search authentication device 
            sprintf(cmd_str, "AT+FSAD=%s\r\n", param); 
            break; 
        
        case HC05_AT_COUNT:  // 24. Get authentication device count 
            strcpy(cmd_str, "AT+ADCN?\r\n");
            break; 
        
        case HC05_AT_RECENT:  // 25. Most recently used authenticated device 
            strcpy(cmd_str, "AT+MRAD?\r\n");
            break; 
        
        case HC05_AT_STATE:  // 26. Get the module working state 
            strcpy(cmd_str, "AT+STATE?\r\n");
            break; 
        
        case HC05_AT_SPP:  // 27. Initialze the SPP profile lib 
            strcpy(cmd_str, "AT+INIT\r\n");
            break; 
        
        case HC05_AT_INQUIRY:  // 28. Inquiry Bluetooth device 
            strcpy(cmd_str, "AT+INQ\r\n");
            break; 
        
        case HC05_AT_CANCEL:  // 29. Cancel inquiry Bluetooth device 
            strcpy(cmd_str, "AT+INQC\r\n");
            break; 
        
        case HC05_AT_MATCH:  // 30. Equipment matching 
            sprintf(cmd_str, "AT+PAIR=%s\r\n", param); 
            break; 
        
        case HC05_AT_CONN_DEV:  // 31. Connect device 
            sprintf(cmd_str, "AT+LINK=%s\r\n", param); 
            break; 
        
        case HC05_AT_DISCONNECT:  // 32. Disconnect 
            strcpy(cmd_str, "AT+DISC\r\n");
            break; 
        
        case HC05_AT_SAVING:  // 33. Energy saving mode 
            sprintf(cmd_str, "AT+ENSNIFF=%s\r\n", param); 
            break; 
        
        case HC05_AT_EXERT:  // 34. Exerts energy saving mode 
            sprintf(cmd_str, "AT+EXSNIFF=%s\r\n", param); 
            break; 
        default:
            break; 
    }
}

#endif  // HC05_AT_CMD_MODE

//=======================================================================================
