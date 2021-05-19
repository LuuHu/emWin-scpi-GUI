#include "CT_I2C.h"

static void delay_us(uint32_t value)
{
	uint32_t i;
	i = value * 250;
	while(i--);
}

static void CT_SDA_IN()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
  GPIO_InitStruct.Pin = SDA_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(SDA_GPIO_PORT, &GPIO_InitStruct);
}

static void CT_SDA_OUT()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = SDA_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(SDA_GPIO_PORT, &GPIO_InitStruct);
}

void CT_I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	SDA_CLK();
	SCL_CLK();

  GPIO_InitStruct.Pin = SCL_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(SCL_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SDA_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SDA_GPIO_PORT, &GPIO_InitStruct);
}

void CT_I2C_Start(void)
{
	CT_SDA_OUT();
	
	SET_SDA();
	delay_us(1);	
	
	SET_CLK();
	delay_us(1);
	
 	CLR_SDA();
	delay_us(1);
	
	CLR_CLK();
	delay_us(1);
}	  

void CT_I2C_Stop(void)
{
	CT_SDA_OUT();
	
	CLR_SDA();
 	delay_us(1);
	
	SET_CLK();
	delay_us(1);
	
	SET_SDA();
	delay_us(1);							   	
}

void CT_I2C_Send_Byte(uint8_t data)
{                        
	uint8_t t; 
  
	CT_SDA_OUT();    
  CLR_CLK();
		
	for(t=0;t<8;t++)
	{              
		if(data&0x80)
		{
			SET_SDA();
		}
		else
		{
			CLR_SDA();
		}

		data<<=1; 	  
		delay_us(1); 

		SET_CLK();
		delay_us(1); 
		CLR_CLK();
		delay_us(1);
	}	 

	SET_SDA();
	delay_us(1);
	CLR_CLK();
	delay_us(1);
} 

uint8_t CT_I2C_Read_Byte(unsigned char ack)
{
	uint8_t i,receive=0;

 	CT_SDA_IN();

  for(i=0;i<8;i++ )
	{
    CLR_CLK(); 
		delay_us(1);
		SET_CLK();
    receive<<=1;
    if(READ_SDA())receive++;   
		delay_us(1);
	}	

	CLR_CLK(); 	
	if (!ack)CT_I2C_NAck();
	else CT_I2C_Ack(); 
 
	return receive;
}

uint8_t CT_I2C_Wait_Ack(void)
{
	uint8_t ucErrTime=0;

	CT_SDA_IN();  
	SET_SDA();
	delay_us(1);	   
	SET_CLK();
	delay_us(1);	
	
	while(READ_SDA())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			CT_I2C_Stop();
			return 1;
		}
	}
	CLR_CLK();
	return 0;  
} 

void CT_I2C_Ack(void)
{
	CT_SDA_OUT();
	CLR_CLK();
	
	CLR_SDA();
	delay_us(1);
	
	SET_CLK();
	delay_us(1);
	
	CLR_CLK();
	delay_us(1);
}
   
void CT_I2C_NAck(void)
{
	CT_SDA_OUT();

	SET_SDA();
	delay_us(1);
	
	SET_CLK();
	delay_us(1);
	
	CLR_CLK();
	delay_us(1);
}					 				     
