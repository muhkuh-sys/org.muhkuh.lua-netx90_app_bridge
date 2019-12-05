/***************************************************************************
 *   Copyright (C) 2013-2014 by Christoph Thelen                           *
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


#include "main_module.h"

#include <string.h>

#include "boot_drv_spi.h"
#include "crc.h"
#include "rdy_run.h"
#include "systime.h"

#if ASIC_TYP==ASIC_TYP_NETX90_APP || ASIC_TYP==ASIC_TYP_NETX90_MPW_APP
#       include "pad_control.h"
#       include "netIOL/netiol_regdef.h"
#endif

/*-------------------------------------------------------------------------*/


SPI_CFG_T s_tSpiCfg;


typedef union PTR_UNION
{
	unsigned long ul;
	unsigned char *puc;
	unsigned short *pus;
	unsigned long *pul;
} PTR_T;

/*-------------------------------------------------------------------------*/

/* #define NETIOL_TEST_AREA Adr_NIOL_dram_base */
/* This area is reserved by the ROM code. */
#define NETIOL_TEST_AREA 0x6ff8

static const unsigned short ausTestPattern[4] =
{
	0x0123U,
	0x4567U,
	0x89abU,
	0xcdefU
};
static unsigned short ausTestPatternBigEndian[4];

static void mirror_test_pattern(void)
{
	unsigned int uiCnt;
	unsigned short usData;
	unsigned short usDataMirror;


	for(uiCnt=0; uiCnt<(sizeof(ausTestPattern)/sizeof(unsigned short)); ++uiCnt)
	{
		usData = ausTestPattern[uiCnt];
		usDataMirror = (unsigned short)(((usData & 0x00ffU) << 8U) | ((usData & 0xff00U) >> 8U));
		ausTestPatternBigEndian[uiCnt] = usDataMirror;
	}
}


/*-------------------------------------------------------------------------*/


static unsigned char aucNodeaddressMirrorTable[128];

static void setup_nodeaddress_mirror_table(void)
{
	unsigned int uiCnt;
	unsigned int uiMirror;


	for(uiCnt=0; uiCnt<sizeof(aucNodeaddressMirrorTable); ++uiCnt)
	{
		/* Mirror the bits 0 - 6. */
		uiMirror = (
			(((uiCnt & 0x01) >> 0) << 6U) |
			(((uiCnt & 0x02) >> 1) << 5U) |
			(((uiCnt & 0x04) >> 2) << 4U) |
			(((uiCnt & 0x08) >> 3) << 3U) |
			(((uiCnt & 0x10) >> 4) << 2U) |
			(((uiCnt & 0x20) >> 5) << 1U) |
			(((uiCnt & 0x40) >> 6) << 0U)
		);
		aucNodeaddressMirrorTable[uiCnt] = (unsigned char)uiMirror;
	}
}


/*-------------------------------------------------------------------------*/


static int open_driver(unsigned int uiUnit, unsigned int uiChipSelect, const BOOT_SPI_CONFIGURATION_T *ptSpiConfiguration, SPI_CFG_T *ptSpiCfg)
{
	int iResult;


	iResult = boot_drv_spi_init(ptSpiCfg, ptSpiConfiguration, uiUnit, uiChipSelect);

	return iResult;
}



static void copy_shifted(unsigned char *pucDst, const unsigned char *pucSrc, unsigned int sizDataInBytes, unsigned int uiShiftInBits)
{
	unsigned int uiShift;
	unsigned char *pucDstEnd;
	unsigned char ucData;


	uiShift = uiShiftInBits & 7U;
	if( uiShift==0U )
	{
		memcpy(pucDst, pucSrc, sizDataInBytes);
	}
	else
	{
		pucDstEnd = pucDst + sizDataInBytes;

		/* Extract the start of the first byte. */
		ucData = *(pucSrc++);
		*pucDst |= (unsigned char)(ucData << uiShift);

		while( pucDst<(pucDstEnd-1U) )
		{
			/* Get the current byte. */
			ucData = *(pucSrc++);
			*(pucDst++) |= (unsigned char)(ucData >> (8U - uiShift));
			*(pucDst)   |= (unsigned char)(ucData << uiShift);
		}

		/* Process the last part. */
		ucData = *(pucSrc++);
		*(pucDst++) |= (unsigned char)(ucData >> (8U - uiShift));
	}
}


