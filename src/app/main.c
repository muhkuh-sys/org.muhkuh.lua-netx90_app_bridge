
#include <string.h>

#include "../app_bridge_interface.h"
#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"
#include "version.h"

/*-------------------------------------------------------------------------*/

static APP_BRIDGE_DPM_T tDpm __attribute__ ((section (".dpm")));

static const unsigned char aucDpmMagic[8] = { APP_BRIDGE_MAGIC_ARRAY };

static const char aucVersion[] = VERSION_ALL;

typedef union VPTR_UNION
{
	unsigned long ul;
	volatile unsigned long *pul;
	volatile unsigned short *pus;
	volatile unsigned char *puc;
} VPTR_T;

typedef union PTR_UNION
{
	unsigned long ul;
	unsigned long *pul;
	unsigned short *pus;
	unsigned char *puc;
} PTR_T;

typedef unsigned long (*PFN_CALL)(unsigned long ulR0, unsigned long ulR1, unsigned long ulR2, unsigned long ulR3);

typedef union CALLPTR_UNION
{
	PFN_CALL pfn;
	unsigned long ul;
} CALLPTR_T;



static APP_STATUS_T dpm_process_command_identify(APP_REQUEST_IDENTIFY_T *ptRequest)
{
	unsigned long ulSize;


	ptRequest->ulVersionMajor = VERSION_MAJOR;
	ptRequest->ulVersionMinor = VERSION_MINOR;
	ptRequest->ulVersionMicro = VERSION_MICRO;

	ulSize = strlen(aucVersion);
	ptRequest->ulLengthInBytes = ulSize;
	memcpy(ptRequest->aucVersionString, aucVersion, ulSize+1);

	return APP_STATUS_Ok;
}



static APP_STATUS_T dpm_process_command_readregister32(APP_REQUEST_READREGISTER32_T *ptRequest)
{
	APP_STATUS_T tStatus;
	VPTR_T tPtr;


	tPtr.ul = ptRequest->ulRegister;

	/* Refuse to read an unaligned address. */
	if( (tPtr.ul&3U)!=0 )
	{
		tStatus = APP_STATUS_UnalignedAddress;
	}
	else
	{
		ptRequest->ulValue = *(tPtr.pul);
		tStatus = APP_STATUS_Ok;
	}

	return tStatus;
}



static APP_STATUS_T dpm_process_command_readarea(APP_REQUEST_READAREA_T *ptRequest)
{
	APP_STATUS_T tStatus;
	unsigned long ulLengthInBytes;
	PTR_T tPtr;


	/* The requested area must not exceed the buffer space. */
	ulLengthInBytes = ptRequest->ulLengthInBytes;
	if( ulLengthInBytes>sizeof(tDpm.tRequest.uData.tReadArea.aucData) )
	{
		tStatus = APP_STATUS_LengthTooLarge;
	}
	else
	{
		/* Copy the data. */
		tPtr.ul = ptRequest->ulAddress;
		memcpy(ptRequest->aucData, tPtr.puc, ulLengthInBytes);
		tStatus = APP_STATUS_Ok;
	}

	return tStatus;
}



static APP_STATUS_T dpm_process_command_writeregister32(APP_REQUEST_WRITEREGISTER32_T *ptRequest)
{
	APP_STATUS_T tStatus;
	VPTR_T tPtr;


	tPtr.ul = ptRequest->ulRegister;

	/* Refuse to writes an unaligned address. */
	if( (tPtr.ul&3U)!=0 )
	{
		tStatus = APP_STATUS_UnalignedAddress;
	}
	else
	{
		*(tPtr.pul) = ptRequest->ulValue;
		tStatus = APP_STATUS_Ok;
	}

	return tStatus;

}


static APP_STATUS_T dpm_process_command_writearea(APP_REQUEST_WRITEAREA_T *ptRequest)
{
	APP_STATUS_T tStatus;
	unsigned long ulLengthInBytes;
	PTR_T tPtr;


	/* The requested area must not exceed the buffer space. */
	ulLengthInBytes = ptRequest->ulLengthInBytes;
	if( ulLengthInBytes>sizeof(tDpm.tRequest.uData.tWriteArea.aucData) )
	{
		tStatus = APP_STATUS_LengthTooLarge;
	}
	else
	{
		/* Copy the data. */
		tPtr.ul = ptRequest->ulAddress;
		memcpy(tPtr.puc, ptRequest->aucData, ulLengthInBytes);
		tStatus = APP_STATUS_Ok;
	}

	return tStatus;
}


