#include "GT911.h"
#include "CT_I2C.h"
#include "main.h"
#include "stdio.h"
#include "usart.h"

#define BAND 10

GT911_Dev Dev_Now,Dev_Backup;

static void GT911_Reset_Sequence()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GT911_RST_CLK();
	GT911_INT_CLK();
	
  GPIO_InitStruct.Pin = GT911_RST_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GT911_RST_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GT911_INT_PIN;
  HAL_GPIO_Init(GT911_INT_PORT, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GT911_RST_PORT,GT911_RST_PIN,GPIO_PIN_RESET);    
	HAL_Delay(100);
	HAL_GPIO_WritePin(GT911_INT_PORT,GT911_INT_PIN,GPIO_PIN_RESET);    
	HAL_Delay(100);
	HAL_GPIO_WritePin(GT911_RST_PORT,GT911_RST_PIN,GPIO_PIN_SET);    
	HAL_Delay(200);
	
	GPIO_InitStruct.Pin = GT911_INT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GT911_INT_PORT, &GPIO_InitStruct);
	
	HAL_Delay(100);
}

static uint8_t GT911_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint8_t ret=0;
	CT_I2C_Start();	
 	CT_I2C_Send_Byte(CT_CMD_WR);   
	CT_I2C_Wait_Ack();
	CT_I2C_Send_Byte(reg>>8);   	
	CT_I2C_Wait_Ack(); 	 										  		   
	CT_I2C_Send_Byte(reg&0XFF);   	
	CT_I2C_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    CT_I2C_Send_Byte(buf[i]);  
		ret=CT_I2C_Wait_Ack();
		if(ret)break;  
	}
  CT_I2C_Stop();					
	return ret; 
}

static void GT911_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
 	CT_I2C_Start();	
 	CT_I2C_Send_Byte(CT_CMD_WR);  
	CT_I2C_Wait_Ack();
 	CT_I2C_Send_Byte(reg>>8);   
	CT_I2C_Wait_Ack(); 	 										  		   
 	CT_I2C_Send_Byte(reg&0XFF);   	
	CT_I2C_Wait_Ack();  
 	CT_I2C_Stop();  
	
 	CT_I2C_Start();  	 	   
	CT_I2C_Send_Byte(CT_CMD_RD);     
	CT_I2C_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
		buf[i]=CT_I2C_Read_Byte(i==(len-1)?0:1); 
	} 
  CT_I2C_Stop();   
}

static uint8_t GT911_ReadStatue(void)
{
	uint8_t buf[4];
	GT911_RD_Reg(GT911_PRODUCT_ID_REG, (uint8_t *)&buf[0], 3);
	GT911_RD_Reg(GT911_CONFIG_REG, (uint8_t *)&buf[3], 1);
	printf("TouchPad_ID:%c,%c,%c\r\nTouchPad_Config_Version:%2x\r\n",buf[0],buf[1],buf[2],buf[3]);
	return buf[3];
}

static uint16_t GT911_ReadFirmwareVersion(void)
{
	uint8_t buf[2];

	GT911_RD_Reg(GT911_FIRMWARE_VERSION_REG, buf, 2);

	printf("FirmwareVersion:%2x\r\n",(((uint16_t)buf[1] << 8) + buf[0]));
	return ((uint16_t)buf[1] << 8) + buf[0];
}