#if 0
static void copy_shifted_bits(unsigned char *pucDst, const unsigned char *pucSrc, unsigned int sizCopyBits, unsigned int uiDstOffsetBits)
{
	const unsigned char *pucSrcCnt;
	const unsigned char *pucSrcEnd;
	unsigned char ucData;
	unsigned int sizCopyBytes;
	unsigned int uiDstOffsetBytes;
	unsigned int uiDstOffsetModBits;


	/* Split the offset in bytes and bits. */
	uiDstOffsetBytes = uiDstOffsetBits >> 3U;
	uiDstOffsetModBits = uiDstOffsetBits & 7U;

	/* Round the size up to the next byte border. */
	sizCopyBytes = (sizCopyBits + 7U) >> 3U;

	if( uiDstOffsetModBits==0 )
	{
		memcpy(pucDst + uiDstOffsetBytes, pucSrc, sizCopyBytes);
	}
	else
	{
		pucSrcCnt = pucSrc;
		pucSrcEnd = pucSrc + sizCopyBytes;
		pucDst = pucDst + uiDstOffsetBytes;
		while( pucSrcCnt<pucSrcEnd )
		{
			ucData = *(pucSrcCnt++);

			*(pucDst++) |= (unsigned char)(ucData >> uiDstOffsetModBits);
			*(pucDst)   |= (unsigned char)(ucData <<(8U-uiDstOffsetModBits));
		}
	}
}
#endif


typedef struct HISPI_PACKET_STRUCT
{
	unsigned int uiNodeAddress;          /* The address of the node starting with 0. */
	unsigned int uiMemoryAddress;        /* The memory address to access. */
	unsigned int uiDataSizeIn16Bits;     /* The number of bytes to read/write. */
	const unsigned short *pusWriteData;  /* NULL for a read operation, a pointer to the write data for a write operation. */
} HISPI_PACKET_T;



static unsigned int assemble_command(unsigned char *pucBuffer, unsigned int sizBuffer, HISPI_PACKET_T *ptPacket)
{
	unsigned int uiResult;
	unsigned int ui16BitLength;
	unsigned int uiByteLength;
	unsigned int uiAddress;
	unsigned int uiNodeAddress;
	unsigned char ucRnW;
	unsigned char ucCrc4;
	unsigned short usCrc12;


	ui16BitLength = ptPacket->uiDataSizeIn16Bits;
	/* Get the byte size of the data. */
	uiByteLength = ui16BitLength << 1U;

	uiAddress = ptPacket->uiMemoryAddress;
	uiNodeAddress = ptPacket->uiNodeAddress;

	/* A command has 60 protocol bits which results in 8 bytes. */
	if( (uiByteLength+8U)>sizBuffer )
	{
		/* Buffer overflow. */
		uiResult = 0;
	}
	else if( ((uiAddress&1U)!=0) || (uiAddress>=0x00010000U) )
	{
		/* Invalid address. */
		uiResult = 0;
	}
	else if( uiNodeAddress>=0x80 )
	{
		uiResult = 0;
	}
	else
	{
		/* Add the start bit and the address. */
		pucBuffer[0x00] = (unsigned char)(0x80U | ((uiAddress & 0xfe00U) >> 9U));
		pucBuffer[0x01] = (unsigned char)((uiAddress & 0x01fe) >> 1U);

		/* Set Rn/W and the node address. */
		ucRnW = 0x00U;
		if( ptPacket->pusWriteData!=NULL )
		{
			ucRnW = 0x80U;
		}
		pucBuffer[0x02] = (unsigned char)(ucRnW | aucNodeaddressMirrorTable[uiNodeAddress & 0x7fU]);

		/* data length - 1 */
		pucBuffer[0x03] = (unsigned char)(ui16BitLength - 1U);

		/* Get the CRC4 over the header. */
		ucCrc4 = crc4(0, pucBuffer, 4);
		/* Set the CRC4 and the IRQs. */
		pucBuffer[0x04] = (unsigned char)(ucCrc4 << 4U);
		pucBuffer[0x05] = 0x00;

		if( ptPacket->pusWriteData==NULL )
		{
			/* Reserve space for the read data. */
			memset(pucBuffer+6, 0x00, uiByteLength);
		}
		else
		{
			/* Copy the write data. */
			memcpy(pucBuffer+6, ptPacket->pusWriteData, uiByteLength);
		}

		/* Build a CRC12 over the complete packet. */
		usCrc12 = crc12(0, pucBuffer, 6+uiByteLength);
		pucBuffer[6+uiByteLength+0] = (unsigned char)((usCrc12 >> 4U) & 0xffU);
		pucBuffer[6+uiByteLength+1] = (unsigned char)((usCrc12 & 0x0fU) << 4U);

		/* Get the size of the packet in bits.
		 * This is...
		 *                 1 bit: start bit
		 *                15 bit: address
		 *                 1 bit: Rn/W
		 *                 7 bit: node address
		 *                 8 bit: data length
		 *                 4 bit: CRC4
		 *                12 bit: IRQ
		 *  uiByteLength * 8 bit: data
		 *                12 bit: CRC12
		 */
		uiResult = 1U + 15U + 1U + 7U + 8U + 4U + 12U + (uiByteLength * 8U) + 12U;
	}

	return uiResult;
}



