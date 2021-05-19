/**
  ******************************************************************************
  * @file    GUI_App.c
  * @author  MCD Application Team
  * @brief   Simple demo drawing "Hello world"  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright © 2018 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include "GUI_App.h"
#include "GUI.h"
#include <stdlib.h>
#include "DIALOG.h"
#include <scpi/scpi.h>
#include <scpi-def.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"

extern WM_HWIN CreateMain(void);
extern WM_HWIN CreateKeyboard(void);

char tell[30] = "";

struct Operate
{
    double ch1_sta; //0 refer to off
    double ch2_sta;
    int window_status;
    //0:sets for ch1-A; 1:sets for ch2-V;
    //2:sets for ch2-V; 2:sets for ch2-A
    double ch_setting[4];
    int inpt_count;
    char sets[8];
} ops = {0., 0., 0, {0., 0., 0., 0.}, 0, ""};

struct Operate *ops_ptr = NULL;

size_t SCPI_Write(scpi_t *context, const char *data, size_t len)
{
    (void)context;
    //HAL_UART_Transmit(&huart1, "\nlogs:SCPI_Write\n", 17, 100);
    HAL_UART_Transmit(&huart1, (uint8_t*)data, len, 100);
    return fwrite(data, 1, len, stdout);
}

scpi_result_t SCPI_Flush(scpi_t *context)
{
    (void)context;
    HAL_UART_Transmit(&huart1, "logs:SCPI_Flush\n", 17, 100);
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t *context, int_fast16_t err)
{
    (void)context;
    //HAL_UART_Transmit(&huart1, "logs:SCPI_Error\n", 17, 100);
    //fprintf(stderr, "**ERROR: %d, \"%s\"\r\n", (int16_t)err, SCPI_ErrorTranslate(err));
    return 0;
}

scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
    (void)context;
    char tmp[60];
    if (SCPI_CTRL_SRQ == ctrl)
    {
        //fprintf(stderr, "**SRQ: 0x%X (%d)\r\n", val, val);
        sprintf(tmp, "**SRQ: 0x%X (%d)\r\n", val, val); 
        HAL_UART_Transmit(&huart1, (uint8_t *)tmp, strlen(tmp), 100);
    }
        
    else
    {
        sprintf(tmp, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
        //fprintf(stderr, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
        HAL_UART_Transmit(&huart1, (uint8_t *)tmp, strlen(tmp), 100);
    }
    HAL_UART_Transmit(&huart1, "logs:SCPI_Control\n", 19, 100);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context)
{
    (void)context;
    //HAL_UART_Transmit(&huart1, "logs:SCPI_Reset\n", 17, 100);
    HAL_UART_Transmit(&huart1, "**Reset\r\n", 10, 100);
    fprintf(stderr, "**Reset\r\n");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t *context)
{
    (void)context;
    return SCPI_RES_ERR;
}

void GRAPHICS_MainTask(void)
{
    
    scpi_context.parser_state.termination = SCPI_MESSAGE_TERMINATION_NL;
    SCPI_Init(&scpi_context,
              scpi_commands,
              &scpi_interface,
              scpi_units_def,
              SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
              scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);
    ops_ptr = &ops;

    HAL_UART_Transmit(&huart1, "logs:job start\n\n\n", 18, 100);
    SCPI_Input(&scpi_context, "*RST\r\n", 7);
    SCPI_Input(&scpi_context, "*IDN?\r\n", 8);
    SCPI_Input(&scpi_context, "\r\n*ID", 6);
    SCPI_Input(&scpi_context, "SYST:VERS?\r\n", 13);
    SCPI_Input(&scpi_context, "CONF:VOLT:DC 19,60\r\n", 21);
    SCPI_Input(&scpi_context, "MEAS:volt:DC? 12,50;*OPC\r\n", 27);
    HAL_UART_Transmit(&huart1, "logs:scpi test OK\n\n\n", 21, 100);

    CreateMain();
    while (1)
    {
        GUI_Delay(10);
    }
}

/*************************** End of file ****************************/
