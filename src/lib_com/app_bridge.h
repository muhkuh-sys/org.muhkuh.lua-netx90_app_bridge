#ifndef __DPM_H__
#define __DPM_H__


typedef enum APP_BRIDGE_RESULT_ENUM
{
	APP_BRIDGE_RESULT_Ok                                     = 0,
	APP_BRIDGE_RESULT_InvalidDpmAddress                      = 1,
	APP_BRIDGE_RESULT_DpmClocksMaskedOut                     = 2,
	APP_BRIDGE_RESULT_AppClocksMaskedOut                     = 3,
	APP_BRIDGE_RESULT_AppFirmwareExceedsFirstSector          = 4,
	APP_BRIDGE_RESULT_AppFirmwareUpdateNeededWhileAppRunning = 5,
	APP_BRIDGE_RESULT_AppFirmwareUpdateFlashFailed           = 6,
	APP_BRIDGE_RESULT_TransferTimeout                        = 7,

	APP_BRIDGE_RESULT_AppStatusInvalidCommand                = 8,
	APP_BRIDGE_RESULT_AppStatusUnalignedAddress              = 9,
	APP_BRIDGE_RESULT_AppStatusLengthTooLarge                = 10,
	APP_BRIDGE_RESULT_AppStatusCallRunning                   = 11,
	APP_BRIDGE_RESULT_AppStatusIdle                          = 12,
	APP_BRIDGE_RESULT_AppStatusUnknown                       = 13
} APP_BRIDGE_RESULT_T;


typedef struct APP_BRIDGE_VERSION_STRUCT
{
	unsigned long ulVersionMajor;
	unsigned long ulVersionMinor;
	unsigned long ulVersionMicro;
	char          acVersionVcs[16];
} APP_BRIDGE_VERSION_T;


APP_BRIDGE_RESULT_T app_bridge_init(void);

void app_bridge_get_version(APP_BRIDGE_VERSION_T *ptAppBridgeVersion);

APP_BRIDGE_RESULT_T app_bridge_read_register(unsigned long ulAddress, unsigned long *pulValue);
APP_BRIDGE_RESULT_T app_bridge_read_area(unsigned long ulAddress, unsigned long ulLengthInBytes, unsigned char *pucData);
APP_BRIDGE_RESULT_T app_bridge_write_register(unsigned long ulAddress, unsigned long ulValue);
APP_BRIDGE_RESULT_T app_bridge_write_register_unlock(unsigned long ulAddress, unsigned long ulValue);
APP_BRIDGE_RESULT_T app_bridge_write_area(unsigned long ulAddress, unsigned long ulLengthInBytes, const unsigned char *pucData);
APP_BRIDGE_RESULT_T app_bridge_call(unsigned long ulAddress, unsigned long ulR0, unsigned long ulR1, unsigned long ulR2, unsigned long ulR3, unsigned long *pulResult);

APP_BRIDGE_RESULT_T app_bridge_module_hispi_initialize(unsigned long ulNumberOfNetiolDevices);
APP_BRIDGE_RESULT_T app_bridge_module_hispi_readRegister16(unsigned char ucNode, unsigned short usAddress, unsigned short *pusValue);
APP_BRIDGE_RESULT_T app_bridge_module_hispi_writeRegister16(unsigned char ucNode, unsigned short usAddress, unsigned short usValue);


#endif  /* __DPM_H__ */