static unsigned int extract_response(const unsigned char *pucDataIn, unsigned char *pucDataOut, unsigned int sizDataIn, unsigned int uiBitOffset)
{
	int iResult;
	unsigned int uiResult;
	unsigned int uiByteCnt;
	unsigned int uiBitShift;
	unsigned char ucData;
	unsigned char ucMask;
	unsigned int uiDataSize;
	unsigned char ucCrc4Resp;
	unsigned char ucCrc4Local;
	unsigned short usCrc12Resp;
	unsigned short usCrc12Local;


	iResult = -1;
	uiResult = 0;
	uiByteCnt = uiBitOffset >> 3U;
	uiBitShift = uiBitOffset & 7U;

	/* Skip 0 bits. */
	do
	{
		ucData = pucDataIn[uiByteCnt];
		ucMask = (unsigned char)(0x80U >> uiBitShift);
		do
		{
			if( (ucData&ucMask)!=0 )
			{
				iResult = 0;
				break;
			}
			else
			{
				ucMask >>= 1U;
				++uiBitShift;
			}
		} while( uiBitShift<8U );

		if( iResult==0 )
		{
			break;
		}
		else
		{
			uiBitShift = 0U;
			++uiByteCnt;
		}
	} while( uiByteCnt<sizDataIn );

	if( iResult==0 )
	{
		memset(pucDataOut, 0, 6U);

		/* Copy the header to get the size of the data. */
		copy_shifted(pucDataOut, pucDataIn+uiByteCnt, 6U, uiBitShift);

		/* Get the length field. */
		uiDataSize = (pucDataOut[3] + 1U) * 2U;
		/* Is the data size valid? */
		if( (uiByteCnt+1U+uiDataSize+8U)>sizDataIn )
		{
			iResult = -1;
		}
		else
		{
			memset(pucDataOut, 0, 8U+uiDataSize);

			/* Now get the complete data. */
			copy_shifted(pucDataOut, pucDataIn+uiByteCnt, 8U+uiDataSize, uiBitShift);

			/* Check the CRCs of the response. */
			/* Build the CRC4. */
			ucCrc4Resp = crc4(0, pucDataOut, 4);
			ucCrc4Local = (unsigned char)(pucDataOut[4] >> 4U);

			/* Build the CRC12. */
			usCrc12Resp = crc12(0, pucDataOut, 6U+uiDataSize);
			usCrc12Local = (unsigned short)(
				((unsigned int)(pucDataOut[6U+uiDataSize]) << 4U) |
				((pucDataOut[6U+uiDataSize+1U] & 0xf0U) >> 4U)
			);

			if( ucCrc4Resp==ucCrc4Local && usCrc12Resp==usCrc12Local )
			{
				/* Return the new bit offset after the processed packet.
				 * This is the start of the packet (uiByteCnt/uiBitShift) plus
				 *           1 bit: start bit
				 *          15 bit: address
				 *           1 bit: Rn/W
				 *           7 bit: node address
				 *           8 bit: data length
				 *           4 bit: CRC4
				 *          12 bit: IRQ
				 *  uiDataSize bit: data
				 *          12 bit: CRC12
				 */
				uiResult  = (uiByteCnt * 8U) + uiBitShift;
				uiResult += 1U + 15U + 1U + 7U + 8U + 4U + 12U + (uiDataSize * 8U) + 12U;
			}
		}
	}

	return uiResult;
}



static int write_single(SPI_CFG_T *ptSpiCfg, unsigned char ucNodeAddress, unsigned short usMemoryAddress, unsigned short usData)
{
	int iResult;
	unsigned int uiResult;
//	unsigned int uiRetries;
	unsigned int uiPacketSizeBits;
	unsigned int uiPacketSizeBytes;
	HISPI_PACKET_T tPacket;
	union {
		unsigned char auc[2];
		unsigned short aus[1];
	} uBuffer;
	unsigned char aucRequest[64];
	unsigned char aucResponseAll[64];
	unsigned char aucResponseDevice[64];


	iResult = -1;
//	uiRetries = 16;

//	do
//	{
		memset(aucRequest, 0, sizeof(aucRequest));
		memset(aucResponseAll, 0, sizeof(aucResponseAll));
		memset(aucResponseDevice, 0, sizeof(aucResponseDevice));

		uBuffer.auc[0] = (unsigned char)((usData & 0xff00) >> 8U);
		uBuffer.auc[1] = (unsigned char) (usData & 0x00ff);

		tPacket.uiNodeAddress = ucNodeAddress;
		tPacket.uiMemoryAddress = usMemoryAddress;
		tPacket.uiDataSizeIn16Bits = 1;
		tPacket.pusWriteData = uBuffer.aus;
		uiPacketSizeBits = assemble_command(aucRequest, sizeof(aucRequest), &tPacket);
		if( uiPacketSizeBits!=0 )
		{
			/* Send 9 bits more for each of the 4 units.
			 * Each unit can have a shift of 2 - 9 bits.
			 */
			uiPacketSizeBits += 9U * 4U;
			/* Round up to the next byte boundary. */
			uiPacketSizeBytes = (uiPacketSizeBits + 7U) >> 3U;

			ptSpiCfg->pfnExchangeData(ptSpiCfg, aucRequest, aucResponseAll, uiPacketSizeBytes);

			uiResult = extract_response(aucResponseAll, aucResponseDevice, sizeof(aucResponseAll), 0);
			if( uiResult!=0 )
			{
				iResult = 0;
			}
		}

//		if( iResult!=0 )
//		{
//			if( uiRetries==0 )
//			{
//				break;
//			}
//			else
//			{
//				--uiRetries;
//			}
//		}
//	} while( iResult!=0 );

	return iResult;
}


