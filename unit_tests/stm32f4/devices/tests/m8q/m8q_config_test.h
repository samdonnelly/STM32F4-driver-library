/**
 * @file m8q_config_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief M8Q configuration messages test interface 
 * 
 * @version 0.1
 * @date 2023-12-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _M8Q_CONFIG_TEST_H_ 
#define _M8Q_CONFIG_TEST_H_ 

//=======================================================================================
// Includes 
//=======================================================================================


//=======================================================================================
// Macros 

// Number of configuration messages 
#define M8Q_CONFIG_NUM_MSG 12 

// Max length of a single config message 
#define M8Q_CONFIG_MAX_MSG_LEN 130 

//=======================================================================================


//=======================================================================================
// Config messages 

// Packet 0 
extern const char m8q_config_pkt[M8Q_CONFIG_NUM_MSG][M8Q_CONFIG_MAX_MSG_LEN]; 

//=======================================================================================

#endif   // _M8Q_CONFIG_TEST_H_ 
