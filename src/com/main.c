
#include <string.h>

#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"
#include "uart_standalone.h"
#include "uprintf.h"
#include "version.h"

/*-------------------------------------------------------------------------*/


UART_STANDALONE_DEFINE_GLOBALS


void demo_main(void);
void demo_main(void)
{
	BLINKI_HANDLE_T tBlinkiHandle;


	systime_init();
	uart_standalone_initialize();

	uprintf("\f. *** APP bridge demo by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch all LEDs off. */
	rdy_run_setLEDs(RDYRUN_OFF);

	rdy_run_blinki_init(&tBlinkiHandle, 0x00000055, 0x00000150);
	while(1)
	{
		rdy_run_blinki(&tBlinkiHandle);
	};
}
