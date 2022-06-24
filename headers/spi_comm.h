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


//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief SPI Baud Rate Control 
 * 
 * @details Divider used to set the baud rate. This value indicated the value that the 
 *          PCLK frequency will be divided by to get the baud rate. The init function 
 *          takes this as an argument. 
 * 
 * @see spi2_init
 * 
 */
typedef enum {
    BR_FPCLK_2,
    BR_FPCLK_4,
    BR_FPCLK_8,
    BR_FPCLK_16,
    BR_FPCLK_32,
    BR_FPCLK_64,
    BR_FPCLK_128,
    BR_FPCLK_256
} spi_baud_rate_ctrl_t;


/**
 * @brief SPI Clock Mode 
 * 
 * @details 
 * 
 */
typedef enum {
    SPI_CLOCK_MODE_0,  // CPOL = 0, CPHA = 0
    SPI_CLOCK_MODE_1,  // CPOL = 0, CPHA = 1
    SPI_CLOCK_MODE_2,  // CPOL = 1, CPHA = 0
    SPI_CLOCK_MODE_3   // CPOL = 1, CPHA = 1
} spi_clock_mode_t;


/**
 * @brief SPI Data Frame Format 
 * 
 * @details Used to set the data frame to either 8-bits or 16-bits when sending and 
 *          receiving data. This parameter is passed as an argument to the init 
 *          function. 
 * 
 * @see spi2_init
 * 
 */
typedef enum {
    DFF_8BIT,
    DFF_16BIT
} spi_dff_t;


/**
 * @brief SPI2 Number of Slaves 
 * 
 * @details 
 * 
 * @see spi2_init
 * 
 */
typedef enum {
    SPI2_1_SLAVE,  // Pin PB9 GPIO 
    SPI2_2_SLAVE   // Pins PB9 and PB12 GPIO 
} spi2_num_slaves_t;


/**
 * @brief SPI2 slave select pin number 
 * 
 * @details 
 * 
 */
typedef enum {
    SPI2_SS_1 = 0x0200,  // PB9 
    SPI2_SS_2 = 0x1000   // PB12 
} spi2_slave_select_pin_t;


/**
 * @brief SPI data length 
 * 
 * @details 
 * 
 */
typedef enum {
    SPI_1_BYTE  = 1,   // 1 byte
    SPI_2_BYTES = 2,   // 2 bytes
    SPI_3_BYTES = 3,   // 3 bytes
    SPI_4_BYTES = 4,   // 4 bytes
    SPI_5_BYTES = 5,   // 5 bytes
    SPI_6_BYTES = 6,   // 6 bytes
} spi_data_length_t;

//=======================================================================================


//=======================================================================================
// Initialization functions 

/**
 * @brief SPI2 initialization 
 * 
 * @details 
 *          SPI2 is currently supported for up to 2 slave devices (2 GPIOs).
 * 
 * @see spi_baud_rate_ctrl_t
 * 
 * @param num_slaves 
 * @param baud_rate_ctrl 
 * @param clock_mode 
 * @param data_frame_format 
 * @return uint8_t 
 */
uint8_t spi2_init(
    uint8_t num_slaves,
    uint8_t baud_rate_ctrl,
    uint8_t clock_mode);

//=======================================================================================


//=======================================================================================
// SPI2 register functions 

/**
 * @brief SPI2 enable 
 * 
 * @details 
 * 
 */
void spi2_enable(void);


/**
 * @brief SPI2 disable 
 * 
 * @details 
 * 
 */
void spi2_disable(void);


/**
 * @brief SPI2 TXE wait 
 * 
 * @details 
 * 
 */
void spi2_txe_wait(void);


/**
 * @brief SPI2 RXNE wait 
 * 
 * @details 
 * 
 */
void spi2_rxne_wait(void);


/**
 * @brief SPI2 BSY wait 
 * 
 * @details 
 * 
 */
void spi2_bsy_wait(void);


/**
 * @brief SPI2 slave select 
 * 
 * @details 
 * 
 * @see spi2_slave_select_pin_t
 * 
 * @param slave_num 
 */
void spi2_slave_select(uint16_t slave_num);


/**
 * @brief SPI2 slave deselect 
 * 
 * @details 
 * 
 * @see spi2_slave_select_pin_t
 * 
 * @param slave_num 
 */
void spi2_slave_deselect(uint16_t slave_num);

//=======================================================================================


//=======================================================================================
// Read and write functions 

/**
 * @brief SPI2 write
 * 
 * @details 
 * 
 * @param write_data 
 * @param data_len 
 */
void spi2_write(
    uint8_t *write_data, 
    uint32_t data_len);


/**
 * @brief SPI2 write then read 
 * 
 * @details 
 *          This can be used to request information from a slave device (write) then 
 *          receive the needed information immediately afterwards (read). 
 * 
 * @param write_data 
 * @param read_data 
 * @param data_len 
 */
void spi2_write_read(
    uint8_t  write_data, 
    uint8_t *read_data, 
    uint32_t data_len);

//=======================================================================================
