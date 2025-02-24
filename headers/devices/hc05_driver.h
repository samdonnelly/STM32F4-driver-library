/**
 * @file hc05_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 bluetooth driver interface 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HC05_DRIVER_H_
#define _HC05_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "uart_comm.h"
#include "gpio_driver.h"

// Other drivers 
#include "timers_driver.h"

// Libraries 
#include <stdio.h>

//=======================================================================================


//=======================================================================================
// Macros 

// HC05 paramters 
#define HC05_INIT_DELAY   100          // ms delay to ensure full power cycle 

// AT Command Mode 
#define HC05_AT_ENABLE      0          // Controls the inclusion of AT command mode code 
#define HC05_AT_CMD_LEN     30         // Max length of command string 
#define HC05_AT_DR_CLR_LEN  4          // Length of "OK\r\n" - follows an AT parameter response 
#define HC05_AT_RESP_STR    43         // 43 == '+' - start of a parmeter response 
#define HC05_AT_RESP_COUNT  65535      // Timout counter to receeive an AT cmd response 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HC05 mode 
 * 
 * @details The HC05 module has two modes: Data mode (default) and AT Command mode. This 
 *          enum is passed as an argument to hc05_change_mode to indicate which mode to 
 *          put the module into. 
 * 
 * @see hc05_change_mode
 */
typedef enum {
    HC05_DATA_MODE, 
    HC05_AT_CMD_MODE
} hc05_mode_t; 


/**
 * @brief Device connection status 
 */
typedef enum {
    HC05_NOT_CONNECTED,       // Not connected to an external device 
    HC05_CONNECTED            // Connected to an external device 
} hc05_connect_status_t; 


/**
 * @brief HC05 AT Commands 
 * 
 * @details This enum contains all the AT Command mode commands used to configure the module. 
 *          These values are used in the hc05_at_command function to match the users requested 
 *          command and generate a command string that gets sent to the module. See the HC05 
 *          documentation for more information on AT commands. 
 * 
 * @see hc05_at_command
 */
typedef enum {
    HC05_AT_TEST = 1,     // 1. Test command 
    HC05_AT_RESET,        // 2. Reset 
    HC05_AT_FIRMWARE,     // 3. Get firmware version 
    HC05_AT_DEFAULT,      // 4. Restore default 
    HC05_AT_ADDRESS,      // 5. Get module address 
    HC05_AT_MOD_NAME,     // 6. Set/check module name 
    HC05_AT_DEV_NAME,     // 7. Get the Bluetooth device name 
    HC05_AT_MODE,         // 8. Set/check module mode 
    HC05_AT_CLASS,        // 9. Set/check device class 
    HC05_AT_GIAC,         // 10. Set/check GIAC (General Inquire Access Code) 
    HC05_AT_QUERY,        // 11. Set/check query access patterns 
    HC05_AT_PIN,          // 12. Set/check pin code 
    HC05_AT_SERIAL,       // 13. Set/check serial parameter 
    HC05_AT_CONNECT,      // 14. Set/check connect mode 
    HC05_AT_FIXED,        // 15. Set/check fixed address 
    HC05_AT_LED,          // 16. Set/check LED IO 
    HC05_AT_PIO,          // 17. Set PIO output 
    HC05_AT_SCAN,         // 18. Set/check scan parameter 
    HC05_AT_SNIFF,        // 19. Set/check SNIFF parameter 
    HC05_AT_SECURITY,     // 20. Set/check security mode 
    HC05_AT_DELETE,       // 21. Delete authentication device 
    HC05_AT_DELETE_ALL,   // 22. Delete all authentication device 
    HC05_AT_SEARCH,       // 23. Search authentication device 
    HC05_AT_COUNT,        // 24. Get authentication device count 
    HC05_AT_RECENT,       // 25. Most recently used authenticated device 
    HC05_AT_STATE,        // 26. Get the module working state 
    HC05_AT_SPP,          // 27. Initialze the SPP profile lib 
    HC05_AT_INQUIRY,      // 28. Inquiry Bluetooth device 
    HC05_AT_CANCEL,       // 29. Cancel inquiry Bluetooth device 
    HC05_AT_MATCH,        // 30. Equipment matching 
    HC05_AT_CONN_DEV,     // 31. Connect device 
    HC05_AT_DISCONNECT,   // 32. Disconnect 
    HC05_AT_SAVING,       // 33. Energy saving mode 
    HC05_AT_EXERT         // 34. Exerts energy saving mode 
} hc05_at_commnds_t; 


/**
 * @brief HC05 AT command operation 
 * 
 * @details This enum is an argument passed to the hc05_at_command function to indicate whether 
 *          the user wants to set a parameter, check a parameter, or do nothing (in cases where
 *          there is only the ability to check). 
 * 
 * @see hc05_at_command
 */
