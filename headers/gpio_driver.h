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
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    GPIO_A,
    GPIO_B,
    GPIO_C
} gpio_x_t; 


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
 * @brief GPIOA initialization 
 * 
 * @details 
 * 
 * @param pin_num
 * @param moder 
 * @param otyper 
 * @param ospeedr 
 * @param pupdr 
 */
void gpioa_init(
    uint8_t  pin_num,
    uint32_t moder,
    uint32_t otyper,
    uint32_t ospeedr,
    uint32_t pupdr);


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
    uint8_t gpio, 
    uint16_t pin_num); 

//=======================================================================================
