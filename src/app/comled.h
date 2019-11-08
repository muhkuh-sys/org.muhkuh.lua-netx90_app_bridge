/***************************************************************************
 *   Copyright (C) 2016 by Christoph Thelen                                *
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


#ifndef __COMLED_H__
#define __COMLED_H__

/*-----------------------------------*/

typedef enum
{
	COMLED_OFF    = 0,
	COMLED_GREEN  = 1,
	COMLED_RED    = 2
} COMLED_T;


typedef struct
{
	unsigned int uiLedIndex;
	unsigned long ulTimer;
	unsigned int uiCnt;
	unsigned long ulMask;
	unsigned long ulState;
} COMLED_BLINKI_HANDLE_T;


int comled_initialize(unsigned int uiLedIndex);
int comled_setLEDs(unsigned int uiLedIndex, COMLED_T tState);
int comled_blinki_init(COMLED_BLINKI_HANDLE_T *ptHandle, unsigned int uiLedIndex, unsigned long ulMask, unsigned long ulState);
void comled_blinki(COMLED_BLINKI_HANDLE_T *ptHandle);

/*-----------------------------------*/

#endif  /* __COMLED_H__ */
