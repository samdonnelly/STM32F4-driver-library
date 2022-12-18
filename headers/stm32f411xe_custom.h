/**
 * @file stm32f411xe_custom.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Adapted STM generated code 
 * 
 * @details This code is copied from the STM generated code for the STM32F411XE and 
 *          adapted to fit into this library. This is done to prevent recreating code  
 *          that is out of the scope/purpose of this library and eliminate unneeded 
 *          overhead. 
 * 
 * @version 0.1
 * @date 2022-12-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _STM32F411XE_CUSTOM_H_ 
#define _STM32F411XE_CUSTOM_H_ 

//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

/* ########################## HSE/HSI Values adaptation ##################### */
/**
 * @brief Adjust the value of External High Speed oscillator (HSE) used in your application.
 *        This value is used by the RCC HAL module to compute the system frequency
 *        (when HSE is used as system clock source, directly or through the PLL).
 */
#define HSE_HZ    8000000U /*!< Value of the External oscillator in Hz */


/**
 * @brief Internal High Speed oscillator (HSI) value.
 *        This value is used by the RCC HAL module to compute the system frequency
 *        (when HSI is used as system clock source, directly or through the PLL).
 */
#define HSI_HZ    ((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz*/



/** @brief  Macro to get the oscillator used as PLL clock source.
  * @retval The oscillator used as PLL clock source. The returned value can be one
  *         of the following:
  *              - RCC_PLLSOURCE_HSI: HSI oscillator is used as PLL clock source.
  *              - RCC_PLLSOURCE_HSE: HSE oscillator is used as PLL clock source.
  */
#define RCC_GET_PLL_OSCSOURCE() ((uint32_t)(RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC))

//=======================================================================================


//=======================================================================================
// Enums 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Stores the system clock frequency for use later 
 * 
 * @details 
 */
void get_sys_clk_init(void); 


/**
 * @brief Returns the HCLK frequency
 * 
 * @note   Each time HCLK changes, this function must be called to update the
 *         right HCLK value. Otherwise, any configuration based on this function will be incorrect.
 *
 * @note   The sys_core_clk CMSIS variable is used to store System Clock Frequency
 *         and updated within this function
 * 
 * @retval HCLK frequency
 * 
 * @return uint32_t 
 */
uint32_t rcc_get_hclk_frq(void);


/**
 * @brief Returns the PCLK1 frequency
 * 
 * @note   Each time PCLK1 changes, this function must be called to update the
 *         right PCLK1 value. Otherwise, any configuration based on this function will be incorrect.
 * 
 * @retval PCLK1 frequency
 * 
 * @return uint32_t 
 */
uint32_t rcc_get_pclk1_frq(void);


/**
 * @brief Returns the PCLK2 frequency
 * 
 * @note   Each time PCLK2 changes, this function must be called to update the
 *         right PCLK2 value. Otherwise, any configuration based on this function will be incorrect.
 * 
 * @retval PCLK2 frequency
 * 
 * @return uint32_t 
 */
uint32_t rcc_get_pclk2_frq(void);

//=======================================================================================

#endif   // _STM32F411XE_CUSTOM_H_
