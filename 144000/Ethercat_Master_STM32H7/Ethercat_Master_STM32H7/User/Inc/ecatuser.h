#ifndef _ECATUSER_H_
#define _ECATUSER_H_


#include "ethercat.h"

#include "osal.h"
#define SYNC0TIME 2000 
///1000us

//根据实际配置
PACKED_BEGIN
typedef struct PACKED
{  

		uint8_t DO8;
		uint8_t SubIndex008;
	
}PDO_Output;
PACKED_END


//根据实际配置
PACKED_BEGIN
typedef struct PACKED
{  
	 
		uint8_t KEY8;
		uint8_t temp2;
		uint8_t temp3;
		uint8_t temp4;
		uint8_t temp5;
		uint8_t temp6;
	
}PDO_Input;
PACKED_END



void ecat_init(void);
void ecat_loop(void);
void ctrl_state(void);



#endif

