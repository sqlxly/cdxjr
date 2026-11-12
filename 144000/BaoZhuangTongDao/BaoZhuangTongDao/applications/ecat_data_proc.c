#include "ecat_data_proc.h"
#include "devConfig.h"
#include "el9800appl.h"
#include "IOdriver.h"

extern void getAllInputPortState(uint8_t *data, uint8_t len);

static AcceptMasterMessageWork *work;
static AcceptMasterMessageStop *stop;
static AcceptMasterControl *ctrl;
static AcceptMasterMessageParam1 *param1;
static AcceptMasterMessageParam2 *param2;
static AcceptMasterMessageParam3 *param3;

static uint32_t dctCtrl = 0;

PushTrigSignal tirgSignal[4];			//数据来自EtherCat

extern ChanbelOperateTypeDef cop;

extern uint8_t channelClearFlag;

//EtherCat从站接收数据处理 只响应实时控制指令 其它指令需要异步线程master_pthread.c处理
void ethercat_slave_recv_data_process(void)
{
	uint16_t command = 0;

	command = sDOOutputs.oData1;
	switch(command)
	{
		case eMasterRunState:				//运行命令
			mainStateNext = eRunState;
			work = (AcceptMasterMessageWork *)&sDOOutputs.oData1;
			dctCtrl = work->dctLine4;
			dctCtrl <<= 6;
			dctCtrl |= work->dctLine3;
			dctCtrl <<= 6;
			dctCtrl |= work->dctLine2;
			dctCtrl <<= 6;
			dctCtrl |= work->dctLine1;
			tirgSignal[0].pushTrig = work->dbFull1;
			tirgSignal[1].pushTrig = work->dbFull2;
			tirgSignal[2].pushTrig = work->dbFull3;
			tirgSignal[3].pushTrig = work->dbFull4;
			break;
		case eMasterStopState: 			//停止命令
			mainStateNext = eStopState;
			stop = (AcceptMasterMessageStop *)&sDOOutputs.oData1;
			if(stop->alarmConfirm == 1) {
				clear_alarm_mark();
			}
			break;
		case eMasterControlState:		//手动控制命令
			ctrl = (AcceptMasterControl *)&sDOOutputs.oData1;
			dctCtrl = ctrl->dctLine4;
			dctCtrl <<= 6;
			dctCtrl |= ctrl->dctLine3;
			dctCtrl <<= 6;
			dctCtrl |= ctrl->dctLine2;
			dctCtrl <<= 6;
			dctCtrl |= ctrl->dctLine1;
			cop.dcfState= ctrl->jthcCtrl;
			rt_memcpy(&mCtrl, ctrl, sizeof(AcceptMasterControl));
			mainStateNext = eControlState;
			break;
		case eMasterEditState:			//新建包装控制命令
			rt_memcpy(&mCtrl, ctrl, sizeof(AcceptMasterControl));
			mainStateNext = eEditState;
			break;
		case eMasterSndParamState1: //设置参数1的命令
			param1 = (AcceptMasterMessageParam1 *)&sDOOutputs.oData1;
			cop.workParam.col = param1->col;
			cop.workParam.row = param1->row;
			cop.workParam.autoTray = param1->autoTray;
			cop.workParam.trayCheck = param1->trayCheck;
			cop.workParam.transportStart = param1->transportStart;
			cop.workParam.packBoxPost1 = param1->packBoxPost1;
			cop.workParam.packBoxPost2 = param1->packBoxPost2;
			break;
		case eMasterSndParamState2: //设置参数2的命令
			param2 = (AcceptMasterMessageParam2 *)&sDOOutputs.oData1;
			cop.workParam.packBoxPost3 = param2->packBoxPost3;
			cop.workParam.packBoxPost4 = param2->packBoxPost4;
			cop.workParam.packBoxPost5 = param2->packBoxPost5;
			cop.workParam.packBoxPost6 = param2->packBoxPost6;
			cop.workParam.packBoxPost7 = param2->packBoxPost7;
			cop.workParam.packBoxPost8 = param2->packBoxPost8;
			cop.workParam.chainLinkCnt = param2->chainLinkCnt;
			break;
		case eMasterSndParamState3:
			param3 = (AcceptMasterMessageParam3 *)&sDOOutputs.oData1;
			if(param3->alarmConfirm == 1) {
				clear_alarm_mark();
			}
			break;
		default:  									//异常指令  停止通道动作 关闭私服电机
			break;
	}

	updateDctIO(dctCtrl);
}

