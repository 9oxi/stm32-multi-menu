#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "IR.h"
#include "KEY.h"
#include "Buzzer.h"
#include "OLED.h"
#include <stdio.h>
#define      MENU_MAIN  0
#define      MENU_SPEED 1
#define      MENU_HIST  2
#define      MENU_CLOCK 3
#define      MENU_TMEP  4
#define      MENU_SAVED 5
static uint8_t state=MENU_MAIN;
static uint8_t cursor=0;
static uint16_t Speedx10=0;
static uint8_t run=0;
static void Show_Line(uint8_t Line,char *txt)
{
	char buf[17];
	uint8_t i;
	for(i=0;i<16;i++)
		buf[i]=' ';
	buf[16]='\0';
	for(i=0;i<16;i++)
	{
		if(txt[i]=='\0')
			break;
		buf[i]=txt[i];
	}	
	OLED_ShowString(Line,1,buf);
}
static void Draw_Main(void)
{
	Show_Line(1,(cursor==0)?">1.Speed":"1.Speed");
	Show_Line(1,(cursor==1)?">2.History":"2.History");
	Show_Line(1,(cursor==2)?">3.Clock":"3.Clock");
	Show_Line(1,(cursor==3)?">4.Temp":"4.Temp");
}
static void Draw_Speed(void)
{
	char buf[17];
	sprintf(buf,"laps:%d",IR_GetLap());
	Show_Line(1,buf);
	sprintf(buf,"Speed:%d.%d",Speedx10/10,Speedx10%10);
	Show_Line(2,buf);
	Show_Line(3,(run==0)?"OK:start":"OK:stop");
	Show_Line(4,"CAN:BACK");
}


















static void on_key(uint8_t KEY)
{
	switch(state)
		case  MENU_MAIN:
	if(KEY==KEY_UP)
		cursor=(cursor==0)?3:cursor-1;
	if(KEY==KEY_DOWN)
		cursor=(cursor==3)?3:cursor+1;
	if(KEY==KEY_OK)
	{
		if(cursor==0) state= MENU_SPEED;
		else if	(cursor==1) state=MENU_HIST;
		else if	(cursor==2) state=MENU_CLOCK;	
		else                state=MENU_TMEP;
			if(state==MENU_SPEED)
			{
				IR_ResetLap();
				run=0;
				Speed=0;
			}
			if(state==MENU_HIST)
				
			if(state==MENU_CLOCK)
				
			if(state==MENU_TMEP)
			
	}
	break;
	case  MENU_SPEED:
	if(KEY==KEY_OK)
	{
		if(run==1)
		{
			run=0;
			
		}
		else run=1;
	}
	if(KEY==KEY_CAN)
	{
		run=0;
		state=MENU_MAIN;
		OLED_Clear();
	}
	break;
}