static APP_STATUS_T dpm_process_command_call(APP_REQUEST_CALL_T *ptRequest)
{
	APP_STATUS_T tStatus;
	CALLPTR_T tPtr;
	unsigned long ulR0;
	unsigned long ulR1;
	unsigned long ulR2;
	unsigned long ulR3;
	unsigned long ulResult;


	tPtr.ul = ptRequest->ulAddress;
	ulR0 = ptRequest->ulR0;
	ulR1 = ptRequest->ulR1;
	ulR2 = ptRequest->ulR2;
	ulR3 = ptRequest->ulR3;
	tDpm.tRequest.tStatus = APP_STATUS_CallRunning;
	ulResult = tPtr.pfn(ulR0, ulR1, ulR2, ulR3);
	ptRequest->ulResult = ulResult;
	tStatus = APP_STATUS_Ok;

	return tStatus;
}


static int dpm_is_valid(void)
{
	int iCmp;
	int iResult;


	/* Is the DPM valid? */
	iResult = 0;
	iCmp = memcmp(tDpm.aucMagic, aucDpmMagic, sizeof(tDpm.aucMagic));
	if( iCmp==0 )
	{
		iResult = 1;
	}

	return iResult;
}


static void dpm_request_poll(void)
{
	APP_COMMAND_T tCommand;
	APP_STATUS_T tStatus;


	/* Is a command waiting? */
	if( tDpm.ulResponseCount!=tDpm.ulRequestCount )
	{
		/* Is the command valid? */
		tStatus = APP_STATUS_InvalidCommand;
		tCommand = tDpm.tRequest.tCommand;
		switch(tCommand)
		{
		case APP_COMMAND_Identify:
		case APP_COMMAND_ReadRegister32:
		case APP_COMMAND_ReadArea:
		case APP_COMMAND_WriteRegister32:
		case APP_COMMAND_WriteArea:
		case APP_COMMAND_Call:
			tStatus = APP_STATUS_Ok;
			break;
		}
		if( tStatus==APP_STATUS_Ok )
		{
			switch(tCommand)
			{
			case APP_COMMAND_Identify:
				tStatus = dpm_process_command_identify(&(tDpm.tRequest.uData.tIdentify));
				break;

			case APP_COMMAND_ReadRegister32:
				tStatus = dpm_process_command_readregister32(&(tDpm.tRequest.uData.tReadRegister32));
				break;

			case APP_COMMAND_ReadArea:
				tStatus = dpm_process_command_readarea(&(tDpm.tRequest.uData.tReadArea));
				break;

			case APP_COMMAND_WriteRegister32:
				tStatus = dpm_process_command_writeregister32(&(tDpm.tRequest.uData.tWriteRegister32));
				break;

			case APP_COMMAND_WriteArea:
				tStatus = dpm_process_command_writearea(&(tDpm.tRequest.uData.tWriteArea));
				break;

			case APP_COMMAND_Call:
				tStatus = dpm_process_command_call(&(tDpm.tRequest.uData.tCall));
				break;
			}
		}

		/* Set the status. */
		tDpm.tRequest.tStatus = tStatus;

		/* The command is processed. */
		tDpm.ulResponseCount = tDpm.ulRequestCount;
	}
}


/*-------------------------------------------------------------------------*/

#if 0
static void mmio_blink_init(TIMER_HANDLE_T *ptTimer)
{
	HOSTDEF(ptMmioCtrlArea);
	HOSTDEF(ptAsicCtrlArea);


	/* Set MMIO4 to output. */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[4] = HOSTMMIO(PIO);

	ptMmioCtrlArea->ulMmio_pio_out_line_reset_cfg0 = 1U << 4U;
	ptMmioCtrlArea->ulMmio_pio_oe_line_set_cfg0 = 1U << 4U;

	systime_handle_start_ms(ptTimer, 125U);
}


static void mmio_blink(TIMER_HANDLE_T *ptTimer, unsigned long ulInterval)
{
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulLedValue;


	if( systime_handle_is_elapsed(ptTimer)!=0 )
	{
		/* Invert MMIO4. */
		ulLedValue  = ptMmioCtrlArea->ulMmio_pio_out_line_cfg0;
		ulLedValue ^= 1U << 4U;
		ptMmioCtrlArea->ulMmio_pio_out_line_cfg0 = ulLedValue;

		/* Restart the timer. */
		systime_handle_start_ms(ptTimer, ulInterval);
	}
}
#endif

void bridge_main(void);
void bridge_main(void)
{
//	TIMER_HANDLE_T tTimer;
//	unsigned long ulBlinkInterval;
	int iDpmIsValid;


	systime_init();
//	mmio_blink_init(&tTimer);

	while(1)
	{
		iDpmIsValid = dpm_is_valid();
		if( iDpmIsValid==0 )
		{
//			ulBlinkInterval = 125U;
		}
		else
		{
//			ulBlinkInterval = 1000U;
			dpm_request_poll();
		}

//		mmio_blink(&tTimer, ulBlinkInterval);
	};
}
