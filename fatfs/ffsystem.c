/**
 * @file ffsystem.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FatFs OS dependent functions 
 * 
 * @version 0.1
 * @date 2025-10-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "ff.h"
#include <stdlib.h>
#include <stdbool.h>
#include "cmsis_os2.h"

//=======================================================================================


//=======================================================================================
// Global data 

static const uint8_t max_mutex = FF_VOLUMES + 1;   // Max number of mutexes 
static osMutexId_t Mutex[max_mutex];               // Table of mutex IDs 

//=======================================================================================


//=======================================================================================
// Dynamic memory allocation 

#if FF_USE_LFN == 3	/* Use dynamic memory allocation */

/**
 * @brief Allocate memory 
 * 
 * @param msize : number of bytes to allocate 
 * @return void : pointer to the allocated memory block (null if not enough core) 
 */
void* ff_memalloc(UINT msize)
{
	return malloc((size_t)msize);
}


/**
 * @brief Free memory 
 * 
 * @param mblock : pointer to the memory block to free (no effect if null) 
 */
void ff_memfree(void *mblock)
{
	free(mblock);
}

#endif

//=======================================================================================


//=======================================================================================
// Sync functions 

#if FF_FS_REENTRANT	// Mutal exclusion 

/**
 * @brief Create a Mutex 
 * 
 * @details This function is called in f_mount function to create a new mutex or 
 *          semaphore for the volume. When a 0 is returned, the f_mount function fails 
 *          with FR_INT_ERR.
 * 
 * @param vol : Mutex ID - Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 * @return int : 1: Function succeeded, 0: Could not create the mutex 
 */
int ff_mutex_create(int vol)
{
	if (vol >= max_mutex)
	{
		return (int)false;
	}

	Mutex[vol] = osMutexNew(NULL);
	return (int)(Mutex[vol] != NULL);
}


/**
 * @brief Delete a Mutex 
 * 
 * @details This function is called in f_mount function to delete a mutex or
 *          semaphore of the volume created with ff_mutex_create function.
 * 
 * @param vol : Mutex ID - Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 */
void ff_mutex_delete(int vol)
{
	if (vol < max_mutex)
	{
		osMutexDelete(Mutex[vol]);
	}
}


/**
 * @brief Request a Grant to Access the Volume 
 * 
 * @details This function is called on enter file functions to lock the volume.
 *          When a 0 is returned, the file function fails with FR_TIMEOUT.
 * 
 * @param vol : Mutex ID - Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 * @return int : 1: Succeeded, 0: Timeout 
 */
int ff_mutex_take(int vol)
{
	if (vol >= max_mutex)
	{
		return (int)false;
	}

	return (int)(osMutexAcquire(Mutex[vol], FF_FS_TIMEOUT) == osOK);
}


/**
 * @brief Release a Grant to Access the Volume 
 * 
 * @details This function is called on leave file functions to unlock the volume. 
 * 
 * @param vol : Mutex ID - Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 */
void ff_mutex_give(int vol)
{
	if (vol < max_mutex)
	{
		osMutexRelease(Mutex[vol]);
	}
}

#endif	// FF_FS_REENTRANT 

//=======================================================================================
