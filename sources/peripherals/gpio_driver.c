/**
 * @file gpio_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPIO driver 
 * 
 * @version 0.1
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "gpio_driver.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Set the GPIO mode 
 * 
 * @details Configures the mode of the pin according to gpio_moder_t
 * 
 * @see gpio_moder_t
 * @see pin_selector_t
 * 
 * @param gpio : pointer to GPIO port 
 * @param moder : mode of the pin 
 * @param pin : pin number 
 */
void gpio_moder(
    GPIO_TypeDef *gpio, 
    gpio_moder_t moder, 
    pin_selector_t pin); 


/**
 * @brief GPIO output type 
 * 
 * @details Sets the output type of the I/O pin using the OTYPER register. 
 * 
 * @see gpio_otyper_t
 * 
 * @param gpio : pointer to GPIO port 
 * @param otyper : output type setting 
 * @param pin : pin number 
 */
void gpio_otyper(
    GPIO_TypeDef *gpio, 
    gpio_otyper_t otyper, 
    pin_selector_t pin); 


/**
 * @brief GPIO output speed 
 * 
 * @details Sets the speed of the I/O pin somewhere from low to high using the OSPEEDR 
 *          register. 
 * 
 * @see gpio_ospeedr_t
 * 
 * @param gpio : pointer to GPIO port 
 * @param ospeedr : output speed setting 
 * @param pin : pin number 
 */
void gpio_ospeedr(
    GPIO_TypeDef *gpio, 
    gpio_ospeedr_t ospeedr, 
    pin_selector_t pin); 


/**
 * @brief GPIO pull-up/pull-down  
 * 
 * @details Sets the I/O pin as either pull-up or pull-down using the PUPDR register. 
 * 
 * @see gpio_pupdr_t
 * 
 * @param gpio : pointer to GPIO port 
 * @param pupdr : pull-up and pull-down setting 
 * @param pin : pin number 
 */
void gpio_pupdr(
    GPIO_TypeDef *gpio, 
    gpio_pupdr_t pupdr, 
    pin_selector_t pin); 

//=======================================================================================


//=======================================================================================
// Initialization 

// GPIO communication initialization 
void gpio_port_init(void)
{    
    // Enable GPIOA clock - RCC_AHB1ENR register, bit 0 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

    // Enable GPIOB clock - RCC_AHB1ENR register, bit 1 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // Enable GPIOC clock - RCC_AHB1ENR register, bit 2 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_2);

    // Enable GPIOD clock - RCC_AHB1ENR register, bit 3 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_3);
}


// GPIO pin initialization 
void gpio_pin_init(
    GPIO_TypeDef  *gpio, 
    pin_selector_t pin_num,
    gpio_moder_t   moder,
    gpio_otyper_t  otyper,
    gpio_ospeedr_t ospeedr,
    gpio_pupdr_t   pupdr)
{
    // Set the mode 
    gpio_moder(gpio, moder, pin_num); 

    // Set the output type 
    gpio_otyper(gpio, otyper, pin_num); 

    // Set the output speed 
    gpio_ospeedr(gpio, ospeedr, pin_num); 

    // Set as pull-up or pull-down 
    gpio_pupdr(gpio, pupdr, pin_num); 
}


// GPIO Alternate Function Register (AFR) configuration 
void gpio_afr(
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    bit_setter_t setpoint)
{
    // Local variables 
    uint8_t afr_index = 0; 

    // Adjust the AFR index and pin offset if needed 
    // The AFR is an array of size 2 - index 0: pins 0-7, index 1: 8-15 
    if (pin > PIN_7) 
    {
        afr_index++; 
        pin -= PIN_8; 
    }

    // Clear and config the AFR 
    // Each pin has 4 bits for configuration, hence multiplication by 4 for pin shifting 
    gpio->AFR[afr_index] &= ~(setpoint << (SHIFT_4*pin)); 
    gpio->AFR[afr_index] |=  (setpoint << (SHIFT_4*pin)); 
}

//=======================================================================================


//=======================================================================================
// Write and read functions 

// GPIOA write 
void gpio_write(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num,
    gpio_pin_state_t pin_state)
{
    switch (pin_state)
    {
        case GPIO_LOW:  // Set pin low 
            gpio->BSRR |= (((uint32_t)pin_num) << SHIFT_16);   // Set the reset bit 
            gpio->BSRR &= ~((uint32_t)pin_num);                // Clear the set bit 
            break;
        
        case GPIO_HIGH: // Set pin high 
            gpio->BSRR &= ~(((uint32_t)pin_num) << SHIFT_16);  // Clear the reset bit 
            gpio->BSRR |=    (uint32_t)pin_num;                // Set the set bit 
            break;
        
        default:  // Nothing done if 0 or 1 is not specified 
            break;
    }
}


// GPIO read 
GPIO_STATE gpio_read(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num)
{
    // Read the GPIO pin 
    if ((gpio->IDR) & pin_num) 
    {
        return GPIO_HIGH; 
    }
    
    return GPIO_LOW; 
}


// GPIO port read 
GPIOX_DR gpio_port_read(GPIO_TypeDef *gpio)
{
    return (GPIOX_DR)(gpio->IDR); 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// GPIO mode 
void gpio_moder(
    GPIO_TypeDef *gpio, 
    gpio_moder_t moder, 
    pin_selector_t pin)
{
    gpio->MODER &= ~(SET_3 << (SHIFT_2*pin)); 
    gpio->MODER |= (moder << (SHIFT_2*pin)); 
}


// GPIO output type 
void gpio_otyper(
    GPIO_TypeDef *gpio, 
    gpio_otyper_t otyper, 
    pin_selector_t pin)
{
    gpio->OTYPER &= ~(SET_BIT << pin); 
    gpio->OTYPER |= (otyper << pin); 
}


// GPIO output speed 
void gpio_ospeedr(
    GPIO_TypeDef *gpio, 
    gpio_ospeedr_t ospeedr, 
    pin_selector_t pin)
{
    gpio->OSPEEDR &= ~(SET_3 << (SHIFT_2*pin)); 
    gpio->OSPEEDR |= (ospeedr << (SHIFT_2*pin)); 
}


// GPIO pull-up/pull-down  
void gpio_pupdr(
    GPIO_TypeDef *gpio, 
    gpio_pupdr_t pupdr, 
    pin_selector_t pin)
{
    gpio->PUPDR &= ~(SET_3 << (SHIFT_2*pin)); 
    gpio->PUPDR |= (pupdr << (SHIFT_2*pin)); 
}

//=======================================================================================
