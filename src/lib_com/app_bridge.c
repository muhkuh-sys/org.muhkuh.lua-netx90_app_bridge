#include "app_bridge.h"

#include <string.h>

#include "../app_bridge_interface.h"
#include "netx_io_areas.h"
#include "systime.h"


static volatile APP_BRIDGE_DPM_T tDpm __attribute__ ((section (".dpm")));

static const unsigned char aucDpmMagic[8] = { APP_BRIDGE_MAGIC_ARRAY };


extern unsigned char _binary_netx90_app_bridge_img_end[];
extern unsigned char _binary_netx90_app_bridge_img_start[];

#define IFLASH_MAZ_V0_PAGE_SIZE_BYTES 16U
#define IFLASH_MAZ_V0_PAGE_SIZE_DWORD 4U

#define IFLASH_MAZ_V0_ROW_SIZE_IN_BYTES 0x0200
#define IFLASH_MAZ_V0_ERASE_BLOCK_SIZE_IN_BYTES 0x1000

#define IFLASH_MODE_READ        0U
#define IFLASH_MODE_PROGRAM     1U
#define IFLASH_MODE_ERASE       2U
#define IFLASH_MODE_MASS_ERASE  3U
#define IFLASH_MODE_MANUAL      4U

typedef union IFLASH_PAGE_BUFFER_UNION
{
	unsigned char auc[IFLASH_MAZ_V0_PAGE_SIZE_BYTES];
	unsigned long aul[IFLASH_MAZ_V0_PAGE_SIZE_DWORD];
} IFLASH_PAGE_BUFFER_T;



static unsigned long flash_isIntflash2Block0Erased(void)
{
	unsigned long ulResult;
	const unsigned long *pulCnt;
	const unsigned long *pulEnd;


	ulResult = 0xffffffffU;
	pulCnt = (const unsigned long*)HOSTADDR(intflash2);
	pulEnd = (const unsigned long*)(HOSTADDR(intflash2) + IFLASH_MAZ_V0_ERASE_BLOCK_SIZE_IN_BYTES);
	do
	{
		ulResult &= *(pulCnt++);
		if( ulResult!=0xffffffffU )
		{
			break;
		}
	} while( pulCnt<pulEnd );

	return ulResult;
}


/* Set the mode (read/erase/program) and select main array or info page */
static void internal_flash_select_mode_and_clear_caches(unsigned long ulMode)
{
	HOSTDEF(ptIflashCfg2Area);


	/* Reset the flash. This clears the "read" caches. */
	ptIflashCfg2Area->ulIflash_reset = HOSTMSK(iflash_reset_reset);
	ptIflashCfg2Area->ulIflash_reset = 0;

	/* Select the main memory. */
	ptIflashCfg2Area->ulIflash_ifren_cfg = 0;

	/* Clear the the CPU caches. */
	__asm__("DSB");
	__asm__("ISB");

	/* Set the TMR line to 1. */
	ptIflashCfg2Area->ulIflash_special_cfg = HOSTMSK(iflash_special_cfg_tmr);

	/* Select "read" mode. */
	ptIflashCfg2Area->ulIflash_mode_cfg = ulMode;
}



static void internal_flash_select_read_mode_and_clear_caches(void)
{
	internal_flash_select_mode_and_clear_caches(IFLASH_MODE_READ);
}



static void iflash_start_and_wait(void)
{
	HOSTDEF(ptIflashCfg2Area);
	unsigned long ulValue;


	/* Start the operation. */
	ptIflashCfg2Area->ulIflash_access = HOSTMSK(iflash_access_run);

	/* Wait for the operation to finish. */
	do
	{
		ulValue  = ptIflashCfg2Area->ulIflash_access;
		ulValue &= HOSTMSK(iflash_access_run);
	} while( ulValue!=0 );
}


static int flash_eraseIntflash2Block0(void)
{
	HOSTDEF(ptIflashCfg2Area);
	int iResult;
	unsigned long ulValue;


	/* Be pessimistic. */
	iResult = -1;

	/* Select "erase" mode and main memory or info page. */
	internal_flash_select_mode_and_clear_caches(IFLASH_MODE_ERASE);

	/* Set the X address. This is fixed to 0 for the first block. */
	ptIflashCfg2Area->ulIflash_xadr = 0;

	/* Set the Y address. This is fixed to 0 for the first block. */
	ptIflashCfg2Area->ulIflash_yadr = 0;

	/* Start erasing. */
	iflash_start_and_wait();

	/* Go back to the read mode. */
	internal_flash_select_read_mode_and_clear_caches();

	/* Check if the block is now erased. */
	ulValue = flash_isIntflash2Block0Erased();
	if( ulValue==0xffffffffU )
	{
		iResult = 0;
	}

	return iResult;
}



