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

#ifndef _GPIO_DRIVER_H_
#define _GPIO_DRIVER_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Alternate functions 
#define GPIO_AF0    0x0     // AF0 
#define GPIO_AF1    0x1     // AF1 
#define GPIO_AF2    0x2     // AF2 
#define GPIO_AF3    0x3     // AF3 
#define GPIO_AF4    0x4     // AF4 
#define GPIO_AF5    0x5     // AF5 
#define GPIO_AF6    0x6     // AF6 
#define GPIO_AF7    0x7     // AF7 
#define GPIO_AF8    0x8     // AF8 
#define GPIO_AF9    0x9     // AF9 
#define GPIO_AF10   0xA     // AF10 
#define GPIO_AF11   0xB     // AF11 
#define GPIO_AF12   0xC     // AF12 
#define GPIO_AF13   0xD     // AF13 
#define GPIO_AF14   0xE     // AF14 
#define GPIO_AF15   0xF     // AF15 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief GPIO pin number 
 * 
 * @details This is used as an argument in the GPIO pin write and read functions to specify 
 *          pin number. The pins numbers are assigned to a register bit. 
 */
typedef enum {
    GPIOX_PIN_0  = 0x0001,
    GPIOX_PIN_1  = 0x0002,
    GPIOX_PIN_2  = 0x0004,
    GPIOX_PIN_3  = 0x0008,
    GPIOX_PIN_4  = 0x0010,
    GPIOX_PIN_5  = 0x0020,
    GPIOX_PIN_6  = 0x0040,
    GPIOX_PIN_7  = 0x0080,
    GPIOX_PIN_8  = 0x0100,
    GPIOX_PIN_9  = 0x0200,
    GPIOX_PIN_10 = 0x0400,
    GPIOX_PIN_11 = 0x0800,
    GPIOX_PIN_12 = 0x1000,
    GPIOX_PIN_13 = 0x2000,
    GPIOX_PIN_14 = 0x4000,
    GPIOX_PIN_15 = 0x8000
} gpio_pin_num_t;


/**
 * @brief GPIO pin state
 */
typedef enum {
    GPIO_LOW,
    GPIO_HIGH
} gpio_pin_state_t;


/**
 * @brief GPIO mode register (MODER) settings 
 */
typedef enum {
    MODER_INPUT,   // Input (reset state)
    MODER_GPO,     // General purpose output mode 
    MODER_AF,      // Alternate function mode 
    MODER_ANALOG   // Analog mode 
} gpio_moder_t;


/**
 * @brief GPIO output type (OTYPER) register settings 
 */
typedef enum {
    OTYPER_PP,  // Output push-pull (reset state) 
    OTYPER_OD   // Output open-drain 
} gpio_otyper_t;


/**
 * @brief GPIO output speed (OSPEEDR) register settings 
 */
typedef enum {
    OSPEEDR_LOW,   // Low speed 
    OSPEEDR_MED,   // Medium speed 
    OSPEEDR_FAST,  // Fast speed 
    OSPEEDR_HIGH   // High speed 
} gpio_ospeedr_t;


/**
 * @brief GPIO pull-up/pull-down (PUPDR) register settings 
 */
typedef enum {
    PUPDR_NO,  // No pull-up, pull-down 
    PUPDR_PU,  // Pull-up 
    PUPDR_PD,  // Pull-down 
    PUPDR_RE   // Reserved 
} gpio_pupdr_t;

//=======================================================================================


//=======================================================================================
// GPIO pin state 

typedef gpio_pin_state_t GPIO_STATE; 

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief GPIO communication initialization 
 * 
 * @details Initializes the GPIO clocks so that all peripheralls can use the different 
 *          GPIO ports (A, B, C...). If the GPIO clocks are not enabled then the peripherals 
 *          will not work. 
 */
void gpio_port_init(void); 


/**
 * @brief GPIO pin initialization 
 * 
 * @see gpio_moder_t
 * @see gpio_otyper_t
 * @see gpio_ospeedr_t
 * @see gpio_pupdr_t
 * 
 * @param gpio : pointer to GPIO port to initialize 
 * @param pin_num : pin number for a given GPIO port to initialize 
 * @param moder : mode register setting 
 * @param otyper : output type register setting 
 * @param ospeedr : output speed register setting 
 * @param pupdr : pull-up and pull-down register settings 
 */
void gpio_pin_init(
    GPIO_TypeDef  *gpio, 
    pin_selector_t pin_num,
    gpio_moder_t   moder,
    gpio_otyper_t  otyper,
    gpio_ospeedr_t ospeedr,
    gpio_pupdr_t   pupdr);

//=======================================================================================


//=======================================================================================
// Write functions 

/**
 * @brief GPIO write
 * 
 * @details This function writes the specified pin to the specified state (high or low). 
 * 
 * @see gpio_pin_num_t
 * @see gpio_pin_state_t
 * 
 * @param gpio : pointer to GPIO port to write to 
 * @param pin_num : GPIO pin number to write to 
 * @param pin_state : pin state to write 
 */
void gpio_write(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num,
    gpio_pin_state_t pin_state);

//=======================================================================================


//=======================================================================================
// Read functions 

/**
 * @brief GPIO read 
 * 
 * @details This function reads the state (high or low) of the specified pin. 
 * 
 * @see gpio_pin_num_t
 * @see gpio_pin_state_t
 * 
 * @param gpio : pointer to GPIO port to read 
 * @param pin_num : GPIO pin number to read from 
 * @return GPIO_STATE : pin state read 
 */
GPIO_STATE gpio_read(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num); 

//=======================================================================================


//=======================================================================================
// Register functions 

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
 * @brief GPIO alternate functions 
 * 
 * @details Sets the alternate function of the pin. There are macros defined at the top 
 *          of this file that can be used as arguments. Refer to the reference manual and 
 *          datasheet of the device for details on what each alternate function does. 
 * 
 * @see pin_selector_t
 * 
 * @param gpio : pointer to GPIO port 
 * @param af : alternate function number 
 * @param pin : pin number 
 */
void gpio_afr(
    GPIO_TypeDef *gpio, 
    uint8_t af, 
    pin_selector_t pin); 

//=======================================================================================

#endif  // _GPIO_DRIVER_H_
