#include "devConfig.h"

enum EnumMainStateManchine mainState;
enum EnumMainStateManchine mainStateNext;
enum EnumBranchStateManchine branchState;

CylinderStateTypeDef cylinderState[SERVO_MAX];

AcceptMasterControl mCtrl = {0};					//包装通道手动控制命令参数

ServoStateInfo sstd_servo = {0,0,0};
ServoStateInfo jsz_servo = {0,0,0};

AlarmMarkTypeDef alarmMark ={0};					//包装通道报警标志位

uint8_t get_input_port_sta(uint8_t *iData, enum EnumInputPort e)
{
	uint8_t col = 0, row = 0;
	
	col = e / 8;
	row = e % 8;
	
	return getbitv(iData[col], row);
}

void clear_alarm_mark(void)
{
	rt_memset(&alarmMark, 0, sizeof(AlarmMarkTypeDef));
}
