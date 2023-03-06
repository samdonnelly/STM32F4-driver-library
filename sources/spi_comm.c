/**
 * @file spi_comm.c
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

#include "spi_comm.h"

//=======================================================================================


//===============================================================================
// Function prototypes 

/**
 * @brief SPI enable 
 * 
 * @details Sets the SPE bit to enable the specified SPI 
 * 
 * @param spi : pointer to spi port 
 */
void spi_enable(
    SPI_TypeDef *spi);


/**
 * @brief SPI disable 
 * 
 * @details Clears the SPE bit to disable the specified SPI 
 * 
 * @param spi : pointer to spi port 
 */
void spi_disable(
    SPI_TypeDef *spi);


/**
 * @brief SPI TXE wait 
 * 
 * @details Wait for the TXE bit to set before proceeding. The TXE bit is the transmit 
 *          buffer empty status which indicates when more data can be loaded into the 
 *          transmit buffer for sending. If data is written to the transmit buffer 
 *          before it is empty then data will be overwritten. 
 * 
 * @param spi : pointer to spi port 
 */
void spi_txe_wait(
    SPI_TypeDef *spi);


/**
 * @brief SPI RXNE wait 
 * 
 * @details Wait for the RXNE bit to set before proceeding. The RXNE bit is the receive 
 *          buffer not empty status which indicates when new data can be read from the data 
 *          register during a read operation. If data is read without this bit being set then 
 *          old data will be read from the receieve buffer. 
 * 
 * @param spi : pointer to spi port 
 */
void spi_rxne_wait(
    SPI_TypeDef *spi);


/**
 * @brief SPI BSY wait 
 * 
 * @details Waits for the busy flag to clear. The busy flag indicates when the SPI is busy 
 *          or when the TX buffer is not empty. This is typically used at the end of read 
 *          and write sequences to make sure the operation is done before ending. 
 * 
 * @param spi : pointer to spi port 
 */
void spi_bsy_wait(
    SPI_TypeDef *spi);


#if SPI_DRAFT 

/**
 * @brief Wait for TXE bit to set - draft 
 * 
 * @param spi : pointer to spi port 
 * @return SPI_STATUS : status of the SPI operation 
 */
SPI_STATUS spi_txe_wait_draft(
    SPI_TypeDef *spi); 


/**
 * @brief Wait for RXNE bit to set - draft 
 * 
 * @param spi : pointer to spi port 
 * @return SPI_STATUS : status of the SPI operation 
 */
SPI_STATUS spi_rxne_wait_draft(
    SPI_TypeDef *spi); 


/**
 * @brief Wait for BSY bit to clear - draft 
 * 
 * @param spi : pointer to spi port 
 * @return SPI_STATUS : status of the SPI operation 
 */
SPI_STATUS spi_bsy_wait_draft(
    SPI_TypeDef *spi); 


/**
 * @brief SPI write - draft 
 * 
 * @details This 
 * 
 * @param spi 
 * @param write_data 
 * @param data_len 
 */
SPI_STATUS spi_write_draft(
    SPI_TypeDef *spi, 
    uint8_t *write_data, 
    uint32_t data_len);


/**
 * @brief SPI write then read - draft 
 * 
 * @details 
 *          This can be used to request information from a slave device (write) then 
 *          receive the needed information immediately afterwards (read). 
 * 
 * @param spi 
 * @param write_data 
 * @param read_data 
 * @param data_len 
 */
SPI_STATUS spi_read_draft(
    SPI_TypeDef *spi, 
    uint8_t  write_data, 
    uint8_t *read_data, 
    uint32_t data_len);

#endif   // SPI_DRAFT 

//===============================================================================


//=======================================================================================
// Inititalization 