static int flash_programIntflash2Block0(const unsigned char *pucData, unsigned int sizData)
{
	HOSTDEF(ptIflashCfg2Area);
	int iResult;
	unsigned long ulOffsetInBytes;
	unsigned long ulXAddr;
	unsigned long ulYAddr;
	const unsigned char *pucFlashData;
	int iCmpResult;
	IFLASH_PAGE_BUFFER_T tWriteBuffer;
	IFLASH_PAGE_BUFFER_T tVerifyBuffer;


	/* Be optimistic. */
	iResult = 0;

	ulOffsetInBytes = 0;
	while( ulOffsetInBytes<sizData )
	{
		/* Fill the write buffer with the next chunk. */
		memcpy(tWriteBuffer.auc, pucData+ulOffsetInBytes, sizeof(tWriteBuffer));

		/* Select read mode and main array or info page */
		internal_flash_select_read_mode_and_clear_caches();

		/* Convert the offset to an X and Y component. */
		ulXAddr = ulOffsetInBytes / IFLASH_MAZ_V0_ROW_SIZE_IN_BYTES;
		ulYAddr  = ulOffsetInBytes;
		ulYAddr -= (ulXAddr * IFLASH_MAZ_V0_ROW_SIZE_IN_BYTES);
		ulYAddr /= IFLASH_MAZ_V0_PAGE_SIZE_BYTES;

		/* Set the TMR line to 1. */
		ptIflashCfg2Area->ulIflash_special_cfg = HOSTMSK(iflash_special_cfg_tmr);

		/* Select "program" mode and main array or info block. */
		internal_flash_select_mode_and_clear_caches(IFLASH_MODE_PROGRAM);

		/* Set the X and Y address. */
		ptIflashCfg2Area->ulIflash_xadr = ulXAddr;
		ptIflashCfg2Area->ulIflash_yadr = ulYAddr;

		/* Set the data for the "program" operation. */
		ptIflashCfg2Area->aulIflash_din[0] = tWriteBuffer.aul[0];
		ptIflashCfg2Area->aulIflash_din[1] = tWriteBuffer.aul[1];
		ptIflashCfg2Area->aulIflash_din[2] = tWriteBuffer.aul[2];
		ptIflashCfg2Area->aulIflash_din[3] = tWriteBuffer.aul[3];

		/* Start programming. */
		iflash_start_and_wait();

		/* Go back to the read mode. */
		internal_flash_select_read_mode_and_clear_caches();

		/* Get a pointer to the data array of the flash. */
		pucFlashData = (const unsigned char*)(HOSTADDR(intflash2) + ulOffsetInBytes);

		/* Verify the data. */
		memcpy(tVerifyBuffer.auc, pucFlashData, IFLASH_MAZ_V0_PAGE_SIZE_BYTES);
		iCmpResult = memcmp(tVerifyBuffer.aul, tWriteBuffer.aul, IFLASH_MAZ_V0_PAGE_SIZE_BYTES);
		if( iCmpResult!=0 )
		{
			iResult = -1;
			break;
		}

		ulOffsetInBytes += IFLASH_MAZ_V0_PAGE_SIZE_BYTES;
	}

	return iResult;
}



static int flash_intflash2(const unsigned char *pucData, unsigned int sizData)
{
	int iResult;
	unsigned long ulIsErased;


	iResult = 0;

	/* Is the flash blank? */
	ulIsErased = flash_isIntflash2Block0Erased();
	if( ulIsErased!=0xffffffffU )
	{
		iResult = flash_eraseIntflash2Block0();
	}
	if( iResult==0 )
	{
		iResult = flash_programIntflash2Block0(pucData, sizData);
	}

	return iResult;
}



