/***************************************************************************
 *   Copyright (C) 2019 by Christoph Thelen                                *
 *   doc_bacardi@users.sourceforge.net                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "boot_spi.h"


#ifndef __MAIN_MODULE_H__
#define __MAIN_MODULE_H__


typedef enum HISPI_RESULT_ENUM
{
	HISPI_RESULT_Ok                         = 0,
	HISPI_RESULT_UnknownCommand             = 1,
	HISPI_RESULT_FailedToOpenSpi            = 2,
	HISPI_RESULT_HiSpiAccessFailed          = 3,
	HISPI_RESULT_UnknownBootCommand         = 4,
	HISPI_RESULT_DeviceDidNotStopBooting    = 5
} HISPI_RESULT_T;


typedef enum HISPI_COMMAND_ENUM
{
	HISPI_COMMAND_Initialize                = 0,
	HISPI_COMMAND_ReadRegister16            = 1,
	HISPI_COMMAND_WriteRegister16           = 2
} HISPI_COMMAND_T;


typedef struct HISPI_PARAMETER_STRUCT
{
	BOOT_SPI_CONFIGURATION_T tSpiConfiguration;
	unsigned int uiUnit;
	unsigned int uiChipSelect;
} HISPI_PARAMETER_T;


unsigned long module(unsigned long ulParameter0, unsigned long ulParameter1, unsigned long ulParameter2, unsigned long ulParameter3);


#endif  /* __MAIN_MODULE_H__ */

