/**
 * @file i2c_comm.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C initialization, read and write 
 * 
 * @version 0.1
 * @date 2022-03-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _I2C_COMM_H_
#define _I2C_COMM_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief I2C1 available SCL pins 
 * 
 * @details These pins are the available I2C1 SCL pins on the chip. The values associated 
 *          with these variables are used in i2c1_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c1_init as 
 *          an argument. 
 * 
 * @see i2c1_init
 * 
 */
typedef enum {
    I2C1_SCL_PB6 = 0,
    I2C1_SCL_PB8 = 2
} i2c1_scl_pin_t;


/**
 * @brief I2C1 available SDA pins 
 * 
 * @details These pins are the available I2C1 SDA pins on the chip. The values associated 
 *          with these variables are used in i2c1_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c1_init as 
 *          an argument.
 * 
 * @see i2c1_init
 * 
 */
typedef enum {
    I2C1_SDA_PB7 = 0,
    I2C1_SDA_PB9 = 2
} i2c1_sda_pin_t;


/**
 * @brief I2C2 available SCL pins 
 * 
 * @details These pins are the available I2C2 SCL pins on the chip. The values associated 
 *          with these variables are used in i2c2_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c2_init as 
 *          an argument. 
 * 
 * @see i2c2_init
 * 
 */
typedef enum {
    I2C2_SCL_PB10
} i2c2_scl_pin_t;


/**
 * @brief I2C2 available SDA pins 
 * 
 * @details These pins are the available I2C3 SDA pins on the chip. The values associated 
 *          with these variables are used in i2c2_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c2_init as 
 *          an argument.
 * 
 * @see i2c2_init
 * 
 */
typedef enum {
    I2C2_SDA_PB3,
    I2C2_SDA_PB9
} i2c2_sda_pin_t;


/**
 * @brief I2C3 available SCL pins 
 * 
 * @details These pins are the available I2C3 SCL pins on the chip. The values associated 
 *          with these variables are used in i2c3_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c3_init as 
 *          an argument. 
 * 
 * @see i2c3_init
 * 
 */
typedef enum {
    I2C3_SCL_PA8
} i2c3_scl_pin_t;


/**
 * @brief I2C3 available SDA pins 
 * 
 * @details These pins are the available I2C3 SDA pins on the chip. The values associated 
 *          with these variables are used in i2c3_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c3_init as 
 *          an argument.
 * 
 * @see i2c3_init
 * 
 */
typedef enum {
    I2C3_SDA_PB4,
    I2C3_SDA_PB8,
    I2C3_SDA_PC9
} i2c3_sda_pin_t;


/**
 * @brief I2C run mode selection 
 * 
 * @details I2C can run in standard (SM) or fast (FM) mode. The mode dictates the range 
 *          of SCL clock frequencies that can be run. The I2C init functions uses this 
 *          enum as an input when the function is called so it can set the desired
 *          run mode. <br><br>
 *          
 *          When I2C is initialized in FM mode for faster clock frequencies the duty 
 *          cycle can be chosen: <br>
 *           - I2C_MODE_FM_2: duty cycle = t_low/t_high = 2 <br>
 *           - I2C_MODE_FM_169: duty cycle = t_low/t_high = 16/9 
 * 
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
 * 
 */
typedef enum {
    I2C_APB1_42MHZ = 42,
    I2C_APB1_84MHZ = 84
} i2c_apb1_freq_t;


/**
 * @brief I2C CCR setpoint
 * 
 * @details The I2C init functions take this an argument to program the clock control 
 *          register when initializing in Fm/Sm mode. <br><br>
 *          
 *          enum code: I2C_CCR_FM_(X_1)_(X_2)_(X_3)           <br>
 *              X_1: Duty cycle - ex. 169 -> 16/9 in Fm mode  <br>
 *              X_2: PCLK1 frquency (MHz)                     <br>
 *              X_3: SCL frquency (kHz)                       <br><br>
 *          
 *          enum code: I2C_CCR_SM_(X_1)_(X_2)  <br>
 *              X_1: PCLK1 frquency (MHz)      <br>
 *              X_2: SCL frquency (kHz)        <br><br>
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual for more information. 
 * 
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
 *          on the run mode. <br><br> 
 *          
 *          enum code: I2C_TRISE_(X_1)_(X_2)  <br>
 *              X_1: Max rise time (ns)       <br>
 *              X_2: PCLK1 frequency (MHz)    <br><br>
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual for more information. 
 * 
 */
