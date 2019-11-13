/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include <string.h>

#include "boot_drv_spi.h"

#include "netx_io_areas.h"
//#include "tools.h"
#if ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
#       include "portcontrol.h"
#endif


#if ASIC_TYP==ASIC_TYP_NETX50 || ASIC_TYP==ASIC_TYP_NETX10 || ASIC_TYP==ASIC_TYP_NETX56
static const MMIO_CFG_T aatMmioValues[2][3][6] =
{
	/* SPI0 */
	{
		/* SPI0 CS0 */
		{
			0xffU,                          /* chip select */
			0xffU,                          /* clock */
			0xffU,                          /* MISO */
			0xffU,                          /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI0 CS1 */
		{
			0xffU,                          /* chip select */
			0xffU,                          /* clock */
			0xffU,                          /* MISO */
			0xffU,                          /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI0 CS2 */
		{
			0xffU,                          /* chip select */
			0xffU,                          /* clock */
			0xffU,                          /* MISO */
			0xffU,                          /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		}
	},
	/* SPI1 */
	{
		/* SPI1 CS0 */
		{
			MMIO_CFG_spi1_cs0n,             /* chip select */
			MMIO_CFG_spi1_clk,              /* clock */
			MMIO_CFG_spi1_miso,             /* MISO */
			MMIO_CFG_spi1_mosi,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI1 CS1 */
		{
			MMIO_CFG_spi1_cs1n,             /* chip select */
			MMIO_CFG_spi1_clk,              /* clock */
			MMIO_CFG_spi1_miso,             /* MISO */
			MMIO_CFG_spi1_mosi,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI1 CS2 */
		{
			MMIO_CFG_spi1_cs2n,             /* chip select */
			MMIO_CFG_spi1_clk,              /* clock */
			MMIO_CFG_spi1_miso,             /* MISO */
			MMIO_CFG_spi1_mosi,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		}
	}
};
#elif ASIC_TYP==ASIC_TYP_NETX4000_RELAXED

static const MMIO_CFG_T aatMmioValues[2][3][6] =
{
	/* SPI0 */
	{
		/* SPI0 CS0 */
		{
			MMIO_CFG_SPI0_CS0N,             /* chip select */
			MMIO_CFG_SPI0_CLK,              /* clock */
			MMIO_CFG_SPI0_MISO,             /* MISO */
			MMIO_CFG_SPI0_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI0 CS1 */
		{
			MMIO_CFG_SPI0_CS1N,             /* chip select */
			MMIO_CFG_SPI0_CLK,              /* clock */
			MMIO_CFG_SPI0_MISO,             /* MISO */
			MMIO_CFG_SPI0_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI0 CS2 */
		{
			MMIO_CFG_SPI0_CS2N,             /* chip select */
			MMIO_CFG_SPI0_CLK,              /* clock */
			MMIO_CFG_SPI0_MISO,             /* MISO */
			MMIO_CFG_SPI0_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},
	},

	/* SPI1 */
	{
		/* SPI1 CS0 */
		{
			MMIO_CFG_SPI1_CS0N,             /* chip select */
			MMIO_CFG_SPI1_CLK,              /* clock */
			MMIO_CFG_SPI1_MISO,             /* MISO */
			MMIO_CFG_SPI1_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI1 CS1 */
		{
			MMIO_CFG_SPI1_CS1N,             /* chip select */
			MMIO_CFG_SPI1_CLK,              /* clock */
			MMIO_CFG_SPI1_MISO,             /* MISO */
			MMIO_CFG_SPI1_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI1 CS2 */
		{
			MMIO_CFG_SPI1_CS2N,             /* chip select */
			MMIO_CFG_SPI1_CLK,              /* clock */
			MMIO_CFG_SPI1_MISO,             /* MISO */
			MMIO_CFG_SPI1_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		}
	}
};

#elif ASIC_TYP==ASIC_TYP_NETX90_MPW
static const MMIO_CFG_T aatMmioValues[2][3][6] =
{
	/* SPI0 */
	{
		/* SPI0 CS0 */
		{
			MMIO_CFG_SPI0_CS0N,             /* chip select */
			MMIO_CFG_SPI0_CLK,              /* clock */
			MMIO_CFG_SPI0_MISO,             /* MISO */
			MMIO_CFG_SPI0_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI0 CS1 */
		{
			MMIO_CFG_SPI0_CS1N,             /* chip select */
			MMIO_CFG_SPI0_CLK,              /* clock */
			MMIO_CFG_SPI0_MISO,             /* MISO */
			MMIO_CFG_SPI0_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI0 CS2 */
		{
			MMIO_CFG_SPI0_CS2N,             /* chip select */
			MMIO_CFG_SPI0_CLK,              /* clock */
			MMIO_CFG_SPI0_MISO,             /* MISO */
			MMIO_CFG_SPI0_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},
	},

	/* SPI1 */
	{
		/* SPI1 CS0 */
		{
			MMIO_CFG_SPI1_CS0N,             /* chip select */
			MMIO_CFG_SPI1_CLK,              /* clock */
			MMIO_CFG_SPI1_MISO,             /* MISO */
			MMIO_CFG_SPI1_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI1 CS1 */
		{
			MMIO_CFG_SPI1_CS1N,             /* chip select */
			MMIO_CFG_SPI1_CLK,              /* clock */
			MMIO_CFG_SPI1_MISO,             /* MISO */
			MMIO_CFG_SPI1_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		},

		/* SPI1 CS2 */
		{
			MMIO_CFG_SPI1_CS2N,             /* chip select */
			MMIO_CFG_SPI1_CLK,              /* clock */
			MMIO_CFG_SPI1_MISO,             /* MISO */
			MMIO_CFG_SPI1_MOSI,             /* MOSI */
			0xffU,                          /* SIO2 */
			0xffU                           /* SIO3 */
		}
	}
};

#elif ASIC_TYP==ASIC_TYP_NETX90_APP
static const NX90_MMIO_CFG_T aatMmioValues[2][3][6] =
{
	/* SPI1_APP */
	{
		/* SPI0 CS0 */
		{
			NX90_MMIO_CFG_SPI1_APP_CS0N,      /* chip select */
			NX90_MMIO_CFG_SPI1_APP_CLK,       /* clock */
			NX90_MMIO_CFG_SPI1_APP_MISO,      /* MISO */
			NX90_MMIO_CFG_SPI1_APP_MOSI,      /* MOSI */
			0xffU,                            /* SIO2 */
			0xffU                             /* SIO3 */
		},

		/* SPI0 CS1 */
		{
			NX90_MMIO_CFG_SPI1_APP_CS1N,      /* chip select */
			NX90_MMIO_CFG_SPI1_APP_CLK,       /* clock */
			NX90_MMIO_CFG_SPI1_APP_MISO,      /* MISO */
			NX90_MMIO_CFG_SPI1_APP_MOSI,      /* MOSI */
			0xffU,                            /* SIO2 */
			0xffU                             /* SIO3 */
		},

		/* SPI0 CS2 */
		{
			NX90_MMIO_CFG_SPI1_APP_CS2N,      /* chip select */
			NX90_MMIO_CFG_SPI1_APP_CLK,       /* clock */
			NX90_MMIO_CFG_SPI1_APP_MISO,      /* MISO */
			NX90_MMIO_CFG_SPI1_APP_MOSI,      /* MOSI */
			0xffU,                            /* SIO2 */
			0xffU                             /* SIO3 */
		},
	},

	/* SPI_XPIC_APP */
	{
		/* SPI1 CS0 */
		{
			NX90_MMIO_CFG_SPI_XPIC_APP_CS0N,  /* chip select */
			NX90_MMIO_CFG_SPI_XPIC_APP_CLK,   /* clock */
			NX90_MMIO_CFG_SPI_XPIC_APP_MISO,  /* MISO */
			NX90_MMIO_CFG_SPI_XPIC_APP_MOSI,  /* MOSI */
			0xffU,                            /* SIO2 */
			0xffU                             /* SIO3 */
		},

		/* SPI1 CS1 */
		{
			NX90_MMIO_CFG_SPI_XPIC_APP_CS1N,  /* chip select */
			NX90_MMIO_CFG_SPI_XPIC_APP_CLK,   /* clock */
			NX90_MMIO_CFG_SPI_XPIC_APP_MISO,  /* MISO */
			NX90_MMIO_CFG_SPI_XPIC_APP_MOSI,  /* MOSI */
			0xffU,                            /* SIO2 */
			0xffU                             /* SIO3 */
		},

		/* SPI1 CS2 */
		{
			NX90_MMIO_CFG_SPI_XPIC_APP_CS2N,  /* chip select */
			NX90_MMIO_CFG_SPI_XPIC_APP_CLK,   /* clock */
			NX90_MMIO_CFG_SPI_XPIC_APP_MISO,  /* MISO */
			NX90_MMIO_CFG_SPI_XPIC_APP_MOSI,  /* MOSI */
			0xffU,                            /* SIO2 */
			0xffU                             /* SIO3 */
		}
	}
};

#endif


static unsigned char spi_exchange_byte(const SPI_CFG_T *ptCfg __attribute__ ((unused)), unsigned char ucByte)
{
	HOSTADEF(SPI) * ptSpi;
	unsigned long ulValue;


	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);

	/* Send one byte. */
	ptSpi->ulSpi_dr = ucByte;

	/* Wait for one byte in the FIFO. */
	do
	{
		ulValue  = ptSpi->ulSpi_sr;
		ulValue &= HOSTMSK(spi_sr_RNE);
	} while( ulValue==0 );

	/* Get the byte. */
	ucByte = (unsigned char)(ptSpi->ulSpi_dr);
	return ucByte;
}


/*-----------------------------------*/


static unsigned long spi_get_device_speed_representation(unsigned int uiSpeedKHz)
{
	unsigned long ulDevSpeed;
	unsigned long ulInputFilter;


	/* ulSpeed is in kHz */

	/* limit speed to upper border */
	if( uiSpeedKHz>50000 )
	{
		uiSpeedKHz = 50000;
	}

	/* convert speed to "multiply add value" */
	ulDevSpeed  = uiSpeedKHz * 4096;

	/* NOTE: do not round up here */
	ulDevSpeed /= 100000;

	/* use input filtering? */
	ulInputFilter = 0;
	if( ulDevSpeed<=0x0200 )
	{
		ulInputFilter = HOSTMSK(spi_cr0_filter_in);
	}

	/* shift to register position */
	ulDevSpeed <<= HOSTSRT(spi_cr0_sck_muladd);

	/* add filter bit */
	ulDevSpeed |= ulInputFilter;

	return ulDevSpeed;
}



static void spi_slave_select(const SPI_CFG_T *ptCfg, int fIsSelected)
{
	HOSTADEF(SPI) * ptSpi;
	unsigned long uiChipSelect;
	unsigned long ulValue;


	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);

	/* Get the chip select value. */
	uiChipSelect  = 0;
	if( fIsSelected!=0 )
	{
		uiChipSelect  = 1U << (HOSTSRT(spi_cr1_fss) + ptCfg->uiChipSelect);
		uiChipSelect &= HOSTMSK(spi_cr1_fss);
	}

	/* get control register contents */
	ulValue  = ptSpi->aulSpi_cr[1];

	/* mask out the slave select bits */
	ulValue &= ~HOSTMSK(spi_cr1_fss);

	/* mask in the new slave id */
	ulValue |= uiChipSelect;

	/* write back new value */
	ptSpi->aulSpi_cr[1] = ulValue;
}



static int spi_send_idle_cycles(const SPI_CFG_T *ptCfg __attribute__((unused)), unsigned int sizIdleCycles __attribute__((unused)))
{
	/* Idle cycles are not supported with this unit. */
	return -1;
}



static int spi_send_dummy(const SPI_CFG_T *ptCfg, unsigned int sizDummyBytes)
{
	unsigned char ucDummyByte;
	unsigned int sizTxChars;
	unsigned int sizRxChars;
	HOSTADEF(SPI) * ptSpi;
	unsigned long ulValue;
	unsigned long ulTxSpace;
	unsigned long ulRxSpace;
	unsigned int sizChunk;


	/* Get the idle byte. */
	ucDummyByte = ptCfg->ucDummyByte;

	/* Get the number of bytes to send and receive. */
	sizTxChars = sizDummyBytes;
	sizRxChars = sizDummyBytes;

	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);

	do
	{
		/* Are some bytes left to send? */
		if( sizTxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue = ptSpi->ulSpi_sr;
			ulTxSpace   = ulValue;
			ulTxSpace  &= HOSTMSK(spi_sr_tx_fifo_level);
			ulTxSpace >>= HOSTSRT(spi_sr_tx_fifo_level);
			/* The FIFO has 16 entries. Get the number of free entries from the fill level. */
			ulTxSpace = 16 - ulTxSpace;

			ulRxSpace   = ulValue;
			ulRxSpace  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulRxSpace >>= HOSTSRT(spi_sr_rx_fifo_level);
			/* The FIFO has 16 entries, but there might be one byte on it's way. Get the number of free entries from the fill level. */
			ulRxSpace = 15 - ulRxSpace;

			/* Limit the chunk by the number of bytes left in both FIFOs.
			 * It is very important to include the space left in the RX FIFO or it will overflow.
			 */
			sizChunk = sizTxChars;
			if( sizChunk>ulTxSpace )
			{
				sizChunk = ulTxSpace;
			}
			if( sizChunk>ulRxSpace )
			{
				sizChunk = ulRxSpace;
			}

			sizTxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Send one idle byte. */
				ptSpi->ulSpi_dr = ucDummyByte;

				--sizChunk;
			}
		}

		/* Are some bytes left to receive? */
		if( sizRxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue   = ptSpi->ulSpi_sr;
			ulValue  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulValue >>= HOSTSRT(spi_sr_rx_fifo_level);
			
			/* Limit the chunk by the number of bytes to transfer. */
			sizChunk = ulValue;
			if( sizChunk>sizRxChars )
			{
				sizChunk = sizRxChars;
			}

			sizRxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Throw away one byte. */
				ptSpi->ulSpi_dr;

				--sizChunk;
			}

		}
	} while( (sizTxChars | sizRxChars)!=0 );

	/* All OK! */
	return 0;
}



static int spi_send_data(const SPI_CFG_T *ptCfg, const unsigned char *pucData, unsigned int sizData)
{
	unsigned int sizTxChars;
	unsigned int sizRxChars;
	HOSTADEF(SPI) * ptSpi;
	unsigned long ulValue;
	unsigned long ulTxSpace;
	unsigned long ulRxSpace;
	unsigned int sizChunk;


	/* Get the number of bytes to send and receive. */
	sizTxChars = sizData;
	sizRxChars = sizData;

	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);

	do
	{
		/* Are some bytes left to send? */
		if( sizTxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue = ptSpi->ulSpi_sr;
			ulTxSpace   = ulValue;
			ulTxSpace  &= HOSTMSK(spi_sr_tx_fifo_level);
			ulTxSpace >>= HOSTSRT(spi_sr_tx_fifo_level);
			/* The FIFO has 16 entries. Get the number of free entries from the fill level. */
			ulTxSpace = 16 - ulTxSpace;

			ulRxSpace   = ulValue;
			ulRxSpace  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulRxSpace >>= HOSTSRT(spi_sr_rx_fifo_level);
			/* The FIFO has 16 entries, but there might be one byte on it's way. Get the number of free entries from the fill level. */
			ulRxSpace = 15 - ulRxSpace;

			/* Limit the chunk by the number of bytes left in both FIFOs.
			 * It is very important to include the space left in the RX FIFO or it will overflow.
			 */
			sizChunk = sizTxChars;
			if( sizChunk>ulTxSpace )
			{
				sizChunk = ulTxSpace;
			}
			if( sizChunk>ulRxSpace )
			{
				sizChunk = ulRxSpace;
			}

			sizTxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Send one idle byte. */
				ptSpi->ulSpi_dr = *(pucData++);

				--sizChunk;
			}
		}

		/* Are some bytes left to receive? */
		if( sizRxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue   = ptSpi->ulSpi_sr;
			ulValue  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulValue >>= HOSTSRT(spi_sr_rx_fifo_level);
			
			/* Limit the chunk by the number of bytes to transfer. */
			sizChunk = ulValue;
			if( sizChunk>sizRxChars )
			{
				sizChunk = sizRxChars;
			}

			sizRxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Throw away one byte. */
				ptSpi->ulSpi_dr;

				--sizChunk;
			}

		}
	} while( (sizTxChars | sizRxChars)!=0 );

	/* All OK! */
	return 0;
}



static int spi_receive_data(const SPI_CFG_T *ptCfg, unsigned char *pucData, unsigned int sizData)
{
	unsigned char ucDummyByte;
	unsigned char ucReceivedChar;
	unsigned int sizTxChars;
	unsigned int sizRxChars;
	HOSTADEF(SPI) * ptSpi;
	unsigned long ulValue;
	unsigned long ulTxSpace;
	unsigned long ulRxSpace;
	unsigned int sizChunk;


	/* Get the idle byte. */
	ucDummyByte = ptCfg->ucDummyByte;

	/* Get the number of bytes to send and receive. */
	sizTxChars = sizData;
	sizRxChars = sizData;

	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);

	do
	{
		/* Are some bytes left to send? */
		if( sizTxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue = ptSpi->ulSpi_sr;
			ulTxSpace   = ulValue;
			ulTxSpace  &= HOSTMSK(spi_sr_tx_fifo_level);
			ulTxSpace >>= HOSTSRT(spi_sr_tx_fifo_level);
			/* The FIFO has 16 entries. Get the number of free entries from the fill level. */
			ulTxSpace = 16 - ulTxSpace;

			ulRxSpace   = ulValue;
			ulRxSpace  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulRxSpace >>= HOSTSRT(spi_sr_rx_fifo_level);
			/* The FIFO has 16 entries, but there might be one byte on it's way. Get the number of free entries from the fill level. */
			ulRxSpace = 15 - ulRxSpace;

			/* Limit the chunk by the number of bytes left in both FIFOs.
			 * It is very important to include the space left in the RX FIFO or it will overflow.
			 */
			sizChunk = sizTxChars;
			if( sizChunk>ulTxSpace )
			{
				sizChunk = ulTxSpace;
			}
			if( sizChunk>ulRxSpace )
			{
				sizChunk = ulRxSpace;
			}

			sizTxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Send one idle byte. */
				ptSpi->ulSpi_dr = ucDummyByte;

				--sizChunk;
			}
		}

		/* Are some bytes left to receive? */
		if( sizRxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue   = ptSpi->ulSpi_sr;
			ulValue  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulValue >>= HOSTSRT(spi_sr_rx_fifo_level);
			
			/* Limit the chunk by the number of bytes to transfer. */
			sizChunk = ulValue;
			if( sizChunk>sizRxChars )
			{
				sizChunk = sizRxChars;
			}

			sizRxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Get one byte. */
				ucReceivedChar = (unsigned char)(ptSpi->ulSpi_dr);
				*(pucData++) = ucReceivedChar;

				--sizChunk;
			}

		}
	} while( (sizTxChars | sizRxChars)!=0 );

	/* All OK! */
	return 0;
}



