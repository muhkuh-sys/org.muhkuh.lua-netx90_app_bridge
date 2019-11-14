#ifndef __APP_BRIDGE_INTERFACE_H__
#define __APP_BRIDGE_INTERFACE_H__

typedef enum APP_COMMAND_ENUM
{
	APP_COMMAND_Identify        = 0,
	APP_COMMAND_ReadRegister32  = 1,
	APP_COMMAND_ReadArea        = 2,
	APP_COMMAND_WriteRegister32 = 3,
	APP_COMMAND_WriteArea       = 4,
	APP_COMMAND_Call            = 5
} APP_COMMAND_T;


typedef enum APP_STATUS_ENUM
{
	APP_STATUS_Ok               = 0,
	APP_STATUS_InvalidCommand   = 1,    /* The command identifier is unknown. */
	APP_STATUS_UnalignedAddress = 2,    /* The address of a register read/write request is not aligned to a 32bit boundary. */
	APP_STATUS_LengthTooLarge   = 3,    /* The length field of an area read/write request exceeds the available buffer space. */
	APP_STATUS_CallRunning      = 4,    /* A call is currently running. */
	APP_STATUS_Idle             = 5
} APP_STATUS_T;


typedef struct APP_REQUEST_IDENTIFY_STRUCT
{
	unsigned long ulVersionMajor;
	unsigned long ulVersionMinor;
	unsigned long ulVersionMicro;
	unsigned long ulLengthInBytes;
	unsigned char aucVersionString[16384];
} APP_REQUEST_IDENTIFY_T;


typedef struct APP_REQUEST_READREGISTER32_STRUCT
{
	unsigned long ulRegister;
	unsigned long ulValue;
} APP_REQUEST_READREGISTER32_T;


typedef struct APP_REQUEST_READAREA_STRUCT
{
	unsigned long ulAddress;
	unsigned long ulLengthInBytes;
	unsigned char aucData[16384];
} APP_REQUEST_READAREA_T;


typedef struct APP_REQUEST_WRITEREGISTER32_STRUCT
{
	unsigned long ulRegister;
	unsigned long ulValue;
} APP_REQUEST_WRITEREGISTER32_T;


typedef struct APP_REQUEST_WRITEAREA_STRUCT
{
	unsigned long ulAddress;
	unsigned long ulLengthInBytes;
	unsigned char aucData[16384];
} APP_REQUEST_WRITEAREA_T;



typedef union APP_REQUEST_CALL_DATA_UNION
{
	unsigned char auc[16384];
	unsigned short aus[16384 / sizeof(unsigned short)];
	unsigned long aul[16384 / sizeof(unsigned long)];
} APP_REQUEST_CALL_DATA_T;



typedef struct APP_REQUEST_CALL_STRUCT
{
	unsigned long ulAddress;
	unsigned long ulR0;
	unsigned long ulR1;
	unsigned long ulR2;
	unsigned long ulR3;
	unsigned long ulResult;
	APP_REQUEST_CALL_DATA_T tData;
} APP_REQUEST_CALL_T;



typedef struct APP_REQUEST_STRUCT
{
	APP_COMMAND_T tCommand;
	APP_STATUS_T tStatus;
	union
	{
		APP_REQUEST_IDENTIFY_T tIdentify;
		APP_REQUEST_READREGISTER32_T tReadRegister32;
		APP_REQUEST_READAREA_T tReadArea;
		APP_REQUEST_WRITEREGISTER32_T tWriteRegister32;
		APP_REQUEST_WRITEAREA_T tWriteArea;
		APP_REQUEST_CALL_T tCall;
	} uData;
} APP_REQUEST_T;


typedef struct APP_BRIDGE_DPM_STRUCT
{
	unsigned char aucMagic[8];
	unsigned long ulRequestCount;
	unsigned long ulResponseCount;
	APP_REQUEST_T tRequest;
} APP_BRIDGE_DPM_T;


#define APP_BRIDGE_MAGIC_STRING "AppBr001"
#define APP_BRIDGE_MAGIC_ARRAY 'A', 'p', 'p', 'B', 'r', '0', '0', '1'


#endif  /* __APP_BRIDGE_INTERFACE_H__ */
