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

//=======================================================================================


//=======================================================================================
// Dynamic memory allocation 

#if FF_USE_LFN == 3	/* Use dynamic memory allocation */

#include <stdlib.h>		/* with POSIX API */


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
void ff_memfree (void* mblock)
{
	free(mblock);
}

#endif

//=======================================================================================


//=======================================================================================
// Sync functions 

#if FF_FS_REENTRANT	// Mutal exclusion 

//==================================================
// Definitions of Mutex 

#define OS_TYPE	0	// 0:Win32, 1:uITRON4.0, 2:uC/OS-II, 3:FreeRTOS, 4:CMSIS-RTOS 

#if   OS_TYPE == 0	// Win32 
#include <windows.h>
static HANDLE Mutex[FF_VOLUMES + 1];	// Table of mutex handle 

#elif OS_TYPE == 1	// uITRON 
#include "itron.h"
#include "kernel.h"
static mtxid Mutex[FF_VOLUMES + 1];		// Table of mutex ID 

#elif OS_TYPE == 2	// uc/OS-II 
#include "includes.h"
static OS_EVENT *Mutex[FF_VOLUMES + 1];	// Table of mutex pinter 

#elif OS_TYPE == 3	// FreeRTOS 
#include "FreeRTOS.h"
#include "semphr.h"
static SemaphoreHandle_t Mutex[FF_VOLUMES + 1];	// Table of mutex handle 

#elif OS_TYPE == 4	// CMSIS-RTOS 
#include "cmsis_os.h"
static osMutexId Mutex[FF_VOLUMES + 1];	// Table of mutex ID 

#endif

//==================================================


/**
 * @brief Create a Mutex 
 * 
 * @details This function is called in f_mount function to create a new mutex or 
 *          semaphore for the volume. When a 0 is returned, the f_mount function fails 
 *          with FR_INT_ERR.
 * 
 * @param vol : Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 * @return int : 1:Function succeeded or 0:Could not create the mutex 
 */
int ff_mutex_create(int vol)
{
#if OS_TYPE == 0	// Win32 
	Mutex[vol] = CreateMutex(NULL, FALSE, NULL);
	return (int)(Mutex[vol] != INVALID_HANDLE_VALUE);
	
#elif OS_TYPE == 1	// uITRON 
	T_CMTX cmtx = {TA_TPRI,1};
	
	Mutex[vol] = acre_mtx(&cmtx);
	return (int)(Mutex[vol] > 0);
	
#elif OS_TYPE == 2	// uC/OS-II 
	OS_ERR err;
	
	Mutex[vol] = OSMutexCreate(0, &err);
	return (int)(err == OS_NO_ERR);
	
#elif OS_TYPE == 3	// FreeRTOS 
	Mutex[vol] = xSemaphoreCreateMutex();
	return (int)(Mutex[vol] != NULL);
	
#elif OS_TYPE == 4	// CMSIS-RTOS 
	osMutexDef(cmsis_os_mutex);

	Mutex[vol] = osMutexCreate(osMutex(cmsis_os_mutex));
	return (int)(Mutex[vol] != NULL);

#endif
}


/**
 * @brief Delete a Mutex 
 * 
 * @details This function is called in f_mount function to delete a mutex or
 *          semaphore of the volume created with ff_mutex_create function.
 * 
 * @param vol : Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 */
void ff_mutex_delete(int vol)
{
#if OS_TYPE == 0	// Win32 
	CloseHandle(Mutex[vol]);
	
#elif OS_TYPE == 1	// uITRON 
	del_mtx(Mutex[vol]);

#elif OS_TYPE == 2	// uC/OS-II 
	OS_ERR err;

	OSMutexDel(Mutex[vol], OS_DEL_ALWAYS, &err);
	
#elif OS_TYPE == 3	// FreeRTOS 
	vSemaphoreDelete(Mutex[vol]);
	
#elif OS_TYPE == 4	// CMSIS-RTOS 
	osMutexDelete(Mutex[vol]);

#endif
}


/**
 * @brief Request a Grant to Access the Volume 
 * 
 * @details This function is called on enter file functions to lock the volume.
 *          When a 0 is returned, the file function fails with FR_TIMEOUT.
 * 
 * @param vol : Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 * @return int : 1:Succeeded or 0:Timeout 
 */
int ff_mutex_take(int vol)
{
#if OS_TYPE == 0	// Win32 
	return (int)(WaitForSingleObject(Mutex[vol], FF_FS_TIMEOUT) == WAIT_OBJECT_0);
	
#elif OS_TYPE == 1	// uITRON 
	return (int)(tloc_mtx(Mutex[vol], FF_FS_TIMEOUT) == E_OK);
	
#elif OS_TYPE == 2	// uC/OS-II 
	OS_ERR err;

	OSMutexPend(Mutex[vol], FF_FS_TIMEOUT, &err));
	return (int)(err == OS_NO_ERR);

#elif OS_TYPE == 3	// FreeRTOS 
	return (int)(xSemaphoreTake(Mutex[vol], FF_FS_TIMEOUT) == pdTRUE);

#elif OS_TYPE == 4	// CMSIS-RTOS 
	return (int)(osMutexWait(Mutex[vol], FF_FS_TIMEOUT) == osOK);

#endif
}


/**
 * @brief Release a Grant to Access the Volume 
 * 
 * @details This function is called on leave file functions to unlock the volume. 
 * 
 * @param vol : Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES) 
 */
void ff_mutex_give(int vol)
{
#if OS_TYPE == 0	// Win32 
	ReleaseMutex(Mutex[vol]);
	
#elif OS_TYPE == 1	// uITRON 
	unl_mtx(Mutex[vol]);
	
#elif OS_TYPE == 2	// uC/OS-II 
	OSMutexPost(Mutex[vol]);
	
#elif OS_TYPE == 3	// FreeRTOS 
	xSemaphoreGive(Mutex[vol]);
	
#elif OS_TYPE == 4	// CMSIS-RTOS 
	osMutexRelease(Mutex[vol]);
	
#endif
}

#endif	// FF_FS_REENTRANT 

//=======================================================================================
