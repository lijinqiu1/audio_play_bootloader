#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__
#include <stdint.h>
#define STM_PAGE_SIZE		2048			//ע�⣺STM32F103ZET6��FLASHҳ��СΪ2K

void Jump2App(uint32_t Addr);    //��Bootloader��ת���û�APP�����ַ�ռ�
void FirmwareUpdate(void);  //�̼���������

#endif