// SPI initialization
// GPIOB and SPI2 was used for all existing spi stuff 
SPI_STATUS spi_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio, 
    spi2_num_slaves_t num_slaves,
    spi_baud_rate_ctrl_t baud_rate_ctrl,
    spi_clock_mode_t clock_mode)
{
    //==============================================================
    // Pin information for SPI
    //  PB9:  NSS --> GPIO slave select 1
    //  PB10: SCK
    //  PB12: NSS --> GPIO slave select 2
    //  PB14: MISO
    //  PB15: MOSI
    //==============================================================

    // Enable the SPI clock 
    RCC->APB1ENR |= (SET_BIT << SHIFT_14);

    // Enable the GPIOB clock
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // Configure the pins for alternative functions
    // Specify SPI pins as using alternative functions
    gpio->MODER |= (SET_2 << SHIFT_20);  // PB10 
    gpio->MODER |= (SET_2 << SHIFT_28);  // PB14 
    gpio->MODER |= (SET_2 << SHIFT_30);  // PB15 

    // Select high speed for the pins 
    gpio->OSPEEDR |= (SET_3 << SHIFT_20);  // PB10 
    gpio->OSPEEDR |= (SET_3 << SHIFT_28);  // PB14 
    gpio->OSPEEDR |= (SET_3 << SHIFT_30);  // PB15 

    // Configure the SPI alternate function in the AFR register 
    gpio->AFR[1] |= (SET_5 << SHIFT_8);   // PB10 
    gpio->AFR[1] |= (SET_5 << SHIFT_24);  // PB14 
    gpio->AFR[1] |= (SET_5 << SHIFT_28);  // PB15 

    // Reset and disable the SPI before making any changes 
    spi->CR1 = CLEAR;

    // Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
    // TODO create pre-defined rates based on clock speed and SPI speed 
    spi->CR1 |= (baud_rate_ctrl << SHIFT_3);

    // Select CPOL and CPHA bits to define data transfer and serial clock relationship.
    spi->CR1 |= (clock_mode << SHIFT_0);

    // Set the DFF bit to define an 8-bit data frame format. 
    spi->CR1 &= ~(SET_BIT << SHIFT_11);

    // Enable software slave management
    spi->CR1 |= (SET_BIT << SHIFT_9);
    spi->CR1 |= (SET_BIT << SHIFT_8);

    // Set full-duplex mode 
    spi->CR1 &= ~(SET_BIT << SHIFT_10);

    // Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
    spi->CR1 &= ~(SET_BIT << SHIFT_7);  // MSB first 

    // Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
    spi->CR2 &= ~(SET_BIT << SHIFT_4);  // No TI protocol 

    // Set the MSTR bit to enable master mode 
    spi->CR1 |= (SET_BIT << SHIFT_2);

    // Set the SPE bit to enable SPI 
    spi_enable(spi);

    // Configure the slave select pins as GPIO 
    switch (num_slaves)
    {
        case SPI_2_SLAVE:  // Initialize PB12 as GPIO
            gpio_pin_init(gpio, PIN_12, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
            spi_slave_deselect(gpio, SPI2_SS_2);  // Deselect slave 
            // Case has no break so PB9 will also be initialized
            // TODO check if no break statement works 

        case SPI_1_SLAVE:  // Initialize PB9 as GPIO 
            gpio_pin_init(gpio, PIN_9, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
            spi_slave_deselect(gpio, SPI2_SS_1);  // Deselect slave 
            break;

        default:  // Invalid number of slaves specified
            return SPI_ERROR;
    }

    // Initialization success 
    return SPI_OK;
}

//=======================================================================================


//=======================================================================================
// SPI register functions 

// Set the SPE bit to enable SPI 
void spi_enable(
    SPI_TypeDef *spi)
{
    spi->CR1 |= (SET_BIT << SHIFT_6);
}


// Clear the SPE bit to disable SPI 
void spi_disable(
    SPI_TypeDef *spi)
{
    spi->CR1 &= ~(SET_BIT << SHIFT_6);
}


// Wait for TXE bit to set 
void spi_txe_wait(
    SPI_TypeDef *spi)
{
    // TODO add timeout & status return 
    while(!(spi->SR & (SET_BIT << SHIFT_1))); 
}

// Wait for RXNE bit to set 
void spi_rxne_wait(
    SPI_TypeDef *spi)
{
    // TODO add timeout & status return 
    while(!(spi->SR & (SET_BIT << SHIFT_0)));
}


// Wait for BSY bit to clear
void spi_bsy_wait(
    SPI_TypeDef *spi)
{
    // TODO add timeout & status return 
    while(spi->SR & (SET_BIT << SHIFT_7));
}


// Select an SPI slave 
void spi_slave_select(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num)
{
    gpio_write(gpio, slave_num, GPIO_LOW);
}


// Deselect an SPI slave 
void spi_slave_deselect(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num)
{
    gpio_write(gpio, slave_num, GPIO_HIGH);
}


#if SPI_DRAFT 

// Wait for TXE bit to set before writing - draft 
SPI_STATUS spi_txe_wait_draft(
    SPI_TypeDef *spi) 
{
    uint16_t timer = SPI_COM_TIMEOUT;   // TODO test/verify timer/counter size 
    while(!(spi->SR & (SET_BIT << SHIFT_1)) && timer--); 
    if (!timer) return SPI_ERROR; 
    return SPI_OK; 
}


// Wait for RXNE bit to set before reading - draft 
SPI_STATUS spi_rxne_wait_draft(
    SPI_TypeDef *spi)
{
    uint16_t timer = SPI_COM_TIMEOUT; 
    while(!(spi->SR & (SET_BIT << SHIFT_0)) && timer--); 
    if (!timer) return SPI_ERROR; 
    return SPI_OK; 
}


// Wait for BSY bit to clear - draft 
SPI_STATUS spi_bsy_wait_draft(
    SPI_TypeDef *spi)
{
    uint16_t timer = SPI_COM_TIMEOUT; 
    while((spi->SR & (SET_BIT << SHIFT_7)) && timer--); 
    if (!timer) return SPI_ERROR; 
    return SPI_OK; 
}

#endif   // SPI_DRAFT 

//=======================================================================================


//=======================================================================================
// Read and write 

// SPI write 
// TODO add timeouts 
void spi_write(
    SPI_TypeDef *spi, 
    const uint8_t *write_data, 
    uint32_t data_len)
{
    // Argument check - NULL pointers and zero length 
    if (spi == NULL || write_data == NULL || !data_len) return; 

    // Iterate through all data to be sent 
    for (uint32_t i = 0; i < data_len; i++)
    {
        spi_txe_wait(spi); 
        spi->DR = *write_data++; 
    }

    // Wait for TXE bit to set 
    spi_txe_wait(spi);

    // Wait for BSY to clear 
    spi_bsy_wait(spi);

    // Read the data and status registers to clear the RX buffer and overrun error bit
    dummy_read(spi->DR); 
    dummy_read(spi->SR); 
}


// SPI write then read 
// TODO add timeouts 
void spi_write_read(
    SPI_TypeDef *spi, 
    uint8_t  write_data, 
    uint8_t *read_data, 
    uint32_t data_len)
{
    // Argument check - NULL pointers and zero length 
    if (spi == NULL || read_data == NULL || !data_len) return; 

    // Write the first piece of data 
    spi_txe_wait(spi); 
    spi->DR = write_data; 

    // Iterate through all data to be sent and received 
    for (uint16_t i = 0; i < data_len-1; i++)
    {
        spi_txe_wait(spi);          // Wait for TXE bit to set 
        spi->DR = write_data;       // Write data to the data register 

        spi_rxne_wait(spi);         // Wait for the RXNE bit to set 
        *read_data++ = spi->DR;     // Read data from the data register 
    }

    // Read the last piece of data
    spi_rxne_wait(spi);
    *read_data = spi->DR;

    // Wait for TXE bit to set 
    spi_txe_wait(spi);

    // Wait for BSY to clear 
    spi_bsy_wait(spi);
}


#if SPI_DRAFT 

// SPI write - draft 
SPI_STATUS spi_write_draft(
    SPI_TypeDef *spi, 
    uint8_t *write_data, 
    uint32_t data_len)
{
    SPI_STATUS status = SPI_OK; 

    // Transmit all the data 
    for (uint32_t i = 0; i < data_len; i++)
    {
        status = spi_txe_wait_draft(spi); 

        if (status)  // SPI transmission fault 
        {
            // Clear registers 
            dummy_read(spi->DR); 
            dummy_read(spi->SR); 
            return status; 
        }

        spi->DR = *write_data++; 
    }

    // Wait for TXE bit to set and BSY bit to clear 
    spi_txe_wait_draft(spi); 
    spi_bsy_wait_draft(spi); 

    // Read the data and status registers to clear the RX buffer and overrun error bit
    dummy_read(spi->DR); 
    dummy_read(spi->SR); 

    return status; 
}


// SPI read - draft 
SPI_STATUS spi_read_draft(
    SPI_TypeDef *spi, 
    uint8_t  write_data, 
    uint8_t *read_data, 
    uint32_t data_len)
{
    // Argument check - NULL pointers and zero length 
    if (spi == NULL || read_data == NULL || !data_len) return SPI_ERROR; 

    // Write the first piece of data 
    if (spi_txe_wait_draft(spi)) return SPI_ERROR; 
    spi->DR = write_data; 

    // Iterate through all data to be sent and received
    for (uint32_t i = 0; i < data_len-1; i++)
    {
        // Write to slave to provide a bus clock 
        if (spi_txe_wait_draft(spi)) return SPI_ERROR; 
        spi->DR = write_data; 

        // Read the slave response 
        if (spi_rxne_wait_draft(spi)) return SPI_ERROR; 
        *read_data++ = spi->DR; 
    }

    // Read the last piece of data 
    if (spi_rxne_wait_draft(spi)) return SPI_ERROR; 
    *read_data++ = spi->DR; 

    // Wait for TXE bit to set and the BSY bit to clear 
    spi_txe_wait_draft(spi);
    spi_bsy_wait_draft(spi); 

    // return status; 
    return SPI_OK; 
}

#endif   // SPI_DRAFT 

//=======================================================================================
