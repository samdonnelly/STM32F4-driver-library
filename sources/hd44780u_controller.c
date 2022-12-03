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
// Notes 
//================================================================================


//================================================================================
// Function prototypes 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_init_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_idle_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_write_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_read_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_low_pwr_trans_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_low_pwr_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_fault_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_reset_state(
    hd44780u_trackers_t hd44780u_device); 

//================================================================================


//================================================================================
// Variables 

// Instance of the device tracker record 
static hd44780u_trackers_t hd44780u_device_trackers; 

// Function pointers 
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
// State functions 

// Initialization state 
void hd44780u_init_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear reset flag 
    hd44780u_device_trackers.reset = CLEAR_BIT; 

    // Clear startup flag 
    hd44780u_device_trackers.startup = CLEAR_BIT; 
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
    hd44780u_device_trackers.write = CLEAR_BIT; 
}


// Read state 
void hd44780u_read_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Check read message for faults and set fault code if needed 

    // Store read messages 

    // Clear the read flag 
    hd44780u_device_trackers.read = CLEAR_BIT; 
}


// Low power mode transition state 
void hd44780u_low_pwr_trans_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Enable or disable low power depending on the low power flag 
    if (hd44780u_device_trackers.low_power)
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
    // Display the fault code on the screen 
}


// Reset state 
void hd44780u_reset_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear the fault code 
    hd44780u_device_trackers.fault_code = CLEAR; 

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
void hd44780_set_write_flag(void)
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
//================================================================================

//================================================================================
