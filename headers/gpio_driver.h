/**
 * @file gpio_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPIO functions
 * 
 * @version 0.1
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief GPIO pin number 
 * 
 * @details 
 * 
 */
typedef enum {
    GPIO_PIN_0  = 0x0001,
    GPIO_PIN_1  = 0x0002,
    GPIO_PIN_2  = 0x0004,
    GPIO_PIN_3  = 0x0008,
    GPIO_PIN_4  = 0x0010,
    GPIO_PIN_5  = 0x0020,
    GPIO_PIN_6  = 0x0040,
    GPIO_PIN_7  = 0x0080,
    GPIO_PIN_8  = 0x0100,
    GPIO_PIN_9  = 0x0200,
    GPIO_PIN_10 = 0x0400,
    GPIO_PIN_11 = 0x0800,
    GPIO_PIN_12 = 0x1000,
    GPIO_PIN_13 = 0x2000,
    GPIO_PIN_14 = 0x4000,
    GPIO_PIN_15 = 0x8000
} gpio_pin_num_t;


/**
 * @brief GPIO pin state
 * 
 * @details 
 * 
 */
typedef enum {
    GPIO_LOW,
    GPIO_HIGH
} gpio_pin_state_t;


/**
 * @brief Pin configuration mode
 * 
 */
typedef enum {
    MODE_INPUT,   // Input (reset state)
    MODE_GPO,     // General purpose output mode 
    MODE_AF,      // Alternate function mode 
    MODE_ANALOG   // Analog mode 
} gpio_pin_mode_t;

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief GPIOB initialization 
 * 
 * @details 
 * 
 * @param pin_num
 * @param moder 
 * @param otyper 
 * @param ospeedr 
 * @param pupdr 
 */
void gpiob_init(
    uint16_t pin_num,
    uint32_t moder,
    uint32_t otyper,
    uint32_t ospeedr,
    uint32_t pupdr);


/**
 * @brief 
 * 
 * @param moder 
 */
void gpiob_moder(
    uint16_t pin_num,
    uint32_t moder);


/**
 * @brief 
 * 
 * @param otyper 
 */
void gpiob_otyper(
    uint16_t pin_num,
    uint32_t otyper);


/**
 * @brief 
 * 
 * @param ospeedr 
 */
void gpiob_ospeedr(
    uint16_t pin_num,
    uint32_t ospeedr);


/**
 * @brief 
 * 
 * @param pupdr 
 */
void gpiob_pupdr(
    uint16_t pin_num,
    uint32_t pupdr);

//=======================================================================================


//=======================================================================================
// Write functions 

/**
 * @brief GPIOA write
 * 
 * @details This function writes "A" pins congigured as GPIO to either high or low (on or
 *          off). Arguments include the pin number to write to and the state to write the 
 *          pin to. The pin number can be specified using the gpio_pin_num_t enum and the 
 *          pin state can be specified using the gpio_pin_state_t enum. 
 * 
 * @see gpio_pin_state_t
 * @see gpio_pin_num_t
 * 
 * @param pin_num : GPIOA pin to write to (0-15)
 * @param pin_state : on (1) or off (0) state
 */
void gpioa_write(
    uint16_t pin_num,
    uint8_t  pin_state);


/**
 * @brief GPIOB write
 * 
 * @details This function writes "B" pins congigured as GPIO to either high or low (on or
 *          off). Arguments include the pin number to write to and the state to write the 
 *          pin to. The pin number can be specified using the gpio_pin_num_t enum and the 
 *          pin state can be specified using the gpio_pin_state_t enum. 
 * 
 * @see gpio_pin_state_t
 * @see gpio_pin_num_t
 * 
 * @param pin_num : GPIOB pin to write to (0-15)
 * @param pin_state : on (1) or off (0) state
 */
void gpiob_write(
    uint16_t pin_num,
    uint8_t  pin_state);


/**
 * @brief GPIOC write
 * 
 * @details This function writes "C" pins congigured as GPIO to either high or low (on or
 *          off). Arguments include the pin number to write to and the state to write the 
 *          pin to. The pin number can be specified using the gpio_pin_num_t enum and the 
 *          pin state can be specified using the gpio_pin_state_t enum. 
 * 
 * @see gpio_pin_state_t
 * @see gpio_pin_num_t
 * 
 * @param pin_num : GPIOC pin to write to (0-15)
 * @param pin_state : on (1) or off (0) state
 */
void gpioc_write(
    uint16_t pin_num,
    uint8_t  pin_state);

//=======================================================================================