void GT911_Scan(void)
{
	uint8_t buf[41];
  uint8_t Clearbuf = 0;
	uint8_t i;

	if (Dev_Now.Touch == 1)
	{	
		Dev_Now.Touch = 0;
		GT911_RD_Reg(GT911_READ_XY_REG, buf, 1);		

		if ((buf[0]&0x80) == 0x00)
		{
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);
			//printf("%x\r\n",buf[0]);
			HAL_Delay(10);
		}
		else
		{
			//printf("bufstat:%x\r\n",buf[0]);
			Dev_Now.TouchpointFlag = buf[0];
			Dev_Now.TouchCount = buf[0]&0x0f;
			//printf("TouchCount:%d\n",Dev_Now.TouchCount);
			if (Dev_Now.TouchCount > 5)
			{
				GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);
				return ;
			}		
			GT911_RD_Reg(GT911_READ_XY_REG+1, &buf[1], Dev_Now.TouchCount*8);
			GT911_WR_Reg(GT911_READ_XY_REG, (uint8_t *)&Clearbuf, 1);
			
			Dev_Now.Touchkeytrackid[0] = buf[1];
			Dev_Now.X[0] = ((uint16_t)buf[3] << 8) + buf[2];
			Dev_Now.Y[0] = ((uint16_t)buf[5] << 8) + buf[4];
			Dev_Now.S[0] = ((uint16_t)buf[7] << 8) + buf[6];

			Dev_Now.Touchkeytrackid[1] = buf[9];
			Dev_Now.X[1] = ((uint16_t)buf[11] << 8) + buf[10];
			Dev_Now.Y[1] = ((uint16_t)buf[13] << 8) + buf[12];
			Dev_Now.S[1] = ((uint16_t)buf[15] << 8) + buf[14];

			Dev_Now.Touchkeytrackid[2] = buf[17];
			Dev_Now.X[2] = ((uint16_t)buf[19] << 8) + buf[18];
			Dev_Now.Y[2] = ((uint16_t)buf[21] << 8) + buf[20];
			Dev_Now.S[2] = ((uint16_t)buf[23] << 8) + buf[22];

			Dev_Now.Touchkeytrackid[3] = buf[25];
			Dev_Now.X[3] = ((uint16_t)buf[27] << 8) + buf[26];
			Dev_Now.Y[3] = ((uint16_t)buf[29] << 8) + buf[28];
			Dev_Now.S[3] = ((uint16_t)buf[31] << 8) + buf[30];

			Dev_Now.Touchkeytrackid[4] = buf[33];
			Dev_Now.X[4] = ((uint16_t)buf[35] << 8) + buf[34];
			Dev_Now.Y[4] = ((uint16_t)buf[37] << 8) + buf[36];
			Dev_Now.S[4] = ((uint16_t)buf[39] << 8) + buf[38];

			for (i=0;i<Dev_Now.TouchCount;i++)
			{
				if(Dev_Now.Y[i]<BAND)Dev_Now.Y[i]=BAND;
				if(Dev_Now.Y[i]>GT911_MAX_HEIGHT-BAND)Dev_Now.Y[i]=GT911_MAX_HEIGHT-BAND;
				//printf("%dY:%d\n",i,Dev_Now.Y[i]);
				if(Dev_Now.X[i]<BAND)Dev_Now.X[i]=BAND;
				if(Dev_Now.X[i]>GT911_MAX_WIDTH-BAND)Dev_Now.X[i]=GT911_MAX_WIDTH-BAND;
				//printf("%dX:%d\n",i,Dev_Now.X[i]);
			}
			
			GT911_StoreAllPiont();
			
			for (i=0;i<Dev_Now.TouchCount;i++)
			{				
				Dev_Backup.X[i] = Dev_Now.X[i];
				Dev_Backup.Y[i] = Dev_Now.Y[i];
				Dev_Backup.TouchCount = Dev_Now.TouchCount;		
			}
		}
	}
	else
	{
		GT911_ResetPoint();
	}
}

void GT911_Init(void)
{
	printf("start gt911 init");
	GT911_Reset_Sequence();

	CT_I2C_Init();
	
	GT911_ReadStatue();
	
	GT911_ReadFirmwareVersion();
		
	GT911_Scan();
}



void GT911_StoreAllPiont(void)
{
	
	
//	GUI_PID_STATE State;
	
	for (uint8_t i=0;i<Dev_Now.TouchCount;i++)
	{
//		   State.Pressed = 1;
//		   State.x = Dev_Now.X[i];
//		   State.y =Dev_Now.Y[i];
//		   State.Layer = 0;
//		   GUI_TOUCH_StoreStateEx(&State);
		
		GUI_TOUCH_StoreState(Dev_Now.X[i],Dev_Now.Y[i]);
		
		
	}
	
}

void GT911_ResetPoint(void)
{
//		GUI_PID_STATE State;
//		State.x = -1;
//		State.y = -1;
//		State.Pressed = 0;
//		State.Layer = 0;
//		GUI_TOUCH_StoreStateEx(&State);
	
	GUI_TOUCH_StoreState(-1,-1);
	
}


