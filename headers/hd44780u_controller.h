/**
 * @file hd44780u_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U + PCF8574 20x4 LCD screen controller 
 * 
 * @version 0.1
 * @date 2022-12-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_CONTROLLER_H_ 
#define _HD44780U_CONTROLLER_H_ 

//================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "hd44780u_driver.h"

//================================================================================


//================================================================================
// Macros 

#define HD44780U_NUM_STATES 8    // Current number of possible states for the controller 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief HD44780U controller states 
 */
typedef enum {
    HD44780U_INIT_STATE,                    // Initialization state 
    HD44780U_IDLE_STATE,                    // Idle state 
    HD44780U_WRITE_STATE,                   // Write state 
    HD44780U_READ_STATE,                    // Read state 
    HD44780U_LOW_PWR_TRANS_STATE,           // Low power mode transition state 
    HD44780U_LOW_PWR_STATE,                 // Low power state 
    HD44780U_FAULT_STATE,                   // Fault state 
    HD44780U_RESET_STATE                    // Reset state 
} hd44780u_states_t; 

//================================================================================


//================================================================================
// Structures 

/**
 * @brief HD44780U controller trackers 
 */
typedef struct hd44780u_trackers_s 
{
    // Device and controller information 
    hd44780u_states_t state;                // State of the controller 
    uint8_t fault_code;                     // Fault code of the device/controller 
    uint8_t read_msg[20];                   // Buffer to store read messages 

    // State flags 
    uint8_t write     : 1;                  // Write state trigger 
    uint8_t read      : 1;                  // Read state trigger 
    uint8_t low_power : 1;                  // Low power state trigger 
    uint8_t reset     : 1;                  // Reset state trigger 
    uint8_t startup   : 1;                  // Ensures the init state is run 
}
hd44780u_trackers_t; 

//================================================================================


//===============================================================================
// Datatypes 

typedef uint8_t FAULT_CODE; 
typedef hd44780u_states_t STATE; 

//===============================================================================


//================================================================================
// Function pointers 

/**
 * @brief HD44780U state machine function pointer 
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
typedef void (*hd44780u_state_functions_t)(
    hd44780u_trackers_t hd44780u_device); 

//================================================================================


//================================================================================
// Control functions 

/**
 * @brief HD44780U controller initialization 
 * 
 * @details Initializes the parameters of the device tracker. This function is called 
 *          during initialization in the application code. 
 * 
 * @see hd44780u_trackers_t
 */
void hd44780u_controller_init(void); 


/**
 * @brief HD44780U controller 
 * 
 * @details This function runs the devices state machine. It reads the status of control 
 *          flags and determine which state to go to. This is repeatedly called by the  
 *          application code. 
 */
void hd44780u_controller(void); 

//================================================================================


//================================================================================
// Setters 

/**
 * @brief HD44780U line 1 setter 
 * 
 * @details Changes the content of line 1 of 4 on the screen. <br> 
 *          
 *          A pointer to a character string of what to write to the line is passed as an 
 *          argument along with the position offset. The position offset determines the 
 *          the character position, starting from the left side of the screen, that the 
 *          string starts writing to. A string longer than the screen line length minus 
 *          the offset will be truncated at the end of the line and not continued onto the 
 *          next line. A line of the screen is 20 characters long. This setter updates only 
 *          the data record of the device, meaning the contents will not be seen on the 
 *          screen until the write state is triggered. Note that this function does not 
 *          erase the old contents of the line, it simply overwrites them starting at the 
 *          offset. 
 * 
 * @see hd44780_set_write_flag
 * 
 * @param display_data : pointer to character string used to update line 1 
 * @param line_offset : offset of where to start writing the character string 
 */
void hd44780u_line1_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief HD44780U line 1 clear 
 * 
 * @details Clears all the contents of line 1 of 4 on the screen. <br> 
 *          
 *          This function will overwrite the existing contents and replace it with blanks. 
 *          This function will update the devices data record and won't be seen on the screen 
 *          until the write state is triggered. 
 */
void hd44780u_line1_clear(void); 


/**
 * @brief HD44780U line 2 setter 
 * 
 * @details Changes the content of line 2 of 4 on the screen. <br> 
 *          
 *          See the description of hd44780u_line1_set for details. 
 * 
 * @see hd44780u_line1_clear
 * 
 * @param display_data : pointer to character string used to update line 2
 * @param line_offset : offset of where to start writing the character string 
 */
