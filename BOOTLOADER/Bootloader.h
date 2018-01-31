#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__
#include <stdint.h>
#define STM_PAGE_SIZE		2048			//注意：STM32F103ZET6的FLASH页大小为2K

void Jump2App(uint32_t Addr);    //从Bootloader跳转到用户APP程序地址空间
void FirmwareUpdate(void);  //固件升级函数

#endif
