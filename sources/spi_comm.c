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
 * @details 
 * 
 */
void spi_enable(
    SPI_TypeDef *spi);


/**
 * @brief SPI disable 
 * 
 * @details 
 * 
 */
void spi_disable(
    SPI_TypeDef *spi);


/**
 * @brief SPI TXE wait 
 * 
 * @details 
 * 
 */
void spi_txe_wait(
    SPI_TypeDef *spi);


/**
 * @brief Wait for TXE bit to set - draft 
 * 
 * @param spi 
 * @return SPI_COM_STATUS 
 */
SPI_COM_STATUS spi_txe_wait_draft(
    SPI_TypeDef *spi); 


/**
 * @brief SPI RXNE wait 
 * 
 * @details 
 * 
 */
void spi_rxne_wait(
    SPI_TypeDef *spi);


/**
 * @brief Wait for RXNE bit to set - draft 
 * 
 * @param spi 
 * @return SPI_COM_STATUS 
 */
SPI_COM_STATUS spi_rxne_wait_draft(
    SPI_TypeDef *spi); 


/**
 * @brief SPI BSY wait 
 * 
 * @details 
 * 
 */
void spi_bsy_wait(
    SPI_TypeDef *spi);


/**
 * @brief Wait for BSY bit to clear - draft 
 * 
 * @param spi 
 * @return SPI_COM_STATUS 
 */
SPI_COM_STATUS spi_bsy_wait_draft(
    SPI_TypeDef *spi); 

//===============================================================================


//=======================================================================================
// Inititalization 

//==============================================================
// SPI Master Mode Setup Steps 
//  1. Enable the SPI clock 
//  2. Enable the GPIOB clock
//  3. Configure the pins for alternative functions
//      a) Specify SPI pins as using alternative functions
//      b) Select high speed for the pins 
//      c) Configure the SPI alternate function in the AFR register 
//  4. Reset and disable the SPI before manking any changes
//  5. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
//  6. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
//  7. Set the DFF bit to define 8-bit or 16-bit data frame format. 
//  8. Enable software slave management
//  9. Set full-duplex mode 
//  10. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
//  11. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
//  12. Set the MSTR bit to enable master mode 
//  13. Set the SPE bit to enable SPI 
//       Note: spi will get disabled in an error state, otherwise it'll be enabled 
//  14. Configure the slave select pins as GPIO 
//==============================================================