//EtherCat从站发送数据处理
void ethercat_slave_send_datat_process(uint16_t cmd)
{
	uint8_t inputData[6];
	uint16_t dataBuf[8], temp = 0;

	SlaveSendMessage *sMsg;

	rt_memset(dataBuf, 0, 16);
	sMsg = (SlaveSendMessage *)dataBuf;

	getAllInputPortState(inputData, 6);
	sMsg->cmd = (0xAA00|(cmd&0x00ff));																								//接收到的命令码前缀从0x55--变为0xAA--
	temp = inputData[1];
	temp <<= 8;
	temp |= inputData[0];
	sMsg->jthcSta 	= (temp&0x0fff); 																									//反馈阶梯缓存接近开关状态 即蛋杯开闭状态
	sMsg->ftjPost 	= get_input_port_sta(inputData, (enum EnumInputPort)(eFSdin3));		//发托机到位状态
	sMsg->dtss_dt 	= get_input_port_sta(inputData, (enum EnumInputPort)(eFSdin2));		//蛋托输送 蛋托检测
	sMsg->dtss_dl 	= get_input_port_sta(inputData, (enum EnumInputPort)(eANdin6));		//蛋托输送 堆料检测
	sMsg->dtss_lw 	= get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin21));	//蛋托输送 零位检测

	sMsg->jsz_dw 		= get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin20));	//接受抓 接受抓到位
	sMsg->jsz_dj 		= get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin19));	//接受抓 蛋夹关闭
	sMsg->jsz_db 		= get_input_port_sta(inputData, (enum EnumInputPort)(eFSdin1));		//接受抓 蛋杯检测 空杯检测
	sMsg->ydhc_dj 	= get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin13));	//移动缓存 蛋夹关闭
	sMsg->ydhc_lw 	= get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin18));	//移动缓存 零位检测 即正对接受抓位置
	sMsg->ydhc_xw1 	= get_input_port_sta(inputData, (enum EnumInputPort)(eXCdin1));		//移动缓存 限位开关1
	sMsg->ydhc_xw2 	= get_input_port_sta(inputData, (enum EnumInputPort)(eXCdin2));		//移动缓存 限位开关2
	sMsg->ydhc_jdw1 = get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin14));	//移动缓存 1号接蛋位检测
	sMsg->ydhc_jdw2 = get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin15));	//移动缓存 2号接蛋位检测
	sMsg->ydhc_jdw3 = get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin16));	//移动缓存 3号接蛋位检测
	sMsg->ydhc_jdw4 = get_input_port_sta(inputData, (enum EnumInputPort)(eJJdin17));	//移动缓存 4号接蛋位检测
	sMsg->jthc_gd1 	= cop.jthcOP[eDCFout1].flag > 0 ? 1 : 0;													//轨道1下面的第一层阶梯缓存状态 空或者有蛋
	sMsg->jthc_gd2 	= cop.jthcOP[eDCFout2].flag > 0 ? 1 : 0;													//轨道2下面的第一层阶梯缓存状态 空或者有蛋
	sMsg->jthc_gd3 	= cop.jthcOP[eDCFout3].flag > 0 ? 1 : 0;													//轨道3下面的第一层阶梯缓存状态 空或者有蛋
	sMsg->jthc_gd4 	= cop.jthcOP[eDCFout4].flag > 0 ? 1 : 0;													//轨道4下面的第一层阶梯缓存状态 空或者有蛋
	sMsg->clearFlag = channelClearFlag > 0 ? 1 : 0;																		//通道清空按钮按下																												//备用

	sMsg->sstdServoPost = sstd_servo.distance / 100;																	//蛋托输送私服电机位置 缩小100倍后传输
	sMsg->jszServoPost = jsz_servo.distance / 1000;  																	//接受抓私服电机位置 伺服电机默认转一圈是100000PUU, 转换成位置百分比后扩大10倍 

	rt_memcpy(&sMsg->alarm, &alarmMark, sizeof(AlarmMarkTypeDef));										//包装通道报警信息
	
	rt_memcpy(&sDIInputs.iData1, dataBuf, 16);																				//数据更新到ethercat发送缓存区
}
