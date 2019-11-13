#ifndef __CRC_H__
#define __CRC_H__


extern const unsigned short ausCrc12[256];


unsigned char crc4(unsigned char ucCrc, const unsigned char *pucData, unsigned int sizData);
unsigned short crc12(unsigned short usCrc, const unsigned char *pucData, unsigned int sizData);

#endif  /* __CRC_H__ */
