#include "stm32f10x.h"
#include "Track.h" 

void Track_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = TRACK_ALL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(TRACK_PORT, &GPIO_InitStructure);
}

uint8_t Track_GetState(void)
{
    uint8_t state = 0;
    
    if(GPIO_ReadInputDataBit(TRACK_PORT, TRACK_PIN0) == 1) state += 0x01;
    if(GPIO_ReadInputDataBit(TRACK_PORT, TRACK_PIN1) == 1) state += 0x02;
    if(GPIO_ReadInputDataBit(TRACK_PORT, TRACK_PIN2) == 1) state += 0x04;
    if(GPIO_ReadInputDataBit(TRACK_PORT, TRACK_PIN3) == 1) state += 0x08;
    
    return state;
}