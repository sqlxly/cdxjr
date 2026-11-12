#include "production_param.h"
#include "spi_flash_sfud.h"
#include "user_kinco.h"
#include <stdio.h>
#include <string.h>

#define TEST_PAGE   99
#define TEST_ADDR   0

#define REPEAT_WRITE_LENTH				256								//循环写入的长度256条 每条数据长度为4Kbyte  4Kbyte * 256 = 1Mbyte
			
#define PACKAGE_BOX_BASE_ADDR			0x0								//从 0byte  地址开始写包装盒信息列表

#define PRODUCT_BASE_ADDR 				0x100000					//从 1Mbyte 地址开始写蛋品管理信息列表
			
#define SUPPLIER_BASE_ADDR 				0x200000					//从 2Mbyte 地址开始写蛋品源头信息列表
			
#define SUBMIT_RECIPE_BASE_ADDR 	0x300000					//从 3Mbyte 地址开始写运行配方任务信息

#define TEAM_GROUP_BASE_ADDR 			0x400000					//从 4Mbyte 地址开始写运行配方任务信息

#define RECIPE_MANAGER_BASE_ADDR 	0x500000					//从 5Mbyte 地址开始写配方管理信息列表 16Kbyte * 256 = 4Mbyte

static uint16_t vSupplierManagePos 	= 0xffff;  			//蛋品源头列表当前写入位置
static uint16_t vPackageBoxPos 			= 0xffff;  			//包装盒列表当前写入位置
static uint16_t vProductManagePos 	= 0xffff;  			//蛋品管理列表当前写入位置
static uint16_t vsubmitRecipePos 		= 0xffff;  			//运行配方任务当前写入位置
static uint16_t vGroupManagePos 		= 0xffff;  			//班组管理列表当前写入位置
static uint16_t vRecipeManagePos 		= 0xffff;				//配方管理列表当前写入位置

static SubmitRecipeParam submitRecipe = {0};				//提交的运行配方参数
static SingleRecipeParam selectRecipe = {0};			  //运行配方编辑时用到的临时运行配方参数

SupplierManageList 	 supplierList  = {0};    				//蛋品源头列表
PackageBoxList 			 boxList 		   = {0};						//包装盒列表
ProductManageList 	 productList   = {0};						//蛋品管理列表
TeamOrGroupParamList groupList     = {0};						//班组管理列表
WorkRecipeParam 		 workRecipe 	 = {0}; 					//当前配方的运行参数 每次系统启动时从saveRecipe变量中提取出来
SortRecipeParamList  recipeList 	 = {0};						//配方管理中的配方列表

SinglePackageWorkParam packageWorkList[SORTER_USED_MAX];		//20个包装通道当前运行的参数

static sfud_flash *sfud_dev = NULL;	
	
static uint16_t pageNumberLast  = 20;								//页面切换时记录前一个页面编码
static uint8_t multiPageFunc 		= 0;								//多页面按钮功能选择
static uint8_t multiPageCount 	= 0;   							//多页面数据翻页显示
static uint8_t multiPageFuncEx 	= 0;								//二级菜单多页面按钮功能选择
static uint8_t multiPageCountEx = 0;   							//二级菜单多页面数据翻页显示
static char cid_iemi[25] = { 0 };										//UID IMEI用于表示设备唯一地址

extern uint16_t *u_RegHoldingPtr;

//遍历norflash地址 找出数据存储的起始地址
void find_storage_descriptor(void)
{
	uint16_t i, number = 0;
	sfud_err err;

	sfud_dev = rt_sfud_flash_find("qspi10"); 				// 获取描述符sfud_dev
	if(sfud_dev == RT_NULL) return;
	
	/* 遍历包装盒信息存储扇区 找到写入起始地址 */
	for(i=0; i<REPEAT_WRITE_LENTH; i++){
		err = sfud_read(sfud_dev, PACKAGE_BOX_BASE_ADDR+i*0x1000, 2, (uint8_t *)&number);	
//		rt_kprintf("a%d err = %d\n", number, err);
		if(err != SFUD_SUCCESS) break;
		if((i+1) != number) break;
	}
	if(i >= REPEAT_WRITE_LENTH){
		vPackageBoxPos = 0;
		rt_memset(&boxList, 0, sizeof(PackageBoxList));
	}else{
		vPackageBoxPos = i;
		if(0 == i){
			rt_memset(&boxList, 0, sizeof(PackageBoxList));
		}else if(i > 0){
			err = sfud_read(sfud_dev, PACKAGE_BOX_BASE_ADDR+(i-1)*0x1000, sizeof(PackageBoxList), (uint8_t *)&boxList);
			if(err != RT_EOK){
				//norflash存储器故障
			}
		}
	}

	/* 遍历蛋品管理存储扇区 找到写入起始地址 */
	for(i=0; i<REPEAT_WRITE_LENTH; i++){
		err = sfud_read(sfud_dev, PRODUCT_BASE_ADDR+i*0x1000, 2, (uint8_t *)&number);	
//		rt_kprintf("b%d err = %d\n", number, err);
		if(err != SFUD_SUCCESS) break;
		if((i+1) != number) break;
	}
	if(i >= REPEAT_WRITE_LENTH){
		vProductManagePos = 0;
		rt_memset(&productList, 0, sizeof(ProductManageList));
	}else{
		vProductManagePos = i;
		if(0 == i){
			rt_memset(&productList, 0, sizeof(ProductManageList));
		}else if(i > 0){
			err = sfud_read(sfud_dev, PRODUCT_BASE_ADDR+(i-1)*0x1000, sizeof(ProductManageList), (uint8_t *)&productList);
			if(err != RT_EOK){
				//norflash存储器故障
			}
		}
	}

	/* 遍历蛋品源头存储扇区 找到写入起始地址 */
	for(i=0; i<REPEAT_WRITE_LENTH; i++){
		err = sfud_read(sfud_dev, SUPPLIER_BASE_ADDR+i*0x1000, 2, (uint8_t *)&number);	
//		rt_kprintf("c%d err = %d\n", number, err);
		if(err != SFUD_SUCCESS) break;
		if((i+1) != number) break;
	}
	if(i >= REPEAT_WRITE_LENTH){
		vSupplierManagePos = 0;
		rt_memset(&supplierList, 0, sizeof(SupplierManageList));
	}else{
		vSupplierManagePos = i;
		if(0 == i){
			rt_memset(&supplierList, 0, sizeof(SupplierManageList));
		}else if(i > 0){
			err = sfud_read(sfud_dev, SUPPLIER_BASE_ADDR+(i-1)*0x1000, sizeof(SupplierManageList), (uint8_t *)&supplierList);
			if(err != RT_EOK){
				//norflash存储器故障
			}
		}
	}

	/* 当前配方的运行参数存储扇区 找到写入起始地址 */
	for(i=0; i<REPEAT_WRITE_LENTH; i++){
		err = sfud_read(sfud_dev, SUBMIT_RECIPE_BASE_ADDR+i*0x1000, 2, (uint8_t *)&number);	
//		rt_kprintf("d%d err = %d\n", number, err);
		if(err != SFUD_SUCCESS) break;
		if((i+1) != number) break;
	}
	if(i >= REPEAT_WRITE_LENTH){
		vsubmitRecipePos = 0;
		rt_memset(&submitRecipe, 0, sizeof(SubmitRecipeParam));
	}else{
		vsubmitRecipePos = i;
		if(0 == i){
			rt_memset(&submitRecipe, 0, sizeof(SubmitRecipeParam));
		}else if(i > 0){
			err = sfud_read(sfud_dev, SUBMIT_RECIPE_BASE_ADDR+(i-1)*0x1000, sizeof(SubmitRecipeParam), (uint8_t *)&submitRecipe);
			if(err != RT_EOK){
				//norflash存储器故障
			}
		}
	}
	
	/* 当前班组参数存储扇区 找到写入起始地址 */
	for(i=0; i<REPEAT_WRITE_LENTH; i++){
		err = sfud_read(sfud_dev, TEAM_GROUP_BASE_ADDR+i*0x1000, 2, (uint8_t *)&number);	
//		rt_kprintf("d%d err = %d\n", number, err);
		if(err != SFUD_SUCCESS) break;
		if((i+1) != number) break;
	}
	if(i >= REPEAT_WRITE_LENTH){
		vGroupManagePos = 0;
		rt_memset(&groupList, 0, sizeof(TeamOrGroupParamList));
	}else{
		vGroupManagePos = i;
		if(0 == i){
			rt_memset(&groupList, 0, sizeof(TeamOrGroupParamList));
		}else if(i > 0){
			err = sfud_read(sfud_dev, TEAM_GROUP_BASE_ADDR+(i-1)*0x1000, sizeof(TeamOrGroupParamList), (uint8_t *)&groupList);
			if(err != RT_EOK){
				//norflash存储器故障
			}
		}
	}
	
	/* 遍历配方管理存储扇区 找到写入起始地址 */
	for(i=0; i<REPEAT_WRITE_LENTH; i++){
		err = sfud_read(sfud_dev, RECIPE_MANAGER_BASE_ADDR+i*0x4000, 2, (uint8_t *)&number);	
//		rt_kprintf("cc d%d err = %d\n", number, err);
		if(err != SFUD_SUCCESS) break;
		if((i+1) != number) break;
	}
	if(i >= REPEAT_WRITE_LENTH){
		vRecipeManagePos = 0;
		rt_memset(&recipeList, 0, sizeof(SortRecipeParamList));
	}else{
		vRecipeManagePos = i;
		if(0 == i){
			rt_memset(&recipeList, 0, sizeof(SortRecipeParamList));
		}else if(i > 0){
			err = sfud_read(sfud_dev, RECIPE_MANAGER_BASE_ADDR+(i-1)*0x4000, sizeof(SortRecipeParamList), (uint8_t *)&recipeList);
			if(err != RT_EOK){
				//norflash存储器故障
			}
		}
	}
}

