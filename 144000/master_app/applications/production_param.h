#ifndef _PRODUCTION_PARAM_H_
#define _PRODUCTION_PARAM_H_

#include "rtdef.h"
#include "dev_config.h"

/* 生产配置数据和运行参数相关配置*/
#define RECIPE_USED_MAX					10		//最多保存10个配方

#define RECIPE_PARAM_LIST_LEN		20		//单个配方里面最多20条参数

#define	PRODUCT_USED_MAX				30		//产品最多30种

#define	PACKAGE_BOX_USED_MAX		20		//包装盒最多20种

#define	SUPPLIER_USED_MAX				50		//蛋品源头最多50种

#define TEAM_GROUP_USED_MAX			10		//最多保存10个班组

//包装盒信息列表
typedef struct {
	uint16_t lseek;							//记录在文件中的位置
	uint16_t name[10];					//包装盒名称
	uint16_t row;								//行数
	uint16_t col;								//列数
	uint16_t calibChannel; 			//校准通道
	uint16_t autoTray;					//自动发托位置
	uint16_t trayCheck;					//蛋托检测位置		
	uint16_t packBoxPost1;			//1排落蛋位置
	uint16_t packBoxPost2;			//2排落蛋位置
	uint16_t packBoxPost3;			//3排落蛋位置
	uint16_t packBoxPost4;			//4排落蛋位置
	uint16_t packBoxPost5;			//5排落蛋位置
	uint16_t packBoxPost6;			//6排落蛋位置
	uint16_t packBoxPost7;			//7排落蛋位置
	uint16_t packBoxPost8;			//8排落蛋位置
	uint16_t transportStart;		//输送启动位置
	uint16_t chainLinkCnt;			//占用装托链条格数
	int16_t originOffset[20];		//包装通道原点偏移 20个通道
} PackageBoxInformation;  		//最多20种包装盒	1480字节

typedef struct {
	uint16_t lseek;																		//记录在文件中的位置
	PackageBoxInformation box[PACKAGE_BOX_USED_MAX]; 	//最多20种包装盒	1480字节
} PackageBoxList;

//产品管理里面的产品信息列表
typedef struct {
	uint16_t lseek;							//记录在文件中的位置
	uint16_t name[10];					//产品名称
	uint16_t weighLevel;				//重量等级
	int16_t  weighLow;					//自定义重量范围 低值
	int16_t  weighHigh;					//自定义重量范围 高值
	uint16_t colorLevel;				//颜色等级
	int16_t  colorLow;					//自定义颜色范围 低值
	int16_t  colorHigh;					//自定义颜色范围 高值
	uint16_t crackLevel;				//裂纹等级
	int16_t  crackLow;					//自定义裂纹范围 低值
	int16_t  crackHigh;					//自定义裂纹范围 高值	
	uint16_t dirtyLevel;				//脏蛋等级
	int16_t  dirtyLow;					//自定义脏蛋范围 低值
	int16_t  dirtyHigh;					//自定义脏蛋范围 高值		
	uint16_t bloodLevel;				//血斑蛋
	uint16_t printNumber;				//喷码编号
	uint16_t mark;							//蛋品标记 脏蛋 裂纹蛋
} ProductManageInformation;

typedef struct {
	uint16_t lseek;													//记录在文件中的位置
	ProductManageInformation product[PRODUCT_USED_MAX]; //最多30种产品 720字节
} ProductManageList;

//蛋品源头供应商管理信息列表
typedef struct {
	uint16_t name[10];											//供应商名称
} SupplierManageInformation;  

typedef struct {
	uint16_t lseek;													//记录在文件中的位置
	SupplierManageInformation supplier[SUPPLIER_USED_MAX]; //最多50个蛋品源头 500字节
} SupplierManageList;

//单个配方中的分级规则参数
typedef struct {
	uint16_t lseek;													//记录在文件中的位置
	uint8_t priority;												//优先级
	uint16_t productName[10];								//蛋品名称(产品名称)	
	uint16_t packageName[10];								//包装盒名称
	uint32_t packageLine;										//包装线 用位标识 1表示启用
	uint32_t targetBox;											//目标产量 盒
	uint32_t targetSC;											//目标产量 枚
	uint8_t remind;													//完成后提示
	uint8_t stop;														//完成后停机
} SortRuleParam;

//单个配方的参数
typedef struct {
	uint16_t lseek;													//记录在文件中的位置
	uint16_t recipeName[10];								//配方名称
	time_t now;															//参数更新的时间
	SortRuleParam sortRule[RECIPE_PARAM_LIST_LEN];//20条分级规则
} SingleRecipeParam;

//配方管理中存储的配方列表
typedef struct {
	uint16_t lseek;														//记录在文件中的位置
	SingleRecipeParam recipe[RECIPE_USED_MAX];//最多保存10个配方
} SortRecipeParamList;