static APP_BRIDGE_RESULT_T app_start(void)
{
	HOSTDEF(ptAsicCtrlArea);
	unsigned long ulValue;
	APP_BRIDGE_RESULT_T tResult;
	int iResult;
	int iCmp;
	const unsigned char *pucFlashData = (const unsigned char*)HOSTADDR(intflash2);
	unsigned int sizFlashImage;


	/* Can the APP clock be activated? */
	ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
	ulValue &= HOSTMSK(clock_enable0_mask_arm_app);
	if( ulValue==0 )
	{
		tResult = APP_BRIDGE_RESULT_AppClocksMaskedOut;
	}
	else
	{
		/* Now be a bit more optimistic. */
		tResult = APP_BRIDGE_RESULT_Ok;

		/* Is the APP side up to date? */
		sizFlashImage = (unsigned int)(_binary_netx90_app_bridge_img_end - _binary_netx90_app_bridge_img_start);
		if( sizFlashImage>IFLASH_MAZ_V0_ERASE_BLOCK_SIZE_IN_BYTES )
		{
			tResult = APP_BRIDGE_RESULT_AppFirmwareExceedsFirstSector;
		}
		else
		{
			iCmp = memcmp(pucFlashData, _binary_netx90_app_bridge_img_start, sizFlashImage);
			if( iCmp!=0 )
			{
				/* Is the APP clock already running? */
				ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
				ulValue &= HOSTMSK(clock_enable0_arm_app);
				if( ulValue!=0 )
				{
					tResult = APP_BRIDGE_RESULT_AppFirmwareUpdateNeededWhileAppRunning;
				}
				else
				{
					/* Update the APP firmware. */
					iResult = flash_intflash2(_binary_netx90_app_bridge_img_start, sizFlashImage);
					if( iResult==0 )
					{
						iCmp = memcmp(pucFlashData, _binary_netx90_app_bridge_img_start, sizFlashImage);
						if( iCmp!=0 )
						{
							tResult = APP_BRIDGE_RESULT_AppFirmwareUpdateFlashFailed;
						}
					}
					else
					{
						tResult = APP_BRIDGE_RESULT_AppFirmwareUpdateFlashFailed;
					}
				}
			}
		}

		if( tResult==APP_BRIDGE_RESULT_Ok )
		{
			/* Activate the APP clock. */
			ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
			ulValue |= HOSTMSK(clock_enable0_arm_app);
			ulValue |= HOSTMSK(clock_enable0_arm_app_wm);

			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
			ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;
		}
	}

	return tResult;
}



static void handshake_deinit(void)
{
	HOSTDEF(ptHandshakeCtrlComArea);
	unsigned int sizCnt;


	/* Read all handshake registers and disable them. */
	sizCnt = sizeof(ptHandshakeCtrlComArea->aulHandshake_hsc_ctrl)/sizeof(unsigned long);
	do
	{
		ptHandshakeCtrlComArea->aulHandshake_hsc_ctrl[--sizCnt] = 0;
	} while( sizCnt!=0 );

	/* Disable all handshake IRQs. */
	ptHandshakeCtrlComArea->ulHandshake_dpm_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrlComArea->ulHandshake_dpm_irq_msk_reset = 0xffffffff;
	ptHandshakeCtrlComArea->ulHandshake_arm_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrlComArea->ulHandshake_arm_irq_msk_reset = 0xffffffff;
	ptHandshakeCtrlComArea->ulHandshake_xpic_irq_raw_clear = 0xffffffff;
	ptHandshakeCtrlComArea->ulHandshake_xpic_irq_msk_reset = 0xffffffff;

	sizCnt = sizeof(ptHandshakeCtrlComArea->asHandshake_buf_man)/sizeof(ptHandshakeCtrlComArea->asHandshake_buf_man[0]);
	do
	{
		--sizCnt;
		ptHandshakeCtrlComArea->asHandshake_buf_man[sizCnt].ulCtrl = HOSTDFLT(handshake_buf_man0_ctrl);
		ptHandshakeCtrlComArea->asHandshake_buf_man[sizCnt].ulStatus_ctrl_netx = HOSTDFLT(handshake_buf_man0_status_ctrl_netx);
		ptHandshakeCtrlComArea->asHandshake_buf_man[sizCnt].ulWin_map = 0;
	} while( sizCnt!=0 );
}

static void idpm0_deinit_registers(void)
{
	HOSTDEF(ptIdpmComArea);


	/*
	 * disable the interface
	 */

	/* Disable all windows and write protect them. */
	ptIdpmComArea->ulIdpm_win1_end = 0U;
	ptIdpmComArea->ulIdpm_win1_map = HOSTMSK(dpm_win1_map_wp_cfg_win);
	ptIdpmComArea->ulIdpm_win2_end = 0U;
	ptIdpmComArea->ulIdpm_win2_map = 0U;
	ptIdpmComArea->ulIdpm_win3_end = 0U;
	ptIdpmComArea->ulIdpm_win3_map = 0U;
	ptIdpmComArea->ulIdpm_win4_end = 0U;
	ptIdpmComArea->ulIdpm_win4_map = 0U;

	/* Disable the tunnel and write protect it. */
	ptIdpmComArea->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_cfg_win);

	handshake_deinit();

	/* Reset all IRQ bits. */
	ptIdpmComArea->ulIdpm_irq_host_mask_reset = 0xffffffff;
	ptIdpmComArea->ulIdpm_firmware_irq_mask = 0;
}