//包装盒信息写入存储器操作
rt_err_t write_package_box_information(size_t size, uint8_t  *write_data)
{
	sfud_err  err;
	uint16_t *pos = 0;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;
	
	if(0 == vPackageBoxPos) {				//恢复出厂设置状态了或者写满一个循环 重新擦除扇区 1Mbyte
		sfud_erase(sfud_dev, PACKAGE_BOX_BASE_ADDR, 0x100000);
	}

	pos = (uint16_t *)write_data;
	*pos = vPackageBoxPos + 1;
	
	err = sfud_write(sfud_dev, PACKAGE_BOX_BASE_ADDR+vPackageBoxPos*0x1000, size, write_data); //写数据

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}
	
	rt_kprintf("write vPackageBoxPos = %d\n", vPackageBoxPos+1);
	
	vPackageBoxPos = (vPackageBoxPos + 1) % REPEAT_WRITE_LENTH;			//更新读写地址
	
	return RT_EOK;
}
//从存储器读取包装盒信息操作
rt_err_t read_package_box_information(size_t size, uint8_t  *read_data)
{
	sfud_err  err;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;   		//描述符错误

	if(0xffff == vPackageBoxPos) return -RT_EEMPTY; //存储为空
	
	err = sfud_read(sfud_dev, PACKAGE_BOX_BASE_ADDR+vPackageBoxPos*0x1000, size, read_data);
	
	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}	
	
	return RT_EOK;
}

//蛋品管理信息写入存储器操作
rt_err_t write_product_manage_information(size_t size, uint8_t  *write_data)
{
	sfud_err  err;
	uint16_t *pos = 0;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;

	if(0 == vProductManagePos) {				//恢复出厂设置状态了或者写满一个循环 重新擦除扇区 1Mbyte
		sfud_erase(sfud_dev, PRODUCT_BASE_ADDR, 0x100000);
	}

	pos = (uint16_t *)write_data;
	*pos = vProductManagePos + 1;

	err = sfud_write(sfud_dev, PRODUCT_BASE_ADDR+vProductManagePos*0x1000, size, write_data); //写数据
	
	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}
	
	rt_kprintf("write vProductManagePos = %d\n", vProductManagePos+1);
	
	vProductManagePos = (vProductManagePos + 1) % REPEAT_WRITE_LENTH;			//更新读写地址
	
	return RT_EOK;
}
//从存储器读取蛋品管理信息操作
rt_err_t read_product_manage_information(size_t size, uint8_t  *read_data)
{
	sfud_err  err;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;   			//描述符错误

	if(0xffff == vProductManagePos) return -RT_EEMPTY; //存储为空
	
	err = sfud_read(sfud_dev, PRODUCT_BASE_ADDR+vProductManagePos*0x1000, size, read_data);
	
	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}	
	
	return RT_EOK;
}

//蛋品源头信息写入存储器操作
rt_err_t write_supplier_manage_information(size_t size, uint8_t  *write_data)
{
	sfud_err  err;
	uint16_t *pos = 0;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;
	
	if(0 == vSupplierManagePos) {					//恢复出厂设置状态了或者写满一个循环 重新擦除扇区 1Mbyte
		sfud_erase(sfud_dev, SUPPLIER_BASE_ADDR, 0x100000);
	}

	pos = (uint16_t *)write_data;
	*pos = vSupplierManagePos + 1;
	
	err = sfud_write(sfud_dev, SUPPLIER_BASE_ADDR+vSupplierManagePos*0x1000, size, write_data); //写数据

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}
	
	rt_kprintf("write vSupplierManagePos=%d %d\n", vSupplierManagePos+1, size);
	
	vSupplierManagePos = (vSupplierManagePos + 1) % REPEAT_WRITE_LENTH;
	
	return RT_EOK;
}

//从存储器读取蛋品源头信息操作
rt_err_t read_supplier_manage_information(size_t size, uint8_t  *read_data)
{
	sfud_err  err;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;   			//描述符错误

	if(0xffff == vSupplierManagePos) return -RT_EEMPTY; //存储为空
	
	err = sfud_read(sfud_dev, SUPPLIER_BASE_ADDR+vSupplierManagePos*0x1000, size, read_data);
	
	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}	
	
	return RT_EOK;
}

//当前配方的运行参数写入存储器操作
rt_err_t write_submit_recipe_information(size_t size, uint8_t  *write_data)
{
	sfud_err  err;
	uint16_t *pos = 0;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;

	if(0 == vsubmitRecipePos) {					//恢复出厂设置状态了或者写满一个循环 重新擦除扇区 1Mbyte
		sfud_erase(sfud_dev, SUBMIT_RECIPE_BASE_ADDR, 0x100000);
	}

	pos = (uint16_t *)write_data;
	*pos = vsubmitRecipePos + 1;

	err = sfud_write(sfud_dev, SUBMIT_RECIPE_BASE_ADDR+vsubmitRecipePos*0x1000, size, write_data); //写数据

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}
	
	rt_kprintf("write vsubmitRecipePos = %d %d\n", vsubmitRecipePos+1, size);
	
	vsubmitRecipePos = (vsubmitRecipePos + 1) % REPEAT_WRITE_LENTH;			//更新读写地址
	
	return RT_EOK;
}
//从存储器读取当前配方的运行参数操作
rt_err_t read_work_recipe_information(size_t size, uint8_t  *read_data)
{
	sfud_err  err;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;   			//描述符错误

	if(0xffff == vsubmitRecipePos) return -RT_EEMPTY; 		//存储为空

	err = sfud_read(sfud_dev, SUBMIT_RECIPE_BASE_ADDR+vsubmitRecipePos*0x1000, size, read_data);

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}	
	
	return RT_EOK;
}

//班组参数写入存储器操作
rt_err_t write_team_group_information(size_t size, uint8_t  *write_data)
{
	sfud_err  err;
	uint16_t *pos = 0;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;

	if(0 == vGroupManagePos) {					//恢复出厂设置状态了或者写满一个循环 重新擦除扇区 1Mbyte
		sfud_erase(sfud_dev, TEAM_GROUP_BASE_ADDR, 0x100000);
	}

	pos = (uint16_t *)write_data;
	*pos = vGroupManagePos + 1;

	err = sfud_write(sfud_dev, TEAM_GROUP_BASE_ADDR+vGroupManagePos*0x1000, size, write_data); //写数据

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}
	
	rt_kprintf("write vGroupManagePos = %d %d\n", vGroupManagePos+1, size);
	
	vGroupManagePos = (vGroupManagePos + 1) % REPEAT_WRITE_LENTH;			//更新读写地址
	
	return RT_EOK;
}
//从存储器读取班组参数操作
rt_err_t read_team_group_information(size_t size, uint8_t  *read_data)
{
	sfud_err  err;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;   			//描述符错误

	if(0xffff == vGroupManagePos) return -RT_EEMPTY; 		//存储为空

	err = sfud_read(sfud_dev, TEAM_GROUP_BASE_ADDR+vGroupManagePos*0x1000, size, read_data);

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}	
	
	return RT_EOK;
}

//配方管理列表写入存储器操作
rt_err_t write_recipe_list_information(size_t size, uint8_t  *write_data)
{
	sfud_err  err;
	uint16_t *pos = 0;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;

	if(0 == vRecipeManagePos) {					//恢复出厂设置状态了或者写满一个循环 重新擦除扇区 4Mbyte
		sfud_erase(sfud_dev, RECIPE_MANAGER_BASE_ADDR, 0x400000);
	}

	pos = (uint16_t *)write_data;
	*pos = vRecipeManagePos + 1;

	err = sfud_write(sfud_dev, RECIPE_MANAGER_BASE_ADDR+vRecipeManagePos*0x4000, size, write_data); //写数据

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}
	
	rt_kprintf("write vRecipeListPos = %d %d\n", vRecipeManagePos+1, size);
	
	vRecipeManagePos = (vRecipeManagePos + 1) % REPEAT_WRITE_LENTH;			//更新读写地址
	
	return RT_EOK;
}
//从存储器读取配方管理列表操作
rt_err_t read_recipe_list_information(size_t size, uint8_t  *read_data)
{
	sfud_err  err;
	
	if(sfud_dev == RT_NULL) return -RT_ERROR;   			//描述符错误

	if(0xffff == vRecipeManagePos) return -RT_EEMPTY; 		//存储为空

	err = sfud_read(sfud_dev, RECIPE_MANAGER_BASE_ADDR+vRecipeManagePos*0x4000, size, read_data);

	if(err != SFUD_SUCCESS){
		return -RT_ERROR;
	}	
	
	return RT_EOK;
}

