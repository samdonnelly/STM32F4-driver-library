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

//================================================================================


//================================================================================
// Macros 

// EXTI Lines 
#define EXTI_L1   0x00000001 
#define EXTI_L2   0x00000002 
#define EXTI_L3   0x00000004 
#define EXTI_L4   0x00000008 
#define EXTI_L5   0x00000010 
#define EXTI_L6   0x00000020 
#define EXTI_L7   0x00000040 
#define EXTI_L8   0x00000080 
#define EXTI_L9   0x00000100 
#define EXTI_L10  0x00000200 
#define EXTI_L11  0x00000400 
#define EXTI_L12  0x00000800 
#define EXTI_L13  0x00001000 
#define EXTI_L14  0x00002000 
#define EXTI_L15  0x00004000 
#define EXTI_L16  0x00008000 
#define EXTI_L17  0x00010000 
#define EXTI_L18  0x00020000 
#define EXTI_L21  0x00100000 
#define EXTI_L22  0x00200000 

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
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    EXTI_PORT_PA,   // 
    EXTI_PORT_PB,   // 
    EXTI_PORT_PC,   // 
    EXTI_PORT_PD,   // 
    EXTI_PORT_PE,   // 
    EXTI_PORT_PH    // 
} exti_port_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    EXTI_RISE_TRIG_DISABLE,   // 
    EXTI_RISE_TRIG_ENABLE     // 
} exti_rise_trigger_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    EXTI_FALL_TRIG_DISABLE,   // 
    EXTI_FALL_TRIG_ENABLE     // 
} exti_fall_trigger_t; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief External interrupt initialization 
 * 
 * @details 
 * 
 */
void exti_init(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param irqn 
 * @param port 
 * @param pin 
 * @param im 
 * @param trig_mode 
 * @param trig 
 * @param priority 
 */
void exti_config(
    exti_port_t port, 
    pin_selector_t pin, 
    uint32_t im, 
    exti_rise_trigger_t rise_trig, 
    exti_fall_trigger_t fall_trig, 
    uint32_t trig); 


/**
 * @brief NVIC configuration 
 * 
 * @details 
 * 
 * @param irqn 
 * @param priority 
 */
void nvic_config(
    IRQn_Type irqn, 
    uint8_t priority); 

//================================================================================


//================================================================================
// SYSCFG Register Functions 

/**
 * @brief SYSCFG register source clear 
 * 
 * @details 
 * 
 */
void syscfg_config_clear(void); 


/**
 * @brief SYSCFG register source set 
 * 
 * @details 
 * 
 * @param port 
 * @param pin 
 */
void syscfg_config(
    exti_port_t port, 
    pin_selector_t pin); 

//================================================================================


//================================================================================
// EXTI Register Functions 

/**
 * @brief Interrupt mask register set 
 * 
 * @details 
 *          Enables the EXTI by unmasking the event request on a given line. 
 * 
 * @param im 
 */
void exti_imr_set(
    uint32_t im); 


/**
 * @brief Interrupt mask register clear 
 * 
 * @details 
 *          Disabled the EXTI by masking the event request on a given line. 
 * 
 * @param im 
 */
void exti_imr_clear(
    uint32_t im); 


/**
 * @brief Event mask register set 
 * 
 * @details 
 * 
 * @param em 
 */
void exti_emr_set(
    uint32_t em); 


/**
 * @brief Event mask register clear 
 * 
 * @details 
 * 
 * @param em 
 */
void exti_emr_clear(
    uint32_t em); 


/**
 * @brief Rising trigger selection register set 
 * 
 * @details 
 * 
 * @param rt 
 */
void exti_rtsr_set(
    uint32_t rt); 


/**
 * @brief Rising trigger selection register clear 
 * 
 * @details 
 * 
 * @param rt 
 */
void exti_rtsr_clear(
    uint32_t rt); 


/**
 * @brief Falling trigger selection register set 
 * 
 * @details 
 * 
 * @param ft 
 */
void exti_ftsr_set(
    uint32_t ft); 


/**
 * @brief Falling trigger selection register clear 
 * 
 * @details 
 * 
 * @param ft 
 */
void exti_ftsr_clear(
    uint32_t ft); 


/**
 * @brief Software interrupt event register set 
 * 
 * @details 
 *          This allows for generation of an interrupt/event request using the software instead 
 *          of an external device/peripheral trigger. 
 * 
 * @param swier 
 */
void exti_swier_set(
    uint32_t swier); 


/**
 * @brief Software interrupt event register clear 
 * 
 * @details 
 * 
 * @param swier 
 */
void exti_swier_clear(
    uint32_t swier); 


/**
 * @brief Pending register set 
 * 
 * @details 
 * 
 * @param pr 
 */
void exti_pr_set(
    uint32_t pr); 


/**
 * @brief Pending register clear 
 * 
 * @details 
 * 
 * @param pr 
 */
void exti_pr_clear(
    uint32_t pr); 

//================================================================================

#endif   // _INTERRUPT_DRIVER_H_
