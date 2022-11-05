/**
 * @file interrupt_driver.c
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

//================================================================================
// Includes 

#include "interrupt_driver.h"

//================================================================================


//================================================================================
// Register Functions 

// Interrupt mask register set 
void exti_imr_set(
    EXTI_TypeDef *exti, 
    uint32_t im)
{
    exti->IMR |= im; 
}


// Interrupt mask register clear 
void exti_imr_clear(
    EXTI_TypeDef *exti, 
    uint32_t im)
{
    exti->IMR &= ~im; 
}


// Event mask register set 
void exti_emr_set(
    EXTI_TypeDef *exti, 
    uint32_t em)
{
    exti->EMR |= em; 
}


// Event mask register clear 
void exti_emr_clear(
    EXTI_TypeDef *exti, 
    uint32_t em)
{
    exti->EMR &= ~em; 
}


// Rising trigger selection register set 
void exti_rtsr_set(
    EXTI_TypeDef *exti, 
    uint32_t rt)
{
    exti->RTSR |= rt; 
}


// Rising trigger selection register clear 
void exti_rtsr_clear(
    EXTI_TypeDef *exti, 
    uint32_t rt)
{
    exti->RTSR &= ~rt; 
}


// Falling trigger selection register set 
void exti_ftsr_set(
    EXTI_TypeDef *exti, 
    uint32_t ft)
{
    exti->FTSR |= ft; 
}


// Falling trigger selection register clear 
void exti_ftsr_clear(
    EXTI_TypeDef *exti, 
    uint32_t ft)
{
    exti->FTSR &= ~ft; 
}


// Software interrupt event register set 
void exti_swier_set(
    EXTI_TypeDef *exti, 
    uint32_t swier)
{
    exti->SWIER |= swier; 
}


// Software interrupt event register clear 
void exti_swier_clear(
    EXTI_TypeDef *exti, 
    uint32_t swier)
{
    exti->SWIER &= ~swier; 
}


// Pending register set 
void exti_pr_set(
    EXTI_TypeDef *exti, 
    uint32_t pr)
{
    exti->PR |= pr; 
}


// Pending register clear 
void exti_pr_clear(
    EXTI_TypeDef *exti, 
    uint32_t pr)
{
    exti->PR &= ~pr; 
}

//================================================================================