/* 设备管理-安装调试-包装线调试 */
void packageLineDebugPage(int index, int len)
{
	int i = 0;
	
	if((index>=5580)&&(index<5645)){
		if((index>=5580)&&(index<5600)){ 					//包装通道选择
			for(i=5580; i<5600; i++){
				if(index != i) u_RegHoldingPtr[i] = 0;
			}
		}else if((index>=5600)&&(index<5604)){ 	  //主轨道选择
			for(i=5600; i<5604; i++){
				if(index != i) u_RegHoldingPtr[i] = 0;
			}
		}else if((index>=5611)&&(index<5614)){  	//缓存层选择
			for(i=5611; i<5614; i++){
				if(index != i) u_RegHoldingPtr[i] = 0;
			}
		} else {
			if(index==5626){												//动作测试 设备选择									
				u_RegHoldingPtr[5633] = 0; 
				u_RegHoldingPtr[5640] = 0; 
			}else if(index==5633){
				u_RegHoldingPtr[5626] = 0; 
				u_RegHoldingPtr[5640] = 0; 
			}else if(index==5640){
				u_RegHoldingPtr[5626] = 0; 
				u_RegHoldingPtr[5633] = 0; 
			}
			
			if(index==5627){												//蛋托输送
				u_RegHoldingPtr[index+1] = 0; 
			}else if(index==5628){
				u_RegHoldingPtr[index-1] = 0;
			}
	
			if(index==5634){												//接受爪
				u_RegHoldingPtr[index+1] = 0; 
			}else if(index==5635){
				u_RegHoldingPtr[index-1] = 0;
			}
	
			if(index==5641){												//移动缓存
				u_RegHoldingPtr[index+1] = 0; 
			}else if(index==5641){
				u_RegHoldingPtr[index-1] = 0;
			}
			if(index==5643){
				u_RegHoldingPtr[index+1] = 0; 
			}else if(index==5644){
				u_RegHoldingPtr[index-1] = 0;
			}
		}
	}
}

/* 设备管理-安装调试-分级线调试 */
void gradeLineDebugPage(int index, int len)
{
	if((index>=5529)&&(index<5533)){
		if(index==5529){
			u_RegHoldingPtr[5530] = 0; 
		}else if(index==5530){
			u_RegHoldingPtr[5529] = 0;
		}

		if(index==5532){
			u_RegHoldingPtr[5533] = 0; 
		}else if(index==5533){
			u_RegHoldingPtr[5532] = 0;
		}
	}
}

/*************************** 蛋品源头管理开始 ***************************/

/* 蛋品源头-源头管理-编辑源头名称 */
void supplierManagerEditPage(int index, int len)
{
	int n = 0, pos;
	
	if(index == 0){  //点击蛋品源头页面
		if(pageNumberLast == 40){
			multiPageFunc = 0;
			multiPageCount = (u_RegHoldingPtr[1482] - 1) % 5;
			for(n=0; n<10; n++){
				pos = multiPageCount*10 + n;
				rt_memcpy(&u_RegHoldingPtr[1490+n*10], &supplierList.supplier[pos].name[0], 20);
			}
		}
	}
	if(index == 1480){
		multiPageFunc = 2;
		n = u_RegHoldingPtr[1480] - 1;
		pos = multiPageCount*10 + n;
		rt_memcpy(&u_RegHoldingPtr[2170], &supplierList.supplier[pos].name[0], 20);
	}else if(index == 1481){
		multiPageFunc = 3;
	}else if(index == 1484){
		if(u_RegHoldingPtr[1484] == 1){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 4) % 5;
			u_RegHoldingPtr[1482] = multiPageCount+1;
		}else if(u_RegHoldingPtr[1484] == 2){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 1) % 5;
			u_RegHoldingPtr[1482] = multiPageCount+1;			
		}
	}

	switch(multiPageFunc)
	{
		case 2:		//编辑蛋品源头
			if(index == 2160){
				if(1 == u_RegHoldingPtr[index]){ 	//确定
					if(u_RegHoldingPtr[2170] != 0x0){
						n = u_RegHoldingPtr[1480] - 1;
						pos = multiPageCount*10 + n;
						rt_memcpy(&supplierList.supplier[pos].name[0], &u_RegHoldingPtr[2170], 20);
						rt_memcpy(&u_RegHoldingPtr[1490+n*10], &u_RegHoldingPtr[2170], 20);
						write_supplier_manage_information(sizeof(SupplierManageList), (uint8_t  *)&supplierList);
					}
				}
			}
			break;
		case 3:		//删除蛋品源头
			if(index == 2190){
				if(1 == u_RegHoldingPtr[index]){ 	//确定
					n = u_RegHoldingPtr[1481] - 1;
					pos = multiPageCount*10+n;
					rt_memset(&supplierList.supplier[pos].name[0], 0, 20);
					rt_memset(&u_RegHoldingPtr[1490+n*10], 0, 20);
					write_supplier_manage_information(sizeof(SupplierManageList), (uint8_t  *)&supplierList);
				}
			}
			break;
		case 4:		//蛋品源头列表翻页
			for(n=0; n<10; n++){
				pos = multiPageCount*10 + n;
				rt_memcpy(&u_RegHoldingPtr[1490+n*10], &supplierList.supplier[pos].name[0], 20);
			}
			break;
		default: break;
	}
}
/* 首页-批次开始-蛋品源头选择 */
void supplierManagerSelectPage(int index, int len)
{
	uint8_t n = 0, pos;
	
	if(index == 0){  //点击蛋品源头选择页面
		multiPageFunc = 0;
		multiPageCount = (u_RegHoldingPtr[1482] - 1) % 5;
		for(n=0; n<10; n++){
			pos = multiPageCount*10 + n;
			rt_memcpy(&u_RegHoldingPtr[1490+n*10], &supplierList.supplier[pos].name[0], 20);
		}
	}
	if(index == 1484){
		if(1 == u_RegHoldingPtr[index]){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 4) % 5;
			u_RegHoldingPtr[1482] = multiPageCount+1;
		}else if(2 == u_RegHoldingPtr[index]){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 1) % 5;
			u_RegHoldingPtr[1482] = multiPageCount+1;
		}
	}else if(index == 320){
		multiPageFunc = 5;
	}
	
	switch(multiPageFunc)
	{
		case 4:		//蛋品源头列表翻页
			if(index == 1484){
				for(n=0; n<10; n++){
					pos = multiPageCount*10 + n;
					rt_memcpy(&u_RegHoldingPtr[1490+n*10], &supplierList.supplier[pos].name[0], 20);
				}
			}
			break;
		case 5:		//蛋品源头列表选择
			if(index == 320){
				n = u_RegHoldingPtr[320] - 1;
				pos = multiPageCount*10 + n;
				rt_memcpy(&u_RegHoldingPtr[290], &supplierList.supplier[pos].name[0], 20);
			}
			break;
		default: break;
	}
}
/* 首页-批次开始 */
void batchStartPage(int index, int len)
{
	struct tm *timeInfo;
	time_t now;
	char batchNo[20];

	if(index == 0){  //点击批次开始页面
		if(pageNumberLast == 20){  //可能需要生成新的批次号
			now = time(RT_NULL);
			timeInfo = localtime(&now);
			strftime(batchNo, sizeof(batchNo), "%Y%m%d%H%M%S", timeInfo);
			rt_memcpy(&u_RegHoldingPtr[280], batchNo, strlen(batchNo));
			rt_memset(&u_RegHoldingPtr[290], 0, 20);
		}
	}
	
	if(index == 270){
		multiPageFunc = 2;
	}
	switch(multiPageFunc)
	{
		case 2:
			if(index == 270){
				if(1 == u_RegHoldingPtr[index]){
					rt_memcpy(&workRecipe.batchName[0], &u_RegHoldingPtr[280], 20);
					rt_memcpy(&workRecipe.supplier.name[0], &u_RegHoldingPtr[290], 20);
					rt_memcpy(&u_RegHoldingPtr[130], &workRecipe.batchName[0], 20);
					rt_memcpy(&u_RegHoldingPtr[100], &workRecipe.supplier.name[0], 20);
				}
			}
			break;
		default: break;
	}
}

/* 首页-批次结束 */
void batchStopPage(int index, int len)
{
	if(index == 270){
		rt_memset(&workRecipe.batchName[0], 0, 20);
		rt_memset(&workRecipe.supplier.name[0], 0, 20);
		rt_memset(&u_RegHoldingPtr[130], 0, 20);
		rt_memset(&u_RegHoldingPtr[100], 0, 20);
	}	
}
/*************************** 蛋品源头管理结束 ***************************/


/*************************** 包装盒管理开始 ***************************/

