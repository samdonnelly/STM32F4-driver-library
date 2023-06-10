/**
 * @file interrupt_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt functions 
 * 
 * @version 0.1
 * @date 2022-11-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _INTERRUPT_DRIVER_H_
#define _INTERRUPT_DRIVER_H_

//================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "gpio_driver.h"

//================================================================================


//================================================================================
// Macros 

// EXTI Lines - lines correspond to pins on a port 
#define EXTI_L0   0x00000001 
#define EXTI_L1   0x00000002 
#define EXTI_L2   0x00000004 
#define EXTI_L3   0x00000008 
#define EXTI_L4   0x00000010 
#define EXTI_L5   0x00000020 
#define EXTI_L6   0x00000040 
#define EXTI_L7   0x00000080 
#define EXTI_L8   0x00000100 
#define EXTI_L9   0x00000200 
#define EXTI_L10  0x00000400 
#define EXTI_L11  0x00000800 
#define EXTI_L12  0x00001000 
#define EXTI_L13  0x00002000 
#define EXTI_L14  0x00004000 
#define EXTI_L15  0x00008000 
#define EXTI_L16  0x00010000 
#define EXTI_L17  0x00020000 
#define EXTI_L18  0x00040000 
#define EXTI_L21  0x00200000 
#define EXTI_L22  0x00400000 

// IRQ Priority 
#define EXTI_PRIORITY_0   0x0   // Highest priority 
#define EXTI_PRIORITY_1   0x1 
#define EXTI_PRIORITY_2   0x2 
#define EXTI_PRIORITY_3   0x3 
#define EXTI_PRIORITY_4   0x4 
#define EXTI_PRIORITY_5   0x5 
#define EXTI_PRIORITY_6   0x6 
#define EXTI_PRIORITY_7   0x7 
#define EXTI_PRIORITY_8   0x8 
#define EXTI_PRIORITY_9   0x9 
#define EXTI_PRIORITY_10  0xA 
#define EXTI_PRIORITY_11  0xB 
#define EXTI_PRIORITY_12  0xC 
#define EXTI_PRIORITY_13  0xD 
#define EXTI_PRIORITY_14  0xE 
#define EXTI_PRIORITY_15  0xF   // Lowest priority 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief GPIO port to use for the EXTI line 
 * 
 * @note This is used as opposed to the GPIO port pointers because an index for the ports is 
 *       needed. 
 */
typedef enum {
    EXTI_PA, 
    EXTI_PB, 
    EXTI_PC, 
    EXTI_PD, 
    EXTI_PE, 
    EXTI_PH 
} exti_port_t; 


/**
 * @brief External interrupt mask configuration 
 */
typedef enum {
    EXTI_INT_MASKED,        // External interrupt masked - interrupt disabled 
    EXTI_INT_NOT_MASKED     // External interrupt not masked - interrupt enabled 
} exti_int_mask_t; 


/**
 * @brief External event mask configuration 
 */
typedef enum {
    EXTI_EVENT_MASKED,         // External event masked - interrupt disabled 
    EXTI_EVENT_NOT_MASKED      // External event not masked - interrupt enabled 
} exti_event_mask_t; 


/**
 * @brief Interrupt rising edge trigger configuration 
 */
typedef enum {
    EXTI_RISE_TRIG_DISABLE,     // Rising edge interrupt trigger disabled 
    EXTI_RISE_TRIG_ENABLE       // Rising edge interrupt trigger enabled 
} exti_rise_trigger_t; 


/**
 * @brief Interrupt falling edge trigger configuration 
 */
typedef enum {
    EXTI_FALL_TRIG_DISABLE,     // Falling edge interrupt trigger disabled 
    EXTI_FALL_TRIG_ENABLE       // Falling edge interrupt trigger enabled 
} exti_fall_trigger_t; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief External interrupt initialization 
 * 
 * @details Enabled the clock needed for external interrupts and resets all EXTI port/pin 
 *          configurations so they can be configured. This function is called once for all EXTI. 
 */
void exti_init(void); 


/**
 * @brief External interrupt configuration 
 * 
 * @details This function configures the behavior of each external interrupt. This is called 
 *          for each EXTI used. 
 * 
 * @see exti_port_t
 * @see pin_selector_t
 * @see gpio_pupdr_t
 * @see exti_int_mask_t
 * @see exti_event_mask_t
 * @see exti_rise_trigger_t
 * @see exti_fall_trigger_t
 * 
 * @param gpio : pointer to GPIO port of EXTI pin 
 * @param port : GPIO port index 
 * @param pin : pin number of EXTI pin 
 * @param pull : pull-up or pull-down pin configuration 
 * @param exti_line : EXTI line being configured - see EXTI_LX macros above 
 * @param int_mask : interrupt mask configuration 
 * @param event_mask : event mask configuration 
 * @param rise_trig : rising edge trigger configuration 
 * @param fall_trig falling edge trigger configuration 
 */
void exti_config(
    GPIO_TypeDef *gpio, 
    exti_port_t port, 
    pin_selector_t pin, 
    gpio_pupdr_t pull, 
    uint32_t exti_line, 
    exti_int_mask_t int_mask, 
    exti_event_mask_t event_mask, 
    exti_rise_trigger_t rise_trig, 
    exti_fall_trigger_t fall_trig); 


/**
 * @brief NVIC configuration 
 * 
 * @details This function sets the interrupt priority and enables the interrupt handler. 
 *          Interrupts with a higher priority get served first. This needs to be called 
 *          for each interrupt (not just external interrupts) being used on the system. 
 * 
 * @param irqn : index of interrupt function handler 
 * @param priority : interrupt priority - see priority macros above 
 */
void nvic_config(
    IRQn_Type irqn, 
    uint8_t priority); 

//================================================================================


//================================================================================
// Control registers 

/**
 * @brief Pending register clear 
 * 
 * @details Clears the pending bit that corresponds to the EXTI triggered. This function is 
 *          needed by the EXTI function handlers in order to exit the handler. 
 * 
 * @param pr : pending flag to clear for EXTI line - use EXTI_LX macros above 
 */
void exti_pr_clear(
    uint32_t pr); 

//================================================================================

#endif   // _INTERRUPT_DRIVER_H_
