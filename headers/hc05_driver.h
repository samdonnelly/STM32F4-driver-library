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
#include "timers.h"

//=======================================================================================


//=======================================================================================
// Macros 

// HC-05 paramters 
#define HC05_ADDRESS      0 
#define HC05_DEFAULT_PIN  1234
#define HC05_INIT_DELAY   100

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
 * @details 
 * 
 */
typedef enum {
    HC05_DATA_MODE, 
    HC05_AT_CMD_MODE
} hc05_mode_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    HC05_PIN34_DISABLE, 
    HC05_PIN34_ENABLE
} hc05_pin34_status_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    HC05_EN_DISABLE, 
    HC05_EN_ENABLE
} hc05_en_status_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    HC05_STATE_DISABLE, 
    HC05_STATE_ENABLE
} hc05_state_status_t; 


// #if HC05_AT_CMD_MODE

/**
 * @brief HC-05 AT Commands 
 * 
 * @details 
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
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    HC05_NONE,
    HC05_SET,
    HC05_CHECK
} hc05_at_operation_t; 

// #endif  // HC05_AT_CMD_MODE

//=======================================================================================


//=======================================================================================
// Data types 

typedef hc05_mode_t HC05_MODE; 
typedef hc05_at_commnds_t AT_CMD; 
typedef hc05_at_operation_t AT_OPR; 

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief HC-05 initialization 
 * 
 * @details 
 * 
 * @param pin34_state 
 * @param en_state 
 * @param state_state 
 */
void hc05_init(
    uint8_t pin34_status,
    uint8_t en_status, 
    uint8_t state_status); 

//=======================================================================================


//=======================================================================================
// Power functions 

/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_pwr_on(void); 


/**
 * @brief 
 * 
 * @details 
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
 *          you to modify the module settings. This function takes the desired operating mode 
 *          and the mode baud rate as arguments. The mode argument requires the use of 
 *          hc05_mode_t and the baud rate argument requires the use of uart_baud_rate_t in 
 *          the uart driver. <br><br>
 *          
 *          AT Command mode has a fixed baud rate of 38400 bps. Data mode is whatever baud 
 *          rate gets set during AT Command mode. THe default Data mode baud rate is 9600 bps.
 *          <br><br> 
 *          
 *          Note that this function is not available when HC05_AT_CMD_MODE is set to 0. If this 
 *          is set to 0 then this indicates in the code that only data mode will be used. 
 *          <br><br> 
 * 
 * @see hc05_mode_t
 * @see uart_baud_rate_t
 * @see hc05_at_command
 * @see hc05_data_mode
 * 
 * @param mode : Moudle mode - either Data mode (default) or AT Command mode 
 * @param baud_rate : Baud rate of the selected mode 
 */
void hc05_change_mode(
    HC05_MODE mode, 
    UART_BAUD baud_rate); 

//=======================================================================================


//=======================================================================================
// Mode functions 

/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_data_mode(void); 


/**
 * @brief HC-05 AT Command Mode 
 * 
 * @details 
 * 
 * @param command 
 * @param operation 
 * @param param 
 * @param response 
 */
void hc05_at_command(
    AT_CMD command, 
    AT_OPR operation, 
    char *param, 
    char *response); 

//=======================================================================================

#endif  // _HC05_DRIVER_H_ 