void updatePackageBoxPage(uint8_t page, int n)
{
	int pos;
	
	pos = page*10+n;
	rt_memcpy(&u_RegHoldingPtr[4480+n*10], &boxList.box[pos].name[0], 20);
	u_RegHoldingPtr[4380+n*7] = boxList.box[pos].row;
	u_RegHoldingPtr[4381+n*7] = boxList.box[pos].col;
	u_RegHoldingPtr[4382+n*7] = boxList.box[pos].autoTray;
	u_RegHoldingPtr[4383+n*7] = boxList.box[pos].trayCheck;
	u_RegHoldingPtr[4384+n*7] = boxList.box[pos].packBoxPost1;
	u_RegHoldingPtr[4385+n*7] = boxList.box[pos].packBoxPost8;
	u_RegHoldingPtr[4386+n*7] = boxList.box[pos].transportStart;
}
void editPackageBoxPage(uint8_t page, int n)
{
	int i = 0, pos;
	
	pos = page*10 + n;
	rt_memcpy(&u_RegHoldingPtr[4780], &boxList.box[pos].name[0], 20);
	u_RegHoldingPtr[4700] = boxList.box[pos].row;
	u_RegHoldingPtr[4701] = boxList.box[pos].col;
	u_RegHoldingPtr[4705] = boxList.box[pos].packBoxPost1;
	u_RegHoldingPtr[4706] = boxList.box[pos].packBoxPost2;
	u_RegHoldingPtr[4707] = boxList.box[pos].packBoxPost3;
	u_RegHoldingPtr[4708] = boxList.box[pos].packBoxPost4;
	u_RegHoldingPtr[4709] = boxList.box[pos].packBoxPost5;
	u_RegHoldingPtr[4710] = boxList.box[pos].packBoxPost6;
	u_RegHoldingPtr[4711] = boxList.box[pos].packBoxPost7;
	u_RegHoldingPtr[4712] = boxList.box[pos].packBoxPost8;
	u_RegHoldingPtr[4702] = boxList.box[pos].calibChannel;
	u_RegHoldingPtr[4703] = boxList.box[pos].autoTray;
	u_RegHoldingPtr[4713] = boxList.box[pos].transportStart;
	u_RegHoldingPtr[4704] = boxList.box[pos].trayCheck;
	u_RegHoldingPtr[4714] = boxList.box[pos].chainLinkCnt;
	for(i=0; i<20; i++){
		u_RegHoldingPtr[4740+i] = boxList.box[pos].originOffset[i];
	}	
}
void savePackageBoxPage(int pos)
{
	int i=0;
	
	rt_memcpy(&boxList.box[pos].name[0], &u_RegHoldingPtr[4780], 20);
	boxList.box[pos].row 					= u_RegHoldingPtr[4700];
	boxList.box[pos].col 					= u_RegHoldingPtr[4701];
	boxList.box[pos].packBoxPost1 = u_RegHoldingPtr[4705];
	boxList.box[pos].packBoxPost2 = u_RegHoldingPtr[4706];
	boxList.box[pos].packBoxPost3 = u_RegHoldingPtr[4707];
	boxList.box[pos].packBoxPost4 = u_RegHoldingPtr[4708];
	boxList.box[pos].packBoxPost5 = u_RegHoldingPtr[4709];
	boxList.box[pos].packBoxPost6 = u_RegHoldingPtr[4710];
	boxList.box[pos].packBoxPost7 = u_RegHoldingPtr[4711];
	boxList.box[pos].packBoxPost8 = u_RegHoldingPtr[4712];
	boxList.box[pos].calibChannel = u_RegHoldingPtr[4702];
	boxList.box[pos].autoTray 		= u_RegHoldingPtr[4703];
	boxList.box[pos].transportStart = u_RegHoldingPtr[4713];
	boxList.box[pos].trayCheck 		= u_RegHoldingPtr[4704];
	boxList.box[pos].chainLinkCnt = u_RegHoldingPtr[4714];
	for(i=0; i<20; i++){
		boxList.box[pos].originOffset[i] = u_RegHoldingPtr[4740+i];
	}
	write_package_box_information(sizeof(PackageBoxList), (uint8_t  *)&boxList);
}
void deletePackageBoxPage(uint8_t page, int n)
{
	int pos;
	
	pos = page*10 + n;
	rt_memset(&u_RegHoldingPtr[4480+n*10], 0, 20);
	u_RegHoldingPtr[4380+n*7] = 0;
	u_RegHoldingPtr[4381+n*7] = 0;
	u_RegHoldingPtr[4382+n*7] = 0;
	u_RegHoldingPtr[4383+n*7] = 0;
	u_RegHoldingPtr[4384+n*7] = 0;
	u_RegHoldingPtr[4385+n*7] = 0;
	u_RegHoldingPtr[4386+n*7] = 0;
	rt_memset(&boxList.box[pos].name[0], 0, sizeof(PackageBoxInformation));
	write_package_box_information(sizeof(PackageBoxList), (uint8_t  *)&boxList);	
}
/* 生产管理-包装管理-编辑包装盒 */
void packageBoxPage(int index, int len)
{
	int n=0, pos;

	if(index == 0){  //点击包装盒编辑页面
		if(pageNumberLast == 40){
			multiPageFunc = 0;
			multiPageCount = (u_RegHoldingPtr[4450] - 1) % 2;
			for(n=0; n<10; n++){
				updatePackageBoxPage(multiPageCount, n);
			}
		}
	}

	if(index==5626){												//动作测试 设备选择									
		u_RegHoldingPtr[5633] = 0; 
	}else if(index==5633){
		u_RegHoldingPtr[5626] = 0; 
	}

	if(index==5627){												//蛋托输送 正反转选择
		u_RegHoldingPtr[index+1] = 0; 
	}else if(index==5628){
		u_RegHoldingPtr[index-1] = 0;
	}
	
	if(index==5634){												//接受爪 正反转选择
		u_RegHoldingPtr[index+1] = 0;
	}else if(index==5635){
		u_RegHoldingPtr[index-1] = 0;
	}

	if(index == 4452){
		if(u_RegHoldingPtr[index] > 0){
			multiPageFunc = 2;
			n = u_RegHoldingPtr[index] - 1;
			editPackageBoxPage(multiPageCount, n);
		}
	}else if(index == 4453){
		multiPageFunc = 3;
	}else if(index == 4454){
		multiPageFunc = 4;
		multiPageCount = (multiPageCount + 1) % 2;
		u_RegHoldingPtr[4450] = multiPageCount+1;
	}

	switch(multiPageFunc)
	{
		case 2:		//编辑包装盒
			if(index == 4731){
				if(1 == u_RegHoldingPtr[index]){ 		//确定
					if(u_RegHoldingPtr[4780] != 0x0){ //名称不为空
						n = u_RegHoldingPtr[4452] - 1;
						pos = multiPageCount*10 + n;
						savePackageBoxPage(pos);					
						for(n=0; n<10; n++){
							updatePackageBoxPage(multiPageCount, n);
						}
					}
				}
			}
			break;
		case 3:		//删除包装盒
			if(index == 4800){
				if(1 == u_RegHoldingPtr[index]){ 	//确定
					n = u_RegHoldingPtr[4453] - 1;
					deletePackageBoxPage(multiPageCount, n);
				}
			}
			break;
		case 4:		//包装盒列表翻页
			for(n=0; n<10; n++){
				updatePackageBoxPage(multiPageCount, n);
			}
			break;
		default: break;
	}
}
/* 首页-发起任务-包装盒选择 */
void packageBoxSelectPage(int index, int len)
{
	uint8_t n = 0, pos;
	
	if(index == 0){	//点击包装盒选择页面
		multiPageFunc = 0;
		multiPageCount = (u_RegHoldingPtr[4450] - 1) % 2;
		for(n=0; n<10; n++){
			updatePackageBoxPage(multiPageCount, n);
		}
	}
	if(index == 4454){
		multiPageFunc = 4;
		multiPageCount = (multiPageCount + 1) % 2;
		u_RegHoldingPtr[4450] = multiPageCount+1;
	}else if(index == 3710){
		multiPageFunc = 5;
	}
	
	switch(multiPageFunc)
	{
		case 4:		//包装盒列表翻页
			for(n=0; n<10; n++){
				updatePackageBoxPage(multiPageCount, n);
			}
			break;
		case 5:		//包装盒列表选择
			if(index == 3710){
				n = u_RegHoldingPtr[3710] - 1;
				pos = multiPageCount*10 + n;
				rt_memcpy(&u_RegHoldingPtr[3660], &boxList.box[pos].name[0], 20);
			}
			break;
		default: break;
	}
}

/*************************** 包装盒管理结束 ***************************/

/*************************** 蛋品管理开始 ***************************/
void updateProductManagePage(uint8_t page, int n)
{
	int i=0, pos, icon=0;
	
	pos = page * 10 + n;
	rt_memcpy(&u_RegHoldingPtr[4050+n*10], &productList.product[pos].name[0], 20);
	
	//称重等级
	icon = 0;
	for(i=0; i<6; i++){
		if(getbitv(productList.product[pos].weighLevel, i)){
			u_RegHoldingPtr[3740+icon+n*22] = i + 1;
			icon += 1;
		}
	}
	for(i=icon; i<6; i++){
		u_RegHoldingPtr[3740+i+n*22] = 0;
	}
	
	//颜色等级
	icon = 0;
	for(i=0; i<5; i++){
		if(getbitv(productList.product[pos].colorLevel, i)){
			u_RegHoldingPtr[3746+icon+n*22] = i + 1;
			icon += 1;
		}
	}
	for(i=icon; i<5; i++){
		u_RegHoldingPtr[3746+i+n*22] = 0;
	}
	
	//裂纹等级
	icon = 0;
	for(i=0; i<5; i++){
		if(getbitv(productList.product[pos].crackLevel, i)){
			u_RegHoldingPtr[3751+icon+n*22] = i + 1;
			icon += 1;
		}
	}
	for(i=icon; i<5; i++){
		u_RegHoldingPtr[3751+i+n*22] = 0;
	}
	
	//脏蛋等级
	icon = 0;
	for(i=0; i<4; i++){
		if(getbitv(productList.product[pos].dirtyLevel, i)){
			u_RegHoldingPtr[3756+icon+n*22] = i + 1;
			icon += 1;
		}
	}
	for(i=0; i<4; i++){
		u_RegHoldingPtr[3756+i+n*22] = 0;
	}
	
	//异常蛋
	icon = 0;
	for(i=0; i<2; i++){
		if(getbitv(productList.product[pos].bloodLevel, i)){
			u_RegHoldingPtr[3760+icon+n*22] = i + 1;
			icon += 1;
		}
	}
	for(i=icon; i<2; i++){
		u_RegHoldingPtr[3760+i+n*22] = 0;
	}
	
	u_RegHoldingPtr[3965+n] = productList.product[pos].printNumber;
}

