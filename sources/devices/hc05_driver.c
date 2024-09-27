/**
 * @file hc05_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 Bluetooth driver 
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
// Variables 

// HC05 data record 
typedef struct hc05_data_record_s
{
    // Peripherals 
    USART_TypeDef *hc05_uart;        // UART used for communication 
    TIM_TypeDef *timer;              // TIM for delays 

    // Pins 
    GPIO_TypeDef *gpio_at_pin;       // GPIO port for AT Command Mode pin 
    gpio_pin_num_t at_pin;           // Pin for AT Command Mode enable 
    GPIO_TypeDef *gpio_en_pin;       // GPIO port for the enable pin 
    gpio_pin_num_t en_pin;           // Pin for power enable 
    GPIO_TypeDef *gpio_state_pin;    // GPIO for the status feedback pin 
    gpio_pin_num_t state_pin;        // Pin for connection status feedback 

    // Status info 
    // 'status' --> bits 0-8: uart status (see uart_status_t) 
    uint8_t status; 
} 
hc05_data_record_t;


// HC05 data record instance 
hc05_data_record_t hc05_data_record;

//=======================================================================================


//=======================================================================================
// Initialization 

//==============================================================
// Pin information for HC05 GPIOs 
// - Pin 34 (on device) - AT cmd mode trigger 
// - STATE pin - indicates connection status 
// - EN pin (enable) - Turns the device on and off 
//==============================================================

// HC05 initialization 
void hc05_init(
    USART_TypeDef *uart, 
    TIM_TypeDef *timer, 
    GPIO_TypeDef *gpio_at, 
    pin_selector_t at, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en, 
    GPIO_TypeDef *gpio_state, 
    pin_selector_t state) 
{
    // Peripherals 
    hc05_data_record.hc05_uart = uart; 
    hc05_data_record.timer = timer; 

    // AT command mode pin 
    hc05_data_record.gpio_at_pin = gpio_at; 
    hc05_data_record.at_pin = SET_BIT << at; 

    // Enable (EN) pin 
    hc05_data_record.gpio_en_pin = gpio_en; 
    hc05_data_record.en_pin = SET_BIT << en; 

    // State pin 
    hc05_data_record.gpio_state_pin = gpio_state; 
    hc05_data_record.state_pin = SET_BIT << state; 

    // AT Command mode enable 
    gpio_pin_init(gpio_at, at, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(
        hc05_data_record.gpio_at_pin, 
        hc05_data_record.at_pin, 
        (gpio_pin_state_t)HC05_DATA_MODE); 
    
    // Module power enable 
    gpio_pin_init(gpio_en, en, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    hc05_off(); 
    tim_delay_ms(hc05_data_record.timer, HC05_INIT_DELAY); 
    hc05_on(); 
    
    // State feedback enable 
    gpio_pin_init(gpio_state, state, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 

    // Clear the UART data register 
    uart_clear_dr(hc05_data_record.hc05_uart); 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Set EN pin high to turn the device on 
void hc05_on(void)
{
    gpio_write(hc05_data_record.gpio_en_pin, hc05_data_record.en_pin, GPIO_HIGH); 
}


// Set EN pin low to turn the device off 
void hc05_off(void)
{
    gpio_write(hc05_data_record.gpio_en_pin, hc05_data_record.en_pin, GPIO_LOW); 
}


// Send a string of data 
void hc05_send(const char *send_data)
{
    uart_sendstring(hc05_data_record.hc05_uart, send_data); 
}


// Check for available data 
HC05_DATA_STATUS hc05_data_status(void)
{
    return uart_data_ready(hc05_data_record.hc05_uart); 
}


// Read a string of data 
void hc05_read(
    char *receive_data, 
    uint8_t data_len)
{
    // Local variables 
    UART_STATUS uart_status = UART_OK; 

    // Get the data if it's available 
    if (uart_data_ready(hc05_data_record.hc05_uart))
    {
        uart_status |= uart_getstr(hc05_data_record.hc05_uart, 
                                   receive_data, 
                                   data_len, 
                                   UART_STR_TERM_CARRIAGE); 
    }

    // Update the driver status 
    hc05_data_record.status |= (uint8_t)uart_status; 
}


// Read the connection status (STATE pin) 
HC05_CONNECT_STATUS hc05_status(void)
{
    return (HC05_CONNECT_STATUS)gpio_read(hc05_data_record.gpio_state_pin, 
                                          hc05_data_record.state_pin); 
}


// Clear the UART data register 
void hc05_clear(void)
{
    uart_clear_dr(hc05_data_record.hc05_uart); 
}

//=======================================================================================


//=======================================================================================
// Setters and getters 

// Get status flag 
uint8_t hc05_get_status(void)
{
    return hc05_data_record.status; 
}


// Clear status flag 
void hc05_clear_status(void)
{
    hc05_data_record.status = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// AT Command Mode functions 

#if HC05_AT_ENABLE

// Change the module mode 
// void hc05_change_mode(
//     hc05_mode_t mode, 
//     uart_baud_rate_t baud_rate, 
//     uart_clock_speed_t clock_speed)
void hc05_change_mode(
    hc05_mode_t mode, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant)
{
    // Turn the module off 
    hc05_off(); 

    // Set pin 34 on the module depending on the requested mode 
    gpio_write(hc05_data_record.gpio_at_pin, hc05_data_record.at_pin, mode); 

    // Short delay to ensure power off 
    tim_delay_ms(hc05_data_record.timer, HC05_INIT_DELAY); 

    // Configure the baud rate depending on the requested mode 
    // uart_set_baud_rate(hc05_data_record.hc05_uart, baud_rate, clock_speed); 
    uart_set_baud_rate(hc05_data_record.hc05_uart, baud_frac, baud_mant); 

    // Turn the module on 
    hc05_on(); 
}


// HC05 AT Command mode - send AT commands and record responses 
void hc05_at_command(
    hc05_at_commnds_t command, 
    hc05_at_operation_t operation, 
    char *param, 
    char *resp, 
    uint8_t resp_len)
{
    char cmd_str[HC05_AT_CMD_LEN];             // String that holds the AT command 
    char clear_dr[HC05_AT_DR_CLR_LEN];         // String used to clear the DR if needed 
    uint16_t at_timeout = HC05_AT_RESP_COUNT;  // AT cmd response timout counter 

    // Create the command string to send based on the specified AT command 
    switch (command)
    {
        // 1. Test command 
        case HC05_AT_TEST: 
            strcpy(cmd_str, "AT\r\n"); 
            break; 
        
        // 2. Reset 
        case HC05_AT_RESET: 
            strcpy(cmd_str, "AT+RESET\r\n");
            break; 
        
        // 3. Get firmware version
        case HC05_AT_FIRMWARE:  
            strcpy(cmd_str, "AT+VERSION?\r\n");
            break; 
        
        // 4. Restore default 
        case HC05_AT_DEFAULT: 
            strcpy(cmd_str, "AT+ORGL\r\n");
            break; 
        
        // 5. Get module address 
        case HC05_AT_ADDRESS: 
            strcpy(cmd_str, "AT+ADDR?\r\n");
            break; 
        
        // 6. Set/check module name 
        case HC05_AT_MOD_NAME: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+NAME=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+NAME?\r\n"); 
            } 
            break; 
        
        // 7. Get the Bluetooth device name 
        case HC05_AT_DEV_NAME: 
            sprintf(cmd_str, "AT+RNAME?%s\r\n", param); 
            break; 
        
        // 8. Set/check module mode 
        case HC05_AT_MODE:  
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+ROLE=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+ROLE?\r\n"); 
            }
            break; 
        
        // 9. Set/check device class 
        case HC05_AT_CLASS:  
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+CLASS=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+CLASS?\r\n"); 
            }
            break; 
        
        // 10. Set/check GIAC (General Inquire Access Code) 
        case HC05_AT_GIAC:  
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+IAC=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+IAC?\r\n");
            }
            break; 
        
        // 11. Set/check query access patterns 
        case HC05_AT_QUERY: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+INQM=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+INQM?\r\n");
            }
            break; 
        
        // 12. Set/check pin code 
        case HC05_AT_PIN: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+PSWD=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+PSWD?\r\n");
            }
            break; 
        
        // 13. Set/check serial parameter 
        case HC05_AT_SERIAL: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+UART=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+UART?\r\n");
            }
            break; 
        
        // 14. Set/check connect mode 
        case HC05_AT_CONNECT: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+CMODE=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+CMODE?\r\n");
            }
            break; 
        
        // 15. Set/check fixed address 
        case HC05_AT_FIXED: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+BIND=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+BIND?\r\n");
            }
            break; 
        
        // 16. Set/check LED IO 
        case HC05_AT_LED: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+POLAR=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+POLAR?\r\n");
            }
            break; 
        
        // 17. Set PIO output 
        case HC05_AT_PIO: 
            sprintf(cmd_str, "AT+PIO=%s\r\n", param); 
            break; 
        
        // 18. Set/check scan parameter 
        case HC05_AT_SCAN: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+IPSCAN=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+IPSCAN?\r\n");
            }
            break; 
        
        // 19. Set/check SNIFF parameter 
        case HC05_AT_SNIFF:  
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+SNIFF=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+SNIFF?\r\n");
            }
            break; 
        
        // 20. Set/check security mode 
        case HC05_AT_SECURITY: 
            if (operation == HC05_SET) 
            {
                sprintf(cmd_str, "AT+SENM=%s\r\n", param); 
            }
            else if (operation == HC05_CHECK) 
            {
                strcpy(cmd_str, "AT+SENM?\r\n");
            }
            break; 
        
        // 21. Delete authentication device 
        case HC05_AT_DELETE:  
            sprintf(cmd_str, "AT+PMSAD=%s\r\n", param); 
            break; 
        
        // 22. Delete all authentication device 
        case HC05_AT_DELETE_ALL: 
            strcpy(cmd_str, "AT+RMAAD\r\n");
            break; 
        
        // 23. Search authentication device 
        case HC05_AT_SEARCH:  
            sprintf(cmd_str, "AT+FSAD=%s\r\n", param); 
            break; 
        
        // 24. Get authentication device count 
        case HC05_AT_COUNT: 
            strcpy(cmd_str, "AT+ADCN?\r\n");
            break; 
        
        // 25. Most recently used authenticated device 
        case HC05_AT_RECENT: 
            strcpy(cmd_str, "AT+MRAD?\r\n");
            break; 
        
        // 26. Get the module working state 
        case HC05_AT_STATE: 
            strcpy(cmd_str, "AT+STATE?\r\n");
            break; 
        
        // 27. Initialze the SPP profile lib 
        case HC05_AT_SPP: 
            strcpy(cmd_str, "AT+INIT\r\n");
            break; 
        
        // 28. Inquiry Bluetooth device 
        case HC05_AT_INQUIRY: 
            strcpy(cmd_str, "AT+INQ\r\n");
            break; 
        
        // 29. Cancel inquiry Bluetooth device 
        case HC05_AT_CANCEL: 
            strcpy(cmd_str, "AT+INQC\r\n");
            break; 
        
        // 30. Equipment matching 
        case HC05_AT_MATCH: 
            sprintf(cmd_str, "AT+PAIR=%s\r\n", param); 
            break; 
        
        // 31. Connect device 
        case HC05_AT_CONN_DEV: 
            sprintf(cmd_str, "AT+LINK=%s\r\n", param); 
            break; 
        
        // 32. Disconnect 
        case HC05_AT_DISCONNECT: 
            strcpy(cmd_str, "AT+DISC\r\n");
            break; 
        
        // 33. Energy saving mode 
        case HC05_AT_SAVING: 
            sprintf(cmd_str, "AT+ENSNIFF=%s\r\n", param); 
            break; 
        
        // 34. Exerts energy saving mode 
        case HC05_AT_EXERT: 
            sprintf(cmd_str, "AT+EXSNIFF=%s\r\n", param); 
            break; 
        
        default:
            strcpy(resp, "Invalid command\r\n");
            return; 
    }

    // Clear the data register before looking for actual data 
    hc05_clear(); 

    // Send the AT command to the module 
    hc05_send(cmd_str); 

    // Wait for data to be sent back until timeout 
    do 
    {
        if (hc05_data_status()) 
        {
            // Read the module response 
            uart_getstr(
                hc05_data_record.hc05_uart, 
                resp, 
                resp_len, 
                UART_STR_TERM_NL); 

            // If a cmd response was received then clear the "OK\r\n" from the DR that follows 
            if (*resp == HC05_AT_RESP_STR) 
            {
                uart_getstr(
                    hc05_data_record.hc05_uart, 
                    clear_dr, 
                    HC05_AT_DR_CLR_LEN, 
                    UART_STR_TERM_NL);
            }

            break; 
        }

        // AT mode doesn't run in real time so blocking is ok 
        tim_delay_us(hc05_data_record.timer, DELAY_2US); 
    }
    while (--at_timeout); 

    if (!at_timeout) 
    {
        strcpy(resp, "Timeout\r\n");  // No response seen 
    }
}

#endif   // HC05_AT_ENABLE

//=======================================================================================
