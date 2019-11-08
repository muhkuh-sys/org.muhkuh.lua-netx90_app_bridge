#include "comled.h"

#include "netx_io_areas.h"
#include "systime.h"
#include <string.h>


#if ASIC_TYP==ASIC_TYP_NETX90
#       define MLED_CTRL_AREA ptMledCtrlComArea
#elif ASIC_TYP==ASIC_TYP_NETX90_APP
#       define MLED_CTRL_AREA ptMledCtrlAppArea
#endif


/*-------------------------------------*/


int comled_initialize(unsigned int uiLedIndex)
{
	HOSTDEF(MLED_CTRL_AREA);
	int iResult;


	if( uiLedIndex>=8 )
	{
		/* Invalid LED index. */
		iResult = -1;
	}
	else
	{
		/* Enable the unit. */
		MLED_CTRL_AREA->ulMled_ctrl_cfg |= HOSTMSK(mled_ctrl_cfg_enable);

		/* Set the MLED to "line". */
		MLED_CTRL_AREA->aulMled_ctrl_output_sel[uiLedIndex] = 1U << HOSTSRT(mled_ctrl_output_sel0_sel);

		/* Set the bit in the line register to 0. */
		MLED_CTRL_AREA->ulMled_ctrl_line0 &= ~(1U << uiLedIndex);

		iResult = 0;
	}

	return iResult;
}


/*-------------------------------------*/


int comled_setLEDs(unsigned int uiLedIndex, COMLED_T tState)
{
	HOSTDEF(MLED_CTRL_AREA);
	int iResult;
	unsigned long ulValue;


	if( uiLedIndex>=8 )
	{
		/* Invalid LED index. */
		iResult = -1;
	}
	else
	{
		ulValue  = MLED_CTRL_AREA->ulMled_ctrl_line0;
		ulValue &= ~(3U << uiLedIndex);

		switch(tState)
		{
		case COMLED_OFF:
			/* Keep the bits 0. */
			break;

		case COMLED_GREEN:
			ulValue |= (1U << uiLedIndex);
			break;

		case COMLED_RED:
			ulValue |= (2U << uiLedIndex);
			break;
		}

		MLED_CTRL_AREA->ulMled_ctrl_line0 = ulValue;

		iResult = 0;
	}

	return iResult;
}


/*-------------------------------------*/


static const COMLED_T atLEDState[4] =
{
	COMLED_OFF,             /* 0: off */
	COMLED_OFF,             /* 1: off */
	COMLED_RED,             /* 2: red */
	COMLED_GREEN            /* 3: green */
};



int comled_blinki_init(COMLED_BLINKI_HANDLE_T *ptHandle, unsigned int uiLedIndex, unsigned long ulMask, unsigned long ulState)
{
	int iResult;


	if( ptHandle==NULL )
	{
		iResult = -1;
	}
	/* The netX90 supports MLED 0-7 . A blinki pair needs 2 consecutive MLEDs. */
	else if( uiLedIndex>=(8-1) )
	{
		/* Invalid MLED index. */
		iResult = -1;
	}
	else
	{
		ptHandle->uiLedIndex = uiLedIndex;
		ptHandle->uiCnt = 0;
		ptHandle->ulTimer = systime_get_ms();
		ptHandle->ulMask = ulMask;
		ptHandle->ulState = ulState;

		iResult = comled_initialize(uiLedIndex);
		if( iResult==0 )
		{
			iResult = comled_initialize(uiLedIndex + 1);
		}
	}

	return iResult;
}


void comled_blinki(COMLED_BLINKI_HANDLE_T *ptHandle)
{
	int iResult;
	unsigned int uiCnt;
	unsigned int uiMaskBit;
	unsigned int uiStateBit;
	unsigned int uiBlinkiBit;
	int f_doRestart;


	/* Did 125ms already pass? */
	iResult = systime_elapsed(ptHandle->ulTimer, 125U);
	if( iResult!=0 )
	{
		/* Get a new timer value. */
		ptHandle->ulTimer = systime_get_ms();

		/* Get the counter from the handle. */
		uiCnt = ptHandle->uiCnt;

		do
		{
			/* Show the next LED state. */
			uiMaskBit = (ptHandle->ulMask>>uiCnt)&1U;
			uiStateBit = (ptHandle->ulState>>uiCnt)&1U;

			/* Reset the counter if the LED state is a masked 1. */
			/* NOTE: do not do this for the first bit (uiHandle==0), this would be an endless loop! */
			f_doRestart = ( uiCnt!=0U && uiMaskBit==0U && uiStateBit!=0U );
			if( f_doRestart==0 )
			{
				/* Increase the counter. */
				++uiCnt;
				uiCnt &= 0x1fU;
			}
			else
			{
				/* Restart the LED sequence. */
				uiCnt = 0U;
			}
		} while( f_doRestart!=0 );

		/* Show the LED state. */
		uiBlinkiBit = (uiMaskBit<<1) | uiStateBit;
		comled_setLEDs(ptHandle->uiLedIndex, atLEDState[uiBlinkiBit]);

		/* Write back the new counter value. */
		ptHandle->uiCnt = uiCnt;
	}
}
