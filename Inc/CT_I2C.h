#ifndef __CT_I2C_H_
#define __CT_I2C_H_

#include "stm32f7xx_hal.h"

#define SDA_CLK()    	  __GPIOD_CLK_ENABLE()
#define SDA_GPIO_PORT   GPIOD
#define SDA_GPIO_PIN    GPIO_PIN_13

#define SCL_CLK()    	  __GPIOD_CLK_ENABLE()
#define SCL_GPIO_PORT   GPIOD
#define SCL_GPIO_PIN    GPIO_PIN_12

#define SET_SDA()     	HAL_GPIO_WritePin(SDA_GPIO_PORT,SDA_GPIO_PIN,GPIO_PIN_SET)    
#define CLR_SDA()     	HAL_GPIO_WritePin(SDA_GPIO_PORT,SDA_GPIO_PIN,GPIO_PIN_RESET)  

#define SET_CLK()     	HAL_GPIO_WritePin(SCL_GPIO_PORT,SCL_GPIO_PIN,GPIO_PIN_SET)    
#define CLR_CLK()     	HAL_GPIO_WritePin(SCL_GPIO_PORT,SCL_GPIO_PIN,GPIO_PIN_RESET)

#define READ_SDA()    	HAL_GPIO_ReadPin(SDA_GPIO_PORT,SDA_GPIO_PIN)

void 		CT_I2C_Init(void);		 
void 		CT_I2C_Start(void);				
void 		CT_I2C_Stop(void);	  			
void 		CT_I2C_Send_Byte(uint8_t data);			
uint8_t CT_I2C_Read_Byte(unsigned char ack);	
uint8_t CT_I2C_Wait_Ack(void); 			
void 		CT_I2C_Ack(void);					
void 		CT_I2C_NAck(void);				

#endif /*__CAP_I2C_H_*/