static int spi_exchange_data(const SPI_CFG_T *ptCfg, const unsigned char *pucDataSend, unsigned char *pucDataReceive, unsigned int sizData)
{
	unsigned char ucReceivedChar;
	unsigned char *pucDataRx;
	const unsigned char *pucDataTx;
	unsigned int sizTxChars;
	unsigned int sizRxChars;
	HOSTADEF(SPI) * ptSpi;
	unsigned long ulValue;
	unsigned long ulTxSpace;
	unsigned long ulRxSpace;
	unsigned int sizChunk;


	pucDataRx = pucDataReceive;
	pucDataTx = pucDataSend;

	/* Get the number of bytes to send and receive. */
	sizTxChars = sizData;
	sizRxChars = sizData;

	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);

	do
	{
		/* Are some bytes left to send? */
		if( sizTxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue = ptSpi->ulSpi_sr;
			ulTxSpace   = ulValue;
			ulTxSpace  &= HOSTMSK(spi_sr_tx_fifo_level);
			ulTxSpace >>= HOSTSRT(spi_sr_tx_fifo_level);
			/* The FIFO has 16 entries. Get the number of free entries from the fill level. */
			ulTxSpace = 16 - ulTxSpace;

			ulRxSpace   = ulValue;
			ulRxSpace  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulRxSpace >>= HOSTSRT(spi_sr_rx_fifo_level);
			/* The FIFO has 16 entries, but there might be one byte on it's way. Get the number of free entries from the fill level. */
			ulRxSpace = 15 - ulRxSpace;

			/* Limit the chunk by the number of bytes left in both FIFOs.
			 * It is very important to include the space left in the RX FIFO or it will overflow.
			 */
			sizChunk = sizTxChars;
			if( sizChunk>ulTxSpace )
			{
				sizChunk = ulTxSpace;
			}
			if( sizChunk>ulRxSpace )
			{
				sizChunk = ulRxSpace;
			}

			sizTxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Send one idle byte. */
				ptSpi->ulSpi_dr = *(pucDataTx++);

				--sizChunk;
			}
		}

		/* Are some bytes left to receive? */
		if( sizRxChars!=0 )
		{
			/* Get the fill level of the FIFO. */
			ulValue   = ptSpi->ulSpi_sr;
			ulValue  &= HOSTMSK(spi_sr_rx_fifo_level);
			ulValue >>= HOSTSRT(spi_sr_rx_fifo_level);

			/* Limit the chunk by the number of bytes to transfer. */
			sizChunk = ulValue;
			if( sizChunk>sizRxChars )
			{
				sizChunk = sizRxChars;
			}

			sizRxChars -= sizChunk;
			while( sizChunk!=0 )
			{
				/* Get one byte. */
				ucReceivedChar = (unsigned char)(ptSpi->ulSpi_dr);
				*(pucDataRx++) = ucReceivedChar;

				--sizChunk;
			}

		}
	} while( (sizTxChars | sizRxChars)!=0 );

	/* All OK! */
	return 0;
}