#if 0
static int write_single_32(SPI_CFG_T *ptSpiCfg, unsigned char ucNodeAddress, unsigned short usMemoryAddress, unsigned long ulData)
{
	int iResult;
	unsigned int uiResult;
//	unsigned int uiRetries;
	unsigned int uiPacketSizeBits;
	unsigned int uiPacketSizeBytes;
	HISPI_PACKET_T tPacket;
	union {
		unsigned char auc[4];
		unsigned short aus[2];
	} uBuffer;
	unsigned char aucRequest[64];
	unsigned char aucResponseAll[64];
	unsigned char aucResponseDevice[64];


	iResult = -1;

	memset(aucRequest, 0, sizeof(aucRequest));
	memset(aucResponseAll, 0, sizeof(aucResponseAll));
	memset(aucResponseDevice, 0, sizeof(aucResponseDevice));

	uBuffer.auc[0] = (unsigned char)((ulData & 0x0000ff00) >>  8U);
	uBuffer.auc[1] = (unsigned char) (ulData & 0x000000ff);
	uBuffer.auc[2] = (unsigned char)((ulData & 0xff000000) >> 24U);
	uBuffer.auc[3] = (unsigned char)((ulData & 0x00ff0000) >> 16U);

	tPacket.uiNodeAddress = ucNodeAddress;
	tPacket.uiMemoryAddress = usMemoryAddress;
	tPacket.uiDataSizeIn16Bits = 2;
	tPacket.pusWriteData = uBuffer.aus;
	uiPacketSizeBits = assemble_command(aucRequest, sizeof(aucRequest), &tPacket);
	if( uiPacketSizeBits!=0 )
	{
		/* Send 9 bits more for each of the 4 units.
		 * Each unit can have a shift of 2 - 9 bits.
		 */
		uiPacketSizeBits += 9U * 4U;
		/* Round up to the next byte boundary. */
		uiPacketSizeBytes = (uiPacketSizeBits + 7U) >> 3U;

		ptSpiCfg->pfnExchangeData(ptSpiCfg, aucRequest, aucResponseAll, uiPacketSizeBytes);

		uiResult = extract_response(aucResponseAll, aucResponseDevice, sizeof(aucResponseAll), 0);
		if( uiResult!=0 )
		{
			iResult = 0;
		}
	}

	return iResult;
}
#endif


static int read_single(SPI_CFG_T *ptSpiCfg, unsigned char ucNodeAddress, unsigned short usMemoryAddress, unsigned short *pusData)
{
	int iResult;
	unsigned int uiResult;
//	unsigned int uiRetries;
	unsigned int uiPacketSizeBits;
	unsigned int uiPacketSizeBytes;
	unsigned short usData;
	HISPI_PACKET_T tPacket;
	unsigned char aucRequest[64];
	unsigned char aucResponseAll[64];
	unsigned char aucResponseDevice[64];


	iResult = -1;
//	uiRetries = 16;

//	do
//	{
		memset(aucRequest, 0, sizeof(aucRequest));
		memset(aucResponseAll, 0, sizeof(aucResponseAll));
		memset(aucResponseDevice, 0, sizeof(aucResponseDevice));

		tPacket.uiNodeAddress = ucNodeAddress;
		tPacket.uiMemoryAddress = usMemoryAddress;
		tPacket.uiDataSizeIn16Bits = 1;
		tPacket.pusWriteData = NULL;
		uiPacketSizeBits = assemble_command(aucRequest, sizeof(aucRequest), &tPacket);
		if( uiPacketSizeBits!=0 )
		{
			/* Send 9 bits more for each of the 4 units.
			 * Each unit can have a shift of 2 - 9 bits.
			 */
			uiPacketSizeBits += 9U * 4U;
			/* Round up to the next byte boundary. */
			uiPacketSizeBytes = (uiPacketSizeBits + 7U) >> 3U;

			ptSpiCfg->pfnExchangeData(ptSpiCfg, aucRequest, aucResponseAll, uiPacketSizeBytes);

			uiResult = extract_response(aucResponseAll, aucResponseDevice, sizeof(aucResponseAll), 0);
			if( uiResult!=0 )
			{
				usData = (unsigned short)(
					(((unsigned int)aucResponseDevice[6]) << 8U) |
					aucResponseDevice[7]
				);
				*pusData = usData;

				iResult = 0;
			}
		}

//		if( iResult!=0 )
//		{
//			if( uiRetries==0 )
//			{
//				break;
//			}
//			else
//			{
//				--uiRetries;
//			}
//		}
//	} while( iResult!=0 );

	return iResult;
}


