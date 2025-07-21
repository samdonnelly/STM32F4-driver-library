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

//=======================================================================================
// Includes 

#include "hd44780u_controller.h"

//=======================================================================================


//=======================================================================================
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
void hd44780u_init_state(hd44780u_trackers_t *hd44780u_device); 


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
void hd44780u_idle_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief Power save state 
 * 
 * @details This state is similar to the idle state except it will turn off the screen 
 *          backlight after a specified period of time. This is done to save system power 
 *          during times when the screen shouldn't be fully shut off but does not need to 
 *          be seen by the user all the time. 
 *          
 *          This state can be enabled using the hd44780u_set_pwr_save_flag setter at which 
 *          point the controller will default to this state when the screen is not being used 
 *          instead of going back to the idle state. This state can be disabled using the 
 *          hd44780u_clear_pwr_save_flag setter which will make the idle state the default 
 *          state again. 
 *          
 *          The time it takes for the screen to turn the bcklight off can be set using 
 *          hd44780u_set_sleep_time. When the backlight is off it can be turned back on using 
 *          hd44780u_wake_up at which point the timer to turn the backlight off will start over. 
 * 
 * @see hd44780u_set_pwr_save_flag 
 * @see hd44780u_clear_pwr_save_flag 
 * @see hd44780u_set_sleep_time 
 * @see hd44780u_wake_up 
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_pwr_save_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief HD44780U write state 
 * 
 * @details Writes the contents of the devices data record to the screen. To trigger this state, 
 *          the write flag should be set via the setter function. At the end of this state, the 
 *          write flag is automatically cleared the state machine returns to idle or power save 
 *          states if no other flags are set. 
 *          
 *          The contents of the data record can be updated through the use of any of the line 
 *          set or line clear functions. The results of updating the data record won't be 
 *          visible on the screen until the write state occurs. 
 * 
 * @see hd44780u_set_write_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_write_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief Clear screen state 
 * 
 * @details Clears the screen of all it's content and clears the data stored in the screen 
 *          driver data record. Once this state is executed it will immediately go back to 
 *          the idle or power save state depending on the application settings. The 
 *          hd44780u_set_clear_flag setter can be used to trigger this state. 
 * 
 * @see hd44780u_set_clear_flag 
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_clear_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief HD44780U low power mode enter state 
 * 
 * @details Allows for entering into the controllers low power mode state. When the low 
 *          power mode flag is set, the controller will call this state which clears the display, 
 *          turns the display off and turns the backlight off. After this state is run the 
 *          controller moves to the low power mode state. 
 * 
 * @see hd44780u_set_low_pwr_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_low_pwr_enter_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief HD44780U low power mode state 
 * 
 * @details This state is entered from the low power mode enter state. In this state the 
 *          controller does nothing while the screen is in low power mode so it consumes minimal 
 *          power. This state can only be left when either the fault code is set, the reset flag 
 *          is set or the low power flag is cleared. If one of these happens then the controller 
 *          enters the low power mode exit state where the screen is brought out of low power 
 *          mode. While in low power mode, the screen cannot be used. 
 * 
 * @see hd44780u_low_pwr_enter_state
 * @see hd44780u_low_pwr_exit_state
 * @see hd44780u_set_low_pwr_flag
 * @see hd44780u_clear_low_pwr_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_low_pwr_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief HD44780U low power mode exit state 
 * 
 * @details This state is entered from the low power mode state when the fault flag is set, 
 *          the reset flag is set or the low power flag is cleared. In this state the screen 
 *          is brought out of low power mode by turning the display on, turning the backlight 
 *          on and setting the cursor to the home position. After this state is executed the 
 *          controller goes to the run state where it can once again be used by the system. 
 * 
 * @see hd44780u_clear_low_pwr_flag
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_low_pwr_exit_state(hd44780u_trackers_t *hd44780u_device); 


