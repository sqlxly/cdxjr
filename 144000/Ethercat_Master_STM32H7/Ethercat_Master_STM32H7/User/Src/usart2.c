/*
 * FILE                : usart.c
 * DESCRIPTION         : This file is iCore4T usart driver demo.
 * Author              : zh.
 * Copyright           :
 *
 * History
 * --------------------
 * Rev                 : 0.00
 * Date                : 11/12/2019
 *
 * create.
 * --------------------
 */

//------------------------ Include files ------------------------//

#include "usart2.h"
#include "usart.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

//--------------------- Function Prototype ----------------------//

static int send_string_to_usart2(char * str);
static int initialize_usart2(unsigned long int baudrate);
static int my_printf2(const char * fmt,...);

//--------------------------- Variable --------------------------// 

USART2_T usart2 = {
	.receive_ok_flag = 0,
	.counter = 0,
	.send_string = send_string_to_usart2,
	.initialize = initialize_usart2,
	.printf = my_printf2
};

extern UART_HandleTypeDef huart2;

//--------------------------- Function --------------------------//
/*
 * Name               : initialize_usart2
 * Description        : ---
 * Author             : zh.
 * 
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 11/12/2019
 *
 * create.
 * -------------------
 */
static int initialize_usart2(unsigned long int baudrate)
{
  huart2.Init.BaudRate = baudrate;
	if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    while(1);
  }	
	HAL_UART_Receive_IT(&huart2, (unsigned char *)&usart2.receive_data,1);
	
	return 0;
}

/*
 * Name               : send_string_to_usart2
 * Description        : ---
 * Author             : zh.
 * 
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 11/12/2019
 *
 * create.
 * -------------------
 */
static int send_string_to_usart2(char * str)
{
	while(*str != '\0'){
		while(!(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) == 1));
		HAL_UART_Transmit(&huart2,(unsigned char*)str++,1,1000);
	}
	while(!(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) == 1));
	
	return 0;
}
/*
 * Name               : HAL_UART_RxCpltCallback
 * Description        : ---
 * Author             : zh.
 * 
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 11/12/2019
 *
 * create.
 * -------------------
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2){
		HAL_UART_IRQHandler(&huart2);

		while(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY);
		while(HAL_UART_Receive_IT	(&huart2,(unsigned char *)&usart2.receive_data,1) != HAL_OK);
		usart2.receive_buffer[usart2.counter ++] = usart2.receive_data;
		if(usart2.counter == UART_BUFFER_SIZE)usart2.counter = 0;
		
		if(usart2.receive_buffer[usart2.counter - 1] =='\n' && usart2.receive_buffer[usart2.counter - 2] == '\r'){
			usart2.receive_buffer[usart2.counter - 1] = 0;
			usart2.counter = 0;
			usart2.receive_ok_flag = 1;
		}		
	}
}
/*
 * Name               : my_printf2
 * Description        : ---
 * Author             : zh.
 * 
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 11/12/2019
 *t
 * create.
 * -------------------
 */
static int my_printf2(const char * fmt,...)
{
	__va_list arg_ptr;
	char buf[UART_BUFFER_SIZE];
	
	memset(buf,'\0',sizeof(buf));
	
	va_start(arg_ptr,fmt);
	vsprintf(buf,fmt,arg_ptr);
	va_end(arg_ptr);
	
	send_string_to_usart2(buf);
	
	return 0;
}