void editProductManagePage(uint8_t page, int n)
{
	int i=0, pos;
	
	pos = page*10 + n;
	rt_memcpy(&u_RegHoldingPtr[4350], &productList.product[pos].name[0], 20);
	for(i=0; i<6; i++){
		if(getbitv(productList.product[pos].weighLevel, i)){
			u_RegHoldingPtr[4300+i] = 1;
		}else u_RegHoldingPtr[4300+i] = 0;
	}
	u_RegHoldingPtr[4329] = productList.product[pos].weighLow;
	u_RegHoldingPtr[4330] = productList.product[pos].weighHigh;
	
	for(i=0; i<5; i++){
		if(getbitv(productList.product[pos].colorLevel, i)){
			u_RegHoldingPtr[4307+i] = 1;
		}else u_RegHoldingPtr[4307+i] = 0;
	}
	u_RegHoldingPtr[4331] = productList.product[pos].colorLow;
	u_RegHoldingPtr[4332] = productList.product[pos].colorHigh;
	
	for(i=0; i<5; i++){
		if(getbitv(productList.product[pos].crackLevel, i)){
			u_RegHoldingPtr[4313+i] = 1;
		}else u_RegHoldingPtr[4313+i] = 0;
	}
	u_RegHoldingPtr[4333] = productList.product[pos].crackLow;
	u_RegHoldingPtr[4334] = productList.product[pos].crackHigh;
	
	for(i=0; i<4; i++){
		if(getbitv(productList.product[pos].dirtyLevel, i)){
			u_RegHoldingPtr[4319+i] = 1;
		}else u_RegHoldingPtr[4319+i] = 0;
	}
	u_RegHoldingPtr[4335] = productList.product[pos].dirtyLow;
	u_RegHoldingPtr[4336] = productList.product[pos].dirtyHigh;	
	
	for(i=0; i<2; i++){
		if(getbitv(productList.product[pos].bloodLevel, i)){
			u_RegHoldingPtr[4324+i] = 1;
		}else u_RegHoldingPtr[4324+i] = 0;
	}
	u_RegHoldingPtr[4306] = productList.product[pos].printNumber;		
	
	for(i=0; i<2; i++){
		if(getbitv(productList.product[pos].mark, i)){
			u_RegHoldingPtr[4326+i] = 1;
		}else u_RegHoldingPtr[4326+i] = 0;
	}	
}
void saveProductManagePage(int pos)
{
	int i=0;
	
	rt_memcpy(&productList.product[pos].name[0], &u_RegHoldingPtr[4350], 20);
	
	productList.product[pos].weighLevel = 0;
	for(i=0; i<6; i++){
		if(1 == u_RegHoldingPtr[4300+i]) setbitv(productList.product[pos].weighLevel, i);
	}
	productList.product[pos].weighLow = u_RegHoldingPtr[4329];
	productList.product[pos].weighHigh = u_RegHoldingPtr[4330];
	
	productList.product[pos].colorLevel = 0;
	for(i=0; i<5; i++){
		if(1 == u_RegHoldingPtr[4307+i]) setbitv(productList.product[pos].colorLevel, i);
	}
	productList.product[pos].colorLow = u_RegHoldingPtr[4331];
	productList.product[pos].colorHigh = u_RegHoldingPtr[4332];
	
	productList.product[pos].crackLevel = 0;
	for(i=0; i<5; i++){
		if(1 == u_RegHoldingPtr[4313+i]) setbitv(productList.product[pos].crackLevel, i);
	}
	productList.product[pos].crackLow = u_RegHoldingPtr[4333];
	productList.product[pos].crackHigh = u_RegHoldingPtr[4334];
	
	productList.product[pos].dirtyLevel = 0;
	for(i=0; i<4; i++){
		if(1 == u_RegHoldingPtr[4319+i]) setbitv(productList.product[pos].dirtyLevel, i);
	}
	productList.product[pos].dirtyLow = u_RegHoldingPtr[4335];
	productList.product[pos].dirtyHigh = u_RegHoldingPtr[4336];	
	
	productList.product[pos].bloodLevel = 0;
	for(i=0; i<2; i++){
		if(1 == u_RegHoldingPtr[4324+i]) setbitv(productList.product[pos].bloodLevel, i);
	}
	
	productList.product[pos].printNumber = u_RegHoldingPtr[4306];
	
	productList.product[pos].mark = 0;
	for(i=0; i<2; i++){
		if(1 == u_RegHoldingPtr[4326+i]) setbitv(productList.product[pos].mark, i);
	}	
	
	write_product_manage_information(sizeof(ProductManageList), (uint8_t  *)&productList);
}
void deleteProductManagePage(uint8_t page, int n)
{
	int pos, i;
	
	pos = page*10 + n;
	rt_memset(&u_RegHoldingPtr[4050+n*10], 0, 20);
	for(i=0; i<6; i++){
		u_RegHoldingPtr[3740+i+n*22] = 0;
	}
	for(i=0; i<5; i++){
		u_RegHoldingPtr[3746+i+n*22] = 0;
	}
	for(i=0; i<5; i++){
		u_RegHoldingPtr[3751+i+n*22] = 0;
	}
	for(i=0; i<4; i++){
		u_RegHoldingPtr[3756+i+n*22] = 0;
	}
	for(i=0; i<2; i++){
		u_RegHoldingPtr[3760+i+n*22] = 0;
	}
	u_RegHoldingPtr[3990+n] = 0;
	rt_memset(&productList.product[pos].name[0], 0, sizeof(ProductManageInformation));
	write_package_box_information(sizeof(ProductManageList), (uint8_t  *)&productList);
}
/* 生产管理-蛋品管理-编辑或删除 */
void productManagePage(int index, int len)
{
	int n=0, pos;
	
	if(index == 0){
		if(pageNumberLast == 40){
			multiPageFunc = 0;
			multiPageCount = (u_RegHoldingPtr[3960] - 1) % 3;
		}
		for(n=0; n<10; n++){
			updateProductManagePage(multiPageCount, n);
		}
	}
	if(index == 3962){
		if(u_RegHoldingPtr[index] > 0){
			multiPageFunc = 2;
			n = u_RegHoldingPtr[index] - 1;
			editProductManagePage(multiPageCount, n);
		}
	}else if(index == 3963){
		multiPageFunc = 3;
	}else if(index == 3964){
		if(1== u_RegHoldingPtr[index]){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 2) % 3;
			u_RegHoldingPtr[3960] = multiPageCount+1;
		}else if(2 == u_RegHoldingPtr[index]){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 1) % 3;
			u_RegHoldingPtr[3960] = multiPageCount+1;
		}
	}
	switch(multiPageFunc)
	{
		case 2:		//编辑产品
			if(index == 4328){
				if(1 == u_RegHoldingPtr[index]){
					if(u_RegHoldingPtr[4350] != 0x0){ //名称不为空
						n = u_RegHoldingPtr[3962] - 1;
						pos = multiPageCount*10 + n;
						saveProductManagePage(pos);
						for(n=0; n<10; n++){
							updateProductManagePage(multiPageCount, n);
						}
					}
				}
			}
			break;
		case 3:		//删除产品
			if(index == 4370){
				if(1 == u_RegHoldingPtr[index]){ 	//确定
					n = u_RegHoldingPtr[3963] - 1;
					deleteProductManagePage(multiPageCount, n);
				}
			}
			break;
		case 4:		//产品列表翻页
			for(n=0; n<10; n++){
				updateProductManagePage(multiPageCount, n);
			}
			break;
		default: break;
	}
}
void productManageSelectPage(int index, int len)
{
	uint8_t n = 0, pos;
	
	if(index == 0){	//点击产品选择页面
		multiPageFunc = 0;
		multiPageCount = (u_RegHoldingPtr[3960] - 1) % 3;
		for(n=0; n<10; n++){
			updateProductManagePage(multiPageCount, n);
		}
	}
	if(index == 3964){
		if(1 == u_RegHoldingPtr[index]){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 2) % 3;
			u_RegHoldingPtr[3960] = multiPageCount+1;
		}else if(2 == u_RegHoldingPtr[index]){
			multiPageFunc = 4;
			multiPageCount = (multiPageCount + 1) % 3;
			u_RegHoldingPtr[3960] = multiPageCount+1;
		}
	}else if(index == 3690){
		multiPageFunc = 5;
	}

	switch(multiPageFunc)
	{
		case 4:		//产品列表翻页
			for(n=0; n<10; n++){
				updateProductManagePage(multiPageCount, n);
			}
			break;
		case 5:		//产品列表选择
			n = u_RegHoldingPtr[3690] - 1;
			pos = multiPageCount*10 + n;
			rt_memcpy(&u_RegHoldingPtr[3650], &productList.product[pos].name[0], 20);
			break;
		default: break;
	}
}
/*************************** 蛋品管理结束 ***************************/

