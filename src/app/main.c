
#include <string.h>

#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"
#include "version.h"

/*-------------------------------------------------------------------------*/


static void mmio_blink_init(TIMER_HANDLE_T *ptTimer)
{
	HOSTDEF(ptMmioCtrlArea);
	HOSTDEF(ptAsicCtrlArea);


	/* Set MMIO4 to output. */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;  /* @suppress("Assignment to itself") */
	ptMmioCtrlArea->aulMmio_cfg[4] = HOSTMMIO(PIO);

	ptMmioCtrlArea->ulMmio_pio_out_line_reset_cfg0 = 1U << 4U;
	ptMmioCtrlArea->ulMmio_pio_oe_line_set_cfg0 = 1U << 4U;

	systime_handle_start_ms(ptTimer, 125U);
}


static void mmio_blink(TIMER_HANDLE_T *ptTimer)
{
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulLedValue;


	if( systime_handle_is_elapsed(ptTimer)!=0 )
	{
		/* Invert MMIO4. */
		ulLedValue  = ptMmioCtrlArea->ulMmio_pio_out_line_cfg0;
		ulLedValue ^= 1U << 4U;
		ptMmioCtrlArea->ulMmio_pio_out_line_cfg0 = ulLedValue;

		/* Restart the timer. */
		systime_handle_start_ms(ptTimer, 125U);
	}
}


void bridge_main(void);
void bridge_main(void)
{
	TIMER_HANDLE_T tTimer;


	systime_init();
	mmio_blink_init(&tTimer);

	while(1)
	{
		mmio_blink(&tTimer);
	};
}
