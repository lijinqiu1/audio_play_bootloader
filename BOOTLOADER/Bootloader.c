#include <stm32f4xx.h>
#include "Bootloader.h"
#include "ff.h"
#include "iap.h"  
#include "oled.h"   
#include "oledfont.h"
#include "main.h"
typedef  void (*fun)(void);				//定义一个函数类型的参数.   
fun AppStart; 

uint8_t Receive_dat_buffer[STM_PAGE_SIZE];			  	//数据接收缓存数组
/****************************
函数名  ：Jump2App
功    能：从Bootloader跳转到用户APP程序地址空间
入口参数：Addr，用户APP的起始执行地址
出口参数：无
返回值  ：无
*****************************/
void Jump2App(uint32_t Addr)
{
	if(((*(volatile uint32_t*)Addr)&0x2FFE0000) == 0x20000000)	//检查栈顶地址是否合法.
	{ 
		AppStart = (fun)(*(volatile uint32_t*)(Addr+4));			//用户代码区第二个字为程序开始地址(复位地址)		
		AppStart();									//跳转到APP.
	}
}

/******************************
函数名  ：FirmwareUpdate
功    能：固件升级函数
入口参数：无
出口参数：无
返回值  ：无
*******************************/
void FirmwareUpdate(void)
{
    FIL file_bin; 
    uint8_t i=0;
    uint8_t res;
    uint16_t br;
    uint16_t readlen;
    uint32_t addrx;
    uint32_t Receive_data=0; //计算接收的总数据数
    uint32_t file_size=0;    //文件size
    
    uint8_t percent=0;       //固件升级的百分比
	char log[40];
	/*查找是否存在要升级的BIN文件*/
//    res = f_open(&file_bin, "RTC.bin", FA_OPEN_EXISTING | FA_READ);
    res = f_open(&file_bin, "0:/UPDATE/UPDATE.bin", FA_OPEN_EXISTING | FA_READ);
    file_size=f_size(&file_bin);    //读取的文件大小Byte
    app_trace_log("读取的文件大小为:%dByte\r\n",file_size);

	if(res!=FR_OK) return;
    addrx=FLASH_APP1_ADDR;
    
	/*执行主要的IAP功能*/
    app_trace_log("开始更新固件...\r\n");
	OLED_Clear();
	OLED_ShowCHinese(8, 0, 0, (uint8_t (*)[32])HzUpdate);
	OLED_ShowCHinese(24, 0, 1, (uint8_t (*)[32])HzUpdate);
    OLED_ShowCHinese(40, 0, 3, (uint8_t (*)[32])HzUpdate);
	while(1)
	{
		/*每次读取2K的数据到内存缓冲区buffer*/
	    res = f_read(&file_bin, Receive_dat_buffer, STM_PAGE_SIZE, (UINT*)&br);
        i++;
        readlen=br;
        Receive_data+=br;   //读取的总字节数
        app_trace_log("第%d次读取的br大小为:%dByte\r\n",i,readlen);
        if (res || br == 0) 
        {
            app_trace_log("总共接收的数据大小为:%dByte\r\n",Receive_data);
			res = f_close(&file_bin);
			if (res != FR_OK)
			{
				app_trace_log("close faild %s %d %d",__FUNCTION__, __LINE__, res);
			}
            break; 
        }
        
        iap_write_appbin(addrx,Receive_dat_buffer,readlen);//将读取的数据写入Flash中
        app_trace_log("第%d次写入的地址为:%d \r\n",i,addrx);
        addrx+=STM_PAGE_SIZE;//偏移2048  512*4=2048
        
        percent=(Receive_data*100)/file_size;   //写入的进度
		sprintf(log,"%d%%",percent);
		OLED_ShowString(8,4,(uint8_t *)log,16);
    }
    HAL_Delay(1500);
//    BEEP_ON();
}
