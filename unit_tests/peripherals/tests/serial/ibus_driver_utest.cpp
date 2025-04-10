/**
 * @file ibus_driver_utest.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief IBUS driver unit tests 
 * 
 * @version 0.1
 * @date 2025-04-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h" 

// Library 
#include <iostream> 

extern "C"
{
	// Add your C-only include files here 
    #include "ibus.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Test data 

// Sample IBUS packet - Source: https://thenerdyengineer.com/ibus-and-arduino/ 

// Packet bytes 
uint8_t ibus_packet_bytes[IBUS_PACKET_BYTES] = 
{
    0x20,   // 0:  Header - Low byte 
    0x40,   // 1:  Header - High byte 
    0xDC,   // 2:  Channel 1 - Low byte 
    0x03,   // 3:  Channel 1 - High byte 
    0xDC,   // 4:  Channel 2 - Low byte 
    0x05,   // 5:  Channel 2 - High byte 
    0xEF,   // 6:  Channel 3 - Low byte 
    0x03,   // 7:  Channel 3 - High byte 
    0xDD,   // 8:  Channel 4 - Low byte 
    0x05,   // 9:  Channel 4 - High byte 
    0xDC,   // 10: Channel 5 - Low byte 
    0x03,   // 11: Channel 5 - High byte 
    0xDC,   // 12: Channel 6 - Low byte 
    0x03,   // 13: Channel 6 - High byte 
    0xDC,   // 14: Channel 7 - Low byte 
    0x03,   // 15: Channel 7 - High byte 
    0xDC,   // 16: Channel 8 - Low byte 
    0x03,   // 17: Channel 8 - High byte 
    0xDC,   // 18: Channel 9 - Low byte 
    0x03,   // 19: Channel 9 - High byte 
    0xDC,   // 20: Channel 10 - Low byte 
    0x05,   // 21: Channel 10 - High byte 
    0xDC,   // 22: Channel 11 - Low byte 
    0x05,   // 23: Channel 11 - High byte 
    0xDC,   // 24: Channel 12 - Low byte 
    0x05,   // 25: Channel 12 - High byte 
    0xDC,   // 26: Channel 13 - Low byte 
    0x05,   // 27: Channel 13 - High byte 
    0xD0,   // 28: Channel 14 - Low byte 
    0x07,   // 29: Channel 14 - High byte 
    0x55,   // 30: Checksum - Low byte 
    0xF3    // 31: Checksum - High byte 
}; 

// Packet items 
uint16_t ibus_packet_items[IBUS_PACKET_ITEMS] = 
{
    16416,   // 0:  Header 
    988,     // 1:  Channel 1 
    1500,    // 2:  Channel 2 
    1007,    // 3:  Channel 3 
    1501,    // 4:  Channel 4 
    988,     // 5:  Channel 5 
    988,     // 6:  Channel 6 
    988,     // 7:  Channel 7 
    988,     // 8:  Channel 8 
    988,     // 9:  Channel 9 
    1500,    // 10: Channel 10 
    1500,    // 11: Channel 11 
    1500,    // 12: Channel 12 
    1500,    // 13: Channel 13 
    2000,    // 14: Channel 14 
    62293    // 15: Checksum 
}; 

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(ibus_driver_test)
{
    // Global test group variables 

    // Constructor 
    void setup()
    {
        // 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 

//=======================================================================================


//=======================================================================================
// Helper functions 

// Packet data check 
void ibus_driver_test_packet_data_check(ibus_packet_t *packet)
{
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_HEADER], packet->items[IBUS_HEADER]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH1], packet->items[IBUS_CH1]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH2], packet->items[IBUS_CH2]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH3], packet->items[IBUS_CH3]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH4], packet->items[IBUS_CH4]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH5], packet->items[IBUS_CH5]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH6], packet->items[IBUS_CH6]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH7], packet->items[IBUS_CH7]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH8], packet->items[IBUS_CH8]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH9], packet->items[IBUS_CH9]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH10], packet->items[IBUS_CH10]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH11], packet->items[IBUS_CH11]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH12], packet->items[IBUS_CH12]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH13], packet->items[IBUS_CH13]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CH14], packet->items[IBUS_CH14]); 
    UNSIGNED_LONGS_EQUAL(ibus_packet_items[IBUS_CHECKSUM], packet->items[IBUS_CHECKSUM]); 
}

//=======================================================================================


//=======================================================================================
// Tests 

// Read: IBUS packet read using DMA (simulated) 
TEST(ibus_driver_test, ibus_packet_read_dma)
{
    ibus_packet_t packet; 

    // Copy the sample data to the packet buffer as if DMA were writing IBUS data to 
    // the buffer as it comes in. 
    for (uint8_t i = CLEAR; i < IBUS_PACKET_BYTES; i++)
    {
        packet.data[i] = ibus_packet_bytes[i]; 
    }

    ibus_driver_test_packet_data_check(&packet); 
}

//=======================================================================================
