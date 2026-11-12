#include <rtthread.h>
#include <board.h>
#include <dfs_posix.h>

typedef struct
{
	char buf[64];
}filenameType;

extern uint32_t program_select;
extern uint32_t program_version;
filenameType filename_map[8];

const rt_uint16_t sdcard_update_ok[20]     = {'*','*','*','*','*','*','*',0xD5A3,0xCCC5,0xC1B6,0xA1C8,0xC9B3,0xA6B9,'*','*','*','*','*','*','*'};//存储卡正常

const rt_uint16_t sdcard_update_err[20]    = {'*','*','*','*','*','*','*',0xB4CE,0xECBC,0xE2B2,0xBDB5,0xD5A3,0xCCC5,'*','*','*','*','*','*','*'};//存储卡异常	

const rt_uint16_t sdcard_update_err_ex[20] = {'*','*','*','*','*','*','*',0xA1A1,0xA1A1,0xA1A1,0xA1A1,0xA1A1,0xA1A1,'*','*','*','*','*','*','*'};//存储卡异常	

extern rt_int8_t udisk_connected;
	
extern rt_err_t user_page_read(rt_uint32_t ReadAddr, rt_uint8_t *pBuffer, rt_uint16_t NumToRead);
extern rt_err_t user_page_write(rt_uint32_t ReadAddr, rt_uint8_t *pBuffer, rt_uint16_t NumToRead);

void update_file_list_display(const char *path, const char *suffix, rt_uint16_t	*pReg)
{
	rt_uint16_t pos = 0, i = 0, dispList = 0;  
	char str[4];
	DIR *dirp;
	struct dirent *d;
	char *ptr_name = rt_malloc(64);

	if(ptr_name == RT_NULL){
		return;
	}	

  dirp  = opendir((char *)path); //打开一个目录
  if (dirp != RT_NULL){
		while ((d = readdir(dirp)) != RT_NULL){     
			if(strlen(d->d_name) > 4){
				strncpy(str, d->d_name+strlen(d->d_name)-4, 4);			
				if(strncmp(str, suffix, 4) == 0){
					if(pos < 8){
						rt_memset(filename_map[pos].buf, '\0', 64);
						sprintf(filename_map[pos].buf, "%s", d->d_name);
						dispList = pos + 1;
					}else{
						for(i=0; i<7; i++){
							rt_memcpy(filename_map[i].buf, filename_map[i+1].buf, 64);
						}
						rt_memset(filename_map[7].buf, '\0', 64);
						sprintf(filename_map[7].buf, "%s", d->d_name);
						dispList = 8;
					}
					pos++;
				}
			}
		}
		closedir(dirp);
		if(dispList > 0){
			pos = 0;
			while(dispList--){
				rt_memset(ptr_name, '\0', 64);
				sprintf(ptr_name, "%d.%s", pos + 1, filename_map[dispList].buf);
//				rt_kprintf("%s\n", ptr_name);
				rt_memcpy((rt_uint8_t *)(pReg+40+20*pos), ptr_name, 40);
				pos++;
			}
		}
	}
	rt_free(ptr_name);
}

void deal_bkupdate_process(rt_uint16_t	*pReg)
{
	static int turn = 0, page = -1;
	
	if(udisk_connected == RT_TRUE){
		if(page != 0){
			page = 0;
			rt_memset((rt_uint8_t *)pReg, 0x20, 400);						 //清除显示文本
			rt_memcpy((rt_uint8_t *)pReg, sdcard_update_ok, 40); //显示SD卡状态 正常
			update_file_list_display("/", ".bin", pReg);
		}
	}else{
		if(page == 0) rt_memset((rt_uint8_t *)pReg, 0x20, 400);	  //清除显示文本
		turn = (turn + 1) % 16;
		if(turn < 10){
			if(page != 1){
				page = 1;
				rt_memcpy((rt_uint8_t *)pReg, sdcard_update_err, 40); //显示SD卡状态 异常
			}
		}else{
			if(page != 2){
				page = 2;
				rt_memcpy((rt_uint8_t *)pReg, sdcard_update_err_ex, 40); //显示SD卡状态 异常
			}
		}
		return;
	}
}
