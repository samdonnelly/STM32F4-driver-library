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


//================================================================================
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

//================================================================================


//=======================================================================================
// Enums 

// TODO when you get rid of HAL then change this back to GPIO_PIN_X
/**
 * @brief GPIO pin number 
 * 
 * @details 
 * 
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
 * 
 * @details 
 * 
 */
typedef enum {
    GPIO_LOW,
    GPIO_HIGH
} gpio_pin_state_t;


/**
 * @brief GPIO port mode register (MODER) register 
 * 
 * @details 
 * 
 */
typedef enum {
    MODER_INPUT,   // Input (reset state)
    MODER_GPO,     // General purpose output mode 
    MODER_AF,      // Alternate function mode 
    MODER_ANALOG   // Analog mode 
} gpio_moder_t;


/**
 * @brief GPIO port output type (OTYPER) register 
 * 
 * @details 
 * 
 */
typedef enum {
    OTYPER_PP,  // Output push-pull (reset state) 
    OTYPER_OD   // Output open-drain 
} gpio_otyper_t;


/**
 * @brief GPIO port output speed (OSPEEDR) register
 * 
 * @details 
 * 
 */
typedef enum {
    OSPEEDR_LOW,   // Low speed 
    OSPEEDR_MED,   // Medium speed 
    OSPEEDR_FAST,  // Fast speed 
    OSPEEDR_HIGH   // High speed 
} gpio_ospeedr_t;


/**
 * @brief GPIO port pull-up/pull-down (PUPDR) register 
 * 
 * @details 
 * 
 */
typedef enum {
    PUPDR_NO,  // No pull-up, pull-down 
    PUPDR_PU,  // Pull-up 
    PUPDR_PD,  // Pull-down 
    PUPDR_RE   // Reserved 
} gpio_pupdr_t;

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief GPIO communication initialization 
 * 
 * @details Initializes the GPIO clocks so that all peripheralls can use the different 
 *          GPIO ports (A, B, C...). If this is not done then initialized pins will 
 *          not work. 
 * 
 */
void gpio_port_init(void); 


/**
 * @brief GPIO pin initialization 
 * 
 * @details 
 * 
 * @param gpio 
 * @param pin_num 
 * @param moder 
 * @param otyper 
 * @param ospeedr 
 * @param pupdr 
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
void gpio_write(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num,
    gpio_pin_state_t pin_state);

//=======================================================================================


//=======================================================================================
// Read functions 

/**
 * @brief GPIOA read 
 * 
 * @details 
 * 
 * @see gpio_pin_num_t
 * 
 * @param gpio : which GPIO to access (A, B, C, etc.) 
 * @param pin_num : GPIO pin specified by gpio_pin_num_t
 * @return uint8_t 
 */
uint8_t gpio_read(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t pin_num); 

//=======================================================================================


//================================================================================
// Register functions 

// TODO move these prototypes over to the source once they're no longer used in other files 

/**
 * @brief Set the GPIO mode 
 * 
 * @details 
 * 
 * @param gpio 
 * @param moder 
 * @param pin 
 */
void gpio_moder(
    GPIO_TypeDef *gpio, 
    gpio_moder_t moder, 
    pin_selector_t pin); 


/**
 * @brief GPIO output type 
 * 
 * @details 
 * 
 * @param gpio 
 * @param otyper 
 * @param pin 
 */
void gpio_otyper(
    GPIO_TypeDef *gpio, 
    gpio_otyper_t otyper, 
    pin_selector_t pin); 


/**
 * @brief GPIO output speed 
 * 
 * @details 
 * 
 * @param gpio 
 * @param ospeedr 
 * @param pin 
 */
void gpio_ospeedr(
    GPIO_TypeDef *gpio, 
    gpio_ospeedr_t ospeedr, 
    pin_selector_t pin); 


/**
 * @brief GPIO pull-up/pull-down  
 * 
 * @details 
 * 
 * @param gpio 
 * @param pupdr 
 * @param pin 
 */
void gpio_pupdr(
    GPIO_TypeDef *gpio, 
    gpio_pupdr_t pupdr, 
    pin_selector_t pin); 


/**
 * @brief GPIO alternate functions 
 * 
 * @details 
 * 
 * @param gpio 
 * @param af 
 * @param pin 
 */
void gpio_afr(
    GPIO_TypeDef *gpio, 
    uint8_t af, 
    pin_selector_t pin); 

//================================================================================

#endif  // _GPIO_DRIVER_H_
