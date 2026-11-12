#include "oled_gui.h"
#include "drv_key.h"
#include "common.h"
#include "font.h"
#include <stdio.h>
#include <board.h>
#include <netif/ethernetif.h>
#include <netdev.h>

//主状态枚举
typedef enum{
	eMainMenu=0,				//主页
	eSelectMenu=1,			//功能选择菜单
	eTerminalSet=2,     //终端启停用
	eNetworkSet=3,			//联网方式设置
	eIpAndGateSet=4,		//IP网关设置
	eSystemInfo=5,			//系统信息
	eFactoryReset,			//恢复出厂设置询问
	eDeviceId,					//设备IEMI 二维码
	eVersionInfo,				//版本信息
 	eUnkown							//未知状态
}EMainStatus;

EMainStatus  gMainState = eMainMenu;			//全局主状态变量
	
static rt_int8_t glOK = RT_FALSE;					//长按ok键返回上级菜单
static rt_int8_t gOK = RT_FALSE;					//OK确定键
static rt_int8_t gLeft = RT_FALSE;				//左键
static rt_int8_t gRight = RT_FALSE;				//右键

rt_uint16_t blacklight_cnt = 0;						//页面待机模式延时
rt_int8_t isRefreshDisplay = RT_TRUE;			//刷新显示
rt_int8_t gStateData = 0;									//状态的数据
rt_int8_t gStateDataEx = 0;								//状态的数据

rt_int8_t  updateDisplay = RT_TRUE; 			//是否刷新显示
rt_int8_t  isClearDisplay = RT_FALSE; 		//清屏开关
rt_int8_t  isInputNumStatus = RT_FALSE; 	//是否在数字输入状态
rt_uint8_t focu_pos_num = 0;   					  //光标位置显示的数字或符号
rt_int8_t	 cursorMark = 0;								//光标闪烁状态

void staticMainMenu(void);
void main_menu_refresh_display(rt_int8_t refresh);
void refreshNetworkSet(TNetworInfo *info, rt_int8_t pos, rt_uint8_t flag);
void display_one_terminal_status(rt_uint8_t page, rt_uint8_t index);
void displaySelectNumber(rt_uint8_t gStateData, rt_uint8_t flag, TNetworInfo *info);

/* MQTT服务连接状态 定义在mqtt_client_proc.c文件中 */
extern int is_online;

/* 网线连接状态 定义在drv_eth.c文件中 */
extern rt_int8_t rj45_status;

/* 网络信号强度 定义在at_device_air720.c文件中 */
extern int rssi; 

void hmi_interface_init(void)
{
	ugui_init();
	
	ugui_display_on(RT_TRUE);
	
	ugui_draw_picture(0, 0, 128, 64, (rt_uint8_t *)&logo_bmp[0], NORMAL);
	
	rt_thread_mdelay(2000);
	
	key_scan_init();
}

void processKeyMsg(rt_ubase_t key)
{
	switch(key) 
	{
		case TAG_KEY_ACTION: 		gOK = RT_TRUE; break;
		case LEFT_KEY_ACTION:		gLeft = RT_TRUE; break;
		case RIGHT_KEY_ACTION: 	gRight = RT_TRUE; break;
		case QUIT_KEY_ACTION: 	glOK = RT_TRUE; break;
		default: break;
	}
}

void erasureKeyPlus(rt_ubase_t key)
{
	switch(key) 
	{
		case TAG_KEY_ACTION: 		gOK = RT_FALSE; break;
		case LEFT_KEY_ACTION:		gLeft = RT_FALSE; break;
		case RIGHT_KEY_ACTION: 	gRight = RT_FALSE; break;
		default: break;
	}
}

//更新光标位置 //左右键
void updateMenuSel_x(rt_uint8_t len)
{
	if(gLeft){//左键
		gStateData = (gStateData+len-1)%len;
	}else if(gRight){//右键
		gStateData = (gStateData+1)%len;
	}
}