static int spi_set_new_speed(const SPI_CFG_T *ptCfg, unsigned long ulDeviceSpecificSpeed)
{
	HOSTADEF(SPI) * ptSpi;
	int iResult;
	unsigned long ulValue;


	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);
	
	/* Expect error. */
	iResult = 1;

	/* All irrelevant bits must be 0. */
	ulValue  = ulDeviceSpecificSpeed;
	ulValue &= ~(HOSTMSK(spi_cr0_sck_muladd)|HOSTMSK(spi_cr0_filter_in));
	if( ulValue!=0 )
	{
//		uprintf("[BootDrvSpi] Invalid new speed: 0x%08x\n", ulDeviceSpecificSpeed);
	}
	else
	{
		/* Mask out all irrelevant bits. */
		ulDeviceSpecificSpeed &= HOSTMSK(spi_cr0_sck_muladd) | HOSTMSK(spi_cr0_filter_in);
		if( ulDeviceSpecificSpeed==0 )
		{
//			uprintf("[BootDrvSpi] Invalid new speed: 0x%08x\n", ulDeviceSpecificSpeed);
		}
		else
		{
//			uprintf("[BootDrvSpi] New Speed: 0x%08x\n", ulDeviceSpecificSpeed);

			ulValue  = ptSpi->aulSpi_cr[0];
			ulValue &= ~(HOSTMSK(spi_cr0_sck_muladd)|HOSTMSK(spi_cr0_filter_in));
			ulValue |= ulDeviceSpecificSpeed;
			ptSpi->aulSpi_cr[0] = ulValue;
			
			/* All OK! */
			iResult = 0;
		}
	}

	return iResult;
}