#if 0
static int read_single_32(SPI_CFG_T *ptSpiCfg, unsigned char ucNodeAddress, unsigned short usMemoryAddress, unsigned long *pulData)
{
	int iResult;
	unsigned int uiResult;
	unsigned int uiPacketSizeBits;
	unsigned int uiPacketSizeBytes;
	unsigned long ulData;
	HISPI_PACKET_T tPacket;
	unsigned char aucRequest[64];
	unsigned char aucResponseAll[64];
	unsigned char aucResponseDevice[64];


	iResult = -1;

	memset(aucRequest, 0, sizeof(aucRequest));
	memset(aucResponseAll, 0, sizeof(aucResponseAll));
	memset(aucResponseDevice, 0, sizeof(aucResponseDevice));

	tPacket.uiNodeAddress = ucNodeAddress;
	tPacket.uiMemoryAddress = usMemoryAddress;
	tPacket.uiDataSizeIn16Bits = 2;
	tPacket.pusWriteData = NULL;
	uiPacketSizeBits = assemble_command(aucRequest, sizeof(aucRequest), &tPacket);
	if( uiPacketSizeBits!=0 )
	{
		/* Send 9 bits more for each of the 4 units.
		 * Each unit can have a shift of 2 - 9 bits.
		 */
		uiPacketSizeBits += 9U * 4U;
		/* Round up to the next byte boundary. */
		uiPacketSizeBytes = (uiPacketSizeBits + 7U) >> 3U;

		ptSpiCfg->pfnExchangeData(ptSpiCfg, aucRequest, aucResponseAll, uiPacketSizeBytes);

		uiResult = extract_response(aucResponseAll, aucResponseDevice, sizeof(aucResponseAll), 0);
		if( uiResult!=0 )
		{
			ulData  = ((unsigned long)aucResponseDevice[6]) <<  8U;
			ulData |=  (unsigned long)aucResponseDevice[7];
			ulData |= ((unsigned long)aucResponseDevice[8]) << 24U;
			ulData |= ((unsigned long)aucResponseDevice[9]) << 16U;
			*pulData = ulData;

			iResult = 0;
		}
	}

	return iResult;
}
#endif


