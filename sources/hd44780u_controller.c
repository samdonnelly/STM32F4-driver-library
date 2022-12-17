/**
 * @file hd44780u_controller.c
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

//================================================================================
// Includes 

#include "hd44780u_controller.h"

//================================================================================


//================================================================================
// Function prototypes 

/**
 * @brief HD44780U initialization state 
 * 
 * @details Initializes the controller, in particular the device tracker parameters. This is the 
 *          first state of the state machine and is called upon startup. Once it is complete it 
 *          directs the state machine into the idle state. This state will only be called again 
 *          if the controller/device undergoes a reset. 
 * 
 * @see hd44780u_trackers_t
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_init_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U idle state 
 * 
 * @details Resting state of the device during normal operation. When the screen is not 
 *          performing a task then the controller defaults to the idle state where no action 
 *          is taken until a flag is set that changes the state. Having this state allows for 
 *          the code to get in and out as quickly as possible when no work needs to be done. 
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_idle_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U write state 
 * 
 * @details Writes the contents of the devices data record to the screen. To trigger this state, 
 *          the write flag should be set via the setter function. At the end of this state, the 
 *          write flag is automatically cleared the state machine returns to idle if no other 
 *          flags are set. <br> 
 *          
 *          The contents of the data record can be updated through the use of any of the line 
 *          set or line clear functions. The results of updating the data record won't be 
 *          visible on the screen until the write state occurs. 
 * 
 * @see hd44780u_set_write_flag
 * @see hd44780u_line1_set
 * @see hd44780u_line1_clear
 * @see hd44780u_line2_set
 * @see hd44780u_line2_clear
 * @see hd44780u_line3_set
 * @see hd44780u_line3_clear
 * @see hd44780u_line4_set
 * @see hd44780u_line4_clear
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_write_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U read state 
 * 
 * @details Currently this state does nothing, it is only a placeholder for a working read 
 *          state. This state is called when the read flag is set via the setter function. 
 *          Once this state is called the read flag is instantly cleared and the controller 
 *          returns to the idle state if no other flags are set. <br> 
 *          
 *          Ideally this state could read status information from the screen such as fault 
 *          codes. However, reading from the screen is not yet supported and it is unknown 
 *          what information can be read. Once read functionaility is implemented, the read 
 *          flag setter function should be changed to take a message specifier as an argument. 
 *          <br> 
 *          
 *          To get the contents of a device read (once functionality is added), the read 
 *          message getter can be called to obtain the return message. 
 * 
 * @see hd44780_set_read_flag
 * @see hd44780u_get_read_msg
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_read_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U low power mode transition state 
 * 
 * @details Allows for transitioning into the controllers low power mode state. When the low 
 *          power mode flag is set, the controller will call this state which in turn shuts 
 *          the screen off and then proceeds immediately into the low power mode state. When 
 *          the low power mode flag is cleared, the controller will move from the low power 
 *          mode state into this state where the screen gets turned back on before returning 
 *          to idle. 
 * 
 * @see hd44780u_set_low_pwr_flag
 * @see hd44780u_clear_low_pwr_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_low_pwr_trans_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U low power mode state 
 * 
 * @details This state is entered when the low power mode flag is set. The controller first 
 *          passes through the low power mode transition state where the screen is turned off. 
 *          After the transition state the controller enters this state where it idles until 
 *          the low power flag is cleared. In this state the sceen cannot be written to or 
 *          read from. This state is used to consume less power at times the screen is not 
 *          needed. 
 * 
 * @see hd44780u_low_pwr_trans_state
 * @see hd44780u_set_low_pwr_flag
 * @see hd44780u_clear_low_pwr_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_low_pwr_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U fault state 
 * 
 * @details Currently this state is not used. To enter this state the fault code must be set 
 *          to non-zero and there are currently no methods in place in the controller to set 
 *          the fault code. Regardless of value, the fault code gets cleared in the reset 
 *          state. <br> 
 *          
 *          This state takes the highest priority meaning the controller will enter this state 
 *          regardless of any other flags set. 
 * 
 * @see hd44780u_get_fault_code
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_fault_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief HD44780U reset state 
 * 
 * @details Resets the device and controller to it's default state. This state is the second 
 *          highest priority behind the fault state and is triggered by setting the reset bit. 
 *          In this state the fault code is cleared and the device initialization is called 
 *          again. The reset state can be called from any state and is used to reset the 
 *          system but in most cases is called after the fault state. 
 * 
 * @see hd44780u_set_reset_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_reset_state(
    hd44780u_trackers_t hd44780u_device); 

//================================================================================


//================================================================================
// Variables 

// Instance of the device tracker record 
static hd44780u_trackers_t hd44780u_device_trackers; 

// Function pointers to controller states 
static hd44780u_state_functions_t state_table[HD44780U_NUM_STATES] = 
{
    &hd44780u_init_state, 
    &hd44780u_idle_state, 
    &hd44780u_write_state, 
    &hd44780u_read_state, 
    &hd44780u_low_pwr_trans_state, 
    &hd44780u_low_pwr_state, 
    &hd44780u_fault_state, 
    &hd44780u_reset_state 
}; 

//================================================================================


//================================================================================
// Control Functions 

// Initialization 
void hd44780u_controller_init(void)
{
    hd44780u_device_trackers.state = HD44780U_INIT_STATE; 

    hd44780u_device_trackers.fault_code = CLEAR; 

    hd44780u_device_trackers.write = CLEAR_BIT; 

    hd44780u_device_trackers.read = CLEAR_BIT; 

    hd44780u_device_trackers.low_power = CLEAR_BIT; 

    hd44780u_device_trackers.reset = CLEAR_BIT; 

    hd44780u_device_trackers.startup = SET_BIT; 
}


// Controller 
void hd44780u_controller(void)
{
    // Local variables 
    hd44780u_states_t next_state = hd44780u_device_trackers.state; 

    //==================================================
    // State machine 

    // Fault state 
    if (hd44780u_device_trackers.fault_code)
    {
        if (next_state == HD44780U_FAULT_STATE)
        {
            if (hd44780u_device_trackers.reset)
                next_state = HD44780U_RESET_STATE; 
        }
        else 
            next_state = HD44780U_FAULT_STATE; 
    }

    // Reset state 
    else if (hd44780u_device_trackers.reset)
    {
        if (next_state == HD44780U_RESET_STATE)
            next_state = HD44780U_INIT_STATE; 
        else 
            next_state = HD44780U_RESET_STATE; 
    }

    // Other states 
    else 
    {
        switch (next_state)
        {
            case HD44780U_INIT_STATE: 
                if (!(hd44780u_device_trackers.startup))
                    next_state = HD44780U_IDLE_STATE; 
                break; 

            case HD44780U_IDLE_STATE: 
                if (hd44780u_device_trackers.write)
                    next_state = HD44780U_WRITE_STATE; 
                
                else if (hd44780u_device_trackers.read)
                    next_state = HD44780U_READ_STATE; 

                else if (hd44780u_device_trackers.low_power)
                    next_state = HD44780U_LOW_PWR_TRANS_STATE; 
                
                break; 

            case HD44780U_WRITE_STATE: 
                if (hd44780u_device_trackers.read)
                    next_state = HD44780U_READ_STATE; 

                else if (!(hd44780u_device_trackers.write))
                    next_state = HD44780U_IDLE_STATE; 
                
                break; 

            case HD44780U_READ_STATE: 
                if (hd44780u_device_trackers.write)
                    next_state = HD44780U_WRITE_STATE; 

                else if (!(hd44780u_device_trackers.read))
                    next_state = HD44780U_IDLE_STATE; 
                
                break; 

            case HD44780U_LOW_PWR_TRANS_STATE: 
                if (hd44780u_device_trackers.low_power)
                    next_state = HD44780U_LOW_PWR_STATE; 
                else 
                    next_state = HD44780U_IDLE_STATE; 
                break; 

            case HD44780U_LOW_PWR_STATE: 
                if (!(hd44780u_device_trackers.low_power))
                    next_state = HD44780U_LOW_PWR_TRANS_STATE; 
                break; 

            default: 
                break; 
        }
    }

    //==================================================

    // Go to state function 
    (state_table[next_state])(hd44780u_device_trackers); 

    // Update the state 
    hd44780u_device_trackers.state = next_state; 
}

//================================================================================


//================================================================================
// State functions 

// Initialization state 
void hd44780u_init_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear reset flag 
    hd44780u_device.reset = CLEAR_BIT; 

    // Clear startup flag 
    hd44780u_device.startup = CLEAR_BIT; 
}


// Idle state 
void hd44780u_idle_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Do nothing when not needed 
}


// Write state 
void hd44780u_write_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Write all line contents 
    hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L1); 

    hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L2); 
    
    hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L3); 
    
    hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L4); 

    // Clear the write flag 
    hd44780u_device.write = CLEAR_BIT; 
}


// Read state 
void hd44780u_read_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Check read message for faults and set fault code if needed 

    // Store read messages 

    // Clear the read flag 
    hd44780u_device.read = CLEAR_BIT; 
}


// Low power mode transition state 
void hd44780u_low_pwr_trans_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Enable or disable low power depending on the low power flag 
    if (hd44780u_device.low_power)
        hd44780u_send_instruc(HD44780U_SETUP_CMD_0x08);   // Turn the display off 
    else 
        hd44780u_send_instruc(HD44780U_SETUP_CMD_0X0C);   // Turn the display on rrr
}


// Low power mode state 
void hd44780u_low_pwr_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Idle state where the controller can do nothing but wait for the low power mode 
    // flag to clear 
}


// Fault state 
void hd44780u_fault_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Waits for the reset state to be called 
}


// Reset state 
void hd44780u_reset_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear the fault code 
    hd44780u_device.fault_code = CLEAR; 

    // Call device init function again 
    hd44780u_re_init(); 
}

//================================================================================


//================================================================================
// Setters 

// Set screen line 1 content 
void hd44780u_line1_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset)
{
    hd44780u_line_set(HD44780U_L1, display_data, line_offset); 
}


// Clear screen line 1 
void hd44780u_line1_clear(void)
{
    hd44780u_line_clear(HD44780U_L1); 
}


// Set screen line 2 content 
void hd44780u_line2_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset)
{
    hd44780u_line_set(HD44780U_L2, display_data, line_offset); 
}


// Clear screen line 2 
void hd44780u_line2_clear(void)
{
    hd44780u_line_clear(HD44780U_L2); 
}


// Set screen line 3 content 
void hd44780u_line3_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset)
{
    hd44780u_line_set(HD44780U_L3, display_data, line_offset); 
}


// Clear screen line 3 
void hd44780u_line3_clear(void)
{
    hd44780u_line_clear(HD44780U_L3); 
}


// Set screen line 4 content 
void hd44780u_line4_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset)
{
    hd44780u_line_set(HD44780U_L4, display_data, line_offset); 
}


// Clear screen line 4 
void hd44780u_line4_clear(void)
{
    hd44780u_line_clear(HD44780U_L4); 
}


// Set write flag 
void hd44780u_set_write_flag(void)
{
    hd44780u_device_trackers.write = SET_BIT; 
}


// Set read flag 
void hd44780u_set_read_flag(void)
{    
    hd44780u_device_trackers.read = SET_BIT; 
}


// Set low power mode flag 
void hd44780u_set_low_pwr_flag(void)
{
    hd44780u_device_trackers.low_power = SET_BIT; 
}


// Clear low power mode flag 
void hd44780u_clear_low_pwr_flag(void)
{
    hd44780u_device_trackers.low_power = CLEAR_BIT; 
}


// Set reset flag 
void hd44780u_set_reset_flag(void)
{
    hd44780u_device_trackers.reset = SET_BIT; 
}

//================================================================================


//================================================================================
// Getters 

// Get state 
HD44780U_STATE hd44780u_get_state(void)
{
    return hd44780u_device_trackers.state; 
}


// Get fault code 
HD44780U_FAULT_CODE hd44780u_get_fault_code(void)
{
    return hd44780u_device_trackers.fault_code; 
}


// Get read message 
void hd44780u_get_read_msg(char *read_msg)
{
    strcpy(read_msg, (char *)hd44780u_device_trackers.read_msg); 
}

//================================================================================

//================================================================================
