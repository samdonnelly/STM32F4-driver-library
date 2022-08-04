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

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

// HC-05 paramters 
#define HC05_ADDRESS 0 
#define HC05_DEFAULT_PIN 1234
#define HC05_INIT_DELAY 500

// AT Command Mode 
#define HC05_AT_CMD_MODE 1   // Controls the inclusion of AT command mode code 
#define HC05_AT_CMD_LEN  30  // Max length of command string 

//=======================================================================================


//=======================================================================================
// Enums 

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
// Data Mode 

/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_goto_data_mode(uint8_t baud_rate); 


/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_data_mode(void); 

//=======================================================================================


//=======================================================================================
// AT Command mode 

/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_goto_at_command(void); 


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