typedef enum {
    I2C_TRISE_0300_42 = 13,
    I2C_TRISE_1000_42 = 43
} i2c_trise_setpoint_t;


/**
 * @brief I2C data size 
 * 
 * @details This is a general enum used for specifying I2C message sizes when sending or 
 *          receiving data. 
 * 
 */
typedef enum {
    I2C_0_BYTE,
    I2C_1_BYTE,
    I2C_2_BYTE,
    I2C_3_BYTE,
    I2C_4_BYTE
} i2c_data_size_t;

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief I2C1 Initialization 
 * 
 * @details Configures I2C1 using two of the pins specified below. One SCL and one SDA 
 *          must be selected. The enums referenced in the comments below are used to 
 *          define the inputs to the function. Refer to the reference manual for 
 *          detailed information on configuring the pins. <br><br>
 *          
 *          Pin information for I2C1: <br>
 *              PB6: SCL              <br>
 *              PB7: SDA              <br> 
 *              PB8: SCL              <br>
 *              PB9: SDA              <br><br>
 *          
 *          Pin information for I2C2: <br>
 *              PB3:  SDA              <br>
 *              PB9:  SDA              <br> 
 *              PB10: SCL              <br><br>
 *          
 *          Pin information for I2C3: <br>
 *              PA8: SCL              <br>
 *              PB4: SDA              <br> 
 *              PB8: SDA              <br>
 *              PC9: SDA              <br><br>
 * 
 * @param i2c : I2C port to initialize 
 * @param sda_pin : pin used for SDA  
 * @param scl_pin : pin used for SCL 
 * @param run_mode : specifies Sm or Fm mode 
 * @param apb1_freq : configured APB1 clock frquency 
 * @param fm_duty_cycle : Fm mode duty cycle (no affect in Sm mode)
 * @param ccr_reg : calculated clock control register value
 * @param trise_reg : calculated trise time 
 * 
 */
void i2c1_init(
    I2C_TypeDef *i2c, 
    i2c1_sda_pin_t sda_pin,
    i2c1_scl_pin_t scl_pin,
    i2c_run_mode_t run_mode,
    i2c_apb1_freq_t apb1_freq,
    i2c_ccr_setpoint_t ccr_reg,
    i2c_trise_setpoint_t trise_reg);

//=======================================================================================


//=======================================================================================
// Register functions 

/**
 * @brief I2C1 start condition generation 
 * 
 * @details This must be called by the master to begin an I2C read or write 
 *          transmisssion. The controller is in slave mode when idle but becomes 
 *          the master when the start condition is generated. 
 * 
 */
void i2c_start(I2C_TypeDef *i2c);


/**
 * @brief I2C1 stop condition condition 
 * 
 * @details This must be called in order to end an I2C read or write transmission. 
 *          Once the stop condition is sent then the controller releases the bus and 
 *          reverts back to slave mode. 
 * 
 */
void i2c_stop(I2C_TypeDef *i2c);


/**
 * @brief I2C1 clear ADDR bit
 * 
 * @details This bit must be cleared before data can start to be sent on the bus. The 
 *          function gets called after the slave has acknowledged the address sent by 
 *          the master. 
 * 
 */
void i2c_clear_addr(I2C_TypeDef *i2c);


/**
 * @brief I2C1 clear acknowledge bit 
 * 
 * @details This function is used to clear the acknowledge bit which sends a NACK pulse to 
 *          the slave device. The NACK pulse is sent after the last byte of data has been 
 *          recieved from the slave. Once the slave sees the pulse it releases control of 
 *          the bus which allows the master to send a stop or restart condition. 
 * 
 */
