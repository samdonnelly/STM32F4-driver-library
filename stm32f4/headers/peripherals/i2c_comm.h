/**
 * @file i2c_comm.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C driver interface 
 * 
 * @version 0.1
 * @date 2022-03-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _I2C_COMM_H_
#define _I2C_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "tools.h" 
#include "gpio_driver.h" 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief I2C read and write bit offset 
 * 
 * @details Used in conjunction with an I2C module address to indicate a read or write 
 *          operation at bit 0. 
 */
typedef enum {
    I2C_W_OFFSET, 
    I2C_R_OFFSET
} i2c_rw_offset_t; 


/**
 * @brief I2C operation status 
 */
typedef enum {
    I2C_OK, 
    I2C_TIMEOUT, 
    I2C_NULL_PTR 
} i2c_status_t; 


/**
 * @brief I2C run mode selection 
 * 
 * @details I2C can run in standard (SM) or fast (FM) mode. The mode dictates the range 
 *          of SCL clock frequencies that can be run. The I2C init functions uses this 
 *          enum as an input when the function is called so it can set the desired
 *          run mode. 
 *          
 *          When I2C is initialized in FM mode for faster clock frequencies the duty 
 *          cycle can be chosen: 
 *           - I2C_MODE_FM_2: duty cycle = t_low/t_high = 2 
 *           - I2C_MODE_FM_169: duty cycle = t_low/t_high = 16/9 
 */
typedef enum {
    I2C_MODE_SM,
    I2C_MODE_FM_2, 
    I2C_MODE_FM_169
} i2c_run_mode_t;


/**
 * @brief I2C AP1 frequency 
 * 
 * @details The I2C init functions take this as an argument in order to program the 
 *          peripheral input clock based in the frequency of APB1. 
 */
typedef enum {
    I2C_APB1_42MHZ = 42,
    I2C_APB1_84MHZ = 84
} i2c_apb1_freq_t;


/**
 * @brief I2C CCR setpoint
 * 
 * @details The I2C init functions take this an argument to program the clock control 
 *          register when initializing in Fm/Sm mode. 
 *          
 *          enum code: I2C_CCR_FM_(X_1)_(X_2)_(X_3) 
 *              X_1: Duty cycle - ex. 169 -> 16/9 in Fm mode 
 *              X_2: PCLK1 frquency (MHz) 
 *              X_3: SCL frquency (kHz) 
 *          
 *          enum code: I2C_CCR_SM_(X_1)_(X_2) 
 *              X_1: PCLK1 frquency (MHz) 
 *              X_2: SCL frquency (kHz) 
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual for more information. 
 */
typedef enum {
    I2C_CCR_FM_169_42_400 = 5,
    I2C_CCR_SM_42_100 = 210
} i2c_ccr_setpoint_t;


/**
 * @brief I2C TRISE setpoint
 * 
 * @details The I2C init functions take this as an argument to program the rise timer 
 *          register based on the clock frequency and max rise time which changes based 
 *          on the run mode. 
 *          
 *          enum code: I2C_TRISE_(X_1)_(X_2) 
 *              X_1: Max rise time (ns) 
 *              X_2: PCLK1 frequency (MHz) 
 *          
 *          NOTE: A calculation must be done to determine the numbers that work together.
 *                See the reference manual for more information. 
 */
typedef enum {
    I2C_TRISE_0300_42 = 13,
    I2C_TRISE_1000_42 = 43
} i2c_trise_setpoint_t;

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef i2c_status_t I2C_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief I2C Initialization 
 * 
 * @details Configures I2C using two of the pins specified below. One SCL and one SDA 
 *          must be selected. The enums referenced in the comments below are used to 
 *          define the inputs to the function. Refer to the reference manual for 
 *          detailed information on configuring the pins. 
 * 
 * @param i2c : I2C port to initialize 
 * @param sda_pin : pin used for SDA  
 * @param sda_gpio : pointer to GPIO port of SDA pin 
 * @param scl_pin : pin used for SCL 
 * @param scl_gpio : pointer to GPIO port of SCL pin 
 * @param run_mode : specifies Sm or Fm mode 
 * @param apb1_freq : configured APB1 clock frquency 
 * @param ccr_reg : calculated clock control register value
 * @param trise_reg : calculated trise time 
 */
void i2c_init(
    I2C_TypeDef *i2c, 
    pin_selector_t sda_pin, 
    GPIO_TypeDef *sda_gpio, 
    pin_selector_t scl_pin, 
    GPIO_TypeDef *scl_gpio, 
    i2c_run_mode_t run_mode,
    i2c_apb1_freq_t apb1_freq,
    i2c_ccr_setpoint_t ccr_reg,
    i2c_trise_setpoint_t trise_reg);

