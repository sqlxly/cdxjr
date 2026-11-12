
#include "string.h"
#include "ff.h"   
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_key.h"
#include "vs1053.h"
#include "flac.h"
#include "modbus_screen.h"
#include <user_set.h>
ALIGN(RT_ALIGN_SIZE)
#define MAX_LEN_FILE_NAME  50    
#define MUSIC_MAX_COUNT  20   //不能大于20  屏要越界
typedef struct 
{
	uint8_t play;            //是否需要播放音乐
	uint8_t replay;          //是否重复播放
	uint8_t status;          //当前状态是否正在播放音乐
	uint8_t number;          //播放music_name_buf里的第几首歌
}mp3_play_type;


static uint8_t music_name_buf[MUSIC_MAX_COUNT][MAX_LEN_FILE_NAME];
static uint8_t databuf[4096+10];	 
static uint8_t mp3_name[20]="/1.mp3";
static mp3_play_type music_play={0,0,0,0};
rt_thread_t mp3_tid=RT_NULL;

void disp_mp3_file()
{
	if(mb_mapping !=NULL)
	{
		for(uint8_t i=0;i<MUSIC_MAX_COUNT;i++)
		{	 
			if(music_name_buf[i][0]!=0)
				snprintf(&mb_mapping->tab_registers[199+10*i],20,"%d.%s",i+1,&music_name_buf[i][0]);
		}
	}
}

uint8_t music_find(uint8_t * path)
{
	  uint8_t music_count=0;
    FRESULT res;
	  DIR DirInfo;
	  FILINFO FilInfo;
    char *fn;   /* This function is assuming non-Unicode cfg. */
	  char *p; 
#if _USE_LFN
    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif
 
    res = f_opendir(&DirInfo,(const char*)path); //??????
    if (res == FR_OK)
    {
        rt_kprintf("\r\n");
        while(1)
        {
            res = f_readdir(&DirInfo, &FilInfo);                   //??????????
            if (res != FR_OK || FilInfo.fname[0] == 0) break;  //???/????,??
 
#if _USE_LFN
            fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
             fn = FilInfo.fname;				  
#endif                                               /* It is a file. */
       if(strstr(fn,"mp3")||strstr(fn,"MP3"))
			 {
				 snprintf(&music_name_buf[music_count][0],MAX_LEN_FILE_NAME-1,"%s",fn);
				 rt_kprintf("name=%s\n",&music_name_buf[music_count][0]);
				 music_count++;
				 if(music_count>=MUSIC_MAX_COUNT)
					music_count =0;
			 }					 					
      }
    }
    return res;
}

uint8_t mp3_play_song(uint8_t *pname)
{	 
 	FIL  fmp3;
    uint16_t br;
	uint8_t res,rval;	  
	  		   
	uint16_t i=0; 
	uint8_t key;  	    
	uint32_t file_len=0;
	uint32_t read_len=0;

	res=f_open(&fmp3,(const TCHAR*)pname,FA_READ);//打开文件	 
	if(res !=0)//打开失败
	{
		stop_play_music();
		return RT_ERROR;
	}
	
	rt_kprintf("play song name=%s\n",pname);
  VS_HD_Reset();
	VS_Soft_Reset();
  music_play.status=1;
//	  VS_Restart_Play();  					//重启播放 
		VS_Set_All();        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 	  
		rval=0; 	

    res=f_typetell(pname);	 	 			//得到文件后缀	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			rt_kprintf("加载PATCH\n");
			VS_Load_Patch((uint16_t*)vs1053b_patch,VS1053B_PATCHLEN);
		}  	
		res=f_open(&fmp3,(const TCHAR*)pname,FA_READ);//打开文件	 
	  file_len = f_size(&fmp3); 
	  br=1;
	  rt_kprintf("time=%d\n",rt_tick_get());
 		if(res ==0)//打开成功.
		{ 					   
			while(read_len<file_len)
			{
				res=f_read(&fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
				read_len+=br;
				for(i=0;i<4096;)
				{
				  if(VS_Send_MusicData(databuf+i,32)==0) //给VS10XX发送音频数据
					{
					  i=i+32;
						if(i>br)
							break;
					}
					rt_thread_mdelay(1); 
				}					
				//rt_kprintf("reading...%d\n",read_len);	
      if(music_play.status==0)
			{
				f_close(&fmp3);
				rt_kprintf("播放停止 file len=%d read len=%d\n",file_len,read_len);		
				return 0xff;
			}				
			}
			f_close(&fmp3);
		}
		else 
		{
		   rt_kprintf("文件打开失败\n");
		} 	//计算器
	 return 0;
}
void mp3play(int argc, char *argv[])
{

    if (argc > 1)
    {
			set_play_music(atoi(argv[1]),atoi(argv[1]));
		}
		else 
		{
			rt_kprintf("usage: mp3play /1.mp3");
		}
}
void set_play_music(uint8_t id,uint8_t replay)
{
	music_play.play=1;
	music_play.replay=replay;
	music_play.status=0;
	music_play.number=id;
}
void stop_play_music()
{
	music_play.play=0;
	music_play.replay=0;
	music_play.status=0;
	music_play.number=0;
}

//  MP3 播放主函数
void mp3_thread_main(void *arg)
{
	user_set_part_type *user_set;
	for(uint8_t i=0;i<MUSIC_MAX_COUNT;i++)
	{	
		rt_memset(&music_name_buf[i][0],0,MAX_LEN_FILE_NAME);
	}
    music_find("/"); 
	get_user_set_point(&user_set);
	set_play_music(user_set->voice.device_open,0);
	while(1)
	{

		rt_thread_mdelay(50);
		if(music_play.play)
		{
			rt_kprintf(" paly music\n");
			music_play.number=(music_play.number>=21)?0:music_play.number;
			if(music_play.number>0)       //MP3音乐
			{
				if(mp3_play_song(&music_name_buf[music_play.number-1][0])!=0xff)
				{
					music_play.status=0;
					music_play.play = (music_play.replay==0)? 0:1;
				}
			}
			else                          //系统默认音
			{
				VS_Sine_Test();
				music_play.status=0;
				music_play.play = (music_play.replay==0)? 0:1;
			}
		}
	}
}
static int play_mange()
{
		
    vs1053_init();
	
	  for(uint8_t i=0;i<MUSIC_MAX_COUNT;i++)
		{	
			rt_memset(&music_name_buf[i][0],0,MAX_LEN_FILE_NAME);
		}
    music_find("/");  
		mp3_tid = rt_thread_create("mp3m", mp3_thread_main, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX-2, 10);
		if (mp3_tid == RT_NULL)
			return RT_ERROR;
    rt_thread_startup(mp3_tid);
    return 0;
	
}

INIT_APP_EXPORT(play_mange);

















