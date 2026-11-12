#ifndef __ALARMLOG_H
#define __ALARMLOG_H

#ifdef __cplusplus
extern "C" {
#endif

//定义报警类型
enum AlarmType{
	eBalanceSensor1Err = 0,     //称重传感器1~6称重故障 (注:判断依据为低于校准零位50克或者高于校准零位200克)
	eBalanceSensor2Err = 1,
	eBalanceSensor3Err = 2,
	eBalanceSensor4Err = 3,
	eBalanceSensor5Err = 4,
	eBalanceSensor6Err = 5,
	
	ePeelWeight1Err		 = 6,			//称重传感器1~6累计去皮报警 (注:判断依据为每个通道的当前皮重值超过累计去皮报警值)
	ePeelWeight2Err		 = 7,
	ePeelWeight3Err		 = 8,
	ePeelWeight4Err		 = 9,
	ePeelWeight5Err		 = 10,
	ePeelWeight6Err		 = 11,
	
	eProximitySwitchErr= 12,		//接近开关故障
	eEncoderErr				 = 13,		//编码器故障
	ePiezoceramicsErr  = 14,		//压电陶瓷传感器故障 (连续多次同一个压电陶瓷传感器敲击信号异常报警)
	
	eCan1CommunicateErr= 15,		//1~6通道CAN通信故障
	eCan2CommunicateErr= 16,		
	eCan3CommunicateErr= 17,		
	eCan4CommunicateErr= 18,		
	eCan5CommunicateErr= 19,		
	eCan6CommunicateErr= 20,		
};

#endif
