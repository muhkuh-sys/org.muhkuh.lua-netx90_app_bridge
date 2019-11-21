#ifndef __DPM_H__
#define __DPM_H__


typedef enum APP_BRIDGE_RESULT_ENUM
{
	APP_BRIDGE_RESULT_Ok                                     = 0,
	APP_BRIDGE_RESULT_DpmClocksMaskedOut                     = 1,
	APP_BRIDGE_RESULT_AppClocksMaskedOut                     = 2,
	APP_BRIDGE_RESULT_AppFirmwareExceedsFirstSector          = 3,
	APP_BRIDGE_RESULT_AppFirmwareUpdateNeededWhileAppRunning = 4,
	APP_BRIDGE_RESULT_AppFirmwareUpdateFlashFailed           = 5,
	APP_BRIDGE_RESULT_TransferTimeout                        = 6,

	APP_BRIDGE_RESULT_AppStatusInvalidCommand                = 7,
	APP_BRIDGE_RESULT_AppStatusUnalignedAddress              = 8,
	APP_BRIDGE_RESULT_AppStatusLengthTooLarge                = 9,
	APP_BRIDGE_RESULT_AppStatusCallRunning                   = 10,
	APP_BRIDGE_RESULT_AppStatusIdle                          = 11,
	APP_BRIDGE_RESULT_AppStatusUnknown                       = 12
} APP_BRIDGE_RESULT_T;



APP_BRIDGE_RESULT_T app_bridge_init(void);

APP_BRIDGE_RESULT_T app_bridge_read_register(unsigned long ulAddress, unsigned long *pulValue);
APP_BRIDGE_RESULT_T app_bridge_read_area(unsigned long ulAddress, unsigned long ulLengthInBytes, unsigned char *pucData);
APP_BRIDGE_RESULT_T app_bridge_write_register(unsigned long ulAddress, unsigned long ulValue);
APP_BRIDGE_RESULT_T app_bridge_write_register_unlock(unsigned long ulAddress, unsigned long ulValue);
APP_BRIDGE_RESULT_T app_bridge_write_area(unsigned long ulAddress, unsigned long ulLengthInBytes, const unsigned char *pucData);
APP_BRIDGE_RESULT_T app_bridge_call(unsigned long ulAddress, unsigned long ulR0, unsigned long ulR1, unsigned long ulR2, unsigned long ulR3, unsigned long *pulResult);

#endif  /* __DPM_H__ */