/**
 * @brief HD44780U fault state 
 * 
 * @details Currently this state is not used. To enter this state the fault code must be set 
 *          to non-zero and there are currently no methods in place in the controller to set 
 *          the fault code. Regardless of value, the fault code gets cleared in the reset 
 *          state. 
 *          
 *          This state takes the highest priority meaning the controller will enter this state 
 *          regardless of any other flags set. 
 * 
 * @see hd44780u_get_fault_code
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
void hd44780u_fault_state(hd44780u_trackers_t *hd44780u_device); 


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
void hd44780u_reset_state(hd44780u_trackers_t *hd44780u_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Instance of the device tracker record 
static hd44780u_trackers_t hd44780u_device_trackers; 


// Function pointers to controller states 
static hd44780u_state_functions_t state_table[HD44780U_NUM_STATES] = 
{
    &hd44780u_init_state, 
    &hd44780u_idle_state, 
    &hd44780u_pwr_save_state, 
    &hd44780u_write_state, 
    &hd44780u_clear_state, 
    &hd44780u_low_pwr_enter_state, 
    &hd44780u_low_pwr_state, 
    &hd44780u_low_pwr_exit_state, 
    &hd44780u_fault_state, 
    &hd44780u_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control Functions 

// Initialization 
void hd44780u_controller_init(TIM_TypeDef *timer)
{
    // Device and controller information 
    hd44780u_device_trackers.state = HD44780U_INIT_STATE; 
    hd44780u_device_trackers.fault_code = CLEAR; 

    // Screen sleep (backlight off) timer 
    hd44780u_device_trackers.timer = timer; 
    hd44780u_device_trackers.sleep_time = CLEAR; 
    hd44780u_device_trackers.sleep_timer.clk_freq = tim_get_pclk_freq(timer); 
    hd44780u_device_trackers.sleep_timer.time_cnt_total = CLEAR; 
    hd44780u_device_trackers.sleep_timer.time_cnt = CLEAR; 
    hd44780u_device_trackers.sleep_timer.time_start = SET_BIT; 
    
    // State flags 
    hd44780u_device_trackers.write = CLEAR_BIT; 
    hd44780u_device_trackers.low_power = CLEAR_BIT; 
    hd44780u_device_trackers.reset = CLEAR_BIT; 
    hd44780u_device_trackers.startup = SET_BIT; 
}


// Controller 
void hd44780u_controller(void)
{
    // Local variables 
    hd44780u_states_t next_state = hd44780u_device_trackers.state; 

    // Check the driver status 
    hd44780u_device_trackers.fault_code |= hd44780u_get_status(); 

    //==================================================
    // Revised State machine 

    switch (next_state)
    {
        case HD44780U_INIT_STATE: 
            // Startup flag cleared 
            if (!(hd44780u_device_trackers.startup))
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            break; 

        case HD44780U_IDLE_STATE: 
            // Reset flag set 
            if (hd44780u_device_trackers.reset)
            {
                next_state = HD44780U_RESET_STATE; 
            }

            // Low power flag set 
            else if (hd44780u_device_trackers.low_power)
            {
                next_state = HD44780U_LOW_PWR_ENTER_STATE;
            } 

            // Power save flag set 
            else if (hd44780u_device_trackers.pwr_save)
            {
                next_state = HD44780U_PWR_SAVE_STATE; 
            }

            // Write flag set 
            else if (hd44780u_device_trackers.write)
            {
                next_state = HD44780U_WRITE_STATE; 
            }

            // Clear flag set 
            else if (hd44780u_device_trackers.clear)
            {
                next_state = HD44780U_CLEAR_STATE; 
            }
            
            break; 

        case HD44780U_PWR_SAVE_STATE: 
            // Reset flag set 
            if (hd44780u_device_trackers.reset)
            {
                next_state = HD44780U_RESET_STATE; 
            }

            // Low power flag set 
            else if (hd44780u_device_trackers.low_power)
            {
                next_state = HD44780U_LOW_PWR_ENTER_STATE;
            } 

            // Write flag set 
            else if (hd44780u_device_trackers.write)
            {
                next_state = HD44780U_WRITE_STATE; 
            }

            // Clear flag set 
            else if (hd44780u_device_trackers.clear)
            {
                next_state = HD44780U_CLEAR_STATE; 
            }

            // Power save flag cleared 
            else if (!hd44780u_device_trackers.pwr_save)
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            break; 

        case HD44780U_WRITE_STATE: 
            // Fault code set 
            if (hd44780u_device_trackers.fault_code)
            {
                next_state = HD44780U_FAULT_STATE; 
            }

            // Reset flag set 
            else if (hd44780u_device_trackers.reset)
            {
                next_state = HD44780U_RESET_STATE; 
            }

            // Power save mode enabled 
            else if (hd44780u_device_trackers.pwr_save)
            {
                next_state = HD44780U_PWR_SAVE_STATE; 
            }

            // Default back to the idle state 
            else 
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            break; 

        case HD44780U_CLEAR_STATE: 
            // Fault code set 
            if (hd44780u_device_trackers.fault_code)
            {
                next_state = HD44780U_FAULT_STATE; 
            }

            // Reset flag set 
            else if (hd44780u_device_trackers.reset)
            {
                next_state = HD44780U_RESET_STATE; 
            }

            // Power save mode enabled 
            else if (hd44780u_device_trackers.pwr_save)
            {
                next_state = HD44780U_PWR_SAVE_STATE; 
            }

            // Default back to the idle state if the clear flag is not set 
            else if (!hd44780u_device_trackers.clear)
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            break; 

        case HD44780U_LOW_PWR_ENTER_STATE: 
            // Go straight to the low power state 
            next_state = HD44780U_LOW_PWR_STATE; 

            break; 

        case HD44780U_LOW_PWR_STATE: 
            // Fault code set, reset flag set, or low power flag cleared 
            if (hd44780u_device_trackers.fault_code || 
                hd44780u_device_trackers.reset || 
              !(hd44780u_device_trackers.low_power))
            {
                next_state = HD44780U_LOW_PWR_EXIT_STATE; 
            }

            break; 

        case HD44780U_LOW_PWR_EXIT_STATE: 
            // Fault code set 
            if (hd44780u_device_trackers.fault_code)
            {
                next_state = HD44780U_FAULT_STATE; 
            }

            // Reset flag set 
            else if (hd44780u_device_trackers.reset)
            {
                next_state = HD44780U_RESET_STATE; 
            }

            // Low power flag cleared - power save mode enabled 
            else if (hd44780u_device_trackers.pwr_save)
            {
                next_state = HD44780U_PWR_SAVE_STATE; 
            }

            // Low power flag cleared - default back to the idle state 
            else 
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            break; 

        case HD44780U_FAULT_STATE: 
            // Reset flag set 
            if (hd44780u_device_trackers.reset)
            {
                next_state = HD44780U_RESET_STATE; 
            }

            // Fault code cleared and power save mode enabled 
            else if (!hd44780u_device_trackers.fault_code && hd44780u_device_trackers.pwr_save)
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            // Fault code cleared and power save mode not enabled 
            else if (!hd44780u_device_trackers.fault_code)
            {
                next_state = HD44780U_IDLE_STATE; 
            }

            break; 

        case HD44780U_RESET_STATE: 
            // Go straight to the init state 
            next_state = HD44780U_INIT_STATE; 

            break; 

        default: 
            // Default to the init state 
            next_state = HD44780U_INIT_STATE; 

            break; 
    }

    //==================================================

    // Go to state function 
    (state_table[next_state])(&hd44780u_device_trackers); 

    // Update the state 
    hd44780u_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// Initialization state 
void hd44780u_init_state(hd44780u_trackers_t *hd44780u_device)
{
    // Clear reset flag 
    hd44780u_device->reset = CLEAR_BIT; 

    // Clear startup flag 
    hd44780u_device->startup = CLEAR_BIT; 
}


// Idle state 
void hd44780u_idle_state(hd44780u_trackers_t *hd44780u_device)
{
    // Do nothing when not needed 
}


// Power save state 
void hd44780u_pwr_save_state(hd44780u_trackers_t *hd44780u_device)
{
    // Put the screen into sleep mode (backlight off) after the specified sleep time 
    if (tim_compare(hd44780u_device->timer, 
                    hd44780u_device->sleep_timer.clk_freq, 
                    hd44780u_device->sleep_time, 
                    &hd44780u_device->sleep_timer.time_cnt_total, 
                    &hd44780u_device->sleep_timer.time_cnt, 
                    &hd44780u_device->sleep_timer.time_start))
    {
        hd44780u_device->sleep_timer.time_start = SET_BIT; 
        hd44780u_backlight_off(); 
    }
}


// Write state 
void hd44780u_write_state(hd44780u_trackers_t *hd44780u_device)
{
    uint8_t line_update = hd44780u_get_line_update(); 

    // Send only the lines with new content 
    for (uint8_t i = 0; i < HD44780U_NUM_LINES; i++)
    {
        if ((line_update >> i) & HD44780U_LINE_UPDATE_MASK)
        {
            hd44780u_send_line((hd44780u_lines_t)i); 
        }
    }

    hd44780u_device->write = CLEAR_BIT; 
}


// Clear screen and line contents state 
void hd44780u_clear_state(hd44780u_trackers_t *hd44780u_device)
{
    // Clear the screen and the line contents 
    hd44780u_clear(); 
    hd44780u_line_clear(HD44780U_L1); 
    hd44780u_line_clear(HD44780U_L2); 
    hd44780u_line_clear(HD44780U_L3); 
    hd44780u_line_clear(HD44780U_L4); 

    hd44780u_device->clear = CLEAR_BIT; 
}


// Enter low power mode 
void hd44780u_low_pwr_enter_state(hd44780u_trackers_t *hd44780u_device)
{
    // Clear the display, turn the backlight off and turn the display off 
    hd44780u_clear(); 
    hd44780u_line_clear(HD44780U_L1); 
    hd44780u_line_clear(HD44780U_L2); 
    hd44780u_line_clear(HD44780U_L3); 
    hd44780u_line_clear(HD44780U_L4); 
    hd44780u_backlight_off(); 
    hd44780u_display_off(); 
}


// Low power mode state 
void hd44780u_low_pwr_state(hd44780u_trackers_t *hd44780u_device)
{
    // Do nothing until the state is left 
}


// Exit low power mode 
void hd44780u_low_pwr_exit_state(hd44780u_trackers_t *hd44780u_device)
{
    // Turn the display on, turn the backlight on and clear the low power flag in the
    // event of a fault or reset 
    hd44780u_display_on(); 
    hd44780u_backlight_on(); 
    hd44780u_clear_low_pwr_flag(); 
}


// Fault state 
void hd44780u_fault_state(hd44780u_trackers_t *hd44780u_device)
{
    // Waits for the reset state to be called or for the fault code to be cleared 
}


// Reset state 
void hd44780u_reset_state(hd44780u_trackers_t *hd44780u_device)
{
    // Clear the fault and status codes 
    hd44780u_device->fault_code = CLEAR; 
    hd44780u_clear_status(); 

    // Call device init function again 
    hd44780u_re_init(); 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set power save mode 
void hd44780u_set_pwr_save_flag(void)
{
    hd44780u_device_trackers.pwr_save = SET_BIT; 
}


// Clear power save mode 
void hd44780u_clear_pwr_save_flag(void)
{
    hd44780u_device_trackers.pwr_save = CLEAR_BIT; 
    hd44780u_wake_up(); 
}


// Reset the power save state 
void hd44780u_wake_up(void)
{
    hd44780u_device_trackers.sleep_timer.time_start = SET_BIT; 
    hd44780u_backlight_on(); 
}


// Set screen sleep time 
void hd44780u_set_sleep_time(uint32_t sleep_time)
{
    hd44780u_device_trackers.sleep_time = sleep_time; 
}


// Message set 
void hd44780u_set_msg(
    hd44780u_msgs_t *msg, 
    uint8_t msg_len)
{
    for (uint8_t i = 0; i < msg_len; i++)
    {
        hd44780u_line_set(msg->line, msg->msg, msg->offset); 
        msg++; 
    }

    hd44780u_set_write_flag(); 
}


// Set write flag 
void hd44780u_set_write_flag(void)
{
    hd44780u_device_trackers.write = SET_BIT; 
}


// Set the clear screen flag 
void hd44780u_set_clear_flag(void)
{
    hd44780u_device_trackers.clear = SET_BIT; 
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

//=======================================================================================


//=======================================================================================
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

//=======================================================================================
