/**
 * @file diskio.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Low level disk I/O module SKELETON for FatFs - (C)ChaN, 2025 
 * 
 * @details The below code is taken from the CHAN distribution of FatFs. The code has 
 *          been modified to adjust formatting and make the interface more univeral for 
 *          different applications. The diskio code will be manually updated to keep up 
 *          to date with the latest FatFs code. 
 *          
 *          If a working storage control module is available, it should be 
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
#include <stddef.h>

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef enum {
	DFLAG_CLEAR,
	DFLAG_SET
} DFLAG;

typedef struct
{
	DFLAG link_status;
	diskio_dispatch_t dispatch;
	DFLAG init_status;
}
diskio_data_t;

//=======================================================================================


//=======================================================================================
// Global data 

static diskio_data_t diskio_data = 
{
	.link_status = DFLAG_CLEAR,
	.dispatch = { NULL, NULL, NULL, NULL, NULL },
	.init_status = DFLAG_CLEAR
}; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief Link the hardware layer to the FatFs layer 
 * 
 * @details This functions links the FatFs layer with the needed hardware layer. This 
 *          function must be called during setup so that when the diskio functions are 
 *          called by FatFs, they are able to dispatch to the code that can carry out 
 *          FatFs operations with the connected hardware. If this is not successfully 
 *          called then no operations will be carried out. 
 *          
 *          A pointer to the dispatch functions is passed as an argument. The I/O layer 
 *          that communicates with hardware should pass pointers to the init, status, 
 *          read, write and ioctl functions so that they can be deployed when the diskio 
 *          functions are called. 
 * 
 * @param dispatch_functions : pointers to functions to call in each diskio function 
 * @return DSTATUS : status of the linkage 
 */
DSTATUS disk_link(const diskio_dispatch_t *dispatch_functions)
{
	DSTATUS status = (DSTATUS)STA_NOINIT;

	if (dispatch_functions != NULL)
	{
		diskio_data.link_status = DFLAG_SET;
		diskio_data.dispatch = *dispatch_functions;
		status = (DSTATUS)STA_OK;
	}

	return status;
}

//=======================================================================================


//=======================================================================================
// Disk control functions 

/**
 * @brief Get Drive Status 
 * 
 * @param pdrv : Physical drive number to identify the drive 
 */
DSTATUS disk_status(BYTE pdrv)
{
	return ((diskio_data.link_status == DFLAG_SET) && (pdrv < FF_VOLUMES)) ? 
			diskio_data.dispatch.disk_status(pdrv) : 
			(DSTATUS)STA_NOINIT;
}


/**
 * @brief Initialize a Drive 
 * 
 * @param pdrv : Physical drive number to identify the drive 
 */
DSTATUS disk_initialize(BYTE pdrv)
{
	DSTATUS status = (DSTATUS)STA_NOINIT;

	if ((diskio_data.link_status == DFLAG_SET) && 
		(diskio_data.init_status == DFLAG_CLEAR) && 
		(pdrv < FF_VOLUMES))
	{
		status = diskio_data.dispatch.disk_initialize(pdrv);

		if (status == (DSTATUS)STA_OK)
		{
			diskio_data.init_status = DFLAG_SET;
		}
	}

	return status;
}


/**
 * @brief Read Sector(s) 
 * 
 * @param pdrv : Physical drive number to identify the drive 
 * @param buff : Data buffer to store read data 
 * @param sector : Start sector in LBA 
 * @param count : Number of sectors to read 
 */
DRESULT disk_read(
	BYTE pdrv,
	BYTE *buff,
	LBA_t sector,
	UINT count)
{
	return ((diskio_data.link_status == DFLAG_SET) && (pdrv < FF_VOLUMES) && (buff != NULL)) ? 
			diskio_data.dispatch.disk_read(pdrv, buff, sector, count) : 
			RES_PARERR;
}


#if FF_FS_READONLY == 0

/**
 * @brief Write Sector(s) 
 * 
 * @param pdrv : Physical drive number to identify the drive 
 * @param buff : Data to be written 
 * @param sector : Start sector in LBA 
 * @param count : Number of sectors to write 
 */
DRESULT disk_write(
	BYTE pdrv,
	const BYTE *buff,
	LBA_t sector,
	UINT count)
{
	return ((diskio_data.link_status == DFLAG_SET) && (pdrv < FF_VOLUMES) && (buff != NULL)) ? 
			diskio_data.dispatch.disk_write(pdrv, buff, sector, count) : 
			RES_PARERR;
}

#endif


/**
 * @brief Miscellaneous Functions 
 * 
 * @param pdrv : Physical drive number (0..) 
 * @param cmd : Control code 
 * @param buff : Buffer to send/receive control data 
 */
DRESULT disk_ioctl(
	BYTE pdrv,
	BYTE cmd,
	void *buff)
{
	return ((diskio_data.link_status == DFLAG_SET) && (pdrv < FF_VOLUMES)) ? 
			diskio_data.dispatch.disk_ioctl(pdrv, cmd, buff) : 
			RES_PARERR;
}

//=======================================================================================
