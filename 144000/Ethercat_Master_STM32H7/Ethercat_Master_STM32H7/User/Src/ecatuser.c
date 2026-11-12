
#include "ecatuser.h"

#include "stdio.h"
#include "string.h"

#include "stm32h7xx_hal.h"
#include "gpio.h"

PDO_Output *outputs1;
PDO_Input *inputs1;


char IOmap[200];
uint32_t dorun=0;
uint32_t OpenReady=0;

int oloop, iloop;

//motor control 
uint16 cur_status;
uint8_t startup_step=0;
int32 cur_pos = 0;
uint16 csp_pos_delay;
int cmdpos_raw;

#define DEBUG 1


int Servosetup(uint16 slave)
{	
	  int retval;
    uint16 u16val;
    uint8  u8val;

    retval = 0;

    u8val = 0;
    retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
    u16val = 0x1601;
    retval += ec_SDOwrite(slave, 0x1c12, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTRXM);
    u8val = 1;
    retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

    u8val = 0;
    retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
    u16val = 0x1a01;
    retval += ec_SDOwrite(slave, 0x1c13, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTRXM);
    u8val = 1;
    retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

    return 1;
}

void ecat_init(void)
{
    int slc;
//    int i,chk;
    int cnt = 1;
		int expectedWKC;
    
    /* initialise SOEM, bind socket to ifname */
	if (ec_init("eth0"))
	{
		printf("ec_init succeeded.\r\n");
         HAL_Delay(100);
		if ( ec_config_init(FALSE) > 0 )
		{
			printf("%d slaves found and configured.\r\n",ec_slavecount);
            
			if ( ec_slavecount >= 1 ) 
			{
                for(slc = 1; slc <= ec_slavecount; slc++)
                 {
                     
                    for(slc = 1; slc <= ec_slavecount; slc++)
										{
											printf("Found %s at position %d\n", ec_slave[slc].name, slc);
											ec_slave[slc].PO2SOconfig = &Servosetup;
										}
                 }
            }
            
            ec_configdc();//DC时钟配置
            ec_config_map(&IOmap);
//            ec_dcsync0(1, TRUE, SYNC0TIME, 250000); // SYNC0 on slave 1
//						ec_dcsync0(2, TRUE, SYNC0TIME, 0); // SYNC0 on slave 2
						
						printf("Slaves mapped, state to SAFE_OP.\n \r");	
						/* wait for all slaves to reach SAFE_OP state */
			      ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);
						/* read indevidual slave state and store in ec_slave[] */
						ec_readstate();
						
						printf("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
						printf("Slave 1 State=0x%04x\r\n",ec_slave[1].state);			
						
						for(cnt = 1; cnt <= ec_slavecount ; cnt++){
						printf("Slave:%d Name:%s Output size:%3dbits Input size:%3dbits State:%2d delay:%d.%d\n \r",
							cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
							ec_slave[cnt].state, (int)ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
						}
            
						oloop = ec_slave[0].Obytes;//主站传给从站的字节数，对于SSC-IO：2
						if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
						if (oloop > 30) oloop = 30;
			
						iloop = ec_slave[0].Ibytes;//从站传给主站的字节数，对于SSC-IO：6		
						if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
						if (iloop > 30) iloop = 30;
			
						printf("oloop:%d iloop:%d\n\r",oloop,iloop);
						
						printf("segments : %d : %d %d %d %d\n \r",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);
						printf("Request operational state for all slaves\n \r");
						expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
						printf("Calculated workcounter %d\n \r", expectedWKC);
						
						/* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            
            ec_writestate(0);
            
						printf("DC capable : %d\r\n",ec_configdc());
            
            HAL_Delay(100);
            
					/* wait for all slaves to reach OP state */
						do{
							ec_slave[0].state = EC_STATE_OPERATIONAL;
							ec_slave[1].state = EC_STATE_OPERATIONAL;
              ec_writestate(0);
							ec_writestate(1);
							printf("-%d %d-",ec_slave[0].state,ec_slave[1].state);
							}
						while ( (ec_slave[0].state != EC_STATE_OPERATIONAL)||(ec_slave[1].state != EC_STATE_OPERATIONAL));
//						while  (ec_slave[0].state != EC_STATE_OPERATIONAL);
						printf("\r\n%d %d\r\n",ec_slave[0].state,ec_slave[1].state);
            if (ec_slave[0].state == EC_STATE_OPERATIONAL )
            {
                
                outputs1 = (PDO_Output *)ec_slave[1].outputs;
                inputs1  = (PDO_Input *)ec_slave[1].inputs;
                dorun = 1;

                printf("all slaves reached operational state.\r\n");
            }
			
							else
						{
							printf("Not all slaves reached operational state.\n \r");
						}
		}
		else
		{
			printf("No slaves found!\r\n");
		}
	}
	else
	{
		printf("No socket connection Excecute as root\r\n");
	}
    
}





uint8 wkc;
//1ms isr
//int nihao = 1;
void ecat_loop(void)
{
    if (dorun>0)
    {
				
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
		}

}