//=======================================================================================


//=======================================================================================
// Register functions 

/**
 * @brief I2C start condition generation 
 * 
 * @details This must be called by the master to begin an I2C read or write 
 *          transmisssion. The controller is in slave mode when idle but becomes 
 *          the master when the start condition is generated. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to I2C port 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_start(I2C_TypeDef *i2c);


/**
 * @brief I2C stop condition condition 
 * 
 * @details This must be called in order to end an I2C read or write transmission. 
 *          Once the stop condition is sent then the controller releases the bus and 
 *          reverts back to slave mode. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_stop(I2C_TypeDef *i2c);


/**
 * @brief I2C clear ADDR bit
 * 
 * @details This bit must be cleared before data can start to be sent on the bus. The 
 *          function gets called after the slave has acknowledged the address sent by 
 *          the master. 
 * 
 * @param i2c : pointer to I2C port 
 */
void i2c_clear_addr(I2C_TypeDef *i2c);

//=======================================================================================


//=======================================================================================
// Write data 

/**
 * @brief I2C write address 
 * 
 * @details Sends the slave address to the bus to initialize communication with a certain 
 *          device. This function is called after the start condition has been generated. 
 *          Only 7-bit addresses are supported. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port 
 * @param i2c_address : 7-bit address of slave device on the bus 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_write_addr(
    I2C_TypeDef *i2c, 
    uint8_t i2c_address);


/**
 * @brief I2C write data
 * 
 * @details Write data to a slave device. This function is called after the ADDR bit has 
 *          been cleared. The function takes a pointer to the data that will be send over 
 *          the bus and the size of the data so it knows how many bytes to send. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port 
 * @param data : pointer to data to be sent over the bus 
 * @param data_size : integer indicating the number of bytes to be sent 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_write(
    I2C_TypeDef *i2c, 
    const uint8_t *data, 
    uint8_t data_size);

//=======================================================================================


//=======================================================================================
// Read data 

/**
 * @brief Read I2C data
 * 
 * @details Read data from a slave device. This function is called after the ADDR bit has 
 *          been cleared. The function takes a pointer where it will store the recieved 
 *          data and the size of the data so it knows how many bytes to read. 
 * 
 * @see i2c_status_t
 *          
 * @param i2c : pointer to the I2C port 
 * @param data : pointer that data is placed into 
 * @param data_size : integer indicating the number of bytes to be receieved 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_read(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size);


/**
 * @brief Clear I2C data 
 * 
 * @param i2c 
 * @param data_size 
 * @return I2C_STATUS 
 */
I2C_STATUS i2c_clear(
    I2C_TypeDef *i2c, 
    uint16_t data_size); 


/**
 * @brief I2C read data until a termination character is seen 
 * 
 * @details This function is for reading data of unknown or varying length. It reads 
 *          data in the same way as i2c_read but takes a termination character
 *          and the number of bytes left to read after the termination character as 
 *          arguments. Once the termination character is seen then the transaction will 
 *          be stopped based on how many bytes remain to be read. There must be at least one 
 *          byte left to read. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port used 
 * @param data : pointer to the buffer that stores read data 
 * @param term_char : termination character to indicate when to stop reading 
 * @param bytes_remain : number of bytes left to read after the termination character 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_read_to_term(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t term_char, 
    uint16_t bytes_remain);


/**
 * @brief I2C read data of a certain length that is defined within the message 
 * 
 * @details This function was created for the M8Q driver. The M8Q message protocol contains 
 *          the message length in one of it's message formats (UBX). There are different 
 *          messages of different lengths within this format and there is no easy indicator 
 *          for the end of the data portion of the message. This function reads the portion 
 *          of the message that specifies the length so the code knows how many bytes to read 
 *          from the device. 
 *          
 *          NOTE: Reading the length in the message is currently only supported 
 *                for little endian format. 
 * 
 * @see i2c_status_t
 * 
 * @param i2c : pointer to the I2C port used 
 * @param address : I2C address of the device 
 * @param data : pointer to buffer that stores the read data  
 * @param len_location : where the message length is located in the message (bytes) 
 * @param len_bytes : number of bytes used to define the length within the message 
 * @param add_bytes : additional bytes to read at the end of the message (if needed) 
 * @return I2C_STATUS : I2C operation status 
 */
I2C_STATUS i2c_read_to_len(
    I2C_TypeDef *i2c, 
    uint8_t address, 
    uint8_t *data, 
    uint8_t len_location, 
    uint8_t len_bytes, 
    uint8_t add_bytes); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _I2C_COMM_H_ 
