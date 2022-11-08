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
// Function Prototypes 

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
    pin_selector_t exti); 


/**
 * @brief Interrupt mask
 * 
 * @details 
 *          Enables the EXTI by unmasking the event request on a given line. 
 *          Disabled the EXTI by masking the event request on a given line. 
 * 
 * @param mask 
 * @param im 
 */
void exti_imr(
    exti_int_mask_t mask, 
    uint32_t im); 


/**
 * @brief Event mask
 * 
 * @details 
 * 
 * @param mask 
 * @param em 
 */
void exti_emr(
    exti_event_mask_t mask, 
    uint32_t em); 


/**
 * @brief Rising trigger selection
 * 
 * @details 
 * 
 * @param rtsr 
 * @param rt 
 */
void exti_rtsr(
    exti_rise_trigger_t rtsr, 
    uint32_t rt); 


/**
 * @brief Falling trigger selection
 * 
 * @details 
 * 
 * @param ftsr 
 * @param ft 
 */
void exti_ftsr(
    exti_fall_trigger_t ftsr, 
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
 * @brief Pending register clear 
 * 
 * @details 
 *          Used the macros defined for the EXTI lines above. 
 *          Calling this function also clears the software interrupt event register. 
 * 
 * @param pr 
 */
void exti_pr_clear(
    uint32_t pr); 

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
    uint32_t exti_line, 
    exti_int_mask_t int_mask, 
    exti_event_mask_t event_mask, 
    exti_rise_trigger_t rise_trig, 
    exti_fall_trigger_t fall_trig)
{
    // Configure the EXTI config register in SYSCFG - Defines the interrupt source 
    syscfg_config(port, pin); 

    // Configure the interrupt mask 
    exti_imr(int_mask, exti_line); 

    // Configure the event mask 
    exti_emr(event_mask, exti_line); 

    // Configure the rising edge trigger 
    exti_rtsr(rise_trig, exti_line); 

    // Configure the falling edge trigger 
    exti_ftsr(fall_trig, exti_line); 
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
void syscfg_config(
    exti_port_t port, 
    pin_selector_t exti)
{
    if (exti < PIN_4)
        SYSCFG->EXTICR[0] |= (port << exti*SHIFT_4); 
    else if (exti < PIN_8)
        SYSCFG->EXTICR[1] |= (port << (exti-PIN_4)*SHIFT_4); 
    else if (exti < PIN_12)
        SYSCFG->EXTICR[2] |= (port << (exti-PIN_8)*SHIFT_4); 
    else 
        SYSCFG->EXTICR[3] |= (port << (exti-PIN_12)*SHIFT_4); 
}

//================================================================================


//================================================================================
// EXTI Register Functions 

// Interrupt mask
void exti_imr(
    exti_int_mask_t mask, 
    uint32_t im)
{
    if (mask) EXTI->IMR |= im; 
    else EXTI->IMR &= ~im; 
}


// Event mask
void exti_emr(
    exti_event_mask_t mask, 
    uint32_t em)
{
    if (mask) EXTI->EMR |= em; 
    else EXTI->EMR &= ~em; 
}


// Rising trigger selection
void exti_rtsr(
    exti_rise_trigger_t rtsr, 
    uint32_t rt)
{
    if (rtsr) EXTI->RTSR |= rt; 
    else EXTI->RTSR &= ~rt; 
}


// Falling trigger selection
void exti_ftsr(
    exti_fall_trigger_t ftsr, 
    uint32_t ft)
{
    if (ftsr) EXTI->FTSR |= ft; 
    else EXTI->FTSR &= ~ft; 
}


// Software interrupt event register set 
void exti_swier_set(uint32_t swier)
{
    EXTI->SWIER |= swier; 
}


// Pending register clear 
void exti_pr_clear(uint32_t pr)
{
    EXTI->PR |= pr; 
}

//================================================================================
