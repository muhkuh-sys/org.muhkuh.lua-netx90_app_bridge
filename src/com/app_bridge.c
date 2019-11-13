#include "app_bridge.h"

#include <string.h>

#include "../app_bridge_interface.h"
#include "netx_io_areas.h"
#include "systime.h"
#include "uprintf.h"


static APP_BRIDGE_DPM_T tDpm __attribute__ ((section (".dpm")));

static const unsigned char aucDpmMagic[8] = { APP_BRIDGE_MAGIC_ARRAY };


static int app_start(void)
{
	HOSTDEF(ptAsicCtrlArea);
	unsigned long ulValue;
	int iResult;


	/* Can the APP clock be activated? */
	ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
	ulValue &= HOSTMSK(clock_enable0_mask_arm_app);
	if( ulValue==0 )
	{
		uprintf("The APP clock can not be activated.");
		iResult = -1;
	}
	else
	{
		/* Activate the APP clock. */
		ulValue  = ptAsicCtrlArea->asClock_enable[0].ulEnable;
		ulValue |= HOSTMSK(clock_enable0_arm_app);
		ulValue |= HOSTMSK(clock_enable0_arm_app_wm);

		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
		ptAsicCtrlArea->asClock_enable[0].ulEnable = ulValue;

		iResult = 0;
	}

	return iResult;
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



int app_bridge_init(void)
{
	HOSTDEF(ptAsicCtrlComArea);
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptIdpmComArea);
	unsigned long ulValue;
	int iResult;


	/* Can the DPM clock be activated? */
	ulValue  = ptAsicCtrlArea->asClock_enable[0].ulMask;
	ulValue &= HOSTMSK(clock_enable0_mask_dpm);
	if( ulValue==0 )
	{
		uprintf("The DPM clock can not be activated.");
		iResult = -1;
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
		memcpy(tDpm.aucMagic, aucDpmMagic, sizeof(tDpm.aucMagic));
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
		iResult = app_start();
	}

	return iResult;
}



int app_bridge_read_register(unsigned long ulAddress, unsigned long *pulValue)
{
	int iResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;
	APP_STATUS_T tStatus;


	/* Be pessimistic. */
	iResult = -1;

	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_ReadRegister32;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tReadRegister32.ulRegister = ulAddress;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tStatus = tDpm.tRequest.tStatus;
			if( tStatus==APP_STATUS_Ok )
			{
				*pulValue = tDpm.tRequest.uData.tReadRegister32.ulValue;
				iResult = 0;
			}
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return iResult;
}



int app_bridge_read_area(unsigned long ulAddress, unsigned long ulLengthInBytes, unsigned char *pucData)
{
	int iResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;
	APP_STATUS_T tStatus;


	/* Be pessimistic. */
	iResult = -1;

	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_ReadArea;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tReadArea.ulAddress = ulAddress;
	tDpm.tRequest.uData.tReadArea.ulLengthInBytes = ulLengthInBytes;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tStatus = tDpm.tRequest.tStatus;
			if( tStatus==APP_STATUS_Ok )
			{
				memcpy(pucData, tDpm.tRequest.uData.tReadArea.aucData, ulLengthInBytes);
				iResult = 0;
			}
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return iResult;
}



int app_bridge_write_register(unsigned long ulAddress, unsigned long ulValue)
{
	int iResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;
	APP_STATUS_T tStatus;


	/* Be pessimistic. */
	iResult = -1;

	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_WriteRegister32;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tWriteRegister32.ulRegister = ulAddress;
	tDpm.tRequest.uData.tWriteRegister32.ulValue = ulValue;

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tStatus = tDpm.tRequest.tStatus;
			if( tStatus==APP_STATUS_Ok )
			{
				iResult = 0;
			}
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return iResult;
}


int app_bridge_write_area(unsigned long ulAddress, unsigned long ulLengthInBytes, const unsigned char *pucData)
{
	int iResult;
	unsigned long ulRequestId;
	unsigned long ulTime;
	int iElapsed;
	APP_STATUS_T tStatus;


	/* Be pessimistic. */
	iResult = -1;

	/* Prepare the request. */
	tDpm.tRequest.tCommand = APP_COMMAND_WriteArea;
	tDpm.tRequest.tStatus = APP_STATUS_Idle;
	tDpm.tRequest.uData.tWriteArea.ulAddress = ulAddress;
	tDpm.tRequest.uData.tWriteArea.ulLengthInBytes = ulLengthInBytes;
	memcpy(tDpm.tRequest.uData.tWriteArea.aucData, pucData, ulLengthInBytes);

	/* Start the request. */
	ulRequestId = tDpm.ulRequestCount + 1U;
	tDpm.ulRequestCount = ulRequestId;

	/* Wait for a response. */
	ulTime = systime_get_ms();
	do
	{
		if( tDpm.ulResponseCount==ulRequestId )
		{
			/* Check the status of the response. */
			tStatus = tDpm.tRequest.tStatus;
			if( tStatus==APP_STATUS_Ok )
			{
				iResult = 0;
			}
			break;
		}
		iElapsed = systime_elapsed(ulTime, 1000U);
	} while( iElapsed==0 );

	return iResult;
}
