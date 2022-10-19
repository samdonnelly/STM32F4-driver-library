/**
 * @file hc05_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC-05 Bluetooth driver 
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
#if HC05_AT_EN
#include <string.h>
#include <stdio.h>
#endif  // HC05_AT_EN

//=======================================================================================

// TODO Device driver todo's: 
//  - Make the code able to support multiple devices (linked list) 

// TODO Control driver todo's: 
//  - Make sure data transfer is complete before turning off the module 
//  - Verify the state pin shows connected before any data transfer 
//  - When about to send data (to Android) look for a prompt message to start 

//=======================================================================================
// Variables 

// Module info 
hc05_mod_info_t hc05_module;

//=======================================================================================


//=======================================================================================
// Initialization 

// HC-05 initialization 
void hc05_init(
    USART_TypeDef *uart, 
    hc05_pin34_status_t pin34_status,
    hc05_en_status_t    en_status, 
    hc05_state_status_t state_status)
{
    //==============================================================
    // Pin information for HC-05 GPIOs 
    //  PA8:  pin 34 (AT cmd mode trigger)
    //  PA11: STATE 
    //  PA12: EN (enable) 
    //==============================================================

    // Initialize module info
    hc05_module.hc05_uart = uart; 
    hc05_module.at_pin = GPIOX_PIN_8; 
    hc05_module.en_pin = GPIOX_PIN_12; 
    hc05_module.state_pin = GPIOX_PIN_11;

    // AT Command mode enable 
    if (pin34_status) 
    {
        gpio_pin_init(GPIOA, PIN_8, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
        gpio_write(GPIOA, hc05_module.at_pin, GPIO_LOW); 
    }
    
    // Module power enable 
    if (en_status) 
    {
        gpio_pin_init(GPIOA, PIN_12, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
        hc05_pwr_off(); 
        tim9_delay_ms(HC05_INIT_DELAY); 
        hc05_pwr_on(); 
    }
    
    // State feedback enable 
    if (state_status) gpio_pin_init(GPIOA, PIN_11, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
}

//=======================================================================================


//=======================================================================================
// Power functions 

// Set EN pin to high to turn on the module 
void hc05_pwr_on(void)
{
    gpio_write(GPIOA, hc05_module.en_pin, GPIO_HIGH); 
}


// Set EN pin to low to turn off the module 
void hc05_pwr_off(void)
{
    gpio_write(GPIOA, hc05_module.en_pin, GPIO_LOW); 
}

//=======================================================================================


//=======================================================================================
// Transition functions 

#if HC05_AT_EN 

// Change the module mode 
void hc05_change_mode(
    hc05_mode_t mode, 
    uart_baud_rate_t baud_rate, 
    uart_clock_speed_t clock_speed)
{
    // Turn the module off 
    hc05_pwr_off(); 

    // Set pin 34 on the module depending on the requested mode 
    gpio_write(GPIOA, hc05_module.at_pin, mode); 

    // Short delay to ensure power off
    tim9_delay_ms(HC05_INIT_DELAY); 

    // Configure the baud rate depending on the requested mode 
    uart_set_baud_rate(hc05_module.hc05_uart, baud_rate, clock_speed);  

    // Turn the module on 
    hc05_pwr_on(); 
}

#endif  // HC05_AT_CMD_MODE

//=======================================================================================


//=======================================================================================
// Mode functions 

// HC-05 data mode - send data 
void hc05_data_mode_send(char *send_data)
{
    // TODO should have the ability to also send digits if desired 
    uart_sendstring(hc05_module.hc05_uart, send_data); 
}


// HC-05 data mode - read data 
void hc05_data_mode_receive(char *receive_data)
{
    uart_getstr(hc05_module.hc05_uart, receive_data, UART_STR_TERM_NL); 
}


#if HC05_AT_EN

// HC-05 AT Command mode - send AT commands and record responses 
void hc05_at_command(
    hc05_at_commnds_t command, 
    hc05_at_operation_t operation, 
    char *param, 
    char *response)
{
    // Local variables 
    char cmd_str[HC05_AT_CMD_LEN];             // String that holds the AT command 
    char clear_dr[HC05_AT_DR_CLR_LEN];         // String used to clear the DR if needed 
    uint16_t at_timeout = HC05_AT_RESP_COUNT;  // AT cmd response timout counter 

    // Create the command string to send based on the specified AT command 
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
            strcpy(response, "Invalid command\r\n");
            return; 
    }

    // Clear the data register before looking for actual data 
    uart_clear_dr(hc05_module.hc05_uart); 

    // Send the AT command to the module 
    uart_sendstring(hc05_module.hc05_uart, cmd_str); 

    // Wait for data to be sent back until timeout 
    do 
    {
        if (hc05_module.hc05_uart->SR & (SET_BIT << SHIFT_5)) 
        {
            // Read the module response 
            uart_getstr(hc05_module.hc05_uart, response, UART_STR_TERM_NL); 

            // If a cmd response was received then clear the "OK\r\n" from the DR that follows 
            if (*response == HC05_AT_RESP_STR) 
                uart_getstr(hc05_module.hc05_uart, clear_dr, UART_STR_TERM_NL);

            break; 
        }
        tim9_delay_us(TIM9_2US);  // AT mode doesn't run in real time so blocking is ok 
    }
    while (--at_timeout); 

    if (!at_timeout) strcpy(response, "Timeout\r\n");  // No response seen 
}

#endif  // HC05_AT_CMD_MODE

//=======================================================================================
