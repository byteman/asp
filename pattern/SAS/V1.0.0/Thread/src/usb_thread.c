#include "includes.h"
//updata file
#include "system_init.h"
#include "net_business_thread.h"
#include "diskio.h"
#include "fattime.h"
#include "ff.h"
#include "ffconf.h"
#include "driver.h"
#include "printf_init.h"
#include "task_def.h"
#include "stm32f4_crc.h"
#include "Keyboard.h"
#include "task_timer.h"
#include "guiwindows.h"
#include "task_timer.h"
#include "Job_USB.h"
#include "softdog.h"
#include "crc.h"

#define  CRC_FILE		"3102AK1.txt"			//crc file
#define  BIN_FILE		"3102A1.bin"

#define  READSIZE		512

static u8 buff[READSIZE+30] = {0};

static FATFS fs;

//erro no define::1-mount or stat;2-open file;3-read crc file or crc length;4-read updata file;
//5-wrong crc;6-more than maxsize;7-erase or store flash
static u8 USB_updata(void)
{
	FRESULT fRes;
	FIL fp;
	FILINFO fno;
	u8 res=0;
	u16 crcRes=0,fileCRC=0;	
	u32 realLen=0,addrCount=0;
	u8 err=0;

	//网络升级时不能优盘升级
	if(Get_NetUpdate_Flag())
		return 0;
	
	memset(&fs,0,sizeof(fs));
	//mount
	if(f_mount(0,&fs)!=FR_OK)
	{
		debug(Debug_Error,"mount fs error!!!\r\n");
		res=1;
		goto out;		//挂载失败
	}
	
	//读取crc文件
	if((err=f_open(&fp,CRC_FILE,FA_OPEN_EXISTING|FA_READ))!=FR_OK)
	{
		debug(Debug_Error,"open crc file error,err no is %d!!!\r\n",err);
		res=2;	//open
		goto out;
	}
	
	if(f_read(&fp,&fileCRC,sizeof(fileCRC),&realLen)!=FR_OK | realLen<sizeof(fileCRC))		//read crc
	{
		debug(Debug_Error,"read crc file error!!!\r\n");
		res=3;		// 读取文件失败	
		goto out;
	}
	
	f_close(&fp);	//close file
	
	
	debug(Debug_Notify,"start to read updata file and copy it!!!\r\n");
	
	//读取升级文件
	if(f_open(&fp,BIN_FILE,FA_OPEN_EXISTING|FA_READ)!=FR_OK)
	{
		debug(Debug_Error,"open updata file error!!!\r\n");
		res=2;		//打开失败;	
		goto out;
	}
		
	if(f_stat(BIN_FILE,&fno))		//get file size 
	{
		debug(Debug_Error,"get file size error!!!\r\n");
		res=1;
		goto out;
	}
	if(fno.fsize>1024*256)
	{
		debug(Debug_Error,"file size is more than maxsize(256Kb)!!!\r\n");
		res=6;
		goto out;
	}
	 
	res = Erase_AppFlash();
	if(res != 0) 
		goto out;
	
	CRC16_ResetDR();
	while(1)
	{
		if((fRes=f_read(&fp,buff,READSIZE,&realLen))!=FR_OK)		
		{
			debug(Debug_Error,"read updata file error,we have read %d bytes,error no is:%d\r\n",addrCount,fRes);
			res=4;
			goto out;		
		}
		
		CRC16(0,buff,realLen);
		feed_dog();	
		if(Flash_APPBak_Store(addrCount,buff,realLen)==0)
		{
			debug(Debug_Error,"store data into flash error,delay 500ms try it again!!!\r\n");
			SysTimeDly(50);
			if(Flash_APPBak_Store(addrCount,buff,realLen)==0)
			{
				debug(Debug_Error,"store data into flash error!!!writen size is:%d...\r\n",addrCount);
				res=7;
				goto out;
			}
		}
		addrCount+=realLen;		//flash address
		if(realLen < READSIZE)
		{
			break;
		}		
	}
	
	crcRes=CRC16(0,NET_Bin_KEY,sizeof(NET_Bin_KEY));	//增加KEY效验
    debug(Debug_Warning,"the file crc is:%x,calculate crc is:%x,and have write %d bytes into flash!\r\n",fileCRC,crcRes,addrCount);
	
	if(crcRes!=fileCRC)
	{
		debug(Debug_Error,"updata file crc caluate error!!!\r\n");
		res=5;
		goto out;
	}
		
	if(Flash_APPInfo_Store(fileCRC,fno.fsize, NETBOOT) == 0)	//store crc and file size
	{
		debug(Debug_Error,"store crc value and file size error!!!\r\n");
		res=7;
		goto out;
	}
	if(Flash_UPFlag()==0)
	{
		debug(Debug_Error,"store updata flag error!!!\r\n");
		res=7;
		goto out;
	}

out:
	f_close(&fp);		//close file
	f_mount(0, NULL); 	//umount fat32
	return res;
}


void USB_Thread(void *arg)
{	
	u8 err,flag=0;
	HANDLE usb_handle=HANDLENULL;
	GuiMsgInfo guiMsg={WM_SHOW,0xf0f0,0};
	
	while(1) 
	{
		if(SysTaskSuspend(SYS_PRIO_SELF)!=SYS_NO_ERR)		//U盘线程开始就挂起，等待升级时由gui线程唤醒
		{
			debug(Debug_Error,"suspend usb thread error!!!\r\n");
		}
		
		if(flag==0)
		{
			if(USB1Job_Init(USBStatus) == 0)
			{
				debug(Debug_Error, "emos usbthread init failed!\r\n");
				guiMsg.lParam = 0;	//初始化失败
				goto out;
			}
			
			SysTimeDly(200);  //延时2s等待底层usb状态机线程运行
			
			usb_handle	= DeviceOpen(BLOCK_USB1,0,&err);
			if(err!=HVL_NO_ERR)		//打开usb设备出错
			{
				debug(Debug_Error,"usb1 status therad error!!!\r\n");
				guiMsg.lParam=0;
				goto out;
			}
			DeviceIoctl(usb_handle,CMD_USB1_REGISTER_FEED_DOG,(u32)feed_dog);

		}
		err=40;
		while(--err)	//每隔1s检测一次u盘连接状态,超时10s
		{
			if(DeviceIoctl(usb_handle,CMD_USB1_READ_STATE,0)==GET_READY_U_DISC)
			{							
				break;
			}
			SysTimeDly(50);
		}		
		
		if(err)
		{
			guiMsg.lParam=10;	//检测到U盘
		}
		else
		{
			guiMsg.lParam=11;	//没有U盘
			goto out;
		}
		guiMsg.pWindow = g_pCurWindow;
		GuiMsgQueuePost(&guiMsg);
		SysTimeDly(50);		//等待GUI线程响应

		//开始升级, 关闭软件看门狗监测
		Set_SoftDog_Changer(TRUE);
		err = USB_updata();
		Set_SoftDog_Changer(FALSE);
		if(err == 0)	//升级成功
		{
			guiMsg.lParam=12;	
			debug(Debug_Notify,"updata system success!\r\n");
		}
		else		//升级失败，出错类型
		{
			debug(Debug_Error,"updata system fail,error no is:%d...\r\n",err);
			guiMsg.lParam=err;	
			SysTimeDly(5);
		}
		
	out:
		flag=1;
		guiMsg.pWindow = g_pCurWindow;
		GuiMsgQueuePost(&guiMsg);
	}
}