typedef enum {
    HC05_NONE,
    HC05_SET,
    HC05_CHECK
} hc05_at_operation_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef hc05_connect_status_t HC05_CONNECT_STATUS; 
typedef uint8_t HC05_DATA_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief HC05 initialization 
 * 
 * @details Enables GPIO pins for use with the module and assigns module information to 
 *          an instance of the hc05_mod_info_t structure that gets referenced throughout 
 *          the driver. <br><br>
 *          
 *          Which GPIOs to enable can be chosen using the arguments to this function. The 
 *          GPIO pins avialable for the module include: <br> 
 *            - AT cmd enable pin (pin 34) (output) --> pin PA8 <br> 
 *            - EN pin (output) --> pin PA11 <br> 
 *            - STATE pin (input) --> pin PA12 <br> 
 * 
 * @param uart : USART port used for the module 
 * @param timer : TIM port for delays 
 * @param pin34_status : AT cmd enable pin status 
 * @param en_status : EN pin status 
 * @param state_status : STATE pin status 
 */
void hc05_init(
    USART_TypeDef *uart, 
    TIM_TypeDef *timer, 
    GPIO_TypeDef *gpio_at, 
    pin_selector_t at, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en, 
    GPIO_TypeDef *gpio_state, 
    pin_selector_t state) ; 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief HC05 power on 
 * 
 * @details Turns the module on by using the EN pin GPIO output. <br> 
 *          NOTE: The EN pin must be enabled for on/off control. If is not enabled then the 
 *                module will remain on whenever there is power supplied to the Vcc pin. 
 * 
 * @see hc05_init
 */
void hc05_on(void); 


/**
 * @brief HC05 power off 
 * 
 * @details Turns the module off by using the EN pin GPIO output. <br> 
 *          NOTE: The EN pin must be enabled for on/off control. If is not enabled then the 
 *                module will remain on whenever there is power supplied to the Vcc pin.
 * 
 * @see hc05_init 
 */
void hc05_off(void);


/**
 * @brief HC05 Data mode send 
 * 
 * @details Sends a string of data to the module which in turn gets sent out over 
 *          Bluetooth to a connected device. 
 * 
 * @param send_data : pointer to the data string to send 
 */
void hc05_send(const char *send_data); 


/**
 * @brief HC05 data status availability 
 * 
 * @details Reads the HC05 UART data register status and returns the result. If data ready 
 *          to be read then logical TRUE will be returned, FALSE otherwise. 
 * 
 * @return HC05_DATA_STATUS 
 */
HC05_DATA_STATUS hc05_data_status(void); 


/**
 * @brief HC05 Data mode receive 
 * 
 * @details Reads a string of data from the module that was obtained over Bluetooth from 
 *          a connected device. 
 * 
 * @param receive_data : pointer to the buffer that stores the received data string 
 * @param data_len : length of receive_data buffer 
 */
void hc05_read(
    char *receive_data, 
    uint8_t data_len); 


/**
 * @brief HC05 connection status 
 * 
 * @details Reads and returns the connection status of the device. The connection status 
 *          is provided by the state pin feedback. This can be used to check for a 
 *          connection before attempting to send or read data. 
 * 
 * @return HC05_CONNECT_STATUS : connection status of the device 
 */
HC05_CONNECT_STATUS hc05_status(void); 


/**
 * @brief Clear the UART data register 
 * 
 * @details This is often used to make sure old data is not read when going to read the 
 *          UART data rgeister. 
 */
void hc05_clear(void); 

//=======================================================================================


//=======================================================================================
// Setters and getters 

/**
 * @brief Get driver status flag 
 * 
 * @details Status flag bits: 
 *          --> bits 0-8: uart status (see uart_status_t) 
 * 
 * @return uint8_t : driver status flag 
 */
uint8_t hc05_get_status(void); 


/**
 * @brief Clear driver status flag 
 */
void hc05_clear_status(void); 

//=======================================================================================


//=======================================================================================
// AT Command Mode functions 

/**
 * @brief HC05 change operating mode 
 * 
 * @details This functions allows for changing between the two operating modes of the module 
 *          through software. The two modes are Data mode (default) which is used to send and 
 *          receive information with other bluetooth devices, and AT Command mode which allows 
 *          you to modify the module settings. 
 *          
 *          AT Command mode has a fixed baud rate of 38400 bps. Data mode is whatever baud 
 *          rate gets set during AT Command mode. The default Data mode baud rate is 9600 bps.
 *          
 *          NOTE: this function is not available when HC05_AT_ENABLE is set to 0. If this 
 *                is 0 then this indicates in the code that only data mode will be used. 
 * 
 * @see hc05_at_command
 * @see hc05_send
 * 
 * @param mode : Moudle mode - either Data mode (default) or AT Command mode 
 * @param baud_frac : Fraction part of buad rate 
 * @param baud_mant : Mantissa part of baud rate 
 */
void hc05_change_mode(
    hc05_mode_t mode, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant); 


/**
 * @brief HC05 AT Command mode 
 * 
 * @details This function is used to send AT commands and read the response from the module. 
 *          The arguments specify the command (see documentation), operation (check, set, or 
 *          neither), and the command parameter (if their is one) desired and use that info 
 *          to generate a command string that gets sent to the module. The modules response 
 *          is recorded in the 'response' buffer. 
 * 
 * @param command : number indicating the AT command 
 * @param operation : indicated whether to set or check a parameter 
 * @param param : parameter used when during set operations 
 * @param resp : buffer that stores the module response 
 * @param resp_len : resp buffer length 
 */
void hc05_at_command(
    hc05_at_commnds_t command, 
    hc05_at_operation_t operation, 
    char *param, 
    char *response, 
    uint8_t resp_len); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _HC05_DRIVER_H_ 
