
#include <string.h>

#include "app_bridge.h"
#include "interface.h"
#include "netx_io_areas.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"


/*-------------------------------------------------------------------------*/


TEST_RESULT_T test(BRIDGE_PARAMETER_T *ptParameter)
{
	TEST_RESULT_T tResult;
	BRIDGE_COMMAND_T tCommand;
	int iResult;
	unsigned long ulValue;


	systime_init();

	if( ptParameter==NULL )
	{
		uprintf("App bridge " VERSION_ALL "\n");
		tResult = TEST_RESULT_OK;
	}
	else
	{
		tResult = TEST_RESULT_ERROR;

		tCommand = ptParameter->tCommand;
		switch( tCommand )
		{
		case BRIDGE_COMMAND_Initialize:
			uprintf("Initialize\n");

			iResult = app_bridge_init();
			if( iResult==0 )
			{
				uprintf("Bridge init OK\n");
				tResult = TEST_RESULT_OK;
			}
			break;

		case BRIDGE_COMMAND_Identify:
			uprintf("Identify\n");

			uprintf("Not yet.\n");
			break;

		case BRIDGE_COMMAND_ReadRegister:
			uprintf("ReadRegister\n");

			iResult = app_bridge_read_register(ptParameter->uData.tReadRegister.ulRegister, &ulValue);
			if( iResult==0 )
			{
				ptParameter->uData.tReadRegister.ulValue = ulValue;
				uprintf("read 0x%08x\n", ulValue);
				tResult = TEST_RESULT_OK;
			}
			break;

		case BRIDGE_COMMAND_ReadArea:
			uprintf("ReadArea\n");

			iResult = app_bridge_read_area(ptParameter->uData.tReadArea.ulAddress, ptParameter->uData.tReadArea.ulLengthInBytes, ptParameter->uData.tReadArea.aucData);
			if( iResult==0 )
			{
				tResult = TEST_RESULT_OK;
			}
			break;

		case BRIDGE_COMMAND_WriteRegister:
			uprintf("WriteRegister\n");

			iResult = app_bridge_write_register(ptParameter->uData.tWriteRegister.ulRegister, ptParameter->uData.tWriteRegister.ulValue);
			if( iResult==0 )
			{
				tResult = TEST_RESULT_OK;
			}
			break;

		case BRIDGE_COMMAND_WriteRegisterUnlock:
			uprintf("WriteRegisterUnlock\n");

			iResult = app_bridge_write_register_unlock(ptParameter->uData.tWriteRegisterUnlock.ulRegister, ptParameter->uData.tWriteRegisterUnlock.ulValue);
			if( iResult==0 )
			{
				tResult = TEST_RESULT_OK;
			}
			break;

		case BRIDGE_COMMAND_WriteArea:
			uprintf("WriteArea\n");

			iResult = app_bridge_write_area(ptParameter->uData.tWriteArea.ulAddress, ptParameter->uData.tWriteArea.ulLengthInBytes, ptParameter->uData.tWriteArea.aucData);
			if( iResult==0 )
			{
				tResult = TEST_RESULT_OK;
			}
			break;

		case BRIDGE_COMMAND_Call:
			uprintf("Call\n");

			iResult = app_bridge_call(ptParameter->uData.tCall.ulAddress, ptParameter->uData.tCall.ulR0, ptParameter->uData.tCall.ulR1, ptParameter->uData.tCall.ulR2, ptParameter->uData.tCall.ulR3, &ulValue);
			if( iResult==0 )
			{
				ptParameter->uData.tCall.ulResult = ulValue;
				tResult = TEST_RESULT_OK;
			}
			break;
		}
	}

	return tResult;
}