APP_BRIDGE_RESULT_T app_bridge_init(void)
{
	HOSTDEF(ptAsicCtrlComArea);
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptIdpmComArea);
	unsigned long ulValue;
	APP_BRIDGE_RESULT_T tResult;


	/* The DPM must start at the beginning of INTRAMHS. */
	ulValue = (unsigned long)(&tDpm);
	if( ulValue!=HOSTADDR(intramhs_mirror_sram) )
	{
		tResult = APP_BRIDGE_RESULT_InvalidDpmAddress;
	}
	else
	{
		/* Can the DPM clock be activated? */
		ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
		ulValue &= HOSTMSK(clock_enable0_mask_dpm);
		if( ulValue==0 )
		{
			tResult = APP_BRIDGE_RESULT_DpmClocksMaskedOut;
		}
		else
		{
			/* Activate the DPM clock. */
			ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
			ulValue |= HOSTMSK(clock_enable0_dpm);
			ulValue |= HOSTMSK(clock_enable0_dpm_wm);

			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
			ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;

			/* Disable the DPM for new configuration. */
			idpm0_deinit_registers();

			/* DPM mapping:
			 * 0x0000 - 0x7fff : intramhs_mirror_sram
			 */
			ulValue  = (HOSTADDR(intramhs_mirror_sram)) & HOSTMSK(idpm_win1_map_win_map);
			ptIdpmComArea->ulIdpm_win1_end = 0x00008000;
			ptIdpmComArea->ulIdpm_win1_map = ulValue;
			ptIdpmComArea->ulIdpm_win2_end = 0;
			ptIdpmComArea->ulIdpm_win2_map = 0;
			ptIdpmComArea->ulIdpm_win3_end = 0;
			ptIdpmComArea->ulIdpm_win3_map = 0;
			ptIdpmComArea->ulIdpm_win4_end = 0;
			ptIdpmComArea->ulIdpm_win4_map = 0;

			ptIdpmComArea->ulIdpm_tunnel_cfg = HOSTMSK(idpm_tunnel_cfg_wp_data);
			ptIdpmComArea->ulIdpm_itbaddr = 0;

			ptIdpmComArea->ulIdpm_addr_cfg = 3U << HOSTSRT(idpm_addr_cfg_cfg_win_addr_cfg);

			/*
			 * setup the netX parameter area
			 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
			memcpy((void*)(tDpm.aucMagic), aucDpmMagic, sizeof(tDpm.aucMagic));
#pragma GCC diagnostic pop
			tDpm.ulRequestCount = 0;
			tDpm.ulResponseCount = 0;

			/* Configure the firmware IRQ. */
			ptIdpmComArea->ulIdpm_firmware_irq_mask = 0;

			/* Enable the DPM.
			 * Enable the configuration window at offset 0.
			 */
			ulValue  = 0U << HOSTSRT(idpm_cfg0x0_endian);
			ulValue |= HOSTMSK(idpm_cfg0x0_enable);
			ptIdpmComArea->ulIdpm_cfg0x0 = ulValue;

			/* Unlock the DPM. */
			ptAsicCtrlComArea->ulNetx_lock = HOSTMSK(netx_lock_unlock_dpm);

			/* Start the APP CPU. */
			tResult = app_start();
		}
	}

	return tResult;
}



static APP_BRIDGE_RESULT_T check_app_status(APP_STATUS_T tStatus)
{
	APP_BRIDGE_RESULT_T tResult;


	tResult = APP_BRIDGE_RESULT_AppStatusUnknown;
	switch(tStatus)
	{
	case APP_STATUS_Ok:
		tResult = APP_BRIDGE_RESULT_Ok;
		break;

	case APP_STATUS_InvalidCommand:
		tResult = APP_BRIDGE_RESULT_AppStatusInvalidCommand;
		break;

	case APP_STATUS_UnalignedAddress:
		tResult = APP_BRIDGE_RESULT_AppStatusUnalignedAddress;
		break;

	case APP_STATUS_LengthTooLarge:
		tResult = APP_BRIDGE_RESULT_AppStatusLengthTooLarge;
		break;

	case APP_STATUS_CallRunning:
		tResult = APP_BRIDGE_RESULT_AppStatusCallRunning;
		break;

	case APP_STATUS_Idle:
		tResult = APP_BRIDGE_RESULT_AppStatusIdle;
		break;
	}

	return tResult;
}



