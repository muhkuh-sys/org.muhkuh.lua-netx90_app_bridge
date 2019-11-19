#ifndef __DPM_H__
#define __DPM_H__


int app_bridge_init(void);

int app_bridge_read_register(unsigned long ulAddress, unsigned long *pulValue);
int app_bridge_read_area(unsigned long ulAddress, unsigned long ulLengthInBytes, unsigned char *pucData);
int app_bridge_write_register(unsigned long ulAddress, unsigned long ulValue);
int app_bridge_write_register_unlock(unsigned long ulAddress, unsigned long ulValue);
int app_bridge_write_area(unsigned long ulAddress, unsigned long ulLengthInBytes, const unsigned char *pucData);
int app_bridge_call(unsigned long ulAddress, unsigned long ulR0, unsigned long ulR1, unsigned long ulR2, unsigned long ulR3, unsigned long *pulResult);

#endif  /* __DPM_H__ */
