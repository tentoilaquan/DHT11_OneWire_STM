#include "stm32f10x.h"                         
#include "string.h"
#include <stdio.h>

uint16_t timerValue;
uint8_t dataBuffer[5];
uint8_t checksum;
unsigned int loopIndex, bitIndex;

void Delay_us(uint32_t delay) {
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < delay);
}

void Delay_ms(uint32_t delay) {
    while (delay--) {
        Delay_us(1000);
    }
}

void USART_InitConfig(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpioInitStructure;

    gpioInitStructure.GPIO_Pin = GPIO_Pin_9;
    gpioInitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInitStructure);

    gpioInitStructure.GPIO_Pin = GPIO_Pin_10;
    gpioInitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpioInitStructure);

    USART_InitTypeDef usartInitStructure;
    usartInitStructure.USART_BaudRate = 9600;
    usartInitStructure.USART_WordLength = USART_WordLength_8b;
    usartInitStructure.USART_StopBits = USART_StopBits_1;
    usartInitStructure.USART_Parity = USART_Parity_No;
    usartInitStructure.USART_Mode = USART_Mode_Tx;
    usartInitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usartInitStructure);

    USART_Cmd(USART1, ENABLE);
}

void USART_SendText(char *text) {
    while (*text) {
        USART_SendData(USART1, *text);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        text++;
    }
}

void USART_SendValue(uint8_t value) {
    char buffer[5];
    sprintf(buffer, "%d", value);
    USART_SendText(buffer);
}

void DHT11_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpioInitStructure;
    gpioInitStructure.GPIO_Pin = GPIO_Pin_12;
    gpioInitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioInitStructure);
}

void Timer_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Period = 0xFFFF;
    timerInitStructure.TIM_Prescaler = 72 - 1;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);
}

int main(void) {
    DHT11_Init();
    USART_InitConfig();
    Timer_Init();

    while (1) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_12); 
        Delay_ms(20);                       
        GPIO_SetBits(GPIOB, GPIO_Pin_12);   

        TIM_SetCounter(TIM2, 0);
        while (TIM_GetCounter(TIM2) < 10) {
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }

        timerValue = TIM_GetCounter(TIM2);
        if (timerValue >= 10) {
            continue;
        }

        TIM_SetCounter(TIM2, 0);
        while (TIM_GetCounter(TIM2) < 45) {
            if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }

        timerValue = TIM_GetCounter(TIM2);
        if ((timerValue >= 45) || (timerValue <= 5)) {
            continue;
        }

        TIM_SetCounter(TIM2, 0);
        while (TIM_GetCounter(TIM2) < 90) {
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }

        timerValue = TIM_GetCounter(TIM2);
        if ((timerValue >= 90) || (timerValue <= 70)) {
            continue;
        }

        TIM_SetCounter(TIM2, 0);
        while (TIM_GetCounter(TIM2) < 95) {
            if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                break;
            }
        }

        timerValue = TIM_GetCounter(TIM2);
        if ((timerValue >= 95) || (timerValue <= 75)) {
            continue;
        }

        for (loopIndex = 0; loopIndex < 5; loopIndex++) {
            for (bitIndex = 0; bitIndex < 8; bitIndex++) {
                TIM_SetCounter(TIM2, 0);
                while (TIM_GetCounter(TIM2) < 65) {
                    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                        break;
                    }
                }

                timerValue = TIM_GetCounter(TIM2);
                if ((timerValue >= 65) || (timerValue <= 45)) {
                    continue;
                }

                TIM_SetCounter(TIM2, 0);
                while (TIM_GetCounter(TIM2) < 80) {
                    if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
                        break;
                    }
                }

                timerValue = TIM_GetCounter(TIM2);
                if ((timerValue >= 80) || (timerValue <= 10)) {
                    continue;
                }

                dataBuffer[loopIndex] <<= 1;
                if (timerValue > 45) {
                    dataBuffer[loopIndex] |= 1;
                } else {
                    dataBuffer[loopIndex] &= ~1;
                }
            }
        }

        checksum = dataBuffer[0] + dataBuffer[1] + dataBuffer[2] + dataBuffer[3];
        if (checksum != dataBuffer[4]) {
            continue;
        }

        USART_SendText("Temperature: ");
        USART_SendValue(dataBuffer[2]);
        USART_SendText("*C\n");
        USART_SendText("Humidity: ");
        USART_SendValue(dataBuffer[0]);
        USART_SendText("%\n");
    }
}