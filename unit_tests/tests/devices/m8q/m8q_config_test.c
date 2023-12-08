/**
 * @file m8q_config_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief M8Q configuration messages test implementation 
 * 
 * @version 0.1
 * @date 2023-12-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "m8q_config_test.h" 

//=======================================================================================


//=======================================================================================
// Config messages 

// Test packet 
const char m8q_config_pkt[M8Q_CONFIG_NUM_MSG][M8Q_CONFIG_MAX_MSG_LEN] = 
{
    // Disable default NMEA messages 
    "$PUBX,40,GGA,0,0,0,0,0,0*",    // GGA disable
    "$PUBX,40,GLL,0,0,0,0,0,0*",    // GLL disable
    "$PUBX,40,GSA,0,0,0,0,0,0*",    // GSA disable
    "$PUBX,40,GSV,0,0,0,0,0,0*",    // GSV disable
    "$PUBX,40,RMC,0,0,0,0,0,0*",    // RMC disable
    "$PUBX,40,VTG,0,0,0,0,0,0*",    // VTG disable 

    // UBX config messages  
    "B5,62,06,01,0800,F1,00,01,00,00,00,00,00*",      // POSITION enable 
    "B5,62,06,01,0800,F1,04,0A,00,00,00,00,00*",      // TIME enable 

    // Power configuration 
    "B5,62,06,3B,3000,02,00,00,00,60104201,E8030000,10270000,00000000,"
    "0000,0000,0000000000000000000000000000000000000000,00000000*",

    // Port configuration 
    "B5,62,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*",
    "B5,62,06,00,1400,00,00,9902,84000000,00000000,0700,0300,0200,0000*", 

    // Save the settings (save mask) 
    "B5,62,06,09,0C00,00000000,FFFFFFFF,00000000*" 
}; 

//=======================================================================================
