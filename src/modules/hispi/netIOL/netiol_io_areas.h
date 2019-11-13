/***************************************************************************
 *   Copyright (C) 2018 by Hilscher GmbH                                   *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/

#ifndef __NETIOL_IO_AREAS_H__
#define __NETIOL_IO_AREAS_H__


#include "netiol_regdef.h"


#define NIOL_DEF_ptAsicCtrlArea NIOL_ASIC_CTRL_AREA_T * const ptAsicCtrlArea = (NIOL_ASIC_CTRL_AREA_T * const)Addr_NIOL_asic_ctrl;
#define NIOL_DEF_ptSpiArea NIOL_SPI_AREA_T * const ptSpiArea = (NIOL_SPI_AREA_T * const)Addr_NIOL_spi;
#define NIOL_DEF_ptPadCtrlArea NIOL_PAD_CTRL_AREA_T * const ptPadCtrlArea = (NIOL_PAD_CTRL_AREA_T * const)Addr_NIOL_pad_ctrl;
#define NIOL_DEF_ptSwTimerArea NIOL_ARM_TIMER_AREA_T * const ptSwTimerArea = (NIOL_ARM_TIMER_AREA_T * const)Addr_NIOL_sw_timer;


#endif  /* __NETIOL_IO_AREAS_H__ */