void i2c_clear_ack(I2C_TypeDef *i2c);


/**
 * @brief I2C1 set acknowledge bit 
 * 
 * @details Setting the acknowledge bit is used to tell a slave device that data has been 
 *          recieved so the slave can proceed to send the next byte of data. This function 
 *          is called immediately after the data register is read. The acknowledge bit must
 *          also be set before generating a start condition. 
 * 
 */
void i2c_set_ack(I2C_TypeDef *i2c);


/**
 * @brief I2C1 wait for RxNE bit to set 
 * 
 * @details The RxNE bit indicates that there is data in the data register to be read from 
 *          the slave. This functions waits for the bit to set before proceeding to read 
 *          the data register. Once the data register is read then this bit clears until 
 *          more data is available. 
 * 
 */
void i2c_rxne_wait(I2C_TypeDef *i2c);

//=======================================================================================


//=======================================================================================
// Write data 

/**
 * @brief I2C1 write address 
 * 
 * @details Sends the slave address to the bus to initialize communication with a certain 
 *          device. This function is called after the start condition has been generated. 
 *          Only 7-bit addresses are supported. 
 * 
 * @param i2c : pointer to the I2C port 
 * @param i2c1_address : 7-bit address of slave device on the bus 
 */
void i2c_write_address(
    I2C_TypeDef *i2c, 
    uint8_t i2c1_address);


/**
 * @brief I2C1 write data
 * 
 * @details Write data to a slave device. This function is called after the ADDR bit has 
 *          been cleared. The function takes a pointer to the data that will be send over 
 *          the bus and the size of the data so it knows how many bytes to send. 
 * 
 * @param i2c : pointer to the I2C port 
 * @param data : pointer to data to be sent over the bus 
 * @param data_size : integer indicating the number of bytes to be sent 
 */
void i2c_write_master_mode(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t data_size);

//=======================================================================================


//=======================================================================================
// Read data 

/**
 * @brief I2C1 read data
 * 
 * @details Read data from a slave device. This function is called after the ADDR bit has 
 *          been cleared. The function takes a pointer where it will store the recieved 
 *          data and the size of the data so it knows how many bytes to read. 
 *          
 * @param i2c : pointer to the I2C port 
 * @param data : pointer that data is placed into 
 * @param data_size : integer indicating the number of bytes to be receieved
 */
void i2c_read_master_mode(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size);


/**
 * @brief I2C read data until a termination character is seen 
 * 
 * @details This function is for reading data of unknown or varying length. It reads 
 *          data in the same way as i2c_read_master_mode but takes a termination character
 *          and the number of bytes left to read after the termination character as 
 *          arguments. Once the termination character is seen then the transaction will 
 *          be stopped based on how many bytes remain to be read. There must be at least one 
 *          byte left to read. 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param data : pointer to the buffer that stores read data 
 * @param term_char : termination character to indicate when to stop reading 
 * @param bytes_remain : number of bytes left to read after the termination character 
 */
void i2c_read_to_term(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t term_char, 
    uint16_t bytes_remain);


/**
 * @brief I2C read data of a certain length that is defined within the message 
 * 
 * @details 
 *          ***Note: Reading the length in the message is currently only supported 
 *                   for little endian format. 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param address : 
 * @param data : pointer to buffer that stores the read data  
 * @param len_location : where the message length is located in the message 
 * @param len_bytes : number of bytes used to define the length within the message 
 * @param add_bytes : additional bytes to read at the end of the message (if needed) 
 */
void i2c_read_to_len(
    I2C_TypeDef *i2c, 
    uint8_t address, 
    uint8_t *data, 
    uint8_t len_location, 
    uint8_t len_bytes, 
    uint8_t add_bytes); 

//=======================================================================================


#endif  // _I2C_COMM_H_ 
