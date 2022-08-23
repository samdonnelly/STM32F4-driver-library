/**
 * @file hc05_driver.h
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

#ifndef _HC05_DRIVER_H_
#define _HC05_DRIVER_H_

//=======================================================================================
// Includes

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "uart_comm.h"
#include "gpio_driver.h"

// Other drivers 
#include "timers.h"

//=======================================================================================


//=======================================================================================
// Macros 

// HC-05 paramters 
#define HC05_INIT_DELAY   100  // ms delay to ensure full power cycle 

// AT Command Mode 
#define HC05_AT_EN          1      // Controls the inclusion of AT command mode code 
#define HC05_AT_CMD_LEN     30     // Max length of command string 
#define HC05_AT_DR_CLR_LEN  4      // Length of "OK\r\n" that follows an AT parameter response 
#define HC05_AT_RESP_STR    43     // 43 == '+' which is the start of a parmeter response 
#define HC05_AT_RESP_COUNT  65535  // Timout counter to receeive an AT cmd response 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HC-05 mode 
 * 
 * @details The HC-05 module has two modes: Data mode (default) and AT Command mode. This 
 *          enum is passed as an argument to hc05_change_mode to indicate which mode to 
 *          put the module into. 
 * 
 * @see hc05_change_mode
 * 
 */
typedef enum {
    HC05_DATA_MODE, 
    HC05_AT_CMD_MODE
} hc05_mode_t; 


/**
 * @brief HC-05 pin 34 status 
 * 
 * @details Pin 34 on the module is used to trigger AT command mode without needing to press
 *          the pushbutton in the module. This enum is passed as an argument to hc05_init 
 *          to indicate whether this functionality will be used or not. If so then a GPIO will 
 *          be configured for it. 
 * 
 * @see hc05_init
 * 
 */
typedef enum {
    HC05_PIN34_DISABLE, 
    HC05_PIN34_ENABLE
} hc05_pin34_status_t; 


/**
 * @brief HC-05 EN pin status 
 * 
 * @details The EN pin is used to enable power to the module so it can be turned on and off. 
 *          This enum is passed as an argument to hc05_init to indicate whether this 
 *          functionality will be used or not. If so then a GPIO will be configured for it. 
 * 
 * @see hc05_init
 * 
 */
typedef enum {
    HC05_EN_DISABLE, 
    HC05_EN_ENABLE
} hc05_en_status_t; 


/**
 * @brief HC-05 STATE pin status 
 * 
 * @details The STATE pin provides feedback as to whether the module is connected to a device 
 *          or not. This enum is passed as an argument to hc05_init to indicate whether this 
 *          feedback will be used or not. If so then a GPIO pin will be configured for it. 
 * 
 * @see hc05_init
 * 
 */
typedef enum {
    HC05_STATE_DISABLE, 
    HC05_STATE_ENABLE
} hc05_state_status_t; 


/**
 * @brief HC-05 AT Commands 
 * 
 * @details This enum contains all the AT Command mode commands used to configure the module. 
 *          These values are used in the hc05_at_command function to match the users requested 
 *          command and generate a command string that gets sent to the module. See the HC-05 
 *          documentation for more information on AT commands. 
 * 
 * @see hc05_at_command
 * 
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
 * @brief HC-05 AT command operation 
 * 
 * @details This enum is an argument passed to the hc05_at_command function to indicate whether 
 *          the user wants to set a parameter, check a parameter, or do nothing (in cases where
 *          there is only the ability to check). 
 * 
 * @see hc05_at_command
 * 
 */
typedef enum {
    HC05_NONE,
    HC05_SET,
    HC05_CHECK
} hc05_at_operation_t; 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief HC-05 module info 
 * 
 * @details Data record of the module that stores device specific information such as the 
 *          GPIO pins used for control and feedback as well as which UART port the module 
 *          is on. 
 * 
 */
typedef struct hc05_mod_info_s
{
    // UART used by the module 
    USART_TypeDef *hc05_uart; 

    // Pin for AT Command mode enable 
    gpio_pin_num_t at_pin; 

    // Pin for power enable 
    gpio_pin_num_t en_pin; 

    // Pin for connection status feedback 
    gpio_pin_num_t state_pin; 

} hc05_mod_info_t;


