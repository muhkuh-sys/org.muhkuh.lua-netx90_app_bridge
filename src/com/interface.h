#ifndef __INTERFACE_H__
#define __INTERFACE_H__


typedef enum BRIDGE_COMMAND_ENUM
{
	BRIDGE_COMMAND_Initialize    = 0,
	BRIDGE_COMMAND_Identify      = 1,
	BRIDGE_COMMAND_ReadRegister  = 2,
	BRIDGE_COMMAND_ReadArea      = 3,
	BRIDGE_COMMAND_WriteRegister = 4,
	BRIDGE_COMMAND_WriteArea     = 5,
	BRIDGE_COMMAND_Call          = 6
} BRIDGE_COMMAND_T;


typedef struct BRIDGE_COMMAND_IDENTIFY_STRUCT
{
	unsigned long ulVersionMajor;
	unsigned long ulVersionMinor;
	unsigned long ulVersionMicro;
	unsigned long ulLengthInBytes;
	unsigned char aucVersionString[16384];
} BRIDGE_COMMAND_IDENTIFY_T;


typedef struct BRIDGE_COMMAND_READREGISTER_STRUCT
{
	unsigned long ulRegister;
	unsigned long ulValue;
} BRIDGE_COMMAND_READREGISTER_T;


typedef struct BRIDGE_COMMAND_READAREA_STRUCT
{
	unsigned long ulAddress;
	unsigned long ulLengthInBytes;
	unsigned char aucData[16384];
} BRIDGE_COMMAND_READAREA_T;


typedef struct BRIDGE_COMMAND_WRITEREGISTER_STRUCT
{
	unsigned long ulRegister;
	unsigned long ulValue;
} BRIDGE_COMMAND_WRITEREGISTER_T;


typedef struct BRIDGE_COMMAND_WRITEAREA_STRUCT
{
	unsigned long ulAddress;
	unsigned long ulLengthInBytes;
	unsigned char aucData[16384];
} BRIDGE_COMMAND_WRITEAREA_T;


typedef struct BRIDGE_COMMAND_CALL_STRUCT
{
	unsigned long ulAddress;
	unsigned long ulR0;
	unsigned long ulR1;
	unsigned long ulR2;
	unsigned long ulR3;
	unsigned long ulResult;
} BRIDGE_COMMAND_CALL_T;


typedef struct BRIDGE_PARAMETER_STRUCT
{
	BRIDGE_COMMAND_T tCommand;
	union
	{
		BRIDGE_COMMAND_IDENTIFY_T tIdentify;
		BRIDGE_COMMAND_READREGISTER_T tReadRegister;
		BRIDGE_COMMAND_READAREA_T tReadArea;
		BRIDGE_COMMAND_WRITEREGISTER_T tWriteRegister;
		BRIDGE_COMMAND_WRITEAREA_T tWriteArea;
		BRIDGE_COMMAND_CALL_T tCall;
	} uData;
} BRIDGE_PARAMETER_T;


typedef enum TEST_RESULT_ENUM
{
	TEST_RESULT_OK = 0,
	TEST_RESULT_ERROR = 1
} TEST_RESULT_T;


TEST_RESULT_T test(BRIDGE_PARAMETER_T *ptParameter);


#endif  /* __INTERFACE_H__ */