static void spi_reconfigure_ios(const SPI_CFG_T *ptCfg)
{
#if ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
	/* Set up the port control unit. */
	portcontrol_apply_mmio(ptCfg->aucMmio, ptCfg->ausPortControl, sizeof(ptCfg->aucMmio));

	/* activate the SPI pins */
	boot_spi_activate_mmio(ptCfg, aatMmioValues[ptCfg->uiUnit][ptCfg->uiChipSelect]);
#else
	/* Activate the SPI pins. */
	boot_spi_activate_mmio(ptCfg, aatMmioValues[ptCfg->uiUnit][ptCfg->uiChipSelect]);
#endif
}



static int spi_set_bus_width(SPI_CFG_T *ptCfg __attribute__((unused)), SPI_BUS_WIDTH_T tBusWidth)
{
	int iResult;


	/* This interface supports only one bus width: 1bit. */

	if( tBusWidth==SPI_BUS_WIDTH_1BIT )
	{
//		uprintf("[BootDrvSpi] New bus width: %d\n", tBusWidth);
		iResult = 0;
	}
	else
	{
//		uprintf("[BootDrvSpi] Invalid bus width: %d\n", tBusWidth);
		iResult = -1;
	}

	return iResult;
}


static unsigned long spi_get_device_specific_sqirom_cfg(SPI_CFG_T *ptCfg __attribute__((unused)), unsigned int uiDummyCycles __attribute__((unused)), unsigned int uiAddrBits __attribute__((unused)), unsigned int uiAddressNibbles __attribute__((unused)))
{
	return 0U;
}