APP_BRIDGE_RESULT_T app_bridge_read_register(unsigned long ulAddress, unsigned long *pulValue)
{
	APP_BRIDGE_RESULT_T tResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;


	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_ReadRegister32;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tReadRegister32.ulRegister = ulAddress;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	tResult = APP_BRIDGE_RESULT_TransferTimeout;
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tResult = check_app_status(tDpm.tRequest.tStatus);
			if( tResult==APP_BRIDGE_RESULT_Ok )
			{
				*pulValue = tDpm.tRequest.uData.tReadRegister32.ulValue;
			}
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return tResult;
}



APP_BRIDGE_RESULT_T app_bridge_read_area(unsigned long ulAddress, unsigned long ulLengthInBytes, unsigned char *pucData)
{
	APP_BRIDGE_RESULT_T tResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;


	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_ReadArea;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tReadArea.ulAddress = ulAddress;
	tDpm.tRequest.uData.tReadArea.ulLengthInBytes = ulLengthInBytes;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	tResult = APP_BRIDGE_RESULT_TransferTimeout;
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tResult = check_app_status(tDpm.tRequest.tStatus);
			if( tResult==APP_BRIDGE_RESULT_Ok )
			{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
				memcpy(pucData, (void*)(tDpm.tRequest.uData.tReadArea.aucData), ulLengthInBytes);
#pragma GCC diagnostic pop
			}
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return tResult;
}



APP_BRIDGE_RESULT_T app_bridge_write_register(unsigned long ulAddress, unsigned long ulValue)
{
	APP_BRIDGE_RESULT_T tResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;


	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_WriteRegister32;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tWriteRegister32.ulRegister = ulAddress;
	tDpm.tRequest.uData.tWriteRegister32.ulValue = ulValue;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	tResult = APP_BRIDGE_RESULT_TransferTimeout;
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tResult = check_app_status(tDpm.tRequest.tStatus);
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return tResult;
}


APP_BRIDGE_RESULT_T app_bridge_write_register_unlock(unsigned long ulAddress, unsigned long ulValue)
{
	APP_BRIDGE_RESULT_T tResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;


	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_WriteRegister32Unlock;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tWriteRegister32Unlock.ulRegister = ulAddress;
	tDpm.tRequest.uData.tWriteRegister32Unlock.ulValue = ulValue;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	tResult = APP_BRIDGE_RESULT_TransferTimeout;
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tResult = check_app_status(tDpm.tRequest.tStatus);
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return tResult;
}


APP_BRIDGE_RESULT_T app_bridge_write_area(unsigned long ulAddress, unsigned long ulLengthInBytes, const unsigned char *pucData)
{
	APP_BRIDGE_RESULT_T tResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;


	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_WriteArea;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tWriteArea.ulAddress = ulAddress;
	tDpm.tRequest.uData.tWriteArea.ulLengthInBytes = ulLengthInBytes;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	memcpy((void*)(tDpm.tRequest.uData.tWriteArea.aucData), pucData, ulLengthInBytes);
#pragma GCC diagnostic pop

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	tResult = APP_BRIDGE_RESULT_TransferTimeout;
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tResult = check_app_status(tDpm.tRequest.tStatus);
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return tResult;
}



APP_BRIDGE_RESULT_T app_bridge_call(unsigned long ulAddress, unsigned long ulR0, unsigned long ulR1, unsigned long ulR2, unsigned long ulR3, unsigned long *pulResult)
{
	APP_BRIDGE_RESULT_T tResult;
	unsigned long ulRequestId;


	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_Call;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tCall.ulAddress = ulAddress;
	tDpm.tRequest.uData.tCall.ulR0 = ulR0;
	tDpm.tRequest.uData.tCall.ulR1 = ulR1;
	tDpm.tRequest.uData.tCall.ulR2 = ulR2;
	tDpm.tRequest.uData.tCall.ulR3 = ulR3;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response without timeout. */
	while( tDpm.ulResponseCount!=ulRequestId ) {}

	/* Check the status of the response. */
	tResult = check_app_status(tDpm.tRequest.tStatus);
	if( tResult==APP_BRIDGE_RESULT_Ok )
	{
		*pulResult = tDpm.tRequest.uData.tCall.ulResult;
	}

	return tResult;
}