static unsigned long netiol_device_init(SPI_CFG_T *ptSpiCfg, const unsigned short *pusMisoCfg, unsigned int uiNumberOfDevices)
{
	unsigned long ulResult;
	int iResult;
	unsigned short usAddr;
	unsigned short usData;
	unsigned char ucNodeAddress;
	unsigned long ulValue;
	unsigned int uiCnt;


	/* Be optimistic. */
	ulResult = HISPI_RESULT_Ok;

	/* Run one dummy read to clean the shift registers. */
	usAddr = 0x0000;
	for(ucNodeAddress=0; ucNodeAddress<uiNumberOfDevices; ++ucNodeAddress)
	{
		read_single(ptSpiCfg, ucNodeAddress, usAddr, &usData);
	}

	/* The BOOT_COMMAND at 0x6fec must be "LOADING" (0x0123) or
	 * "STOP_LOADER" (0x4567).
	 */
	usAddr = 0x6fecU;
	for(ucNodeAddress=0; ucNodeAddress<uiNumberOfDevices; ++ucNodeAddress)
	{
		iResult = read_single(ptSpiCfg, ucNodeAddress, usAddr, &usData);
		if( iResult!=0 )
		{
			ulResult = HISPI_RESULT_HiSpiAccessFailed;
			break;
		}
		else if( usData!=0x0123 && usData!=0x4567 )
		{
			ulResult = HISPI_RESULT_UnknownBootCommand;
			break;
		}
	}

	if( ulResult==HISPI_RESULT_Ok )
	{
		/* Stop the loader to enable ram access. */
		for(ucNodeAddress=0; ucNodeAddress<uiNumberOfDevices; ++ucNodeAddress)
		{
			iResult = write_single(ptSpiCfg, ucNodeAddress, usAddr, 0x4567);
			if( iResult!=0 )
			{
				ulResult = HISPI_RESULT_HiSpiAccessFailed;
				break;
			}
		}

		if( ulResult==HISPI_RESULT_Ok )
		{
			/* Wait for all devices to stop booting. */
			for(ucNodeAddress=0; ucNodeAddress<uiNumberOfDevices; ++ucNodeAddress)
			{
				uiCnt = 2048;
				usAddr = 0x6ff0U;
				do
				{
					iResult = read_single(ptSpiCfg, ucNodeAddress, usAddr, &usData);
					if( iResult==0 )
					{
						if( usData==0x5678U )
						{
							break;
						}
						else
						{
							iResult = -1;
						}
					}

					--uiCnt;
				} while( uiCnt!=0 );

				if( iResult!=0 )
				{
					ulResult = HISPI_RESULT_DeviceDidNotStopBooting;
					break;
				}
			}
		}
	}

	if( ulResult==HISPI_RESULT_Ok )
	{
		/* Activate the PLL.
		 * Settings for 400MHz PLL clock and 100MHZ System clk (clk).
		 * Refclock = 8,333 MHz
		 */
		for(ucNodeAddress=0; ucNodeAddress<uiNumberOfDevices; ++ucNodeAddress)
		{
			/* Is the PLL already running?
			 * Do not configure it twice. This hurts.
			 */
			iResult = read_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_clk_sys_config, &usData);
			if( iResult==0 )
			{
				ulValue  = (unsigned long)usData;
				ulValue &= MSK_NIOL_asic_ctrl_clk_sys_config_src;
				if( ulValue==0 )
				{
					/* PLL feedback divider  fvco/ffb = (d_fd + 2) * 2 */
					ulValue  =   94U << SRT_NIOL_asic_ctrl_pll_config0_pll_fd;
					ulValue |= 0x36U << SRT_NIOL_asic_ctrl_pll_config0_pw;
					iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_pll_config0, (unsigned short)ulValue);
					if( iResult==0 )
					{
						ulValue  =    0U << SRT_NIOL_asic_ctrl_pll_config1_pll_oe_n;
						ulValue |=    1U << SRT_NIOL_asic_ctrl_pll_config1_pll_pd;
						ulValue |=    0U << SRT_NIOL_asic_ctrl_pll_config1_pll_rd;
						ulValue |=    0U << SRT_NIOL_asic_ctrl_pll_config1_pll_od;
						ulValue |=    0U << SRT_NIOL_asic_ctrl_pll_config1_pll_bypass;
						ulValue |= 0x1aU << SRT_NIOL_asic_ctrl_pll_config1_pw;
						iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_pll_config1, (unsigned short)ulValue);
						if( iResult==0 )
						{
							ulValue &= ~(MSK_NIOL_asic_ctrl_pll_config1_pll_pd);
							iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_pll_config1, (unsigned short)ulValue);
							if( iResult==0 )
							{
								/* Delay 20ms. */
								systime_delay_ms(20);

								/* Switch SYS CLK to PLL. */
								ulValue  =     0U << SRT_NIOL_asic_ctrl_clk_sys_config_src;
								ulValue |=     1U << SRT_NIOL_asic_ctrl_clk_sys_config_div;
								ulValue |= 0x56cU << SRT_NIOL_asic_ctrl_clk_sys_config_pw;
								iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_clk_sys_config, (unsigned short)ulValue);
								if( iResult==0 )
								{
									ulValue |=    1U << SRT_NIOL_asic_ctrl_clk_sys_config_src;
									iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_clk_sys_config, (unsigned short)ulValue);
									if( iResult==0 )
									{
										/* Is this really necessary? */
										ulValue  =   25U << SRT_NIOL_asic_ctrl_ofc_clk_bg_div_rld;
										ulValue |=   25U << SRT_NIOL_asic_ctrl_ofc_clk_vref_div_rld;
										iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_asic_ctrl_ofc_clk, (unsigned short)ulValue);
									}
								}
							}
						}
					}
				}
			}

			if( iResult!=0 )
			{
				ulResult = HISPI_RESULT_HiSpiAccessFailed;
				break;
			}
		}

	}

	/* Set the last device to "early MISO". */
	if( ulResult==HISPI_RESULT_Ok )
	{
		for(ucNodeAddress=0; ucNodeAddress<uiNumberOfDevices; ++ucNodeAddress)
		{
			usData = pusMisoCfg[ucNodeAddress];
			iResult = write_single(ptSpiCfg, ucNodeAddress, Adr_NIOL_hispi_cfg_miso, usData);

			if( iResult!=0 )
			{
				ulResult = HISPI_RESULT_HiSpiAccessFailed;
				break;
			}
		}
	}

	return ulResult;
}



static unsigned long hispi_initialize(const HISPI_PARAMETER_T *ptParameter, const unsigned short *pusMisoCfg, unsigned long uiNumberOfDevices)
{
	unsigned long ulResult;
	int iResult;


	iResult = open_driver(ptParameter->uiUnit, ptParameter->uiChipSelect, &(ptParameter->tSpiConfiguration), &s_tSpiCfg);
	if( iResult!=0 )
	{
		ulResult = HISPI_RESULT_FailedToOpenSpi;
	}
	else
	{
		ulResult = netiol_device_init(&s_tSpiCfg, pusMisoCfg, uiNumberOfDevices);
	}

	return ulResult;
}


/*-------------------------------------------------------------------------*/


static const unsigned char aucPadCtrlSpiAppIndex[4] =
{
	PAD_AREG2OFFSET(mmio, 0),    /* MMIO0 */
	PAD_AREG2OFFSET(mmio, 1),    /* MMIO1 */
	PAD_AREG2OFFSET(mmio, 2),    /* MMIO2 */
	PAD_AREG2OFFSET(mmio, 3)     /* MMIO3 */
};



static const unsigned char aucPadCtrlSpiAppConfig[4] =
{
	PAD_CONFIGURATION(PAD_DRIVING_STRENGTH_High, PAD_PULL_Disable, PAD_INPUT_Enable),
	PAD_CONFIGURATION(PAD_DRIVING_STRENGTH_High, PAD_PULL_Disable, PAD_INPUT_Enable),
	PAD_CONFIGURATION(PAD_DRIVING_STRENGTH_High, PAD_PULL_Disable, PAD_INPUT_Enable),
	PAD_CONFIGURATION(PAD_DRIVING_STRENGTH_High, PAD_PULL_Disable, PAD_INPUT_Disable)
};