static int spi_activate_sqirom(SPI_CFG_T *ptCfg __attribute__((unused)), unsigned long ulSettings __attribute__((unused)))
{
//	uprintf("[BootDrvSpi] Sqi Rom Unsupported\n");
	return -1;
}


static int spi_deactivate_sqirom(SPI_CFG_T *ptCfg __attribute__((unused)))
{
	return 0;
}


/*-------------------------------------------------------------------------*/


static void spi_deactivate(const SPI_CFG_T *ptCfg)
{
	HOSTADEF(SPI) * ptSpi;
	unsigned long ulValue;
	
	
	ptSpi = (HOSTADEF(SPI) *)(ptCfg->pvArea);
	
	/* Deactivate all IRQs. */
	ptSpi->ulSpi_imsc = 0;
	
	/* Clear all pending IRQs. */
	ulValue  = HOSTMSK(spi_icr_RORIC);
	ulValue |= HOSTMSK(spi_icr_RTIC);
	ulValue |= HOSTMSK(spi_icr_RXIC);
	ulValue |= HOSTMSK(spi_icr_TXIC);
	ulValue |= HOSTMSK(spi_icr_rxneic);
	ulValue |= HOSTMSK(spi_icr_rxfic);
	ulValue |= HOSTMSK(spi_icr_txeic);
	ptSpi->ulSpi_icr = ulValue;
	
	/* Deactivate DMAs. */
	ptSpi->ulSpi_dmacr = 0;
	
	/* Deactivate the unit. */
	ptSpi->aulSpi_cr[0] = 0;
	ptSpi->aulSpi_cr[1] = 0;

#if ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
	/* Restore the default settings for the port control unit. */
	portcontrol_restore_mmio(ptCfg->aucMmio, sizeof(ptCfg->aucMmio));

	/* Deactivate the SPI pins. */
	boot_spi_deactivate_mmio(ptCfg, aatMmioValues[ptCfg->uiUnit][ptCfg->uiChipSelect]);
#else
	/* Deactivate the SPI pins. */
	boot_spi_deactivate_mmio(ptCfg, aatMmioValues[ptCfg->uiUnit][ptCfg->uiChipSelect]);
#endif
}



