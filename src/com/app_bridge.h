#ifndef __DPM_H__
#define __DPM_H__


int app_bridge_init(void);

int app_bridge_read_register(unsigned long ulAddress, unsigned long *pulValue);
int app_bridge_write_register(unsigned long ulAddress, unsigned long ulValue);


#endif  /* __DPM_H__ */