static void setup_padctrl(void)
{
	pad_control_apply(aucPadCtrlSpiAppIndex, aucPadCtrlSpiAppConfig, sizeof(aucPadCtrlSpiAppIndex));
}



static void netiol_setup_refclk(void)
{
	HOSTDEF(ptGpioAppArea);
	HOSTDEF(ptMmioCtrlArea);
	HOSTDEF(ptAsicCtrlArea);


	/* Set the GPIO pin to input. */
	ptGpioAppArea->aulGpio_app_cfg[0] = 0;
	/* Stop the timer. */
	ptGpioAppArea->aulGpio_app_counter_ctrl[0] = 0;
	/* Setup the timer. */
	ptGpioAppArea->aulGpio_app_counter_cnt[0] = 0;
	ptGpioAppArea->aulGpio_app_counter_max[0] = 11;
	ptGpioAppArea->aulGpio_app_tc[0] = 6;
	/* Start the timer. */
	ptGpioAppArea->aulGpio_app_counter_ctrl[0] = HOSTMSK(gpio_app_counter0_ctrl_run);
	/* Set the GPIO to PWM mode. */
	ptGpioAppArea->aulGpio_app_cfg[0] = 7U << HOSTSRT(gpio_app_cfg0_mode);

	/* Set the MMIO3 to GPIO0. */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[3] = NX90_MMIO_CFG_GPIO0;
}


/*-------------------------------------------------------------------------*/

static const HISPI_PARAMETER_T s_tParameter =
{
	.tSpiConfiguration = {
		.ulSpeedFifoKhz = 1000,
		.ulSpeedSqiRomKhz = 0,
		.ausPortControl = {
		/* NOTE: these values are netX4000 only. See "aucPadCtrlSpiAppConfig" for the values. */
			0,  /* CSn */
			0,  /* CLK */
			0,  /* MISO */
			0,  /* MOSI */
			0,  /* SIO2 */
			0   /* SIO3 */
		},
		.aucMmio = {
			0xff,  /* CSn */
			0,     /* CLK */
			2,     /* MISO */
			1,     /* MOSI */
			0xff,  /* SIO2 */
			0xff   /* SIO3 */
		},
		.ucDummyByte = 0x00,
		.ucMode = SPI_MODE1,
		.ucIdleConfiguration = 0
	},
	.uiUnit = 0,
	.uiChipSelect = 0
};



/* Set the netIOL units to normal or "MISO early" mode.
 * Add MSK_NIOL_hispi_cfg_miso_early to a line to enable it.
 * */
static const unsigned short ausMisoCfg[4] =
{
	(0U << SRT_NIOL_hispi_cfg_miso_delay),  /* netIOL0 */
	(0U << SRT_NIOL_hispi_cfg_miso_delay),  /* netIOL1 */
	(0U << SRT_NIOL_hispi_cfg_miso_delay) | MSK_NIOL_hispi_cfg_miso_early,  /* netIOL2 */
	(0U << SRT_NIOL_hispi_cfg_miso_delay)   /* netIOL4 */
};



static unsigned long module_command_read16(unsigned long ulNodeAddress, unsigned long ulRegisterAddress, unsigned short *pusData)
{
	unsigned long ulResult;
	int iResult;
	unsigned char ucNodeAddress;
	unsigned short usAddr;


	ucNodeAddress = (unsigned char)(ulNodeAddress & 0xffU);
	usAddr = (unsigned short)(ulRegisterAddress & 0xffffU);

	iResult = read_single(&s_tSpiCfg, ucNodeAddress, usAddr, pusData);
	if( iResult!=0 )
	{
		ulResult = HISPI_RESULT_HiSpiAccessFailed;
	}
	else
	{
		ulResult = HISPI_RESULT_Ok;
	}

	return ulResult;
}



static unsigned long module_command_write16(unsigned long ulNodeAddress, unsigned long ulRegisterAddress, unsigned short usData)
{
	unsigned long ulResult;
	int iResult;
	unsigned char ucNodeAddress;
	unsigned short usAddr;


	ucNodeAddress = (unsigned char)(ulNodeAddress & 0xffU);
	usAddr = (unsigned short)(ulRegisterAddress & 0xffffU);

	iResult = write_single(&s_tSpiCfg, ucNodeAddress, usAddr, usData);
	if( iResult!=0 )
	{
		ulResult = HISPI_RESULT_HiSpiAccessFailed;
	}
	else
	{
		ulResult = HISPI_RESULT_Ok;
	}

	return ulResult;
}



extern unsigned char __buffer_start__[];

