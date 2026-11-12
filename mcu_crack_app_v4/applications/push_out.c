#include "push_out.h"

#define PUSH_IO1		GET_PIN(C, 6)
#define PUSH_IO2		GET_PIN(C, 7)
#define PUSH_IO3		GET_PIN(C, 8)
#define PUSH_IO4		GET_PIN(C, 9)
#define PUSH_IO5		GET_PIN(A, 9)
#define PUSH_IO6		GET_PIN(A, 10)

rt_base_t push_io_mp[6] = {PUSH_IO1, PUSH_IO2, PUSH_IO3, PUSH_IO4, PUSH_IO5, PUSH_IO6};

void push_out_gpio_init(void)
{
	for(int i=0; i<6; i++){
		rt_pin_mode(push_io_mp[i], PIN_MODE_OUTPUT);
		rt_pin_write(push_io_mp[i], PIN_LOW);
	}
}

void push_out_control(int ch, rt_base_t val)
{
	rt_pin_write(push_io_mp[ch], val);
}

#include "stdio.h"
#include <stdlib.h>

void pushOut(int argc, char *argv[])
{
	int ch, val;
	ch = atoi(argv[1]);
	val = atoi(argv[2]);
	push_out_control(ch, val);
}

MSH_CMD_EXPORT(pushOut, push out io control);

void pushOutAll(int argc, char *argv[])
{
	int val;
	
	val = atoi(argv[1]);
	push_out_control(0, val);
	push_out_control(1, val);
	push_out_control(2, val);
	push_out_control(3, val);
	push_out_control(4, val);
	push_out_control(5, val);
}

MSH_CMD_EXPORT(pushOutAll, push out all io control);