//当前配方的运行参数
typedef struct {
	uint16_t batchName[10];									//批次编号
	SupplierManageInformation supplier;			//蛋品源头
	uint16_t workSpeed;											//产量速度 60% 70% 80% 90% 100% 或者自定义
	uint16_t workSpeedEx;										//自定义速度值
	SingleRecipeParam singleRecipe;					//运行的配方
} WorkRecipeParam;

//当前提交的配方参数
typedef struct {
	uint16_t lseek;													//记录在文件中的位置
	uint16_t workSpeed;											//产量速度 60% 70% 80% 90% 100% 或者自定义
	uint16_t workSpeedEx;										//自定义速度值
	uint16_t selRecipeName[10];							//选择的配方名称
} SubmitRecipeParam;

typedef struct {
	uint16_t groupName[10];									//班组名称
	uint16_t startHour;
	uint16_t startMin;
	uint16_t startSecond;
	uint16_t stopHour;
	uint16_t stopMin;
	uint16_t stopSecond;	
} SingleGroupParam;

//班组管理中存储的班组列表
typedef struct {
	uint16_t lseek;														//记录在文件中的位置
	SingleGroupParam group[TEAM_GROUP_USED_MAX];//最多保存10个班组
} TeamOrGroupParamList;

/* 单个通道包装当前的运行参数 数据通过当前运行的配方参数里面提取 */
typedef struct {
	uint8_t isAct;													//通道启停用
	uint16_t packageName[10];								//包装名称
	uint16_t row;														//行数
	uint16_t col;														//列数
	uint16_t autoTray;											//自动发托位置
	uint16_t trayCheck;											//蛋托检测位置 (包装盒检测位置)
	uint16_t packBoxPost1;									//1排落蛋位置
	uint16_t packBoxPost2;									//2排落蛋位置
	uint16_t packBoxPost3;									//3排落蛋位置
	uint16_t packBoxPost4;									//4排落蛋位置
	uint16_t packBoxPost5;									//5排落蛋位置
	uint16_t packBoxPost6;									//6排落蛋位置
	uint16_t packBoxPost7;									//7排落蛋位置
	uint16_t packBoxPost8;									//8排落蛋位置
	uint16_t transportStart;								//输送启动位置	(接受爪上升到何处位置时链条开始启动)
	uint16_t chainLinkCnt;									//占用装托链条格数
} SinglePackageWorkParam;

extern TeamOrGroupParamList groupList;		//班组管理列表
extern SupplierManageList supplierList;   //蛋品源头列表
extern PackageBoxList boxList;					  //包装盒列表
extern ProductManageList productList;		  //产品管理列表
extern SortRecipeParamList recipeList;	  //配方管理中的配方列表
extern WorkRecipeParam workRecipe;			  //当前要运行的配方
extern SinglePackageWorkParam packageWorkList[SORTER_USED_MAX];		//20个包装通道当前运行的参数

//遍历norflash地址 找出数据存储的起始地址
extern void find_storage_descriptor(void);

//包装盒信息写入存储器操作
extern rt_err_t write_package_box_information(size_t size, uint8_t  *write_data);

//从存储器读取包装盒信息操作
extern rt_err_t read_package_box_information(size_t size, uint8_t  *read_data);

//蛋品管理信息写入存储器操作
extern rt_err_t write_product_manage_information(size_t size, uint8_t  *write_data);

//从存储器读取蛋品管理信息操作
extern rt_err_t read_product_manage_information(size_t size, uint8_t  *read_data);

//蛋品源头信息写入存储器操作
extern rt_err_t write_supplier_manage_information(size_t size, uint8_t  *write_data);

//从存储器读取蛋品源头信息操作
extern rt_err_t read_supplier_manage_information(size_t size, uint8_t  *read_data);

//班组参数写入存储器操作
extern rt_err_t write_team_group_information(size_t size, uint8_t  *write_data);

//从存储器读取班组参数操作
extern rt_err_t read_team_group_information(size_t size, uint8_t  *read_data);

//当前配方的运行参数写入存储器操作
extern rt_err_t write_submit_recipe_information(size_t size, uint8_t  *write_data);

//从存储器读取当前配方的运行参数操作
extern rt_err_t read_work_recipe_information(size_t size, uint8_t  *read_data);

//配方管理列表写入存储器操作
extern rt_err_t write_recipe_list_information(size_t size, uint8_t  *write_data);

//从存储器读取配方管理列表操作
extern rt_err_t read_recipe_list_information(size_t size, uint8_t  *read_data);

//根据步科屏幕页面地址处理数据
extern void externalPageAddressProc(int index, int len);

//屏幕参数初始化
extern void externalPageAddressInit(void);

//根据名称查找蛋品源头参数
extern ProductManageInformation *getProductManageInformation(uint16_t *name);

//根据名称查找包装盒参数
extern PackageBoxInformation *getPackageBoxInformation(uint16_t *name);

//根据配方名查找配方参数
extern SingleRecipeParam *getSingleRecipeParam(uint16_t *name);

//遍历当前运行的配方参数 提取出当前每个包装通道的运行参数
void getPackageWorkInformation(WorkRecipeParam *recipe, SinglePackageWorkParam *pack);

#endif
