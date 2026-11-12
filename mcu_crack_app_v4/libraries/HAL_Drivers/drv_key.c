#include "drv_key.h"
#include <rtdevice.h>
#include <board.h>

#define EC11_TAG_PIN            GET_PIN(A, 10)
#define EC11_PHB_PIN            GET_PIN(A, 9)
#define EC11_PHA_PIN            GET_PIN(E, 11)

#define EC11_TAG_LEVEL          rt_pin_read(EC11_TAG_PIN)
#define EC11_PHA_LEVEL          rt_pin_read(EC11_PHA_PIN)
#define EC11_PHB_LEVEL          rt_pin_read(EC11_PHB_PIN)

#pragma pack(1)
//Ðý×ª±àÂëÆ÷ÐÅÏ¢
typedef struct
{
    rt_uint8_t a_last;      //AÏàÉÏÒ»´Î×´Ì¬
    rt_uint8_t b_last;      //BÏàÉÏÒ»´ÎµÄ×´Ì¬
    rt_uint8_t type;        //ÀàÐÍ£¬0£ºÒ»¶¨Î»Ò»Âö³å£¬1£ºÁ½¶¨Î»Ò»Âö³å
}ec11_info_t;
#pragma pack()

static struct rt_mailbox mkey;
static char mb_pool[32];
static ec11_info_t ec11;

/*
*\°´¼ü³õÊ¼»¯
*/
static void key_pin_init(void)
{
    rt_pin_mode(EC11_PHA_PIN, PIN_MODE_INPUT_PULLUP);
   
    rt_pin_mode(EC11_PHB_PIN, PIN_MODE_INPUT_PULLUP);
    
    rt_pin_mode(EC11_TAG_PIN, PIN_MODE_INPUT_PULLUP);   
}
/*
*\¶ÁÈ¡Ðý×ª±àÂëÆ÷°´¼ü¶¯×÷
*\return:¼üÖµ
*/
static rt_uint8_t ec11_tag_scan(void)
{
    static rt_uint32_t key_tag = 0x01,press_cnt = 0;
    rt_uint8_t key = 0,tag = 0;

    key = rt_pin_read(EC11_TAG_PIN);
    if(!key)
    {
        if(key_tag != key)
        {
            key_tag = key;
            press_cnt = 0;
        }
        press_cnt++;
        if(press_cnt >= 200)
            press_cnt = 201;
    }
    else
    {
        if(press_cnt >= 2)
            tag = TAG_KEY_ACTION;
        press_cnt = 0;
        key_tag = 0x01;
    }

    return tag;
}
/*
*\Ò»¶¨Î»Ò»Âö³åÀàÐÍÐý×ª±àÂëÆ÷½âÎö
*/
static rt_uint8_t ec11_single_scan(void)
{
    rt_uint8_t result = 0;

    if(EC11_PHA_LEVEL != ec11.a_last)
    {
        if(EC11_PHA_LEVEL == 0)
        {
            if(EC11_PHB_LEVEL != 0)
                result = LEFT_KEY_ACTION;
            else
                result = RIGHT_KEY_ACTION;
        }
        ec11.a_last = EC11_PHA_LEVEL;
        ec11.b_last = EC11_PHB_LEVEL;
    }

    return result;
}
/*
*\Á½¶¨Î»Ò»Âö³åÀàÐÍÐý×ª±àÂëÆ÷¼ÆÏ¢
*/
static rt_uint8_t ec11_double_scan(void)
{
    rt_uint8_t result = 0;

    if(EC11_PHA_LEVEL != ec11.a_last)
    {
        if(EC11_PHA_LEVEL != 0)
        {
            if((ec11.b_last != 0)&&(EC11_PHB_LEVEL == 0))
                result = LEFT_KEY_ACTION;
            if((ec11.b_last == 0)&&(EC11_PHB_LEVEL != 0))
                result = RIGHT_KEY_ACTION;
            
            if((ec11.b_last == EC11_PHB_LEVEL)&&(EC11_PHB_LEVEL == 0))
                result = LEFT_KEY_ACTION;

            if((ec11.b_last == EC11_PHB_LEVEL)&&(EC11_PHB_LEVEL != 0))
                result = RIGHT_KEY_ACTION;
        }
        else
        {
            if((ec11.b_last != 0)&&(EC11_PHB_LEVEL == 0))
                result = RIGHT_KEY_ACTION;
            if((ec11.b_last == 0)&&(EC11_PHB_LEVEL != 0))
                result = LEFT_KEY_ACTION;
            
            if((ec11.b_last == EC11_PHB_LEVEL)&&(EC11_PHB_LEVEL == 0))
                result = RIGHT_KEY_ACTION;

            if((ec11.b_last == EC11_PHB_LEVEL)&&(EC11_PHB_LEVEL != 0))
                result = LEFT_KEY_ACTION;
        }

        ec11.a_last = EC11_PHA_LEVEL;
        ec11.b_last = EC11_PHB_LEVEL;
    }

    return result;
}
/*
*\Ðý×ª±àÂëÆ÷¶¯×÷½âÎö
*/
static rt_uint8_t ec11_action_analysis(void)
{
    rt_uint8_t result = 0;

    if(ec11.type == 0)
    {
        result = ec11_single_scan();
        if(result != 0)
            return result;
    }
    else
    {
        result = ec11_double_scan();
        if(result != 0)
            return result;
    }

    result = ec11_tag_scan();

    return result;
}
/*
*\Èí¼þ¶¨Ê±Æ÷»Øµ÷º¯Êý
*/
static void key_timer_cb(void *parameters)
{
    rt_uint8_t key = 0;

    key = ec11_action_analysis();
    if(key != 0)
		{
        rt_mb_send_wait(&mkey, key, 100);
		}
}

//´ÓÓÊÏäÖÐ¶ÁÈ¡°´¼ü
rt_ubase_t wait_key_plus(rt_uint32_t timeout)
{
	rt_ubase_t keyBtn = 0;
	if (rt_mb_recv(&mkey, (rt_ubase_t *)&keyBtn, timeout) == RT_EOK)
	{
//		rt_kprintf("a mail from mailbox. <0x%03x>\n", keyBtn);
		return keyBtn;
	}
	return RT_FALSE;
}

/*
*\³õÊ¼»¯°´¼üÏß³Ì
*/
int key_scan_init(void)
{
    static rt_timer_t ktimer;

    rt_memset((rt_uint8_t *)&ec11.a_last, 0, sizeof(ec11_info_t));

    key_pin_init();

		rt_err_t result;

		/* mailbox */
		result = rt_mb_init(&mkey,
												"mbkey",                    /* Ãû³Æ mbt */
												&mb_pool[0],                /* ÓÊÏäÄÚ´æ³Ø mb_pool */
												sizeof(mb_pool) / 4,        /* ÓÊÏäÖÐµÄÓÊ¼þÊýÄ */
												RT_IPC_FLAG_FIFO);          /* ²ÉÓÃ FIFO ·½Ê½½øÐÐÏß³ÌµÈ´ý */
		if (result != RT_EOK)
		{
				rt_kprintf("init mailbox failed.\n");
		}

    /*´´½¨Ò»¸ö¶¨Ê±¶ÁÈ¡±àÂëÆ÷PWMÊäÈëµÄ¶¨Ê±Æ÷*/
    ktimer = rt_timer_create("ktimer", 
                            key_timer_cb,
                            RT_NULL, 
                            2, 
                            RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);
    if(ktimer != RT_NULL)
        rt_timer_start(ktimer);
    else
        rt_kprintf("create soft ktimer failed\r\n");

    return RT_EOK;
}
