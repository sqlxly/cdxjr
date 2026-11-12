#ifndef OLED_MENU_H_
#define OLED_MENU_H_

#include <rtthread.h>
#include <rtdevice.h>

#define TABLE_NUMBER 54

typedef struct {
	unsigned char current;
	unsigned char up;
	unsigned char down;
	unsigned char enter;
	void (*current_operation)();
}Key_table;


void menu_init();
	void main_menu();


	 Key_table menu_table[TABLE_NUMBER]={
		{0, 0, 0, 1, (*main_menu)},//Ö÷½çÃæ


	};

#endif
