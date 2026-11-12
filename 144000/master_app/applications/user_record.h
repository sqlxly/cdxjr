#ifndef _USER_RECORDH_
#define _USER_RECORDH_

#include <drv_common.h>

//单条记录模版
typedef struct {
	uint32_t amount;			//数量
	uint32_t totalWeight; //总重量kg
	uint16_t avgWeighht;	//平均重量g
	uint16_t percent;			//百分比
}ReportStyle;

//重量分级模版
typedef struct {
	ReportStyle level[6];		//超小、小、中、大、超大、自定义 共6种等级
	ReportStyle total;
}WeightReportTemplate;

//颜色分级模版
typedef struct {
	ReportStyle level[5];		//绿色、白色、粉色、褐色、自定义 共5种等级
	ReportStyle total;
}ColorReportTemplate;

//品质分级-裂纹模版
typedef struct {
	ReportStyle level[5];		//优、隐纹、破口、漏液、自定义 共5种等级
	ReportStyle total;
}CrackReportTemplate;

//品质分级-脏蛋模版
typedef struct {
	ReportStyle level[4];		//优、良、差、自定义 共4种等级
	ReportStyle total;
}DirtyReportTemplate;

//品质分级-异常蛋 血斑蛋模版
typedef struct {
	ReportStyle level[2];		//优、异常 共2种等级
	ReportStyle total;
}BloodReportTemplate;

//报表数据公共模版
typedef struct {
	time_t now;												//日期时间
	uint32_t totalOutput;							//生产总量
	uint16_t rateNormal;							//成品率
	uint32_t totalDirty;							//脏蛋数量 这个是在新建产品时在产品类型里面标记成脏蛋的蛋品统计
	uint16_t rateDirty;								//脏蛋率
	uint32_t totalCrack;							//裂纹数量 这个是在新建产品时在产品类型里面标记成裂纹的蛋品统计
	uint16_t rateCrack;								//裂纹率
	uint32_t manualEntryCnt;					//人工录入数量 枚
	uint32_t manualEntryWeight;				//人工录入重量 kg
	uint32_t estimatedCnt;						//蛋液估算出枚数 按照均重50.0g来估算
	
	/* 报表详细中使用 */
	WeightReportTemplate weighReport;	//重量分级
	ColorReportTemplate colorReport;	//颜色分级
	CrackReportTemplate crackReport;	//裂纹分级
	DirtyReportTemplate dirtyReport;  //脏蛋分级
	BloodReportTemplate bloodReport;	//异常蛋分级
}PubicReportTemplate;

//批次报表数据结构
typedef struct {
	PubicReportTemplate report;
	uint16_t batchNum;		//批次
} BatchReportFormStruct;

//班组报表数据结构
typedef struct {
	PubicReportTemplate report;
	uint16_t groupNum;		//班次
} GroupReportFormStruct;

//日报表数据结构
typedef struct {
	PubicReportTemplate report;
} DailyReportFormStruct;

#endif