int boot_drv_spi_init(SPI_CFG_T *ptCfg, const BOOT_SPI_CONFIGURATION_T *ptSpiCfg, unsigned int uiSpiUnit, unsigned int uiChipSelect)
{
	HOSTADEF(SPI) * ptSpi;
#if ASIC_TYP==ASIC_TYP_NETX50
	HOSTDEF(ptSpi0Area);
	HOSTDEF(ptSpi1Area);
#elif ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
	HOSTDEF(ptSpiArea);
	HOSTDEF(ptSpiXpic3Area);
#elif ASIC_TYP==ASIC_TYP_NETX90_APP
	HOSTDEF(ptSpi1AppArea);
	HOSTDEF(ptSpiXpicAppArea);
#endif
	unsigned long ulValue;
	int iResult;


	ptSpi = NULL;
#if ASIC_TYP==ASIC_TYP_NETX50
	if( uiSpiUnit==0 )
	{
		ptSpi = ptSpi0Area;
	}
	else if( uiSpiUnit==1 )
	{
		ptSpi = ptSpi1Area;
	}
#elif ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
	if( uiSpiUnit==0 )
	{
		ptSpi = ptSpiArea;
	}
	else if( uiSpiUnit==1 )
	{
		ptSpi = ptSpiXpic3Area;
	}
#elif ASIC_TYP==ASIC_TYP_NETX90_APP
	if( uiSpiUnit==0 )
	{
		ptSpi = ptSpi1AppArea;
	}
	else if( uiSpiUnit==1 )
	{
		ptSpi = ptSpiXpicAppArea;
	}
#endif

	if( ptSpi==NULL )
	{
		/* Error: the unit is invalid! */
//		uprintf("[BootDrvSpi] Invalid unit: %d\n", uiSpiUnit);
		iResult = -1;
	}
	else
	{
		ptCfg->pvArea = ptSpi;
		ptCfg->pvSqiRom = NULL;
		ptCfg->ulSpeedFifoKhz = ptSpiCfg->ulSpeedFifoKhz;   /* Speed for the FIFO mode in kHz */
		ptCfg->ucDummyByte = ptSpiCfg->ucDummyByte;     /* the idle configuration */
		ptCfg->uiIdleConfiguration = 0;
		ptCfg->tMode = ptSpiCfg->ucMode;                /* bus mode */
		ptCfg->uiUnit = uiSpiUnit;                      /* the unit */
		ptCfg->uiChipSelect = uiChipSelect;             /* chip select */

		/* set the function pointers */
		ptCfg->pfnSelect = spi_slave_select;
		ptCfg->pfnExchangeByte = spi_exchange_byte;
		ptCfg->pfnSendIdleCycles = spi_send_idle_cycles;
		ptCfg->pfnSendDummy = spi_send_dummy;
		ptCfg->pfnSendData = spi_send_data;
		ptCfg->pfnReceiveData = spi_receive_data;
		ptCfg->pfnExchangeData = spi_exchange_data;
		ptCfg->pfnSetNewSpeed = spi_set_new_speed;
		ptCfg->pfnGetDeviceSpeedRepresentation = spi_get_device_speed_representation;
		ptCfg->pfnReconfigureIos = spi_reconfigure_ios;
		ptCfg->pfnSetBusWidth = spi_set_bus_width;
		ptCfg->pfnGetDeviceSpecificSqiRomCfg = spi_get_device_specific_sqirom_cfg;
		ptCfg->pfnActivateSqiRom = spi_activate_sqirom;
		ptCfg->pfnDeactivateSqiRom = spi_deactivate_sqirom;
		ptCfg->pfnDeactivate = spi_deactivate;

		/* Copy the MMIO pins. */
		memcpy(ptCfg->aucMmio, ptSpiCfg->aucMmio, sizeof(ptSpiCfg->aucMmio));
#if ASIC_TYP==ASIC_TYP_NETX4000_RELAXED
		/* Set up the port control unit. */
		portcontrol_apply_mmio(ptSpiCfg->aucMmio, ptSpiCfg->ausPortControl, sizeof(ptSpiCfg->aucMmio));
#endif

		/* Do not use IRQs in the boot loader. */
		ptSpi->ulSpi_imsc = 0;
		/* Clear all pending IRQs. */
		ulValue  = HOSTMSK(spi_icr_RORIC);
		ulValue |= HOSTMSK(spi_icr_RTIC);
		ulValue |= HOSTMSK(spi_icr_RXIC);
		ulValue |= HOSTMSK(spi_icr_TXIC);
		ulValue |= HOSTMSK(spi_icr_rxneic);
		ulValue |= HOSTMSK(spi_icr_rxfic);
		ulValue |= HOSTMSK(spi_icr_txeic);
		ptSpi->ulSpi_icr = ulValue;

		/* Do not use DMAs */
		ptSpi->ulSpi_dmacr = 0;

		/* set 8 bits */
		ulValue  = 7 << HOSTSRT(spi_cr0_datasize);
		/* set speed and filter */
		ulValue |= spi_get_device_speed_representation(ptCfg->ulSpeedFifoKhz);
		/* set the clock polarity  */
		if( (ptCfg->tMode==SPI_MODE2) || (ptCfg->tMode==SPI_MODE3) )
		{
			ulValue |= HOSTMSK(spi_cr0_SPO);
		}
		/* set the clock phase     */
		if( (ptCfg->tMode==SPI_MODE1) || (ptCfg->tMode==SPI_MODE3) )
		{
			ulValue |= HOSTMSK(spi_cr0_SPH);
		}
		ptSpi->aulSpi_cr[0] = ulValue;


		/* Manual chip select. */
		ulValue  = HOSTMSK(spi_cr1_fss_static);
		/* enable the interface */
		ulValue |= HOSTMSK(spi_cr1_SSE);
		/* Clear both FIFOs. */
		ulValue |= HOSTMSK(spi_cr1_rx_fifo_clr)|HOSTMSK(spi_cr1_tx_fifo_clr);
		ptSpi->aulSpi_cr[1] = ulValue;

		/* transfer control base is unused in this driver */
		ptCfg->ulTrcBase = 0;

		/* activate the SPI pins */
		boot_spi_activate_mmio(ptCfg, aatMmioValues[uiSpiUnit][uiChipSelect]);

		iResult = 0;
	}

	return iResult;
}


