#ifndef __Usart1_H  
#define __Usart1_H

#define USART1_PORT     GPIOA
#define USART1_TX_PIN   GPIO_Pin_9
#define USART1_RX_PIN   GPIO_Pin_10

void Usart1_Init(void);
void Usart1_SendByte(uint8_t dat);

#endif