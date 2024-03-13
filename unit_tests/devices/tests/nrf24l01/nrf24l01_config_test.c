/**
 * @file nrf24l01_config_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 configuration file test implementation 
 * 
 * @version 0.1
 * @date 2024-03-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "nrf24l01_config_test.h" 

//=======================================================================================


//=======================================================================================
// RF pipe addresses 

// Address sent by the PTX and address accepted by the PRX 
const uint8_t nrf24l01_pipe_addr_test[NRF24l01_ADDR_WIDTH] = 
{
    0xB3, 0xB4, 0xB5, 0xB6, 0x05
}; 

//=======================================================================================