// SPI initialization
// GPIOB and SPI2 was used for all existing spi stuff 
uint8_t spi_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio, 
    uint8_t num_slaves,
    uint8_t baud_rate_ctrl,
    uint8_t clock_mode)
{
    //==============================================================
    // Pin information for SPI
    //  PB9:  NSS --> GPIO slave select 1
    //  PB10: SCK
    //  PB12: NSS --> GPIO slave select 2
    //  PB14: MISO
    //  PB15: MOSI
    //==============================================================

    // 1. Enable the SPI clock 
    RCC->APB1ENR |= (SET_BIT << SHIFT_14);

    // 2. Enable the GPIOB clock
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // 3. Configure the pins for alternative functions

    // a) Specify SPI pins as using alternative functions
    gpio->MODER |= (SET_2 << SHIFT_20);  // PB10 
    gpio->MODER |= (SET_2 << SHIFT_28);  // PB14 
    gpio->MODER |= (SET_2 << SHIFT_30);  // PB15 

    // b) Select high speed for the pins 
    gpio->OSPEEDR |= (SET_3 << SHIFT_20);  // PB10 
    gpio->OSPEEDR |= (SET_3 << SHIFT_28);  // PB14 
    gpio->OSPEEDR |= (SET_3 << SHIFT_30);  // PB15 

    // c) Configure the SPI alternate function in the AFR register 
    gpio->AFR[1] |= (SET_5 << SHIFT_8);   // PB10 
    gpio->AFR[1] |= (SET_5 << SHIFT_24);  // PB14 
    gpio->AFR[1] |= (SET_5 << SHIFT_28);  // PB15 

    // 4. Reset and disable the SPI before manking any changes
    spi->CR1 = CLEAR;

    // 5. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
    // TODO create pre-defined rates based on clock speed and SPI speed 
    spi->CR1 |= (baud_rate_ctrl << SHIFT_3);

    // 6. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
    spi->CR1 |= (clock_mode << SHIFT_0);

    // 7. Set the DFF bit to define an 8-bit data frame format. 
    spi->CR1 &= ~(SET_BIT << SHIFT_11);

    // 8. Enable software slave management
    spi->CR1 |= (SET_BIT << SHIFT_9);
    spi->CR1 |= (SET_BIT << SHIFT_8);

    // 9. Set full-duplex mode 
    spi->CR1 &= ~(SET_BIT << SHIFT_10);

    // 10. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
    spi->CR1 &= ~(SET_BIT << SHIFT_7);  // MSB first 

    // 11. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
    spi->CR2 &= ~(SET_BIT << SHIFT_4);  // No TI protocol 

    // 12. Set the MSTR bit to enable master mode 
    spi->CR1 |= (SET_BIT << SHIFT_2);

    // 13. Set the SPE bit to enable SPI 
    spi_enable(spi);

    // 14. Configure the slave select pins as GPIO 
    switch (num_slaves)
    {
        case SPI2_2_SLAVE:  // Initialize PB12 as GPIO
            gpio_pin_init(gpio, PIN_12, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
            spi_slave_deselect(gpio, SPI2_SS_2);  // Deselect slave 
            // Case has no break so PB9 will also be initialized
            // TODO check if no break statement works 

        case SPI2_1_SLAVE:  // Initialize PB9 as GPIO 
            gpio_pin_init(gpio, PIN_9, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
            spi_slave_deselect(gpio, SPI2_SS_1);  // Deselect slave 
            break;

        default:  // Invalid number of slaves specified
            return FALSE;
    }

    // Return true for passing initialization 
    return TRUE;
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


// Wait for TXE bit to set before writing - draft 
SPI_COM_STATUS spi_txe_wait_draft(
    SPI_TypeDef *spi) 
{
    uint16_t timer = SPI_COM_TIMEOUT;   // TODO test/verify timer/counter size 
    while(!(spi->SR & (SET_BIT << SHIFT_1)) && timer--); 
    if (!timer) return SPI_ERROR; 
    return SPI_OK; 
}


// Wait for RXNE bit to set 
void spi_rxne_wait(
    SPI_TypeDef *spi)
{
    // TODO add timeout & status return 
    while(!(spi->SR & (SET_BIT << SHIFT_0)));
}


// Wait for RXNE bit to set before reading - draft 
SPI_COM_STATUS spi_rxne_wait_draft(
    SPI_TypeDef *spi)
{
    uint16_t timer = SPI_COM_TIMEOUT; 
    while(!(spi->SR & (SET_BIT << SHIFT_0)) && timer--); 
    if (!timer) return SPI_ERROR; 
    return SPI_OK; 
}


// Wait for BSY bit to clear
void spi_bsy_wait(
    SPI_TypeDef *spi)
{
    // TODO add timeout & status return 
    while(spi->SR & (SET_BIT << SHIFT_7));
}


// Wait for BSY bit to clear - draft 
SPI_COM_STATUS spi_bsy_wait_draft(
    SPI_TypeDef *spi)
{
    uint16_t timer = SPI_COM_TIMEOUT; 
    while((spi->SR & (SET_BIT << SHIFT_7)) && timer--); 
    if (!timer) return SPI_ERROR; 
    return SPI_OK; 
}


// Select an SPI slave 
void spi_slave_select(
    GPIO_TypeDef *gpio, 
    uint16_t slave_num)
{
    gpio_write(gpio, slave_num, GPIO_LOW);
}


// Deselect an SPI slave 
void spi_slave_deselect(
    GPIO_TypeDef *gpio, 
    uint16_t slave_num)
{
    gpio_write(gpio, slave_num, GPIO_HIGH);
}

//=======================================================================================


//=======================================================================================
// Read and write 

//==============================================================
// Write sequence
//  1. Enable the SPI. 
//      Note: this is done in the init function. 
//  2. Loop through the data to be written. 
//      a) Wait for the TXE bit to set. 
//      b) Write data to the data register. 
//      c) Increment to the next piece of data. 
//  3. Wait for the TXE bit to set. 
//  4. Wait for the BSY flag to clear. 
//  5. Read the data and status registers to clear the RX buffer and overrun error bit. 
//  6. Disable the SPI. 
//      Note: this is not done here. spi is disabled only in an error state. 
//==============================================================

// SPI write 
void spi_write(
    SPI_TypeDef *spi, 
    const uint8_t *write_data, 
    uint32_t data_len)
{
    // Iterate through all data to be sent 
    for (uint32_t i = 0; i < data_len; i++)
    {
        spi_txe_wait(spi);          // Wait for TXE bit to set 
        // TODO abort if TXE bit not set and there's a timeout 
        spi->DR = *write_data;      // Write data to the data register 
        write_data++; 
    }

    // Wait for TXE bit to set 
    spi_txe_wait(spi);

    // Wait for BSY to clear 
    spi_bsy_wait(spi);

    // Read the data and status registers to clear the RX buffer and overrun error bit
    dummy_read(spi->DR); 
    dummy_read(spi->SR); 
}


// SPI write - draft 
SPI_COM_STATUS spi_write_draft(
    SPI_TypeDef *spi, 
    uint8_t *write_data, 
    uint32_t data_len)
{
    SPI_COM_STATUS status = SPI_OK; 

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


//==============================================================
// Write-Read sequence
//  1. Enable SPI by setting the SPE bit to 1. 
//      Note: this is done in the init function. 
//  2. Write the first data item to be transmitted into the SPI_DR register (clears TXE
//     flag)
//  3. Wait until TXE=1 and write the second data item to be transmitted
//  4. Wait until RXNE=1 and read the SPI_DR regsiter to get the first received data 
//     item (clears RXNE bit) 
//  5. Repeat operations 3 and 4 for each data item to be transmitted/received until the 
//     n-1 received data. 
//  6. Wait for RXNE=1 and read the last received data 
//  7. Wait until TXE=1 and then wait until BSY=0 before disabling SPI 
//      Note: disabling is not done here. spi is disabled only in an error state. 
//==============================================================

// SPI write then read 
void spi_write_read(
    SPI_TypeDef *spi, 
    uint8_t  write_data, 
    uint8_t *read_data, 
    uint32_t data_len)
{
    if (spi == NULL) return;          // Null pointer 
    if (read_data == NULL) return;    // Null pointer 
    if (!data_len) return;            // Zero data length 

    // Write the first piece of data 
    // spi_txe_wait(spi); 
    // spi_bsy_wait(spi); 
    spi->DR = write_data; 

    // Iterate through all data to be sent and received
    for (uint16_t i = 0; i < data_len-1; i++)
    {
        spi_txe_wait(spi);          // Wait for TXE bit to set 
        // TODO abort if TXE bit not set and there's a timeout 
        spi->DR = write_data;       // Write data to the data register 

        spi_rxne_wait(spi);         // Wait for the RXNE bit to set
        // TODO abort if RXNE bit not set and there's a timeout 
        *read_data = spi->DR;       // Read data from the data register 
        read_data++; 
    }

    // Read the last piece of data
    // TODO add a timer here in case data doesn't get sent back 
    // TODO test to see if there is an overrun error that needs clearing
    spi_rxne_wait(spi);
    *read_data = spi->DR;

    // Wait for TXE bit to set 
    spi_txe_wait(spi);

    // Wait for BSY to clear 
    spi_bsy_wait(spi);
}


// SPI read - draft 
SPI_COM_STATUS spi_read_draft(
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

//=======================================================================================