//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief HC-05 initialization 
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
 * @param pin34_status : AT cmd enable pin status 
 * @param en_status : EN pin status 
 * @param state_status : STATE pin status 
 */
void hc05_init(
    USART_TypeDef *uart, 
    hc05_pin34_status_t pin34_status,
    hc05_en_status_t    en_status, 
    hc05_state_status_t state_status); 

//=======================================================================================


//=======================================================================================
// Power functions 

/**
 * @brief HC-05 power on 
 * 
 * @details Turns the module on by using the EN pin GPIO output. <br> 
 *          NOTE: The EN pin must be enabled for on/off control. If is not enabled then the 
 *                module will remain on whenever there is power supplied to the Vcc pin. 
 * 
 * @see hc05_init
 * 
 */
void hc05_pwr_on(void); 


/**
 * @brief HC-05 power off 
 * 
 * @details Turns the module off by using the EN pin GPIO output. <br> 
 *          NOTE: The EN pin must be enabled for on/off control. If is not enabled then the 
 *                module will remain on whenever there is power supplied to the Vcc pin.
 * 
 * @see hc05_init 
 * 
 */
void hc05_pwr_off(void); 

//=======================================================================================


//=======================================================================================
// Transition functions 

/**
 * @brief HC-05 change operating mode 
 * 
 * @details This functions allows for changing between the two operating modes of the module 
 *          through software. The two modes are Data mode (default) which is used to send and 
 *          receive information with other bluetooth devices, and AT Command mode which allows 
 *          you to modify the module settings. <br><br>
 *          
 *          AT Command mode has a fixed baud rate of 38400 bps. Data mode is whatever baud 
 *          rate gets set during AT Command mode. The default Data mode baud rate is 9600 bps.
 *          <br><br> 
 *          
 *          NOTE: this function is not available when HC05_AT_CMD_MODE is set to 0. If this 
 *          is set to 0 then this indicates in the code that only data mode will be used. 
 *          <br><br> 
 * 
 * @see hc05_at_command
 * @see hc05_data_mode_send
 * 
 * @param mode : Moudle mode - either Data mode (default) or AT Command mode 
 * @param baud_rate : Baud rate of the selected mode 
 * @param clock_speed : Clock speed of the USART port used by the module 
 */
void hc05_change_mode(
    hc05_mode_t mode, 
    uart_baud_rate_t baud_rate, 
    uart_clock_speed_t clock_speed); 

//=======================================================================================


//=======================================================================================
// Mode functions 

/**
 * @brief HC-05 Data mode send 
 * 
 * @details Sends a string of data to the module which in turn gets sent out over 
 *          Bluetooth to a connected device. 
 * 
 * @param send_data : pointer to the data string to send 
 */
void hc05_data_mode_send(char *send_data); 


/**
 * @brief HC-05 Data mode receive 
 * 
 * @details Reads a string of data from the module that was obtained over Bluetooth from 
 *          a connected device. 
 * 
 * @param receive_data : pointer to the buffer that stores the received data string 
 */
void hc05_data_mode_receive(char *receive_data); 


/**
 * @brief HC-05 AT Command mode 
 * 
 * @details This function is used to send AT commands and read the response from the module. 
 *          The arguments specify the command (see documentation), operation (check, set, or 
 *          neither), and the command parameter (if their is one) desired and use that info 
 *          to generate a command string that gets sent to the module. The modules response 
 *          is recorded in the 'response' buffer. 
 *          
 * 
 * @param command : number indicating the AT command 
 * @param operation : indicated whether to set or check a parameter 
 * @param param : parameter used when during set operations 
 * @param response : pointer to a buffer that stores the module response 
 */
void hc05_at_command(
    hc05_at_commnds_t command, 
    hc05_at_operation_t operation, 
    char *param, 
    char *response); 

//=======================================================================================

#endif  // _HC05_DRIVER_H_ 