/*************************** 配方管理开始 ***************************/
#include <string.h>
#include <stdio.h>
//更新单个配方中的分级规则列表
void updateSortRulePage(uint8_t page, int n)
{
	int pos, i;
	char buf[100], tbuf[10];
	
	pos = page*10+n;
	rt_memcpy(&u_RegHoldingPtr[2610+n*40], &selectRecipe.sortRule[pos].productName, 20);//产品名
	rt_memcpy(&u_RegHoldingPtr[2620+n*40], &selectRecipe.sortRule[pos].packageName, 20);//包装盒名
	rt_memset(buf, 0, 50);
	for(i=0; i<SORTER_USED_MAX; i++){
		if(getbitv(selectRecipe.sortRule[pos].packageLine, i)){	//包装线显示
			rt_sprintf(tbuf, "A%02d,",i+1);
			strcat(buf, tbuf);
		}
	}
	rt_memcpy(&u_RegHoldingPtr[2630+n*40], buf, 20);
	setModbusHoldingRegU32(2510+n*4, selectRecipe.sortRule[pos].targetBox);
	setModbusHoldingRegU32(2512+n*4, selectRecipe.sortRule[pos].targetSC);
//	u_RegHoldingPtr[TEST_ADDR+n] = tempRecipe.sortRule[pos].remind;//stop
}
//编辑配方中的单条分级规则
void editSortRulePage(uint8_t page, int n)
{
	int i, pos;
		
	pos = page*10 + n;
	u_RegHoldingPtr[3600] = selectRecipe.sortRule[pos].priority;
	rt_memcpy(&u_RegHoldingPtr[3650], &selectRecipe.sortRule[pos].productName, 20); //蛋品名
	rt_memcpy(&u_RegHoldingPtr[3660], &selectRecipe.sortRule[pos].packageName, 20);	//包装盒名	
	for(i=0; i<SORTER_USED_MAX; i++){	//包装线 通道
		if(getbitv(selectRecipe.sortRule[pos].packageLine, i)){
			u_RegHoldingPtr[3602+i] = 1;
		}else{
			u_RegHoldingPtr[3602+i] = 0;
		}
	}
	setModbusHoldingRegU32(3640, selectRecipe.sortRule[pos].targetBox);
	setModbusHoldingRegU32(3642, selectRecipe.sortRule[pos].targetSC);
	u_RegHoldingPtr[3622] = selectRecipe.sortRule[pos].remind;
	u_RegHoldingPtr[3623] = selectRecipe.sortRule[pos].stop;
}
//保存单个配方中的分级规则
void saveSortRulePage(int pos)
{
	int i=0;
	
	selectRecipe.sortRule[pos].priority = u_RegHoldingPtr[3600]; //优先级
	rt_memcpy(&selectRecipe.sortRule[pos].productName, &u_RegHoldingPtr[3650], 20); //蛋品名
	rt_memcpy(&selectRecipe.sortRule[pos].packageName, &u_RegHoldingPtr[3660], 20);	//包装盒名
	
	for(i=0; i<SORTER_USED_MAX; i++){	//包装线 通道
		if(1 == u_RegHoldingPtr[3602+i]){
			setbitv(selectRecipe.sortRule[pos].packageLine, i);
		}else{
			clrbitv(selectRecipe.sortRule[pos].packageLine, i);
		}
	}

	selectRecipe.sortRule[pos].targetBox = getModbusHoldingRegU32(3640);
	selectRecipe.sortRule[pos].targetSC = getModbusHoldingRegU32(3642);
	selectRecipe.sortRule[pos].remind = u_RegHoldingPtr[3622];
	selectRecipe.sortRule[pos].stop = u_RegHoldingPtr[3623];
}

/*x 生产管理-分级生产-配方管理-详情 单个配方编辑页面的分级规则操作 编辑或删除*/
void sortRulePage(int index, int len)
{
	int n=0, pos;
	
	if(index == 2501){
		if(u_RegHoldingPtr[index] > 0){
			multiPageFuncEx = 2;
			n = u_RegHoldingPtr[2501] - 1;
			editSortRulePage(multiPageCountEx, n);
		}
	}else if(index == 2502){
		multiPageFuncEx = 3;
	}else if(index == 2505){
		multiPageFuncEx = 4;
		multiPageCountEx = (multiPageCountEx + 1) % 2;
		u_RegHoldingPtr[2503] = multiPageCountEx+1;
	}

	switch(multiPageFuncEx)
	{
		case 2:		//配方中编辑分级规则
			if(index == 3624){
				if(1 == u_RegHoldingPtr[3624]){ 	//确定操作
					n = u_RegHoldingPtr[2501] - 1;
					pos = multiPageCountEx*10 + n;
					saveSortRulePage(pos);					
					for(n=0; n<10; n++){
						updateSortRulePage(multiPageCountEx, n);
					}
				}
			}
			break;
		case 3:		//配方中删除分级规则
			if(index == 3730){
				if(1 == u_RegHoldingPtr[3730]){ 	//确定删除这条规则
					n = u_RegHoldingPtr[2502] - 1;
					pos = multiPageCountEx*10 + n;
					rt_memset(&u_RegHoldingPtr[2610+n*40], 0, 20);
					rt_memset(&u_RegHoldingPtr[2620+n*40], 0, 20);
					rt_memset(&u_RegHoldingPtr[2630+n*40], 0, 20);
					setModbusHoldingRegU32(2510+n*4, selectRecipe.sortRule[pos].targetBox);
					setModbusHoldingRegU32(2512+n*4, selectRecipe.sortRule[pos].targetSC);
					rt_memset(&selectRecipe.sortRule[pos], 0, sizeof(SingleRecipeParam));
				}
			}
			break;
		case 4:		//配方中分级规则列表翻页
			for(n=0; n<10; n++){
				updateSortRulePage(multiPageCountEx, n);
			}
			break;		
	}
}

//更新配方列表
void updateRecipeManagerPage(uint8_t page, int n)
{
	int pos;
	time_t time;
	
	pos = page*10+n;
	rt_memcpy(&u_RegHoldingPtr[2210+n*10], &recipeList.recipe[pos].recipeName[0], 20);
	if(u_RegHoldingPtr[2210+n*10] != 0x0){
		time = recipeList.recipe[pos].now;
		u_RegHoldingPtr[2460+n*2] = time&0xffff; 
		u_RegHoldingPtr[2460+n*2+1] = (time>>16)&0xffff;
	}else{
		u_RegHoldingPtr[2460+n*2] = 0;
		u_RegHoldingPtr[2460+n*2+1] = 0;
	}
}

void editRecipeManagerPage(void *data, uint8_t page, int n)
{
	int i=0, j=0;
	char buf[100], tbuf[10];
	SingleRecipeParam *precipe;
	
	precipe = (SingleRecipeParam *)data;
	for(i=0; i<10; i++){
		rt_memcpy(&u_RegHoldingPtr[2610+i*40], &precipe->sortRule[i].productName, 20);//产品名
		rt_memcpy(&u_RegHoldingPtr[2620+i*40], &precipe->sortRule[i].packageName, 20);//包装盒名
		rt_memset(buf, 0, 50);
		for(j=0; j<20; j++){
			if(getbitv(precipe->sortRule[i].packageLine, j)){	//包装线显示
				rt_sprintf(tbuf, "A%02d,",j+1);
				strcat(buf, tbuf);
			}
		}
		rt_memcpy(&u_RegHoldingPtr[2630+i*40], buf, 20);
		setModbusHoldingRegU32(2510+i*4, precipe->sortRule[i].targetBox);
		setModbusHoldingRegU32(2512+i*4, precipe->sortRule[i].targetSC);
//		u_RegHoldingPtr[TEST_ADDR+n] = precipe->sortRule[i].remind;//stop
	}
}

void saveRecipeManagerPage(int n)
{
	rt_memcpy(&selectRecipe.recipeName[0], &u_RegHoldingPtr[3000], 20);
	selectRecipe.now = time(RT_NULL);
	rt_memcpy(&recipeList.recipe[n], &selectRecipe, sizeof(SingleRecipeParam));
	write_recipe_list_information(sizeof(SortRecipeParamList), (uint8_t  *)&recipeList);
}

/* 生产管理-配方管理-编辑或删除 */
void recipeManagerPage(int index, int len)
{
	int n=0, pos;
	
	if(index == 0){  //点击配方管理编辑页面
		if(40 == pageNumberLast){
			multiPageFunc = 0;
			multiPageCount = 0;
			for(n=0; n<10; n++){
				updateRecipeManagerPage(multiPageCount, n);
			}
		}
	}

	sortRulePage(index, len);

	if(index == 2200){						//编辑单个配方
		if(u_RegHoldingPtr[index] > 0){
			multiPageFuncEx = 0;
			multiPageCountEx = (u_RegHoldingPtr[2503] - 1) % 2;
			multiPageFunc = 2;
			n = u_RegHoldingPtr[index] - 1;
			pos = multiPageCount*10 + n;
			rt_memcpy(&selectRecipe, &recipeList.recipe[pos], sizeof(SingleRecipeParam)); //读取需要编辑的这条配方参数到临时变量中
			rt_memcpy(&u_RegHoldingPtr[3000], &selectRecipe.recipeName[0], 20);						//配方名
			editRecipeManagerPage(&selectRecipe, multiPageCountEx, n); 										//把数据更新到配方编辑页面中去
		}
	}else if(index == 2201){	 //删除单个配方
		multiPageFunc = 3;
	}else if(index == 2500){   //保存单配配方
		multiPageFunc = 2;
	}

	switch(multiPageFunc)
	{
		case 2:		//编辑配方
			if(index == 2500){
				if(1 == u_RegHoldingPtr[2500]){//保存配方
					if(u_RegHoldingPtr[3000] != 0x0){ //名称不为空
						n = u_RegHoldingPtr[2200] - 1;
						pos = multiPageCount*10 + n;
						saveRecipeManagerPage(pos);					
						for(n=0; n<10; n++){
							updateRecipeManagerPage(multiPageCount, n);
						}
					}
				}
			}
			break;
		case 3:		//删除配方
			if(index == 2490){
				if(1 == u_RegHoldingPtr[2490]){ 	//确定删除
					n = u_RegHoldingPtr[2201] - 1;
					pos = multiPageCount*10 + n;
					rt_memset(&u_RegHoldingPtr[2210+n*10], 0, 20);
					u_RegHoldingPtr[2460+n*2] = 0;
					u_RegHoldingPtr[2460+n*2+1] = 0;
					rt_memset(&recipeList.recipe[pos], 0, sizeof(SingleRecipeParam));
					write_recipe_list_information(sizeof(SortRecipeParamList), (uint8_t  *)&recipeList);
				}
			}
			break;
		default: break;
	}
}

