/**
 * @file diskio_utest.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FatFs diskio unit tests 
 * 
 * @version 0.1
 * @date 2025-11-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "diskio.h"
}

//=======================================================================================


//=======================================================================================
// Global data 

static uint8_t dispatch_status;

static constexpr uint8_t buff_size = 10;

//=======================================================================================


//=======================================================================================
// Prototypes 

void DiskLink(void);
DSTATUS DiskInitializeDispatch(BYTE pdrv);
DSTATUS DiskStatusDispatch(BYTE pdrv);
DRESULT DiskReadDispatch(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
DRESULT DiskWriteDispatch(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
DRESULT DiskIOctlDispatch(BYTE pdrv, BYTE cmd, void* buff);
void DiskDispatch(BYTE pdrv, BYTE *read_buff, BYTE *write_buff, void *ioctl_buff);

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(diskio_test)
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

// Link the dispatch functions to the diskio layer 
void DiskLink(void)
{
    diskio_dispatch_t dispatch_functions = 
    {
        .disk_initialize = &DiskInitializeDispatch,
        .disk_status = &DiskStatusDispatch,
        .disk_read = &DiskReadDispatch,
        .disk_write = &DiskWriteDispatch,
        .disk_ioctl = &DiskIOctlDispatch,
    };

    disk_link(&dispatch_functions);
}

// Disk Initialize Dispatch 
DSTATUS DiskInitializeDispatch(BYTE pdrv)
{
    dispatch_status |= 1 << 0;
    return 0;
}

// Disk Status Dispatch 
DSTATUS DiskStatusDispatch(BYTE pdrv)
{
    dispatch_status |= 1 << 1;
    return 0;
}

// Disk Read Dispatch 
DRESULT DiskReadDispatch(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    dispatch_status |= 1 << 2;
    return RES_OK;
}

// Disk Write Dispatch 
DRESULT DiskWriteDispatch(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    dispatch_status |= 1 << 3;
    return RES_OK;
}

// Disk ioctl Dispatch 
DRESULT DiskIOctlDispatch(BYTE pdrv, BYTE cmd, void* buff)
{
    dispatch_status |= 1 << 4;
    return RES_OK;
}


// Dispatch to the hardware-specific disk IO layer functions 
void DiskDispatch(
    BYTE pdrv,
    BYTE *read_buff,
    BYTE *write_buff,
    void *ioctl_buff)
{
    LBA_t sector = 0;
    UINT count = 0;
    BYTE cmd = 0;

    disk_initialize(pdrv);
    disk_status(pdrv);
    disk_read(pdrv, read_buff, sector, count);
    disk_write(pdrv, write_buff, sector, count);
    disk_ioctl(pdrv, cmd, ioctl_buff);
}

//=======================================================================================


//=======================================================================================
// Tests 

// diskio dispatches to assigned functions 
TEST(diskio_test, diskio_dispatch)
{
    // Test data 
    dispatch_status = 0;
    BYTE pdrv = 0;
    BYTE read_buff[buff_size], write_buff[buff_size], ioctl_buff[buff_size];

    // Attempt to dispatch to the hardware-specific disk IO layer functions before 
    // linking them then check if each dispatch function has been called. This shows 
    // that no attempt to dispatch to a function will be made if the functions are not 
    // linked first. 
    DiskDispatch(pdrv, read_buff, write_buff, (void *)ioctl_buff);
    UNSIGNED_LONGS_EQUAL(0x00, dispatch_status);

    // Link the hardware-specific disk IO layer functions to the diskio module 
    DiskLink();

    // Attempt to dispatch to the hardware-specific disk IO layer functions once 
    // linking is done but the drive number is now out of range, then check if each 
    // dispatch function has been called. This shows that no attempt to dispatch to a 
    // function will be made if the drive number is invalid. 
    pdrv = FF_VOLUMES;
    DiskDispatch(pdrv, read_buff, write_buff, (void *)ioctl_buff);
    UNSIGNED_LONGS_EQUAL(0x00, dispatch_status);

    // Reset the drive number and attempt to dispatch to the hardware-specific read, 
    // write and ioctl disk IO layer functions but with invalid buffers, then check if 
    // their dispatch functions have been called. This shows that no attempt to dispatch 
    // to the read, write or ioctl functions will be made if their buffers are not valid. 
    pdrv = 0;
    DiskDispatch(pdrv, NULL, NULL, NULL);
    UNSIGNED_LONGS_EQUAL(0x03, dispatch_status);

    // Reset the dispatch status and attempt to dispatch to all the hardware-specific 
    // disk IO layer functions once linking is done, the drive number is valid and the 
    // buffers are valid. Check if each dispatch function has been called. This shows 
    // that all dispatch functions except the initialization function are successfully 
    // called now that all conditions are met. The initialization function does not 
    // dispatch because it was already dispatched in the previous check and therefore 
    // initialization cannot happen again. 
    dispatch_status = 0;
    pdrv = 0;
    DiskDispatch(pdrv, read_buff, write_buff, (void *)ioctl_buff);
    UNSIGNED_LONGS_EQUAL(0x1E, dispatch_status);
}

//=======================================================================================
