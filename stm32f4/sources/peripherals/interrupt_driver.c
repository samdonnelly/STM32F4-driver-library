/**
 * @file interrupt_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt driver 
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
 * @details Clears the external interrupt congiguration registers so that they can be 
 *          configured without conflict. This is called once at the beginning of the code. 
 */
void syscfg_config_clear(void); 


/**
 * @brief SYSCFG register source set 
 * 
 * @details This function defines the source of the interrupt. The port is written to the 
 *          given event line defined by the pin number. EXTI line numbers correspond to pin 
 *          numbers in a GPIO port. 
 * 
 * @see exti_port_t
 * @see pin_selector_t
 * 
 * @param port : GPIO port index 
 * @param pin : pin number of EXTI pin 
 */
void syscfg_config(
    exti_port_t port, 
    pin_selector_t exti); 


/**
 * @brief Interrupt mask
 * 
 * @details Enables or disables the external interrupt on a given line. 
 * 
 * @see exti_int_mask_t
 * 
 * @param mask : interrupt mask configuration 
 * @param im : EXTI line to configure an external interrupt - use EXTI_LX macros above 
 */
void exti_imr(
    exti_int_mask_t mask, 
    uint32_t im); 


/**
 * @brief Event mask
 * 
 * @details Enables or disables the external event on a given line. 
 * 
 * @see exti_event_mask_t
 * 
 * @param mask : event mask configuration 
 * @param em : EXTI line to configure an external event - use EXTI_LX macros above 
 */
void exti_emr(
    exti_event_mask_t mask, 
    uint32_t em); 


/**
 * @brief Rising trigger selection
 * 
 * @details Allows for configuring the rising edge signal on the EXTI to trigger an interrupt. 
 *          Note that a pin can have both rising and falling edge triggers. 
 * 
 * @see exti_rise_trigger_t
 * 
 * @param rtsr : rising edge interrupt trigger configuration 
 * @param rt : EXTI line to configure the rising edge trigger - use EXTI_LX macros above 
 */
void exti_rtsr(
    exti_rise_trigger_t rtsr, 
    uint32_t rt); 


/**
 * @brief Falling trigger selection
 * 
 * @details Allows for configuring the falling edge signal on the EXTI to trigger an interrupt. 
 *          Note that a pin can have both rising and falling edge triggers. 
 * 
 * @see exti_fall_trigger_t
 * 
 * @param ftsr : falling edge interrupt trigger configuration 
 * @param ft : EXTI line to configure the falling edge trigger - use EXTI_LX macros above 
 */
void exti_ftsr(
    exti_fall_trigger_t ftsr, 
    uint32_t ft); 


/**
 * @brief Software interrupt event register set 
 * 
 * @details This allows for generation of an interrupt/event request using the software instead 
 *          of an external device/peripheral trigger. 
 * 
 * @param swier : EXTI line for software to trigger an event on - use EXTI_LX macros above 
 */
void exti_swier_set(uint32_t swier); 

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
    GPIO_TypeDef *gpio, 
    exti_port_t port, 
    pin_selector_t pin, 
    gpio_pupdr_t pull, 
    uint32_t exti_line, 
    exti_int_mask_t int_mask, 
    exti_event_mask_t event_mask, 
    exti_rise_trigger_t rise_trig, 
    exti_fall_trigger_t fall_trig)
{
    // Configure an input pin for the interrupt 
    gpio_pin_init(gpio, pin, MODER_INPUT, OTYPER_PP, OSPEEDR_FAST, pull); 

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
    NVIC_SetPriority(irqn, priority);    // Built-in NVIC function 

    // Enable the interrupt 
    NVIC_EnableIRQ(irqn);                // Built-in NVIC function 
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
    {
        SYSCFG->EXTICR[0] |= (port << exti*SHIFT_4); 
    }
    else if (exti < PIN_8)
    {
        SYSCFG->EXTICR[1] |= (port << (exti-PIN_4)*SHIFT_4); 
    }
    else if (exti < PIN_12)
    {
        SYSCFG->EXTICR[2] |= (port << (exti-PIN_8)*SHIFT_4); 
    }
    else 
    {
        SYSCFG->EXTICR[3] |= (port << (exti-PIN_12)*SHIFT_4); 
    }
}

//================================================================================


//================================================================================
// EXTI Register Functions 

// Interrupt mask
void exti_imr(
    exti_int_mask_t mask, 
    uint32_t im)
{
    mask ? (EXTI->IMR |= im) : (EXTI->IMR &= ~im); 
}


// Event mask
void exti_emr(
    exti_event_mask_t mask, 
    uint32_t em)
{
    mask ? (EXTI->EMR |= em) : (EXTI->EMR &= ~em); 
}


// Rising trigger selection
void exti_rtsr(
    exti_rise_trigger_t rtsr, 
    uint32_t rt)
{
    rtsr ? (EXTI->RTSR |= rt) : (EXTI->RTSR &= ~rt); 
}


// Falling trigger selection
void exti_ftsr(
    exti_fall_trigger_t ftsr, 
    uint32_t ft)
{
    ftsr ? (EXTI->FTSR |= ft) : (EXTI->FTSR &= ~ft); 
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
