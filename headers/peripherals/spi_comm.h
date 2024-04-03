/**
 * @file spi_comm.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SPI communication driver
 * 
 * @version 0.1
 * @date 2022-05-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _SPI_COMM_H_ 
#define _SPI_COMM_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "gpio_driver.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Timers 
#define SPI_COM_TIMEOUT 0x03FF     // Timer for SPI communication sequence timeouts 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief SPI Baud Rate Control 
 * 
 * @details Divider used to set the baud rate. This value indicated the value that the 
 *          PCLK frequency will be divided by to get the baud rate. The init function 
 *          takes this as an argument. 
 */
typedef enum {
    SPI_BR_FPCLK_2,         // F_PCLK/2 
    SPI_BR_FPCLK_4,         // F_PCLK/4 
    SPI_BR_FPCLK_8,         // F_PCLK/8 
    SPI_BR_FPCLK_16,        // F_PCLK/16 
    SPI_BR_FPCLK_32,        // F_PCLK/32 
    SPI_BR_FPCLK_64,        // F_PCLK/64 
    SPI_BR_FPCLK_128,       // F_PCLK/128 
    SPI_BR_FPCLK_256        // F_PCLK/256 
} spi_baud_rate_ctrl_t;


/**
 * @brief SPI Clock Mode 
 * 
 * @details Sets the clock polarity and phase of the SPI. The modes are defined as follows: 
 *          - CPOL : Clock polarity 
 *             - 0: CK to 0 when idle 
 *             - 1: CK to 1 when idle 
 *          
 *          - CPHA : Clock phase 
 *             - 0: First clock transition is the first data capture edge 
 *             - 1: Second clock transition is the first data capture edge 
 *          
 *          Note that CK is the clock pin. So CPOL sets the the pin state during idle. 
 */
typedef enum {
    SPI_CLOCK_MODE_0,  // CPOL = 0, CPHA = 0
    SPI_CLOCK_MODE_1,  // CPOL = 0, CPHA = 1
    SPI_CLOCK_MODE_2,  // CPOL = 1, CPHA = 0
    SPI_CLOCK_MODE_3   // CPOL = 1, CPHA = 1
} spi_clock_mode_t;


/**
 * @brief SPI communication operation status - used for timout detection 
 */
typedef enum {
    SPI_OK, 
    SPI_ERROR 
} spi_com_status_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef spi_com_status_t SPI_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief SPI initialization 
 * 
 * @details Configures the SPI port and slave select pins. The slave select pins are just 
 *          GPIO pins configured as outputs so a slave device can be selected. Currently 
 *          only two slaves can be set up with this function. 
 * 
 * @param spi : pointer to SPI port to initialize 
 * @param gpio : pointer to GPIO port used for the SPI port chosen 
 * @param sck_pin : 
 * @param miso_pin : 
 * @param mosi_pin : 
 * @param baud_rate_ctrl : communication speed to use 
 * @param clock_mode : SPI clock mode - polarity and phase 
 */
void spi_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_sck, 
    pin_selector_t sck_pin, 
    GPIO_TypeDef *gpio_data, 
    pin_selector_t miso_pin, 
    pin_selector_t mosi_pin, 
    spi_baud_rate_ctrl_t baud_rate_ctrl,
    spi_clock_mode_t clock_mode); 


/**
 * @brief SPI slave select pin init 
 * 
 * @details 
 * 
 * @param gpio : GPIO port used for slave select pin 
 * @param ss_pin : pin number for slave select pin 
 */
void spi_ss_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t ss_pin); 

//=======================================================================================


//=======================================================================================
// SPI register functions 

/**
 * @brief SPI slave select 
 * 
 * @details Selects a slave device so it can be communicated with over SPI. This is done 
 *          by setting the GPIO pin configured as a slave select pin to low. 
 * 
 * @param gpio : pointer to GPIO port of ss pin 
 * @param slave_num : bit that selects the pin number of the gpio port 
 */
void spi_slave_select(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num);


/**
 * @brief SPI slave deselect 
 * 
 * @details Deselects a slave device so it can ignore communication on the SPI bus. This 
 *          is done by setting the GPIO pin configured as a slave select pin to high. 
 * 
 * @param gpio : pointer to GPIO port of ss pin 
 * @param slave_num : bit that selects the pin number of the gpio port 
 */
void spi_slave_deselect(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num);

//=======================================================================================


//=======================================================================================
// Read and write functions 

/**
 * @brief SPI write
 * 
 * @details Writes data up to a certain length to the selected SPI slave device. 
 *          
 *          Note that the slave device must be selected before calling this function or else 
 *          communication may fail. 
 * 
 * @param spi : pointer to SPI port 
 * @param write_data : buffer that contains the data to write 
 * @param data_len : length of the data (bytes) in the buffer 
 * @return SPI_STATUS : SPI status 
 */
SPI_STATUS spi_write(
    SPI_TypeDef *spi, 
    const uint8_t *write_data, 
    uint32_t data_len);


/**
 * @brief SPI write then read 
 * 
 * @details Reads data of a certain length from the selected slave device. Since the 
 *          controller is set up in master mode and devices connected to the controller 
 *          are slaves, the slaves require the controller to control the clock pin in order 
 *          to send data back to the controller. For this reason, the function must write 
 *          to the slave (dummy data) in order to provide a clock for the slave to send 
 *          data back. 
 *          
 *          Note that the slave device must be selected before calling this function or else 
 *          communication may fail. 
 * 
 * @param spi : pointer to SPI port 
 * @param write_data : dummy data to send to run the SPI clock so data can be read 
 * @param read_data : buffer to store the read data 
 * @param data_len : length of the data (bytes) to be read 
 * @return SPI_STATUS : SPI status 
 */
SPI_STATUS spi_write_read(
    SPI_TypeDef *spi, 
    uint8_t write_data, 
    uint8_t *read_data, 
    uint32_t data_len);

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _SPI_COMM_H_ 
