#ifndef __Track_H  // 首字母大写，其余小写，严格对应 Track.h
#define __Track_H

#define TRACK_PORT      GPIOA
#define TRACK_PIN0      GPIO_Pin_0
#define TRACK_PIN1      GPIO_Pin_1
#define TRACK_PIN2      GPIO_Pin_2
#define TRACK_PIN3      GPIO_Pin_3
#define TRACK_ALL_PIN   (TRACK_PIN0 | TRACK_PIN1 | TRACK_PIN2 | TRACK_PIN3)

void Track_Init(void);
uint8_t Track_GetState(void);

#endif