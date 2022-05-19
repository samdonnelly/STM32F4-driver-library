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


//=======================================================================================
// Inititalization 

//==============================================================
// SPI Setup Steps 
//  1. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
//  2. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
//  3. Set the DFF bit to define 8-bit or 16-bit data frame format. 
//  4. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
//  5. Configure the NSS pin. 
//     a) 
//     b) 
//  6. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
//  7. Set the MSTR and SPE bits. 
//==============================================================


// SPI2 initialization
void spi2_init(void)
{
    //==============================================================
    // Pin information for SPI2
    //  PB10: SCK
    //  PB12: NSS
    //  PB14: MISO
    //  PB15: MOSI
    //==============================================================
}

//=======================================================================================


//=======================================================================================
// Read and write 

//==============================================================
// Receive sequence 
//==============================================================

// SPI2 Read
void spi2_read(void)
{
    // 
}


//==============================================================
// Transmit sequence
//  - Transmit sequence begins when a byte is written to the TX buffer 
//  - TXE flag must be set to 1 to indicate that the data has been moved to the 
//    shift register and you can load more data to the TX register. 
//  
//  TODO : figure out which transmission type you need (full-duplex, half-duplex, etc.)
//==============================================================

// SPI2 write 
void spi2_write(void)
{
    // 
}

//=======================================================================================
