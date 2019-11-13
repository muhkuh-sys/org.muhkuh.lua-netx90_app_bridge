/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include "boot_spi.h"
#include "boot_drv_spi.h"
//#include "boot_drv_sqi.h"



void boot_spi_activate_mmio(const SPI_CFG_T *ptCfg, const NX90_MMIO_CFG_T *ptMmioValues)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	const unsigned char *pucCnt;
	const unsigned char *pucEnd;
	unsigned int uiMmioPin;
	NX90_MMIO_CFG_T tMmioCfg;


	/* Loop over all pins. */
	pucCnt = ptCfg->aucMmio;
	pucEnd = pucCnt + sizeof(ptCfg->aucMmio);
	do
	{
		/* Get the MMIO configuration. */
		tMmioCfg = *(ptMmioValues++);
		/* Get the pin index. */
		uiMmioPin = *(pucCnt++);
		/* Is the index valid? */
		if( tMmioCfg!=0xffU && uiMmioPin!=0xffU )
		{
			/* Activate the pin. */
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
			ptMmioCtrlArea->aulMmio_cfg[uiMmioPin] = tMmioCfg;
		}
	} while( pucCnt<pucEnd );
}



void boot_spi_deactivate_mmio(const SPI_CFG_T *ptCfg, const NX90_MMIO_CFG_T *ptMmioValues)
{
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	const unsigned char *pucCnt;
	const unsigned char *pucEnd;
	unsigned int uiMmioPin;
	NX90_MMIO_CFG_T tMmioCfg;


	/* Loop over all pins. */
	pucCnt = ptCfg->aucMmio;
	pucEnd = pucCnt + sizeof(ptCfg->aucMmio);
	do
	{
		/* Get the MMIO configuration. */
		tMmioCfg = *(ptMmioValues++);
		/* Get the pin index. */
		uiMmioPin = *(pucCnt++);
		/* Is the index valid? */
		if( tMmioCfg!=0xffU && uiMmioPin!=0xffU )
		{
			/* Deactivate the pin. */
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
			ptMmioCtrlArea->aulMmio_cfg[uiMmioPin] = NX90_MMIO_CFG_DISABLE;
		}
	} while( pucCnt<pucEnd );
}