/*************************** 配方管理结束 ***************************/


/*************************** 生产配置函数 ***************************/
void submitAndSaveWorkRecipe(void)
{
	int i, needSave = 0;
	SingleRecipeParam *pRecipe;
	
	if(RT_FALSE == compare_names(&u_RegHoldingPtr[230], submitRecipe.selRecipeName, 10)) needSave = 1;
	if(submitRecipe.workSpeed != u_RegHoldingPtr[210]) needSave = 1;
	if(submitRecipe.workSpeedEx != u_RegHoldingPtr[215]) needSave = 1;
	if(1 == needSave){			//更新提交的配方并保存
		rt_memcpy(submitRecipe.selRecipeName, &u_RegHoldingPtr[230], 20);
		submitRecipe.workSpeed = u_RegHoldingPtr[210];
		submitRecipe.workSpeedEx = u_RegHoldingPtr[215];
		write_submit_recipe_information(sizeof(SubmitRecipeParam), (uint8_t *)&submitRecipe);
	}

	needSave= 0;
	pRecipe = getSingleRecipeParam(submitRecipe.selRecipeName);
	if(pRecipe == RT_NULL) return;
	for(i=0; i<RECIPE_PARAM_LIST_LEN; i++){
		if(selectRecipe.sortRule[i].priority != pRecipe->sortRule[i].priority) needSave = 1;
		if(RT_FALSE == compare_names(selectRecipe.sortRule[i].productName, pRecipe->sortRule[i].productName, 10)) needSave = 1;
		if(RT_FALSE == compare_names(selectRecipe.sortRule[i].packageName, pRecipe->sortRule[i].packageName, 10)) needSave = 1;
		if(selectRecipe.sortRule[i].packageLine != pRecipe->sortRule[i].packageLine) needSave = 1;
		if(selectRecipe.sortRule[i].targetBox != pRecipe->sortRule[i].targetBox) needSave = 1;
		if(selectRecipe.sortRule[i].targetSC != pRecipe->sortRule[i].targetSC) needSave = 1;
		if(selectRecipe.sortRule[i].remind != pRecipe->sortRule[i].remind) needSave = 1;
		if(selectRecipe.sortRule[i].stop != pRecipe->sortRule[i].stop) needSave = 1;
	}

	if(1 == needSave){     //更新配方数据列表
		for(i=0; i<RECIPE_USED_MAX; i++){
			if(RT_TRUE == compare_names(recipeList.recipe[i].recipeName, selectRecipe.recipeName, 10)){
				rt_memcpy(&recipeList.recipe[i], &selectRecipe, sizeof(SingleRecipeParam));
				break;
			}
		}
		write_recipe_list_information(sizeof(SortRecipeParamList), (uint8_t *)&recipeList);
	}
}

 /*x 首页-生产配置-配方详情 */
void workRecipeSetPage(int index, int len)
{
	int n = 0;
	SingleRecipeParam *pRecipe;

	if(index == 0){
		if((24 == u_RegHoldingPtr[index])&&(pageNumberLast == 20)){			//配方详情
			multiPageCountEx = 0;
			pRecipe = getSingleRecipeParam(submitRecipe.selRecipeName);		//根据配方名读取当前选择的这条配方参数
			if(pRecipe == RT_NULL) rt_memset(&selectRecipe, 0, sizeof(SingleRecipeParam));
			else rt_memcpy(&selectRecipe, pRecipe, sizeof(SingleRecipeParam)); //数据复制到当前选择配方临时变量中
			u_RegHoldingPtr[210] = submitRecipe.workSpeed;								//速度选择按钮
			u_RegHoldingPtr[215] = submitRecipe.workSpeedEx;							//速度变量	
			rt_memcpy(&u_RegHoldingPtr[230], &submitRecipe.selRecipeName[0], 20);//配方名
			editRecipeManagerPage(&selectRecipe, multiPageCount, n); 			//把数据更新到配方编辑页面中去
		}
	}
	if(index == 216){			//保存并提交运行配方
		/*
			1.如果只修改了速度 批次 蛋品源头等参数 则只更新submitRecipe并存储
			2.比较selectRecipe与配方列表中名字相同的这条配方是否有修改 如果修改了则需要更新recipeList中这条配方的参数并存储
		*/
		submitAndSaveWorkRecipe();
	}
	
	if(index == 2501){    		//编辑
		if(u_RegHoldingPtr[index] > 0){
			multiPageFuncEx = 2;
			n = u_RegHoldingPtr[2501] - 1;
			editSortRulePage(multiPageCountEx, n);
		}
	}else if(index == 2505){  //翻页
		multiPageFuncEx = 4;
		multiPageCountEx = (multiPageCountEx + 1) % 2;
		u_RegHoldingPtr[2503] = multiPageCountEx+1;
		for(n=0; n<10; n++){
			updateSortRulePage(multiPageCountEx, n);
		}
	}
}

/* 首页-生产配置-配方选择 */
void recipeSelectPage(int index, int len)
{
	uint8_t n = 0, pos;

	if(index == 0){	//点击产品选择页面
		multiPageFunc = 0;
		multiPageCount = 0;
		for(n=0; n<10; n++){
			updateRecipeManagerPage(multiPageCount, n);
		}
	}

	if(index == 260){ //选择确认
		n = u_RegHoldingPtr[260] - 1;
		pos = multiPageCount*10 + n;
		rt_memcpy(&selectRecipe, &recipeList.recipe[pos], sizeof(SingleRecipeParam)); //读取需要编辑的这条配方参数到临时变量中
		rt_memcpy(&u_RegHoldingPtr[230], &selectRecipe.recipeName[0], 20);						//显示选择的配方名
		editRecipeManagerPage(&selectRecipe, multiPageCount, n); 											//把数据更新到配方编辑页面中去
	}
}

/*************************** 生产配置结束 ***************************/


/*************************** 班组管理开始 ***************************/
void updateTeamOrGroupManager(int n)
{
	int pos;
	struct tm time_info;
	time_t timestamp;
	
	pos = n;
	rt_memcpy(&u_RegHoldingPtr[4900+n*10], &groupList.group[pos].groupName[0], 20);
	memset(&time_info, 0, sizeof(struct tm));
	time_info.tm_year = 2026 - 1900; 
	time_info.tm_mon  = 7 - 1;       
	time_info.tm_mday = 24;          
	time_info.tm_hour = groupList.group[pos].startHour;
	time_info.tm_min  = groupList.group[pos].startMin;
	time_info.tm_sec  = groupList.group[pos].startSecond;
	time_info.tm_isdst = -1;
	timestamp = mktime(&time_info);
	//开始时间
	u_RegHoldingPtr[5150+4*n] = timestamp&0xffff;
	u_RegHoldingPtr[5151+4*n] = (timestamp>>16)&0xffff;
	//结束时间
	time_info.tm_hour = groupList.group[pos].stopHour;
	time_info.tm_min  = groupList.group[pos].stopMin;
	time_info.tm_sec  = groupList.group[pos].stopSecond;
	timestamp = mktime(&time_info);
	u_RegHoldingPtr[5152+4*n] = timestamp&0xffff; 
	u_RegHoldingPtr[5153+4*n] = (timestamp>>16)&0xffff;
}

void editTeamOrGroupManager(int pos)
{
	rt_memcpy(&u_RegHoldingPtr[5210], &groupList.group[pos].groupName[0], 20);
	u_RegHoldingPtr[5200] = groupList.group[pos].startHour;
	u_RegHoldingPtr[5201] = groupList.group[pos].startMin;
	u_RegHoldingPtr[5202] = groupList.group[pos].startSecond;
	u_RegHoldingPtr[5203] = groupList.group[pos].stopHour;
	u_RegHoldingPtr[5204] = groupList.group[pos].stopMin;
	u_RegHoldingPtr[5205] = groupList.group[pos].stopSecond;
}

/* 生产管理-分级生产-班组管理 */
void teamOrGroupManagerPage(int index, int len)
{
	int n = 0;
	
	if(index == 0){  //点击班组管理页面
		if(pageNumberLast == 40){
			multiPageFunc = 0;
			for(n=0; n<10; n++){
				updateTeamOrGroupManager(n);
			}
		}
	}
	if(index == 4890){
		multiPageFunc = 2;
		n = u_RegHoldingPtr[4890] - 1;
		editTeamOrGroupManager(n);
	}else if(index == 4891){
		multiPageFunc = 3;
	}
	
	switch(multiPageFunc)
	{	
		case 2:		//编辑班组
			if(index == 5206){
				if(1 == u_RegHoldingPtr[index]){ 	//确定
					if(u_RegHoldingPtr[5210] != 0x0){
						n = u_RegHoldingPtr[4890] - 1;
						rt_memcpy(&groupList.group[n].groupName[0], &u_RegHoldingPtr[5210], 20);
						groupList.group[n].startHour 		= u_RegHoldingPtr[5200];
						groupList.group[n].startMin			= u_RegHoldingPtr[5201];
						groupList.group[n].startSecond  = u_RegHoldingPtr[5202];
						groupList.group[n].stopHour			= u_RegHoldingPtr[5203];
						groupList.group[n].stopMin			= u_RegHoldingPtr[5204];
						groupList.group[n].stopSecond		= u_RegHoldingPtr[5205];
						updateTeamOrGroupManager(n);
						write_team_group_information(sizeof(TeamOrGroupParamList), (uint8_t  *)&groupList);
					}
				}
			}
			break;
		case 3:		//删除班组
			if(index == 5230){
				if(1 == u_RegHoldingPtr[index]){ 	//确定
					n = u_RegHoldingPtr[4891] - 1;
					rt_memset(&groupList.group[n], 0, sizeof(SingleGroupParam));
					rt_memset(&u_RegHoldingPtr[4900+n*10], 0, 20);
					u_RegHoldingPtr[5150+4*n] = 0;
					u_RegHoldingPtr[5151+4*n] = 0;
					u_RegHoldingPtr[5152+4*n] = 0; 
					u_RegHoldingPtr[5153+4*n] = 0;
					write_team_group_information(sizeof(TeamOrGroupParamList), (uint8_t  *)&groupList);
				}
			}
			break;
		default: break;
	}
}