static unsigned long module_command_sequence(unsigned long ulSequenceSize)
{
	unsigned long ulResult;
	unsigned char *pucSequenceCnt;
	unsigned char *pucSequenceEnd;
	unsigned char *pucOutCnt;
	unsigned long ulSizeLeft;
	HISPI_COMMAND_T tCommand;
	unsigned long ulNodeAddress;
	unsigned long ulRegisterAddress;
	unsigned short usData;


	ulResult = HISPI_RESULT_Ok;

	pucOutCnt = __buffer_start__;

	/* Loop over all sequences. */
	pucSequenceCnt = __buffer_start__;
	pucSequenceEnd = pucSequenceCnt + ulSequenceSize;
	while( pucSequenceCnt<pucSequenceEnd )
	{
		ulResult = HISPI_RESULT_InvalidSequenceCommand;
		tCommand = (HISPI_COMMAND_T)(pucSequenceCnt[0]);
		switch(tCommand)
		{
		case HISPI_COMMAND_Initialize:
			/* The "initialize" command can not be used in a sequence. */
			break;

		case HISPI_COMMAND_ReadRegister16:
		case HISPI_COMMAND_WriteRegister16:
			ulResult = HISPI_RESULT_Ok;
			break;

		case HISPI_COMMAND_RunSequence:
			/* The "run sequence" command can not be used in a sequence. */
			break;
		}
		if( ulResult==HISPI_RESULT_Ok )
		{
			ulSizeLeft = (unsigned long)(pucSequenceEnd - pucSequenceCnt);

			ulResult = HISPI_RESULT_InvalidSequenceCommand;
			switch(tCommand)
			{
			case HISPI_COMMAND_Initialize:
				/* The "initialize" command can not be used in a sequence. */
				break;

			case HISPI_COMMAND_ReadRegister16:
				/* The ReadRegister16 command needs 4 bytes. */
				if( ulSizeLeft<4U )
				{
					ulResult = HISPI_RESULT_NotEnoughSequenceData;
				}
				else
				{
					ulNodeAddress = (unsigned long)(pucSequenceCnt[1]);
					ulRegisterAddress  = (unsigned long)(pucSequenceCnt[2]);
					ulRegisterAddress |= (unsigned long)(pucSequenceCnt[3] << 8U);
					ulResult = module_command_read16(ulNodeAddress, ulRegisterAddress, &usData);
					if( ulResult==HISPI_RESULT_Ok )
					{
						pucOutCnt[0] = (unsigned char)( usData       & 0xffU);
						pucOutCnt[1] = (unsigned char)((usData >> 8) & 0xffU);

						pucSequenceCnt += 4U;
						pucOutCnt += 2U;
					}
				}
				break;

			case HISPI_COMMAND_WriteRegister16:
				/* The WriteRegister16 command needs 6 bytes. */
				if( ulSizeLeft<6U )
				{
					ulResult = HISPI_RESULT_NotEnoughSequenceData;
				}
				else
				{
					ulNodeAddress = (unsigned long)(pucSequenceCnt[1]);
					ulRegisterAddress  = (unsigned long)(pucSequenceCnt[2]);
					ulRegisterAddress |= (unsigned long)(pucSequenceCnt[3] << 8U);
					usData = (unsigned short)(((unsigned long)(pucSequenceCnt[4])) | ((unsigned long)(pucSequenceCnt[5]<<8U)));
					ulResult = module_command_write16(ulNodeAddress, ulRegisterAddress, usData);
					if( ulResult==HISPI_RESULT_Ok )
					{
						pucSequenceCnt += 6U;
					}
				}
				break;

			case HISPI_COMMAND_RunSequence:
				/* The "run sequence" command can not be used in a sequence. */
				break;
			}

			if( ulResult!=HISPI_RESULT_Ok )
			{
				break;
			}
		}
	}

	return ulResult;
}



unsigned long module(unsigned long ulParameter0, unsigned long ulParameter1, unsigned long ulParameter2, unsigned long ulParameter3)
{
	unsigned long ulResult;
	PTR_T tPtr;
	unsigned short usData;


	if( ulParameter0==HISPI_COMMAND_Initialize )
	{
		/* Initialize. */
		mirror_test_pattern();
		setup_nodeaddress_mirror_table();

		setup_padctrl();
		netiol_setup_refclk();

		/* Delay for 500ms. */
		systime_delay_ms(500);

		ulResult = hispi_initialize(&s_tParameter, ausMisoCfg, ulParameter1);
	}
	else if( ulParameter0==HISPI_COMMAND_ReadRegister16 )
	{
		/* Read 16 */
		tPtr.ul = ulParameter3;
		ulResult = module_command_read16(ulParameter1, ulParameter2, tPtr.pus);
	}
	else if( ulParameter0==HISPI_COMMAND_WriteRegister16 )
	{
		/* Write 16 */
		usData = (unsigned short)(ulParameter3 & 0xffffU);
		ulResult = module_command_write16(ulParameter1, ulParameter2, usData);
	}
	else if( ulParameter0==HISPI_COMMAND_RunSequence )
	{
		ulResult = module_command_sequence(ulParameter1);
	}
	else
	{
		ulResult = HISPI_RESULT_UnknownCommand;
	}

	return ulResult;
}

/*-----------------------------------*/
