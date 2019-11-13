/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#ifndef __BOOT_DRV_SPI_H__
#define __BOOT_DRV_SPI_H__

#include "boot_spi.h"

/*-------------------------------------*/


int boot_drv_spi_init(SPI_CFG_T *ptCfg, const BOOT_SPI_CONFIGURATION_T *ptSpiCfg, unsigned int uiSpiUnit, unsigned int uiChipSelect);


/*-------------------------------------*/


#endif  /* __BOOT_DRV_SPI_H__ */

