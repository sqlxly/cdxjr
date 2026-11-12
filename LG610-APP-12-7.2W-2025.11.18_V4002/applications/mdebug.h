
#include <stdio.h>

#ifndef _MDEBUG_H__
#define _MDEBUG_H__

#include "common.h"

#define MDEBUG_ON
#define NONE         "\033[0m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"


#ifdef  MDEBUG_ON
	#define DEBUG_ERR(fmt, args...)  rt_kprintf(LIGHT_RED"[%s:%d]:" LIGHT_RED fmt NONE, __func__, __LINE__, ## args)
	#define DEBUG_WARN(fmt, args...)  rt_kprintf(YELLOW"[%s:%d]:" YELLOW fmt NONE, __func__ , __LINE__, ## args)
	#define DEBUG_INFO(fmt, args...)  rt_kprintf(LIGHT_GREEN"[%s:%d]:" LIGHT_GREEN fmt NONE, __func__, __LINE__, ## args)
	#define DEBUG_CARE(fmt, args...)  rt_kprintf(LIGHT_PURPLE"[%s:%d]:" LIGHT_PURPLE fmt NONE, __func__, __LINE__, ## args)
#else
	#define DEBUG_ERR(fmt, args...)  
	#define DEBUG_WARN(fmt, args...) 
	#define DEBUG_INFO(fmt, args...)  
	#define DEBUG_CARE(fmt, args...) 
#endif

#endif  	// #define _MDEBUG_H__
