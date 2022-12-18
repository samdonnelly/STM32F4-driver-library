/**
 * @file stm32f411xe_custom.c
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

//=======================================================================================
// Includes 

#include "stm32f411xe_custom.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Returns the SYSCLK frequency
 *
 * @note   The system frequency computed by this function is not the real
 *         frequency in the chip. It is calculated based on the predefined
 *         constant and the selected clock source:
 * @note     If SYSCLK source is HSI, function returns values based on HSI_HZ(*)
 * @note     If SYSCLK source is HSE, function returns values based on HSE_HZ(**)
 * @note     If SYSCLK source is PLL, function returns values based on HSE_HZ(**)
 *           or HSI_HZ(*) multiplied/divided by the PLL factors.
 * @note     (*) HSI_HZ is a constant defined in stm32f4xx_hal_conf.h file (default value
 *               16 MHz) but the real value may vary depending on the variations
 *               in voltage and temperature.
 * @note     (**) HSE_HZ is a constant defined in stm32f4xx_hal_conf.h file (default value
 *                25 MHz), user has to ensure that HSE_HZ is same as the real
 *                frequency of the crystal used. Otherwise, this function may
 *                have wrong result.
 *
 * @note   The result of this function could be not correct when using fractional
 *         value for HSE crystal.
 *
 * @note   This function can be used by the user application to compute the
 *         baudrate for the communication peripherals or configure other parameters.
 *
 * @note   Each time SYSCLK changes, this function must be called to update the
 *         right SYSCLK value. Otherwise, any configuration based on this function will be incorrect.
 *
 *
 * @retval SYSCLK frequency
 * 
 * @return uint32_t 
 */
uint32_t rcc_get_sys_clk_frq(void);

//=======================================================================================


//=======================================================================================
// Global variables 

static uint32_t sys_clk_frq; 
const  uint8_t  ahb_psc_table[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const  uint8_t  apb_psc_table[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

//=======================================================================================


//=======================================================================================
// System clock functions 

/*
 * These functions (aside from the init function) are used to determine the system clock 
 * and therefore the APB1 and APB2 peripheral frequencies the controller is configured 
 * to run on. These functions were taken from the HAL library. 
 * 
 * The init function is a custom function not found in the STM generated code. It is used
 * to record the clock frequencies so they only have to be read once during initialization. 
 * 
 * The timers driver uses these functions to create non-blocking delays that will produce 
 * the same delay regardless of the clock speed set for the given timer. 
 */


// Stores the system clock frequency for use later 
void get_sys_clk_init(void)
{
    sys_clk_frq = (rcc_get_sys_clk_frq() >> ahb_psc_table[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos]); 
}


// Returns the SYSCLK frequency
uint32_t rcc_get_sys_clk_frq(void)
{
    uint32_t pllm = 0U; 
    uint32_t pllvco = 0U; 
    uint32_t pllp = 0U;
    uint32_t sysclockfreq = 0U;

    /* Get SYSCLK source -------------------------------------------------------*/
    switch (RCC->CFGR & RCC_CFGR_SWS)
    {
        case RCC_CFGR_SWS_HSI:  /* HSI used as system clock source */
        {
            sysclockfreq = HSI_HZ;
            break;
        }

        case RCC_CFGR_SWS_HSE:  /* HSE used as system clock  source */
        {
            sysclockfreq = HSE_HZ;
            break;
        }

        case RCC_CFGR_SWS_PLL:  /* PLL used as system clock  source */
        {
            /* PLL_VCO = (HSE_HZ or HSI_HZ / PLLM) * PLLN
            SYSCLK = PLL_VCO / PLLP */
            pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

            if (RCC_GET_PLL_OSCSOURCE() != RCC_PLLCFGR_PLLSRC_HSI)
            {
                // HSE used as PLL clock source  
                pllvco = (uint32_t)((((uint64_t)HSE_HZ * ((uint64_t)(
                                   (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / 
                                   (uint64_t)pllm); 
            }

            else
            {
                // HSI used as PLL clock source 
                pllvco = (uint32_t)((((uint64_t)HSI_HZ * ((uint64_t)(
                                   (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / 
                                   (uint64_t)pllm); 
            }

            pllp = ((((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos) + 1U) *2U);

            sysclockfreq = pllvco/pllp;

            break;
        }

        default:
        {
            sysclockfreq = HSI_HZ;
            break;
        }
    }
    
    return sysclockfreq;
}


// Returns the HCLK frequency
uint32_t rcc_get_hclk_frq(void)
{
    return sys_clk_frq; 
}


// Returns the PCLK1 frequency
uint32_t rcc_get_pclk1_frq(void)
{
    /* Get HCLK source and Compute PCLK1 frequency ---------------------------*/
    return (rcc_get_hclk_frq() >> apb_psc_table[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]);
}


// Returns the PCLK2 frequency
uint32_t rcc_get_pclk2_frq(void)
{
    /* Get HCLK source and Compute PCLK2 frequency ---------------------------*/
    return (rcc_get_hclk_frq() >> apb_psc_table[(RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos]);
}

//=======================================================================================