void hd44780u_line2_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief HD44780U line 2 clear 
 * 
 * @details Clears all the contents of line 2 of 4 on the screen. <br> 
 *          
 *          See the description of hd44780u_line1_clear for details. 
 * 
 * @see hd44780u_line1_clear
 */
void hd44780u_line2_clear(void); 


/**
 * @brief HD44780U line 3 setter 
 * 
 * @details Changes the content of line 3 of 4 on the screen. <br> 
 *          
 *          See the description of hd44780u_line1_set for details. 
 * 
 * @see hd44780u_line1_clear
 * 
 * @param display_data : pointer to character string used to update line 3
 * @param line_offset : offset of where to start writing the character string 
 */
void hd44780u_line3_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief HD44780U line 3 clear 
 * 
 * @details Clears all the contents of line 3 of 4 on the screen. <br> 
 *          
 *          See the description of hd44780u_line1_clear for details. 
 * 
 * @see hd44780u_line1_clear
 */
void hd44780u_line3_clear(void); 


/**
 * @brief HD44780U line 4 setter 
 * 
 * @details Changes the content of line 4 of 4 on the screen. <br> 
 *          
 *          See the description of hd44780u_line1_set for details. 
 * 
 * @see hd44780u_line1_clear
 * 
 * @param display_data : pointer to character string used to update line 4
 * @param line_offset : offset of where to start writing the character string 
 */
void hd44780u_line4_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief HD44780U line 4 clear 
 * 
 * @details Clears all the contents of line 4 of 4 on the screen. <br> 
 *          
 *          See the description of hd44780u_line1_clear for details. 
 * 
 * @see hd44780u_line1_clear
 */
void hd44780u_line4_clear(void); 


/**
 * @brief HD44780U set write flag 
 * 
 * @details Sets the write flag which is used to trigger the write state. In the write state
 *          the contents of the devices data record is written to the screen. This flag will 
 *          clear after the write state is complete. The write state will not occur if the 
 *          controller is in fault, reset or low power mode states. 
 */
void hd44780_set_write_flag(void); 


/**
 * @brief HD44780U set read flag 
 * 
 * @details Sets the read flag which is used to trigger the read state. The read state 
 *          currently has no functionality so this function will not change anything. The 
 *          flag will clear after the read state is complete. The read state will not occur 
 *          if the controller is in fault, reset or low power mode states. 
 */
void hd44780u_set_read_flag(void); 


/**
 * @brief HD44780U set low power mode flag 
 * 
 * @details Sets the low power mode flag which is used to enter low power mode. In low power
 *          mode the screen is shut off and cannot be written to or read from. To exit this 
 *          state the hd44780u_clear_low_pwr_flag setter must be called. 
 * 
 * @see hd44780u_clear_low_pwr_flag
 */
void hd44780u_set_low_pwr_flag(void); 


/**
 * @brief HD44780U clear low power mode flag 
 * 
 * @details Clears the low power mode flag which is used to bring the controller out of the 
 *          low power state. When the controller exits the low power state the screen is 
 *          turned back on. 
 * 
 * @see hd44780u_set_low_pwr_flag
 */
void hd44780u_clear_low_pwr_flag(void); 


/**
 * @brief HD44780U set reset flag 
 * 
 * @details Sets the reset flag which is used to trigger a controller and device reset. 
 *          When this flag is set then the controller will start the initialization 
 *          process over again. This flag can be called from any state but is meant 
 *          for when the screen encounters a fault and needs to reset. 
 */
void hd44780u_set_reset_flag(void); 

//================================================================================


//================================================================================
// Getters 

/**
 * @brief HD44780U state getter 
 * 
 * @details Returns the current state of the state machine. The states are numbered 
 *          according to hd44780u_states_t. 
 * 
 * @see hd44780u_states_t
 * 
 * @return STATE : current state machine state of the HD44780U controller 
 */
STATE hd44780u_get_state(void); 


/**
 * @brief HD44780U fault code getter 
 * 
 * @details Returns the current fault code of the controller. This information can be 
 *          used for logging. There is only one reset all possible fault codes. 
 * 
 * @return FAULT_CODE : fault code of the device 
 */
FAULT_CODE hd44780u_get_fault_code(void); 


/**
 * @brief HD44780U read message getter 
 * 
 * @details Copies the contents of the most recent message read from the screen into a 
 *          character string buffer passed to the function. The contents of the read 
 *          message will only change when the read state is called and the desired 
 *          read message is specified. 
 * 
 * @param read_msg : buffer to store the contents of the most recent read message 
 */
void hd44780u_get_read_msg(char *read_msg); 

//================================================================================

#endif   // _HD44780U_CONTROLLER_H_ 
