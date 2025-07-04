
#include <string.h>

#include "../lib_com/app_bridge.h"
#include "interface.h"
#include "netx_io_areas.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"


/*-------------------------------------------------------------------------*/


unsigned long test(BRIDGE_PARAMETER_T *ptParameter)
{
	BRIDGE_COMMAND_T tCommand;
	APP_BRIDGE_RESULT_T tAppBridgeResult;
	unsigned long ulValue;


	systime_init();

	if( ptParameter==NULL )
	{
		uprintf("App bridge " VERSION_ALL "\n");
		tAppBridgeResult = APP_BRIDGE_RESULT_Ok;
	}
	else
	{
		tAppBridgeResult = APP_BRIDGE_RESULT_AppStatusInvalidCommand;

		tCommand = ptParameter->tCommand;
		switch( tCommand )
		{
		case BRIDGE_COMMAND_Initialize:
			uprintf("Initialize\n");

			tAppBridgeResult = app_bridge_init();
			if( tAppBridgeResult==APP_BRIDGE_RESULT_Ok )
			{
				uprintf("Bridge init OK\n");
			}
			break;

		case BRIDGE_COMMAND_Identify:
			uprintf("Identify\n");

			uprintf("Not yet.\n");
			break;

		case BRIDGE_COMMAND_ReadRegister:
			uprintf("ReadRegister\n");

			tAppBridgeResult = app_bridge_read_register(ptParameter->uData.tReadRegister.ulRegister, &ulValue);
			if( tAppBridgeResult==APP_BRIDGE_RESULT_Ok )
			{
				ptParameter->uData.tReadRegister.ulValue = ulValue;
				uprintf("read 0x%08x\n", ulValue);
			}
			break;

		case BRIDGE_COMMAND_ReadArea:
			uprintf("ReadArea\n");

			tAppBridgeResult = app_bridge_read_area(ptParameter->uData.tReadArea.ulAddress, ptParameter->uData.tReadArea.ulLengthInBytes, ptParameter->uData.tReadArea.aucData);
			break;

		case BRIDGE_COMMAND_WriteRegister:
			uprintf("WriteRegister\n");

			tAppBridgeResult = app_bridge_write_register(ptParameter->uData.tWriteRegister.ulRegister, ptParameter->uData.tWriteRegister.ulValue);
			break;

		case BRIDGE_COMMAND_WriteRegisterUnlock:
			uprintf("WriteRegisterUnlock\n");

			tAppBridgeResult = app_bridge_write_register_unlock(ptParameter->uData.tWriteRegisterUnlock.ulRegister, ptParameter->uData.tWriteRegisterUnlock.ulValue);
			break;

		case BRIDGE_COMMAND_WriteArea:
			uprintf("WriteArea\n");

			tAppBridgeResult = app_bridge_write_area(ptParameter->uData.tWriteArea.ulAddress, ptParameter->uData.tWriteArea.ulLengthInBytes, ptParameter->uData.tWriteArea.aucData);
			break;

		case BRIDGE_COMMAND_Call:
			uprintf("Call\n");

			tAppBridgeResult = app_bridge_call(ptParameter->uData.tCall.ulAddress, ptParameter->uData.tCall.ulR0, ptParameter->uData.tCall.ulR1, ptParameter->uData.tCall.ulR2, ptParameter->uData.tCall.ulR3, &ulValue);
			if( tAppBridgeResult==APP_BRIDGE_RESULT_Ok )
			{
				ptParameter->uData.tCall.ulResult = ulValue;
			}
			break;
		}
	}

	return tAppBridgeResult;
}
