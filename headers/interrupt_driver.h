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

// Communication drivers 

// Other drivers 

//================================================================================


//================================================================================
// Macros 

// EXTI Lines 
#define EXTI_LINE_1   0x00000001 
#define EXTI_LINE_2   0x00000002 
#define EXTI_LINE_3   0x00000004 
#define EXTI_LINE_4   0x00000008 
#define EXTI_LINE_5   0x00000010 
#define EXTI_LINE_6   0x00000020 
#define EXTI_LINE_7   0x00000040 
#define EXTI_LINE_8   0x00000080 
#define EXTI_LINE_9   0x00000100 
#define EXTI_LINE_10  0x00000200 
#define EXTI_LINE_11  0x00000400 
#define EXTI_LINE_12  0x00000800 
#define EXTI_LINE_13  0x00001000 
#define EXTI_LINE_14  0x00002000 
#define EXTI_LINE_15  0x00004000 
#define EXTI_LINE_16  0x00008000 
#define EXTI_LINE_17  0x00010000 
#define EXTI_LINE_18  0x00020000 
#define EXTI_LINE_21  0x00100000 
#define EXTI_LINE_22  0x00200000 

//================================================================================


//================================================================================
// Enums 
//================================================================================


//================================================================================
// Register Functions 

/**
 * @brief Interrupt mask register set 
 * 
 * @details 
 * 
 * @param exti 
 * @param im 
 */
void exti_imr_set(
    EXTI_TypeDef *exti, 
    uint32_t im); 


/**
 * @brief Interrupt mask register clear 
 * 
 * @details 
 * 
 * @param exti 
 * @param im 
 */
void exti_imr_clear(
    EXTI_TypeDef *exti, 
    uint32_t im); 


/**
 * @brief Event mask register set 
 * 
 * @details 
 * 
 * @param exti 
 * @param em 
 */
void exti_emr_set(
    EXTI_TypeDef *exti, 
    uint32_t em); 


/**
 * @brief Event mask register clear 
 * 
 * @details 
 * 
 * @param exti 
 * @param em 
 */
void exti_emr_clear(
    EXTI_TypeDef *exti, 
    uint32_t em); 


/**
 * @brief Rising trigger selection register set 
 * 
 * @details 
 * 
 * @param exti 
 * @param rt 
 */
void exti_rtsr_set(
    EXTI_TypeDef *exti, 
    uint32_t rt); 


/**
 * @brief Rising trigger selection register clear 
 * 
 * @details 
 * 
 * @param exti 
 * @param rt 
 */
void exti_rtsr_clear(
    EXTI_TypeDef *exti, 
    uint32_t rt); 


/**
 * @brief Falling trigger selection register set 
 * 
 * @details 
 * 
 * @param exti 
 * @param ft 
 */
void exti_ftsr_set(
    EXTI_TypeDef *exti, 
    uint32_t ft); 


/**
 * @brief Falling trigger selection register clear 
 * 
 * @details 
 * 
 * @param exti 
 * @param ft 
 */
void exti_ftsr_clear(
    EXTI_TypeDef *exti, 
    uint32_t ft); 


/**
 * @brief Software interrupt event register set 
 * 
 * @details 
 * 
 * @param exti 
 * @param swier 
 */
void exti_swier_set(
    EXTI_TypeDef *exti, 
    uint32_t swier); 


/**
 * @brief Software interrupt event register clear 
 * 
 * @details 
 * 
 * @param exti 
 * @param swier 
 */
void exti_swier_clear(
    EXTI_TypeDef *exti, 
    uint32_t swier); 


/**
 * @brief Pending register set 
 * 
 * @details 
 * 
 * @param exti 
 * @param pr 
 */
void exti_pr_set(
    EXTI_TypeDef *exti, 
    uint32_t pr); 


/**
 * @brief Pending register clear 
 * 
 * @details 
 * 
 * @param exti 
 * @param pr 
 */
void exti_pr_clear(
    EXTI_TypeDef *exti, 
    uint32_t pr); 

//================================================================================

#endif   // _INTERRUPT_DRIVER_H_
