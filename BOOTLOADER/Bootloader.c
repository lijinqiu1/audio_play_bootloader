#include <stm32f4xx.h>
#include "Bootloader.h"
#include "ff.h"
#include "iap.h"  
#include "oled.h"   
#include "oledfont.h"
#include "main.h"
typedef  void (*fun)(void);				//����һ���������͵Ĳ���.   
fun AppStart; 

uint8_t Receive_dat_buffer[STM_PAGE_SIZE];			  	//���ݽ��ջ�������
/****************************
������  ��Jump2App
��    �ܣ���Bootloader��ת���û�APP�����ַ�ռ�
��ڲ�����Addr���û�APP����ʼִ�е�ַ
���ڲ�������
����ֵ  ����
*****************************/
void Jump2App(uint32_t Addr)
{
	if(((*(volatile uint32_t*)Addr)&0x2FFE0000) == 0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		AppStart = (fun)(*(volatile uint32_t*)(Addr+4));			//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		AppStart();									//��ת��APP.
	}
}

/******************************
������  ��FirmwareUpdate
��    �ܣ��̼���������
��ڲ�������
���ڲ�������
����ֵ  ����
*******************************/
void FirmwareUpdate(void)
{
    FIL file_bin; 
    uint8_t i=0;
    uint8_t res;
    uint16_t br;
    uint16_t readlen;
    uint32_t addrx;
    uint32_t Receive_data=0; //������յ���������
    uint32_t file_size=0;    //�ļ�size
    
    uint8_t percent=0;       //�̼������İٷֱ�
	char log[40];
	/*�����Ƿ����Ҫ������BIN�ļ�*/
//    res = f_open(&file_bin, "RTC.bin", FA_OPEN_EXISTING | FA_READ);
    res = f_open(&file_bin, "0:/UPDATE/UPDATE.bin", FA_OPEN_EXISTING | FA_READ);
    file_size=f_size(&file_bin);    //��ȡ���ļ���СByte
    app_trace_log("��ȡ���ļ���СΪ:%dByte\r\n",file_size);

	if(res!=FR_OK) return;
    addrx=FLASH_APP1_ADDR;
    
	/*ִ����Ҫ��IAP����*/
    app_trace_log("��ʼ���¹̼�...\r\n");
	OLED_Clear();
	OLED_ShowCHinese(8, 0, 0, (uint8_t (*)[32])HzUpdate);
	OLED_ShowCHinese(24, 0, 1, (uint8_t (*)[32])HzUpdate);
    OLED_ShowCHinese(40, 0, 3, (uint8_t (*)[32])HzUpdate);
	while(1)
	{
		/*ÿ�ζ�ȡ2K�����ݵ��ڴ滺����buffer*/
	    res = f_read(&file_bin, Receive_dat_buffer, STM_PAGE_SIZE, (UINT*)&br);
        i++;
        readlen=br;
        Receive_data+=br;   //��ȡ�����ֽ���
        app_trace_log("��%d�ζ�ȡ��br��СΪ:%dByte\r\n",i,readlen);
        if (res || br == 0) 
        {
            app_trace_log("�ܹ����յ����ݴ�СΪ:%dByte\r\n",Receive_data);
			res = f_close(&file_bin);
			if (res != FR_OK)
			{
				app_trace_log("close faild %s %d %d",__FUNCTION__, __LINE__, res);
			}
            break; 
        }
        
        iap_write_appbin(addrx,Receive_dat_buffer,readlen);//����ȡ������д��Flash��
        app_trace_log("��%d��д��ĵ�ַΪ:%d \r\n",i,addrx);
        addrx+=STM_PAGE_SIZE;//ƫ��2048  512*4=2048
        
        percent=(Receive_data*100)/file_size;   //д��Ľ���
		sprintf(log,"%d%%",percent);
		OLED_ShowString(8,4,(uint8_t *)log,16);
    }
    HAL_Delay(1500);
//    BEEP_ON();
}