void staticMainMenu(void)
{
	main_menu_refresh_display(RT_TRUE);

	if(getTNetworInfo_net_mode() == 1){
		ugui_printf_string(1,  LINE_01, "4G网络", &Font16, NORMAL);
	}else{
		ugui_printf_string(1,  LINE_01, "有线网络", &Font16, NORMAL);
	}
	ugui_printf_string(1,  LINE_02, "是否在线", &Font16, NORMAL);
	ugui_printf_string(1,  LINE_03, "终端数", &Font16, NORMAL);
	/* 最后一行显示报警信息 */
}

void staticSelectMenu(void)
{
	ugui_printf_string(17,  LINE_01, "1.终端启停用", &Font16, NORMAL);
	ugui_printf_string(17,  LINE_02, "2.网络设置", &Font16, NORMAL);	
	ugui_printf_string(17,  LINE_03, "3.系统信息", &Font16, NORMAL);	
	ugui_printf_string(17,  LINE_04, "退出", &Font16, NORMAL);
	
	ugui_draw_picture(0, gStateData*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
}

void staticTerminalSet(void)
{
	display_one_terminal_status(LINE_01, 0);
	display_one_terminal_status(LINE_02, 1);
	display_one_terminal_status(LINE_03, 2);
	display_one_terminal_status(LINE_04, 3);
	
	ugui_draw_picture(0, LINE_01, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
}

void staticNetworkSet(void)
{
	TNetworInfo *info;
	
	info = getTNetworInfo();
	ugui_printf_string(1,  LINE_01, "联网方式", &Font16, NORMAL);
	if(info->netMode == 1){
		ugui_printf_string(88,  LINE_01, " 4G ", &Font16, NORMAL);
	}else{
		ugui_printf_string(88,  LINE_01, "有线", &Font16, NORMAL);
	}
	
	ugui_printf_string(1,  LINE_02, "IP模式", &Font16, NORMAL);
	if(info->ipMode == 1){
		ugui_printf_string(88,  LINE_02, "手动", &Font16, NORMAL);
	}else{
		ugui_printf_string(88,  LINE_02, "自动", &Font16, NORMAL);
	}
	
	if((info->netMode == 1)||(info->ipMode == 0)){		//自动IP模式或者4G网络时 只能查看系统自动获取到的IP信息
		ugui_printf_string(1,  LINE_03, "查看IP信息", &Font16, NORMAL);
	}else{
		ugui_printf_string(1,  LINE_03, "有线IP设置", &Font16, NORMAL);
	}
	
	ugui_printf_string(88,  LINE_03, "-->", &Font16, NORMAL);
	
	ugui_printf_string(1,  LINE_04, "退出", &Font16, NORMAL);
	
	refreshNetworkSet(info, gStateData, ANTI);
}

void staticIpAndGateSetPage(rt_uint8_t gStateData)
{
	struct netdev *netdev = RT_NULL;
	TNetworInfo *info, tInfo;
	uint32_t add_t = 0;
	uint8_t *ipByte = 0;
	char buf[32];
	
	if(getTNetworInfo_net_mode() == 1){ //4G模式
		tInfo.netMode = 1;
		netdev = netdev_get_by_name("air720");
	}else{															//有线网络
		tInfo.netMode = 0;
		netdev = netdev_get_by_name("e0");
	}
	add_t = netdev->ip_addr.addr;
	ipByte = (uint8_t *)&add_t;
	tInfo.ip[0] = *ipByte;
	tInfo.ip[1] = *(ipByte+1);
	tInfo.ip[2] = *(ipByte+2);
	tInfo.ip[3] = *(ipByte+3);	
	
	add_t = netdev->netmask.addr;
	ipByte = (uint8_t *)&add_t;
	tInfo.nm[0] = *ipByte;
	tInfo.nm[1] = *(ipByte+1);
	tInfo.nm[2] = *(ipByte+2);
	tInfo.nm[3] = *(ipByte+3);

	add_t = netdev->gw.addr;		
	ipByte = (uint8_t *)&add_t;
	tInfo.gw[0] = *ipByte;
	tInfo.gw[1] = *(ipByte+1);
	tInfo.gw[2] = *(ipByte+2);
	tInfo.gw[3] = *(ipByte+3);
	
	tInfo.ipMode = getTNetworInfo_ip_mode();
	setNetworkDynamicInfo(tInfo);
		
	info = getTNetworInfo();
	if(gStateData < 8){
		ugui_printf_string(0,  LINE_01, "IP地址", &Font16, NORMAL);
		sprintf(buf, "%3d.%3d.%3d.%3d", info->ip[0], info->ip[1], info->ip[2], info->ip[3]);
		ugui_printf_string(0,  LINE_02, buf, &Font16, NORMAL);
		
		ugui_printf_string(0,  LINE_03, "子网掩码", &Font16, NORMAL);
		sprintf(buf, "%3d.%3d.%3d.%3d", info->nm[0], info->nm[1], info->nm[2], info->nm[3]);
		ugui_printf_string(0,  LINE_04, buf, &Font16, NORMAL);

		displaySelectNumber(gStateData, ANTI, info);
	}else{
		ugui_printf_string(1,  LINE_01, "网关", &Font16, NORMAL);
		sprintf(buf, "%3d.%3d.%3d.%3d", info->gw[0], info->gw[1], info->gw[2], info->gw[3]);
		ugui_printf_string(0,  LINE_02, buf, &Font16, NORMAL);
		
		ugui_printf_string(1,  LINE_04, "退出", &Font16, NORMAL);	
		
		displaySelectNumber(gStateData, ANTI, info);
	}
}

void staticSystemInfo(void)
{
	ugui_printf_string(16,  LINE_01, "1.恢复默认设置", &Font16, NORMAL);
	ugui_printf_string(16,  LINE_02, "2.设备IEMI", &Font16, NORMAL);
	ugui_printf_string(16,  LINE_03, "3.程序版本", &Font16, NORMAL);
	ugui_printf_string(16,  LINE_04, "退出", &Font16, NORMAL);
	
	ugui_draw_picture(0, gStateData*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
}

void staticFactoryReset(void)
{
	ugui_printf_string(1,   LINE_02, "恢复默认设置?", &Font16, NORMAL);
	ugui_printf_string(1,  	LINE_04, "是", &Font16, ANTI);
	ugui_printf_string(111, LINE_04, "否", &Font16, NORMAL);
}

void staticDeviceId(void)
{
	char buf[10];
	
	ugui_draw_picture(1, LINE_01, 58, 64, (rt_uint8_t *)qrcodeData, NORMAL);
	
	rt_memcpy(buf, &stm32_uniqueId[0], 8);
	buf[8] = '\0';
	ugui_printf_string(64,  LINE_01, buf, &Font16, NORMAL);

	rt_memcpy(buf, &stm32_uniqueId[8], 8);
	buf[8] = '\0';
	ugui_printf_string(64,  LINE_02, buf, &Font16, NORMAL);
	
	rt_memcpy(buf, &stm32_uniqueId[16], 8);
	buf[8] = '\0';
	ugui_printf_string(64,  LINE_03, buf, &Font16, NORMAL);
	
	rt_memcpy(buf, &stm32_uniqueId[24], 8);
	buf[8] = '\0';
	ugui_printf_string(64,  LINE_04, buf, &Font16, NORMAL);
}

void staticVersionInfo(void)
{
	ugui_printf_string(1,  LINE_02, "版本:", &Font16, NORMAL);
	ugui_printf_string(42,  LINE_02, VERSION, &Font16, NORMAL);
	ugui_printf_string(1,  LINE_04, "日期:", &Font16, NORMAL);
	ugui_printf_string(42,  LINE_04, DATETIME, &Font16, NORMAL);
}

static rt_uint8_t alarmQueue[ALARM_TYPE_LEN];

void _roll_display_alarm_info(void)
{
	char alarmStr[32];
	rt_uint8_t i, turn = 0, online = 0, total = 0;
	static rt_uint8_t showAlarmPos = 0;
		
	rt_memset(alarmQueue, 0, ALARM_TYPE_LEN);
	for(i=0; i<20; i++){
		if(getModbusCardStatus(i) == 1){
			if(alarmMark1[i] == 2){
				alarmQueue[turn] = i+1;
				turn++;
			}else if(alarmMark1[i] == 1){
				online++;
			}
			total++;
		}
	}
	for(i=0; i<20; i++){
		if(getModbusCardStatus(20+i) == 1){
			if(alarmMark2[i] == 2){
				alarmQueue[turn] = 21+i;
				turn++;
			}else if(alarmMark2[i] == 1){
				online++;
			}
			total++;
		}
	}
	for(i=0; i<20; i++){
		if(getModbusCardStatus(40+i) == 1){
			if(alarmMark3[i] == 2){ 
				alarmQueue[turn] = 41+i;
				turn++;
			}else if(alarmMark3[i] == 1){
				online++;
			}
			total++;
		}
	}

	/* 显示在线终端数量 */
	sprintf(alarmStr, "%d/%d", online, total);
	ugui_printf_string(80,  LINE_03, alarmStr, &Font16, NORMAL);
	
	if(turn){
		showAlarmPos = (showAlarmPos + 1) % turn;
		rt_memset(alarmStr, '\0', 10);
		sprintf(alarmStr, "终端%02d通信失败  ", alarmQueue[showAlarmPos]);
		ugui_printf_string(1,  LINE_04, alarmStr, &Font16, ANTI);
	} else { //正常
		showAlarmPos = 0;
		if(total == 0){
			ugui_printf_string(1,  LINE_04, "终端未启用      ", &Font16, ANTI);
		}else{
			ugui_printf_string(1,  LINE_04, "终端通信正常    ", &Font16, ANTI);
		}
	}
}

void display4GModeRssi(rt_int8_t rssi)
{
	char buf[10];
	
	if((rssi < 2)||(rssi > 31)){
		ugui_printf_string(80,  LINE_01, "无信号", &Font16, NORMAL);
	}else{
		sprintf(buf, "%2ddb  ", rssi);
		ugui_printf_string(80,  LINE_01, buf, &Font16, NORMAL);
	}
}

void main_menu_refresh_display(rt_int8_t refresh)
{
	static rt_int8_t showAlarmCnt = 0;
	
	if(refresh){
		showAlarmCnt = 8;
	}
	showAlarmCnt++;
	if(showAlarmCnt > 8){
		showAlarmCnt = 0;

		/* 显示信号强度 SIM卡状态 有线网连接状态 */
		if(getTNetworInfo_net_mode() == 1){
			display4GModeRssi(rssi);
		}else{
			if(rj45_status == 1){
				ugui_printf_string(80,  LINE_01, "连接  ", &Font16, NORMAL);
			}else{
				ugui_printf_string(80,  LINE_01, "未连接", &Font16, NORMAL);
			}
		}

		/* 显示MQTT状态 */
		if(is_online == 1){
			ugui_printf_string(80,  LINE_02, "在线", &Font16, NORMAL);
		}else{
			ugui_printf_string(80,  LINE_02, "离线", &Font16, NORMAL);
		}
		
		/* 添加滚动显示报警信息 在线数量也在此函数中显示 */
		_roll_display_alarm_info();
	}
	
}

void keyMainMenu(void)
{
	main_menu_refresh_display(RT_FALSE);
	
	/* 处理按键逻辑 */
	if(gOK){
		gStateData = 0;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
		gMainState = eSelectMenu;
	}
}

void keySelectMenu(void)
{
	if(gLeft||gRight){
		if(gStateData < 3) ugui_fill_area(0, gStateData*2, 16, 16, NORMAL);
		else ugui_printf_string(17,  LINE_04, "退出", &Font16, NORMAL);
	}
	updateMenuSel_x(4);
	if(gLeft||gRight){
		if(gStateData < 3) ugui_draw_picture(0, gStateData*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
		else ugui_printf_string(17,  LINE_04, "退出", &Font16, ANTI);
	}
	if(gOK){
		switch(gStateData){
			case 0: gMainState = eTerminalSet; break;
			case 1: gMainState = eNetworkSet;  break;
			case 2: gMainState = eSystemInfo;  break;
			case 3: gMainState = eMainMenu;    break;
			default: break;
		}
		gStateData = 0;
		gStateDataEx = 0;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
	}
}

void display_one_terminal_status(rt_uint8_t page, rt_uint8_t index)
{
	rt_uint8_t sta = 0;
	char buf[16];
	
	sprintf(buf, "终端%02d", index + 1);
	ugui_printf_string(17,  page, buf, &Font16, NORMAL);
	
	sta = getModbusCardStatus(index);
	if(sta == 1) sprintf(buf, "启用");
	else sprintf(buf, "停用");
	ugui_printf_string(88,  page, buf, &Font16, NORMAL);
}

void display_terminal_exit(rt_uint8_t flag)
{
	ugui_printf_string(17,  LINE_04, "退出", &Font16, flag);
	ugui_fill_area(49, LINE_04, 16, 16, NORMAL);
	ugui_fill_area(88, LINE_04, 32, 16, NORMAL);
}

void keyTerminalSet(void)
{
	char buf[10];
	
	if(isInputNumStatus == RT_FALSE){
		if(gLeft||gRight){
			if(gStateData == 60) display_terminal_exit(NORMAL);
			else ugui_fill_area(0, gStateDataEx*2, 16, 16, NORMAL);
		}
		
		if(gLeft){
			if(gStateData == 0){
				ugui_fill_area(0, gStateDataEx*2, 16, 16, NORMAL);
				display_one_terminal_status(LINE_01, 57);
				display_one_terminal_status(LINE_02, 58);
				display_one_terminal_status(LINE_03, 59);
				display_terminal_exit(ANTI);
				gStateDataEx = 3;
			}else{
				if(gStateDataEx > 0){
					gStateDataEx = gStateDataEx - 1;
				}else{
					display_one_terminal_status(LINE_01, gStateData-1);
					display_one_terminal_status(LINE_02, gStateData);
					display_one_terminal_status(LINE_03, gStateData+1);
					display_one_terminal_status(LINE_04, gStateData+2);						
				}
				ugui_draw_picture(0, gStateDataEx*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
			}
		}
		if(gRight){
			if(gStateData == 59){
				ugui_fill_area(0, gStateDataEx*2, 16, 16, NORMAL);
				display_one_terminal_status(LINE_01, 57);
				display_one_terminal_status(LINE_02, 58);
				display_one_terminal_status(LINE_03, 59);
				display_terminal_exit(ANTI);
				gStateDataEx = 3;
			}else if(gStateData == 60){
				display_one_terminal_status(LINE_01, 0);
				display_one_terminal_status(LINE_02, 1);
				display_one_terminal_status(LINE_03, 2);
				display_one_terminal_status(LINE_04, 3);	
				gStateDataEx = 0;
				ugui_draw_picture(0, gStateDataEx*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
			}else{
				if(gStateDataEx < 3){
					gStateDataEx = gStateDataEx + 1;
				}else{
					display_one_terminal_status(LINE_01, gStateData-2);
					display_one_terminal_status(LINE_02, gStateData-1);
					display_one_terminal_status(LINE_03, gStateData);
					display_one_terminal_status(LINE_04, gStateData+1);
				}
				ugui_draw_picture(0, gStateDataEx*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
			}
		}
		updateMenuSel_x(61);
		if(gOK){
			if(gStateData == 60){		//退出
				gStateData = 0;
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gMainState = eSelectMenu;
			}else{									//切换到输入状态
				isInputNumStatus = RT_TRUE;
				focu_pos_num = getModbusCardStatus(gStateData);
			}
		}
		if(glOK){
			gStateData = 0;
			isClearDisplay = RT_TRUE;
			updateDisplay = RT_TRUE;	
			gMainState = eSelectMenu;
		}
	}else{
		cursorMark = (cursorMark + 1) % 2;
		if(gRight||gLeft){
			focu_pos_num = (focu_pos_num+1)%2;
			cursorMark = 1;
		}
		if(gOK||glOK){
			isInputNumStatus = RT_FALSE;
			cursorMark = 0;
			setModbusCardStatus(gStateData, focu_pos_num);
		}
		if(focu_pos_num == 1) sprintf(buf, "启用");
		else sprintf(buf, "停用");
		if(cursorMark){
			ugui_printf_string(88,  gStateDataEx*2, buf, &Font16, ANTI);
		}else{
			ugui_printf_string(88,  gStateDataEx*2, buf, &Font16, NORMAL);
		}
	}
}

void refreshNetworkSet(TNetworInfo *info, rt_int8_t pos, rt_uint8_t flag)
{
	switch(pos){
		case 0:
			if(info->netMode == 1){
				ugui_printf_string(88,  LINE_01, " 4G ", &Font16, flag);
			}else{
				ugui_printf_string(88,  LINE_01, "有线", &Font16, flag);
			}
			break;
		case 1:
			if(info->ipMode == 1){
				ugui_printf_string(88,  LINE_02, "手动", &Font16, flag);
			}else{
				ugui_printf_string(88,  LINE_02, "自动", &Font16, flag);
			}
			break;
		case 2:
			ugui_printf_string(88,  LINE_03, "-->", &Font16, flag);
			break;
		case 3:
			ugui_printf_string(1,  LINE_04, "退出", &Font16, flag);
			break;
		default: break;
	}
}

void keyNetworkSet(void)
{
	TNetworInfo *netInfo;
	rt_uint8_t flag;
	
	netInfo = getTNetworInfo();
	if(isInputNumStatus == RT_FALSE){
		if(gRight||gLeft){
			refreshNetworkSet(netInfo, gStateData, NORMAL);
		}
		updateMenuSel_x(4);
		if(gRight||gLeft){
			refreshNetworkSet(netInfo, gStateData, ANTI);
		}
		if(gOK){
			switch(gStateData){
				case 0:
					cursorMark = 0;
					isInputNumStatus = RT_TRUE;
					focu_pos_num = netInfo->netMode;
					break;
				case 1:
					cursorMark = 0;
					isInputNumStatus = RT_TRUE;
					focu_pos_num = netInfo->ipMode;
					break;
				case 2:
					gStateData = 0;
					gStateDataEx = 0;
					isClearDisplay = RT_TRUE;
					updateDisplay = RT_TRUE;
					gMainState = eIpAndGateSet;
					break;
				case 3:
					gStateData = 1;
					isClearDisplay = RT_TRUE;
					updateDisplay = RT_TRUE;
					gMainState = eSelectMenu;
					break;
				default: break;
			}
		}
	}else{
		cursorMark = (cursorMark + 1) % 2;
		if(gRight||gLeft){
			focu_pos_num = (focu_pos_num+1)%2;
			cursorMark = 1;
		}
		if(gOK||glOK){
			isInputNumStatus = RT_FALSE;
			cursorMark = 1;
			if(gStateData == 0){
				setNetworkMode(focu_pos_num);
			}else if(gStateData == 1){
				setGetIpMode(focu_pos_num);
			}
			if((netInfo->netMode == 1)||(netInfo->ipMode == 0)){		//自动IP模式或者4G网络时 只能查看系统自动获取到的IP信息
				ugui_printf_string(1,  LINE_03, "查看IP信息", &Font16, NORMAL);
			}else{
				ugui_printf_string(1,  LINE_03, "有线IP设置", &Font16, NORMAL);
			}
		}
		
		if(cursorMark){
			flag = ANTI;
		}else{
			flag = NORMAL;
		}
		
		if(gStateData == 0){
			if(focu_pos_num == 1){
				ugui_printf_string(88,  LINE_01, " 4G ", &Font16, flag);
			}else{
				ugui_printf_string(88,  LINE_01, "有线", &Font16, flag);
			}
		}else if(gStateData == 1){
			if(focu_pos_num == 1){
				ugui_printf_string(88,  LINE_02, "手动", &Font16, flag);
			}else{
				ugui_printf_string(88,  LINE_02, "自动", &Font16, flag);
			}
		}
	}
}

void displaySelectNumber(rt_uint8_t gStateData, rt_uint8_t flag, TNetworInfo *info)
{
	char buf[8];
	rt_uint8_t pos = 0;
	
	pos = gStateData % 4;
	if(gStateData < 4){																		//IP
		sprintf(buf, "%3d", info->ip[pos]);
		if(flag == ANTI) focu_pos_num = info->ip[pos];
		ugui_printf_string(32*pos,  LINE_02, buf, &Font16, flag);
	}else if((gStateData >= 4)&&(gStateData < 8)){				//掩码
		sprintf(buf, "%3d", info->nm[pos]);
		if(flag == ANTI) focu_pos_num = info->nm[pos];
		ugui_printf_string(32*pos,  LINE_04, buf, &Font16, flag);	
	}else if((gStateData >= 8)&&(gStateData < 12)){				//网关
		sprintf(buf, "%3d", info->gw[pos]);
		if(flag == ANTI) focu_pos_num = info->gw[pos];
		ugui_printf_string(32*pos,  LINE_02, buf, &Font16, flag);
	}else if(gStateData == 12){														//退出
		ugui_printf_string(1,  LINE_04, "退出", &Font16, flag);	
		focu_pos_num = 0;
	}
}

void displayInputNumber(rt_uint8_t gStateData, rt_uint8_t flag, int num)
{
	char buf[8];
	rt_uint8_t pos = 0;
	
	pos = gStateData % 4;
	sprintf(buf, "%3d", num);
	if(gStateData < 4){																		//IP
		ugui_printf_string(32*pos,  LINE_02, buf, &Font16, flag);
	}else if((gStateData >= 4)&&(gStateData < 8)){				//掩码
		ugui_printf_string(32*pos,  LINE_04, buf, &Font16, flag);	
	}else if((gStateData >= 8)&&(gStateData < 12)){				//网关
		ugui_printf_string(32*pos,  LINE_02, buf, &Font16, flag);
	}
}

void keyIpAndGateSet(void)
{
	TNetworInfo *info;
	
	info = getTNetworInfo();
	
	if(isInputNumStatus == RT_FALSE){
		if(gLeft){
			if(gStateData == 0){  //选中退出按钮
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gStateData = (gStateData+13-1)%13;
			}else if(gStateData == 8){
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gStateData = (gStateData+13-1)%13;				
			}else{
				displaySelectNumber(gStateData, NORMAL, info);
				gStateData = (gStateData+13-1)%13;
				displaySelectNumber(gStateData, ANTI, info);
			}
		}else if(gRight){//右键
			if(gStateData == 7){
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;			
				gStateData = (gStateData+1)%13;
			}else if(gStateData == 12){
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gStateData = (gStateData+1)%13;
			}else{
				displaySelectNumber(gStateData, NORMAL, info);
				gStateData = (gStateData+1)%13;
				displaySelectNumber(gStateData, ANTI, info);
			}
		}

		if(gOK){
			if(gStateData == 12){
				gStateData = 2;
				cursorMark = 0;
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gMainState = eNetworkSet;
			}else{
				/* 使用有线网络 并且手动设置IP模式才允许编辑 否则只能查看系统自动获取的IP */
				if((info->ipMode == 1)&&(info->netMode == 0)){
					cursorMark = 0;
					isInputNumStatus = RT_TRUE;
				}
			}
		}
	}else{
		cursorMark = (cursorMark + 1) % 2;
		if(gLeft){//左键
			focu_pos_num = (focu_pos_num+256-1)%256;
			cursorMark = 1;
		}else if(gRight){//右键
			focu_pos_num = (focu_pos_num+1)%256;
			cursorMark = 1;
		}
		if(gOK||glOK){
			if(gStateData < 4){																		//IP
				setIpAddress(gStateData%4, focu_pos_num);			
			}else if((gStateData >= 4)&&(gStateData < 8)){				//掩码
				setNmAddress(gStateData%4, focu_pos_num);
			}else if((gStateData >= 8)&&(gStateData < 12)){				//网关
				setGwAddress(gStateData%4, focu_pos_num);	
			}
			isInputNumStatus = RT_FALSE;
			cursorMark = 1;
		}
		if(cursorMark){
			displayInputNumber(gStateData, ANTI, focu_pos_num);
		}else{
			displayInputNumber(gStateData, NORMAL, focu_pos_num);
		}
	}
}

void keySystemInfo(void)
{
	if(gLeft||gRight){
		if(gStateData < 3) ugui_fill_area(0, gStateData*2, 16, 16, NORMAL);
		else ugui_printf_string(17,  LINE_04, "退出", &Font16, NORMAL);
	}
	updateMenuSel_x(4);
	if(gLeft||gRight){
		if(gStateData < 3) ugui_draw_picture(0, gStateData*2, 16, 16, (rt_uint8_t *)&bmp_cursor[0], NORMAL);
		else ugui_printf_string(17,  LINE_04, "退出", &Font16, ANTI);
	}
	if(gOK){
		switch(gStateData){
			case 0: gMainState = eFactoryReset; break;
			case 1: gMainState = eDeviceId;  	  break;
			case 2: gMainState = eVersionInfo;  break;
			case 3: gMainState = eSelectMenu; 	break;
			default: break;
		}
		if(gStateData == 3) gStateData = 2;
		else gStateData = 0;
		gStateDataEx = 0;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
	}
}

void keyFactoryReset(void)
{
	if(gLeft||gRight){
		gStateData = (gStateData + 1) % 2;
		if(gStateData == 0){
			ugui_printf_string(1,    LINE_04, "是", &Font16, ANTI);
			ugui_printf_string(111,  LINE_04, "否", &Font16, NORMAL);
		}else{
			ugui_printf_string(1,    LINE_04, "是", &Font16, NORMAL);
			ugui_printf_string(111,  LINE_04, "否", &Font16, ANTI);			
		}
	}
	if(gOK){
		if(gStateData == 0){		//恢复出厂设置
			//写EEPROM 并且重启系统
			__set_FAULTMASK(1); 
			NVIC_SystemReset();	
		}else{
			gStateData = 0;
			isInputNumStatus = RT_FALSE;
			isClearDisplay = RT_TRUE;
			updateDisplay = RT_TRUE;
			gMainState = eSystemInfo;
		}
	}
}

void keyDeviceId(void)
{
	if(gOK||gLeft||gRight){
		gStateData = 1;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
		gMainState = eSystemInfo;		
	}
}

void keyVersionInfo(void)
{
	if(gOK||gLeft||gRight){
		gStateData = 2;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
		gMainState = eSystemInfo;		
	}
}

void keyStateMachine(rt_ubase_t key)
{
	if(isRefreshDisplay == RT_TRUE){		//上电初始化或者屏幕待机唤醒时需要刷新显示
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;	
		isRefreshDisplay	= RT_FALSE;		
	}else{
		isClearDisplay = RT_FALSE;
		updateDisplay = RT_FALSE;
	}
	
	/* 解析按键值 */
	processKeyMsg(key);
	
	switch(gMainState){
		case eMainMenu:
			keyMainMenu();
			break;
		case eSelectMenu:
			keySelectMenu();
			break;
		case eTerminalSet:
			keyTerminalSet();
			break;
		case eNetworkSet:
			keyNetworkSet();
			break;
		case eIpAndGateSet:
			keyIpAndGateSet();
			break;
		case eSystemInfo:
			keySystemInfo();
			break;
		case eFactoryReset:
			keyFactoryReset();
			break;
		case eDeviceId:
			keyDeviceId();
			break;
		case eVersionInfo:
			keyVersionInfo();
			break;
		default: break;
	}

	/* 清除处理过的按键值 */
	erasureKeyPlus(key);
}

void staticDisplayMenu(void)
{
	if(updateDisplay == RT_FALSE) return;  	//没有要更新的显示

	if(isClearDisplay) ugui_clear_screen(); //强制清屏

	switch(gMainState){
		case eMainMenu:
			staticMainMenu();
			break;
		case eSelectMenu:
			staticSelectMenu();
			break;
		case eTerminalSet:
			staticTerminalSet();
			break;
		case eNetworkSet:
			staticNetworkSet();
			break;
		case eIpAndGateSet:
			staticIpAndGateSetPage(gStateData);
			break;
		case eSystemInfo:
			staticSystemInfo();
			break;
		case eFactoryReset:
			staticFactoryReset();
			break;
		case eDeviceId:
			staticDeviceId();
			break;
		case eVersionInfo:
			staticVersionInfo();
			break;
		default: break;	
	}
}

void hmi_interface_process(void)
{
	rt_ubase_t keySta = RT_FALSE;

	keySta = wait_key_plus(300);
	if(keySta != RT_FALSE)
	{
		if(blacklight_cnt >= 400){
			gMainState = eMainMenu;
			isRefreshDisplay = RT_TRUE;
		}
		blacklight_cnt = 0;
	}else{
		blacklight_cnt++;
		if(blacklight_cnt >= 400){	//约五分钟无按键操作 屏幕待机 清除显示内容
			blacklight_cnt = 400;
		  gMainState = eUnkown;
			ugui_clear_screen(); 			//强制清屏
		}
	}

	keyStateMachine(keySta);			//按键处理
	
	staticDisplayMenu();					//静态页面
}
