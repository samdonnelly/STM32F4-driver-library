/**
 * @file nrf24l01_config_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 configuration file test interface 
 * 
 * @version 0.1
 * @date 2024-03-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _NRF24L01_CONFIG_TEST_H_ 
#define _NRF24L01_CONFIG_TEST_H_ 

//=======================================================================================
// Includes 

#include "nrf24l01_driver.h" 

//=======================================================================================


//=======================================================================================
// RF pipe addresses 

// Address sent by the PTX and address accepted by the PRX 
extern const uint8_t nrf24l01_pipe_addr_test[NRF24l01_ADDR_WIDTH]; 

//=======================================================================================

#endif   // _NRF24L01_CONFIG_TEST_H_ 
