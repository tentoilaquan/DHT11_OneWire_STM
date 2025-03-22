#include "stm32f10x.h"                 
#include "misc.h"                       
#include "stdio.h"

void Timer_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Period = 0xFFFF;
    timerInitStructure.TIM_Prescaler = 72 - 1;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_Cmd(TIM2, ENABLE);
}

void Delay_us(uint32_t delay) {
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < delay);
}

void delay_ms(uint32_t delay) {
    while (delay--) {
        Delay_us(1000);
    }
}

void UART_Init()
{
	GPIO_InitTypeDef gpio_structure;
	USART_InitTypeDef usart_structure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	gpio_structure.GPIO_Pin = GPIO_Pin_9;
	gpio_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpio_structure);

	gpio_structure.GPIO_Pin = GPIO_Pin_10;
	gpio_structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpio_structure);
	
	usart_structure.USART_BaudRate = 9600;
	usart_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_structure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart_structure.USART_Parity = USART_Parity_No;
	usart_structure.USART_StopBits = USART_StopBits_1;
	usart_structure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&usart_structure);
	
	USART_Cmd(USART1,ENABLE);
}

void I2C_Config()
{
	GPIO_InitTypeDef gpio_structure;
	I2C_InitTypeDef i2c_structure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	gpio_structure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	gpio_structure.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio_structure);
	
	i2c_structure.I2C_Mode = I2C_Mode_I2C;
	i2c_structure.I2C_Ack = I2C_Ack_Enable;
	i2c_structure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	i2c_structure.I2C_ClockSpeed = 100000;
	i2c_structure.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c_structure.I2C_OwnAddress1 = 0x00;
	I2C_Init(I2C1,&i2c_structure);
	I2C_Cmd(I2C1,ENABLE);
}

void usart_sendchar(char ch)
{
	USART_SendData(USART1,ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

struct FILE__
{
	int dummy;
};
FILE __stdout;
int fputc(int ch,FILE*f)
{
	usart_sendchar(ch);
	return ch;
}

void I2C_START()
{
	I2C_GenerateSTART(I2C1,ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
}

void I2C_SENDADDRESS(uint8_t addr, uint8_t direct)
{
	if(direct == 0)
	{
		addr &= ~0x01;
	}
	else
	{
		addr |= 0x01;
	}
	I2C_Send7bitAddress(I2C1, addr, direct);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
}

void I2C_SENDBYTE(uint8_t data)
{
	I2C_SendData(I2C1,data);
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
}

void I2C_STOP()
{
	I2C_GenerateSTOP(I2C1,ENABLE);
}

uint8_t I2C_RECEIVE()
{
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	return I2C_ReceiveData(I2C1);
}

uint16_t I2C_ReadLight()
{
	uint8_t data[2];
	I2C_START();
	I2C_SENDADDRESS(0x23<<1,0);
	I2C_SENDBYTE(0x10);
	I2C_STOP();
	delay_ms(200);
	I2C_START();
	I2C_SENDADDRESS(0x23<<1,1);
	data[0]= I2C_RECEIVE();
	data[1]= I2C_RECEIVE();
	I2C_STOP();
	return (data[0]<<8) | data[1];
}

int main(){
	I2C_Config();
	Timer_Init();
	UART_Init();
	
	while(1)
	{
		uint16_t light = I2C_ReadLight();
		printf("Cuong do anh sang: %f lux\n", (float)light/1.2);
		delay_ms(500);
	}
}