/*************************** 班组管理结束***************************/

/*V 生产管理界面 数据初始化 清除一些标志位 */
void externalPageAddressInit(void)
{
	time_t now;
	
	now = time(RT_NULL);   //记录开机时间
	u_RegHoldingPtr[6602] = now&0xffff; 
	u_RegHoldingPtr[6603] = (now>>16)&0xffff;
	
	rt_snprintf(cid_iemi, sizeof(cid_iemi), "%08x%08x%08x", HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2()); //UID IMEI用于表示设备唯一地址
	rt_memcpy(&u_RegHoldingPtr[6610], cid_iemi, 24);	
	
	u_RegHoldingPtr[5580] = 1;	//为一些多通道选择按钮给初始值
	u_RegHoldingPtr[5600] = 1;
	u_RegHoldingPtr[5611] = 1;
	
	u_RegHoldingPtr[0] = 20;

	workRecipe.workSpeed = submitRecipe.workSpeed;
	workRecipe.workSpeedEx = submitRecipe.workSpeedEx;
	rt_memcpy(&workRecipe.singleRecipe.recipeName[0], &submitRecipe.selRecipeName[0], 20);
	rt_memcpy(&u_RegHoldingPtr[110], &workRecipe.singleRecipe.recipeName[0], 20);//当前配方名
	
	/* 初始页面翻页编号 */
	u_RegHoldingPtr[1482] = 1;  //蛋品源头页面
	u_RegHoldingPtr[1483] = 5;
	
	u_RegHoldingPtr[2503] = 1;	//配方管理 单个配方的分级规则页面
	u_RegHoldingPtr[2504] = 2;
	
	u_RegHoldingPtr[3960] = 1;	//蛋品管理
	u_RegHoldingPtr[3961] = 3;
	
	u_RegHoldingPtr[4450] = 1;	//包装管理
	u_RegHoldingPtr[4451] = 2;
}

// 根据步科屏幕页面地址处理数据
void externalPageAddressProc(int index, int len)
{
	switch(u_RegHoldingPtr[0])
	{
		case 103: /*ok 设备管理-安装调试-分级线调试 */
			gradeLineDebugPage(index, len);
			break;
		case 104:	/*ok 设备管理-安装调试-输送线调试 */
			
			break;
		case 105: /*ok 设备管理-安装调试-包装线调试 */
			packageLineDebugPage(index, len);
			break;
		case 41:	/*ok 蛋品源头-源头管理-列表 */ 
		case 43:	/*ok 蛋品源头-源头管理-编辑 */ 
		case 44:  /*ok 蛋品源头-源头管理-删除 */ 
			supplierManagerEditPage(index, len);
			break;
		case 27:	/*ok 首页-批次开始 */
			batchStartPage(index, len);
			break;
		case 28:  /*ok 首页-批次开始-蛋品源头选择 */
			supplierManagerSelectPage(index, len);
			break;
		case 29:	/*ok 首页-批次结束 */
			batchStopPage(index, len);
			break;
		case 59:	/*ok 生产管理-包装管理-列表 */
		case 61:	/*ok 生产管理-包装管理-编辑 */
		case 62:	/*ok 生产管理-包装管理-删除 */
			packageBoxPage(index, len);
			break;
		case 52:	/*ok 首页-发起任务-包装盒选择 */
			packageBoxSelectPage(index, len);
			break;
		case 55:	/*ok 生产管理-蛋品管理-列表 */
		case 57:  /*ok 生产管理-蛋品管理-编辑 */
		case 58:  /*ok 生产管理-蛋品管理-删除 */
			productManagePage(index, len);
			break;
		case 50:	/*ok 生产管理-蛋品管理-蛋品选择 */
			productManageSelectPage(index, len);
			break;
		case 45:	/*ok 生产管理-分级生产-配方管理-显示配方列表 */
		case 46: 	/*ok 生产管理-分级生产-配方管理-详情 删除确认 */
		case 47:  /*ok 生产管理-分级生产-配方管理-详情 显示规则列表 */
		case 49:	/*ok 生产管理-分级生产-配方管理-详情 编辑规则 */
		case 54:  /*ok 生产管理-分级生产-配方管理-详情 删除规则 */
			recipeManagerPage(index, len);
			break;
		case 24:  /*ok 首页-生产配置-配方详情展示 编辑 删除 提交 */
		case 25:  /*ok 首页-生产配置-蛋品参数展示 */
			workRecipeSetPage(index, len);
			break;
		case 26:  /*ok 首页-生产配置-配方选择 */
			recipeSelectPage(index, len);
			break;
		
		case 70: /* 生产管理-分级生产-班组管理 */
		case 71: /* 生产管理-分级生产-班组管理 编辑 */
		case 72: /* 生产管理-分级生产-班组管理 删除 */
			teamOrGroupManagerPage(index, len);
			break;
		default: break;
	}
	if(0 == index){
		pageNumberLast = u_RegHoldingPtr[index];
	}
}

//根据名称查找蛋品源头参数
ProductManageInformation *getProductManageInformation(uint16_t *name)
{
	int i=0;
	
	for(i=0; i<PRODUCT_USED_MAX; i++){
		if(name_is_empty(name, 10) == RT_TRUE) continue;
		if(RT_TRUE == compare_names(name, productList.product[i].name, 10))
			return &productList.product[i];
	}
	return RT_NULL;
}

//根据名称查找包装盒参数
PackageBoxInformation *getPackageBoxInformation(uint16_t *name)
{
	int i=0;
	
	for(i=0; i<PACKAGE_BOX_USED_MAX; i++){
		if(name_is_empty(name, 10) == RT_TRUE) continue;
		if(RT_TRUE == compare_names(name, boxList.box[i].name, 10))
			return &boxList.box[i];
	}
	return RT_NULL;	
}

//根据配方名查找配方参数
SingleRecipeParam *getSingleRecipeParam(uint16_t *name)
{
	int i=0;
	
	for(i=0; i<RECIPE_USED_MAX; i++){
		if(name_is_empty(name, 10) == RT_TRUE) continue;
		if(RT_TRUE == compare_names(name, recipeList.recipe[i].recipeName, 10))
			return &recipeList.recipe[i];
	}
	return RT_NULL;
}

//遍历当前运行的配方参数 提取出当前每个包装通道的运行参数
void getPackageWorkInformation(WorkRecipeParam *recipe, SinglePackageWorkParam *pack)
{
	int i = 0, j = 0;
	uint32_t line;
	PackageBoxInformation *box;
	SingleRecipeParam *pRecipe;
	
	//机器启动前 先将最后一次提交的运行配置参数复制到当前运行参数中去
	recipe->workSpeed = submitRecipe.workSpeed;
	recipe->workSpeedEx = submitRecipe.workSpeedEx;
	pRecipe = getSingleRecipeParam(submitRecipe.selRecipeName);
	if(pRecipe == RT_NULL) rt_memset(&recipe->singleRecipe, 0, sizeof(SingleRecipeParam));
	else rt_memcpy(&recipe->singleRecipe, pRecipe, sizeof(SingleRecipeParam));

	rt_memcpy(&u_RegHoldingPtr[110], &recipe->singleRecipe.recipeName[0], 20);

	//清除包装通道运行参数
	rt_memset(pack, 0, sizeof(SinglePackageWorkParam)*SORTER_USED_MAX);
	
	//重新获取每个包装通道运行参数
	for(i=0; i<RECIPE_PARAM_LIST_LEN; i++){
		box = getPackageBoxInformation(recipe->singleRecipe.sortRule[i].packageName);
		if(RT_NULL == box){
			continue;
		}else{
			line = recipe->singleRecipe.sortRule[i].packageLine;
			for(j=0; j<SORTER_USED_MAX; j++){
				if(getbitv(line, j)){
					pack[j].isAct = 1;
					rt_memcpy(pack[j].packageName, box->name, 20);
					pack[j].row = box->row;
					pack[j].col = box->col;
					pack[j].autoTray = box->autoTray;
					pack[j].trayCheck = box->trayCheck;
					pack[j].trayCheck = box->trayCheck;
					pack[j].packBoxPost1 = box->packBoxPost1 + box->originOffset[j];
					pack[j].packBoxPost2 = box->packBoxPost2 + box->originOffset[j];
					pack[j].packBoxPost3 = box->packBoxPost3 + box->originOffset[j];
					pack[j].packBoxPost4 = box->packBoxPost4 + box->originOffset[j];
					pack[j].packBoxPost5 = box->packBoxPost5 + box->originOffset[j];
					pack[j].packBoxPost6 = box->packBoxPost6 + box->originOffset[j];
					pack[j].packBoxPost7 = box->packBoxPost7 + box->originOffset[j];
					pack[j].packBoxPost8 = box->packBoxPost8 + box->originOffset[j];
					pack[j].transportStart = box->transportStart;
					pack[j].chainLinkCnt = box->chainLinkCnt;
				}
			}
		}
	}
}
