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
// Initialization 

// Note that setting up interrupts is done by configuring the type of interrupt (ex. EXTI, 
// ADC, etc. - independent) then configuring the interrupt lines (common) separately. 

// External interrupt initialization 
void exti_init(void)
{
    // Enable the system configuration controller clock 
    RCC->APB2ENR |= (SET_BIT << SHIFT_14); 

    // Clear SYSCFG registers 
    syscfg_config_clear(); 
}


// External interrupt configuration 
void exti_config(
    exti_port_t port, 
    pin_selector_t pin, 
    uint32_t im, 
    exti_rise_trigger_t rise_trig, 
    exti_fall_trigger_t fall_trig, 
    uint32_t trig)
{
    // Configure the EXTI config register in SYSCFG - Define the interrupt source 
    syscfg_config(port, pin); 

    // Enable the EXTI 
    exti_imr_set(im); 

    // Configure the rising edge trigger 
    if (rise_trig) exti_rtsr_set(trig); 
    else exti_rtsr_set(trig); 

    // Configure the falling edge trigger 
    if (fall_trig) exti_ftsr_set(trig); 
    else exti_ftsr_set(trig); 
}


// NVIC configuration 
void nvic_config(
    IRQn_Type irqn, 
    uint8_t priority)
{
    // Set the interrupt priority 
    NVIC_SetPriority(irqn, priority);    // Built in NVIC function 

    // Enable the interrupt 
    NVIC_EnableIRQ(irqn);                // Built in NVIC function 
}

//================================================================================


//================================================================================
// SYSCFG Register Functions 

// SYSCFG register source clear 
void syscfg_config_clear(void)
{
    SYSCFG->EXTICR[0] = CLEAR; 
    SYSCFG->EXTICR[1] = CLEAR; 
    SYSCFG->EXTICR[2] = CLEAR; 
    SYSCFG->EXTICR[3] = CLEAR; 
}


// SYSCFG register source set 
// TODO 
// - this function uses "pin" instead of the appropriate EXTI 
void syscfg_config(
    exti_port_t port, 
    pin_selector_t pin)
{
    if (pin < PIN_4)
        SYSCFG->EXTICR[0] |= (port << pin*SHIFT_4); 
    else if (pin < PIN_8)
        SYSCFG->EXTICR[1] |= (port << (pin-PIN_4)*SHIFT_4); 
    else if (pin < PIN_12)
        SYSCFG->EXTICR[2] |= (port << (pin-PIN_8)*SHIFT_4); 
    else 
        SYSCFG->EXTICR[3] |= (port << (pin-PIN_12)*SHIFT_4); 
}

//================================================================================


//================================================================================
// EXTI Register Functions 

// Interrupt mask register set 
void exti_imr_set(uint32_t im)
{
    EXTI->IMR |= im; 
}


// Interrupt mask register clear 
void exti_imr_clear(uint32_t im)
{
    EXTI->IMR &= ~im; 
}


// Event mask register set 
void exti_emr_set(uint32_t em)
{
    EXTI->EMR |= em; 
}


// Event mask register clear 
void exti_emr_clear(uint32_t em)
{
    EXTI->EMR &= ~em; 
}


// Rising trigger selection register set 
void exti_rtsr_set(uint32_t rt)
{
    EXTI->RTSR |= rt; 
}


// Rising trigger selection register clear 
void exti_rtsr_clear(uint32_t rt)
{
    EXTI->RTSR &= ~rt; 
}


// Falling trigger selection register set 
void exti_ftsr_set(uint32_t ft)
{
    EXTI->FTSR |= ft; 
}


// Falling trigger selection register clear 
void exti_ftsr_clear(uint32_t ft)
{
    EXTI->FTSR &= ~ft; 
}


// Software interrupt event register set 
void exti_swier_set(uint32_t swier)
{
    EXTI->SWIER |= swier; 
}


// Software interrupt event register clear 
void exti_swier_clear(uint32_t swier)
{
    EXTI->SWIER &= ~swier; 
}


// Pending register set 
void exti_pr_set(uint32_t pr)
{
    EXTI->PR |= pr; 
}


// Pending register clear 
void exti_pr_clear(uint32_t pr)
{
    EXTI->PR &= ~pr; 
}

//================================================================================
