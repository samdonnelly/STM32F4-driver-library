/**
 * @file diskio.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Low level disk I/O module SKELETON for FatFs - (C)ChaN, 2025 
 * 
 * @details If a working storage control module is available, it should be 
 *          attached to the FatFs via a glue function rather than modifying it. 
 *          This is an example of glue functions to attach various exsisting 
 *          storage control modules to the FatFs module with a defined API. 
 * 
 * @version 0.1
 * @date 2025-10-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "diskio.h"

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef struct
{
	uint8_t init_status[FF_VOLUMES];
	const diskio_dispatch_t *drv[FF_VOLUMES];
	volatile uint8_t vol_num;
}
diskio_data_t;

//=======================================================================================


//=======================================================================================
// Global data 

static diskio_data_t diskio_data; 

//=======================================================================================


//=======================================================================================
// Initialization 
//=======================================================================================


//=======================================================================================
// Disk control functions 

/**
 * @brief Get Drive Status 
 * 
 * @param pdrv : Physical drive nmuber to identify the drive 
 */
DSTATUS disk_status(BYTE pdrv)
{
	return diskio_data.drv[pdrv]->disk_status(pdrv);
}


/**
 * @brief Inidialize a Drive 
 * 
 * @param pdrv : Physical drive nmuber to identify the drive 
 */
DSTATUS disk_initialize(BYTE pdrv)
{
	DSTATUS status = RES_OK;
	return status;
}


/**
 * @brief Read Sector(s) 
 * 
 * @param pdrv : Physical drive nmuber to identify the drive 
 * @param buff : Data buffer to store read data 
 * @param sector : Start sector in LBA 
 * @param count : Number of sectors to read 
 */
DRESULT disk_read (
	BYTE pdrv,
	BYTE *buff,
	LBA_t sector,
	UINT count)
{
	DSTATUS status = RES_OK;
	return status;
}


#if FF_FS_READONLY == 0

/**
 * @brief Write Sector(s) 
 * 
 * @param pdrv : Physical drive nmuber to identify the drive 
 * @param buff : Data to be written 
 * @param sector : Start sector in LBA 
 * @param count : Number of sectors to write 
 */
DRESULT disk_write (
	BYTE pdrv,
	const BYTE *buff,
	LBA_t sector,
	UINT count)
{
	DSTATUS status = RES_OK;
	return status;
}

#endif


/**
 * @brief Miscellaneous Functions 
 * 
 * @param pdrv : Physical drive nmuber (0..) 
 * @param cmd : Control code 
 * @param buff : Buffer to send/receive control data 
 */
DRESULT disk_ioctl (
	BYTE pdrv,
	BYTE cmd,
	void *buff)
{
	DSTATUS status = RES_OK;
	return status;
}

//=======================================================================================
