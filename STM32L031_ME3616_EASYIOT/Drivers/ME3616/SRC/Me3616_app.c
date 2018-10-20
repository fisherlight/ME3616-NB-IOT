/**
  ******************************************************************************
  * @file    me3616_app.c
  * @author  Simon Luk (simonluk@unidevelop.net)
  * @brief   This file implement the NB-IoT applications
  *          for GOSUNCN ME3616 NB-IoT Module Made by emakerzone
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2018 Simon Luk </center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Simon Luk nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include <stdlib.h>

#include "me3616.h"
#include "easyiot.h"
#include "TestDevice.h"

#define EASYIOT_MSG_BUFF_MAX_SIZE			400
#define EASYIOT_MSG_BUFF_ACK_MAX_SIZE		300
#define EASYIOT_CMD_BUFF_MAX_SIZE			400
#define EASYIOT_CMD_BUFF_ACK_MAX_SIZE		300
#define EASYIOT_CONVERT_AT_BUFF_MAX_SIZE	400


char * client_imei = "86966203070xxxx";
char * client_imsi = "46011300950xxxx";


int32_t Signal_val = 0;		//�ź�ǿ��
uint8_t Battery_val = 0;	//����
uint16_t last_dtag_mid = 0;	//��Ҫ�ظ�����һ����Ϣ/�����dtag


uint8_t msg_buff[EASYIOT_MSG_BUFF_MAX_SIZE] = {0};				//��Ϣbuff
//uint8_t msg_buff_ack[EASYIOT_MSG_BUFF_ACK_MAX_SIZE] = {0};		//��Ϣbuff ack
uint8_t cmd_buff[EASYIOT_CMD_BUFF_MAX_SIZE] = {0};				//����buff
uint8_t cmd_ack_buff[EASYIOT_CMD_BUFF_ACK_MAX_SIZE] = {0};		//����buff ack
uint8_t convert_buff[EASYIOT_CONVERT_AT_BUFF_MAX_SIZE] = {0};


void ME3616_APP_ErrorHandler(char *file, int line, char * pch)
{
    UNUSED(file);
	UNUSED(line);
	DBG_Print((uint8_t *)pch, DBG_DIR_APP);
	Set_Sys_State(&ME3616_Instance, SYS_STATE_ERR);
	//Halt and do nothing for this Demo.	
	while(1);
}

void Command_Response(Me3616_DeviceType * Me3616, char * pch, uint16_t len)
{
	DBG_Print((uint8_t *)pch, DBG_DIR_RX);

	char tmp_string[5] = {0};
	
	//���´�����ʾ��λ����һ��ATָ��Ļظ�
	//ME3616Ĭ�ϴ���������ԣ������һ�λ��յ����ص�����ж���AT��ͷ����ԡ�
	//�����ڴ�ʹ����������ʱ������
	if((Get_Last_AT_CMD(Me3616) == AT_CMD_MODULE_CIMI) &&
		(strstr(pch, "AT") == NULL))
	{
		HAL_GPIO_WritePin (LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
		DBG_Print("( APP-Demo ) I light up a LED for you  :)", DBG_DIR_APP);
        
        //remember, set last cmd none to prevent execute more than once.
		Set_Last_AT_CMD_None(Me3616);
	}

    //����ź�ǿ��
	if((Get_Last_AT_CMD(Me3616) == AT_CMD_NETWORK_CESQ) &&
			(strstr(pch, "AT") == NULL))
	{
		memset(tmp_string, 0, 5);
		strncpy(tmp_string, pch + 7, 2);
		if(tmp_string[1] == ',') tmp_string[1] = '\0';
		Signal_val = atoi (tmp_string);
		Signal_val += -110;
		
		//remember, set last cmd none to prevent execute more than once.
		Set_Last_AT_CMD_None(Me3616);
	}
			
	//ģ��ͨ��ADC��õ�ص���
	if((Get_Last_AT_CMD(Me3616) == AT_CMD_HARDWARE_ZADC) &&
			(strstr(pch, "AT") == NULL))
	{
		memset(tmp_string, 0, 5);
		strcpy(tmp_string, pch + 6);
		
		Battery_val = atoi (tmp_string);
 		Battery_val = Battery_val * 100 / 255;  //ת��Ϊ�ٷֱ�
		
		//remember, set last cmd none to prevent execute more than once.
		Set_Last_AT_CMD_None(Me3616);
	}
		
}


//ƽ̨���ص����ݴ���easy iot sdk
void M2MCLIRECV_Callback(Me3616_DeviceType * Me3616, char * pch, uint16_t len)
{
	DBG_Print("M2MCLIRECV Below:",  DBG_DIR_AT);
	DBG_Print((uint8_t *)pch,  DBG_DIR_RX);

	CoapHexInputStatic(pch + 12, cmd_buff, EASYIOT_CMD_BUFF_MAX_SIZE);
}



//��ѯ������ģ����Ϣ
void me3616_test_information(Me3616_DeviceType * Me3616)
{
	//����ATӦ��	
	ME3616_Send_AT_Command(Me3616, AT_CMD_NONE, AT_BASE, true, NULL);
	HAL_Delay(1000);

    // ��ѯģ����Ϣ
    // ATI
    if (ME3616_Send_AT_Command(Me3616, AT_CMD_MODULE_I, AT_BASE, false, NULL) == false) 
        ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

    // ��ѯ��ǰ BAND ֵ
    // AT*MBAND?
    if (ME3616_Send_AT_Command(Me3616, AT_CMD_NETWORK_MBAND, AT_READ, false, NULL) == false) 
        ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
    
    // ��ȡ SIM ���� ICCID
    // AT*MICCID
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_SIM_MICCID, AT_BASE, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

	// ��ѯ�����ƶ�̨�豸��ʶ
	// AT+CIMI
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_MODULE_CIMI, AT_BASE, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
		
	// ��ѯ��Ʒ����IMEISV
	// AT+CGSN=2
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_MODULE_CGSN, AT_SET, false, "2") == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
	
    // ��ѯ����ע��״̬
    // AT+CEREG?
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_NETWORK_CEREG, AT_READ, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
	
    // ��ѯADC��ѹֵ
    // AT+ZADC?
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_HARDWARE_ZADC, AT_READ, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

    // ��ѯĬ�ϵ� PSD ��������
    // AT*MCGDEFCONT?
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_PDN_MCGDEFCONT, AT_READ, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
	
    // ��ѯ��ǰ����״̬��С����Ϣ
    // AT*MENGINFO=0
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_NETWORK_MENGINFO, AT_SET, false, "0") == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

    // ��ȡ�ٶ�www.baidu.com��IP��ַ
    // AT+EDNS="www.baidu.com"
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_DNS_EDNS, AT_SET, false, "\"www.baidu.com\"") == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

	// �������      ��---ע�⣬��������ʱ�ϳ������׵���AT��ʱ������Ĭ�Ͽ���������---��
	// AT+IPERF=-c 219.144.130.27 -u -p 7000 -I 5 -t 10
	//if (ME3616_Send_AT_Command(Me3616, AT_CMD_IPERF_IPERF, AT_SET, true, "-c 219.144.130.27 -u -p 7000 -I 5 -t 10") == false) 
	//	ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
	
}


//����LWM2M�����ӡ�������ʹ����ʾIMEI�ţ��������������ƽ̨ע��
void me3616_test_lwm(Me3616_DeviceType * Me3616)
{
	//���Ѿ���ɵ���easy-iotע�ᣬ�밴��Ҫ���޸�
	//����Ϊ��ATָ���ֲ�V1.8.pdf���ṩ����ʾimei
	char * imei = "123456789012396";  
	
	//���ŷ�������ַ,�˿�
	char * server_IP_Port = "180.101.147.115,5683";

	//����ʱ��	
	char * client_Lifetime = "300";
	char command_string[50] = {0};

	//��Ҫ���͵����ݡ���ע�⣬���ݳ���Ҫ��Ϊż����
	char * client_data = "AA123456,1";

	
	sprintf(command_string, "%s,\"%s\",%s", server_IP_Port, imei, client_Lifetime);

	//����ATӦ��	
	ME3616_Send_AT_Command(Me3616, AT_CMD_NONE, AT_BASE, true, NULL);
	HAL_Delay(1000);


	// ע�����IOTƽ̨
	// AT+M2MCLINEW=180.101.147.115,5683,"123456789012396",300
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_LWM_M2MCLINEW, AT_SET, false, command_string) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

	//�ȴ�ע��ɹ�
	while(Get_Sys_State (Me3616, SYS_STATE_LWM_OBSERVE_SUCCESS) == false);
	
	// ���ݷ���
	// AT+M2MCLISEND=AA123456,1
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_LWM_M2MCLISEND, AT_SET, false, client_data) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

	//�ȴ��ظ��ɹ�
	while(Get_Sys_State (Me3616, SYS_STATE_LWM_NOTIFY_SUCCESS) == false);

	// ע������IOTƽ̨
	// AT+M2MCLIDEL
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_LWM_M2MCLIDEL, AT_BASE, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

}







/*

����Ϊ easy iot ƽ̨��sdk��ش����ʹ����ʾ

*/

uint8_t getBattery(void)
{
	return Battery_val;
}

uint64_t getTimestamp(void)
{
	return (uint64_t)HAL_GetTick();
}

int32_t getSignal(void)
{
	return Signal_val;
}


void ack_handler(struct Messages* req)
{
	DBG_Print ("msg_ack received.", DBG_DIR_APP);
}

void SendtoModule(const uint8_t* data, uint16_t inLength)
{
	//����ת��Ϊ�ַ���
	Hex2Str((char *)convert_buff, (char *)data, inLength);
	convert_buff[inLength * 2 ] = '\0';
    if((inLength / 2) == 1)
    {
        inLength++;
		convert_buff[inLength * 2 ] = '0';
        convert_buff[inLength * 2 +1] = '\0';
    }
    
	if (ME3616_Send_AT_Command(&ME3616_Instance, AT_CMD_LWM_M2MCLISEND, AT_SET, false, (char *)convert_buff) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "easy-iot LWM2M send failed.");
}


void SendtoDBG(const uint8_t* data, uint16_t inLength)
{
	DBG_Print((uint8_t *)data, DBG_DIR_APP);	
}



void cmd_handler_callback(struct Messages* req)
{
	int8_t ret = 0;
	int8_t cmd_value = 0;
	ret = GetInt8(req, LED_GREEN_TLV_PARAMID, &cmd_value);
	if( ret < 0) 
	{
		Logging (LOG_WARNING, "get cmd %d value fail", req->msgid);
		return;
	}
	else
	{
        if(cmd_value == 0)
        {
            HAL_GPIO_WritePin (LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
        }
        else
        {
            HAL_GPIO_WritePin (LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        }
	}
	last_dtag_mid = req->dtag_mid;
	Set_Sys_State (&ME3616_Instance, SYS_STATE_LWM_NEED_CMD_ACK);	
}



void me3616_test_easyiot(Me3616_DeviceType * Me3616)
{
    char command_string[50] = {0};
    //����ʱ��	
	char * client_Lifetime = "90";

    //���ŷ�������ַ,�˿�
	char * server_IP_Port = "117.60.157.137,5683";
    sprintf(command_string, "%s,\"%s\",%s", server_IP_Port, client_imei, client_Lifetime);

	//����ATӦ��	
	ME3616_Send_AT_Command(Me3616, AT_CMD_NONE, AT_BASE, true, NULL);
	HAL_Delay(1000);

	// ��ѯ�����ź�ǿ��
	// AT+CESQ
	if (ME3616_Send_AT_Command(Me3616, AT_CMD_NETWORK_CESQ, AT_BASE, false, NULL) == false) 
		ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

	if(Signal_val > -48 ) DBG_Print ("No Signal or Out of range.", DBG_DIR_AT);
	
	// ��ѯADC��ѹֵ
    // AT+ZADC?
    if (ME3616_Send_AT_Command(Me3616, AT_CMD_HARDWARE_ZADC, AT_READ, false, NULL) == false) 
	    ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");

	HAL_Delay(3000);


	for(uint8_t i = 0; i < 10; i++)
	{
		if(ME3616_Send_AT_Command(Me3616, AT_CMD_LWM_M2MCLINEW, AT_SET, false, command_string) == false) 
			ME3616_APP_ErrorHandler(__FILE__, __LINE__, "APP Command fault, Halt.");
        
        HAL_Delay(5000);
		if(Get_Sys_State(Me3616, SYS_STATE_LWM_REGISTER_SUCCESS) == true &&
		   Get_Sys_State(Me3616, SYS_STATE_LWM_OBSERVE_SUCCESS) == true)
			break;
		
		if(i >= 5 ) ME3616_APP_ErrorHandler(__FILE__, __LINE__, "LWM2M register/observe failed.");
	}
	

	HAL_Delay(3000);


	
	EasyIotInit(client_imei, client_imsi);

	setsTimestampCb(getTimestamp);
	setSignalCb(getSignal);
	setBatteryCb(getBattery);
	
	setLogSerialOutputCb(SendtoDBG);
	setNbSerialOutputCb(SendtoModule);

	//���������callback
	setCmdHandler(CMD_1_CMDID, cmd_handler_callback);
	//������Ϣack��callback
	setAckHandler(ack_handler);
    
    
	struct Messages * msg = NewMessageStatic(msg_buff, EASYIOT_MSG_BUFF_MAX_SIZE);

	setMessages(msg, CMT_USER_UP, MSG_1_MSGID);
	msg->dtag_mid = last_dtag_mid++;
	AddInt8(msg, SENSOR_1_TLV_PARAMID, 50);
	AddInt32 (msg, SENSOR_2_TLV_PARAMID, 666);
	AddInt8 (msg, LED_GREEN_TLV_PARAMID, 0);
	pushMessages(msg);

	FreeMessage(msg);

	HAL_Delay(2000);
	
	while(1)
	{
		if(Get_Sys_State(Me3616, SYS_STATE_LWM_NEED_CMD_ACK) == true)
		{
			//response cmd ack
			struct Messages * msg = NewMessageStatic(cmd_ack_buff, EASYIOT_CMD_BUFF_ACK_MAX_SIZE);
			setMessages(msg, CMT_USER_CMD_RSP, CMD_1_CMDID);
			msg->dtag_mid = last_dtag_mid;
			AddInt8(msg, 0, 0);		//ִ�н���ظ�
			if(HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) == GPIO_PIN_SET)
			{
				AddInt8(msg, LED_GREEN_TLV_PARAMID, 1);
			}
			else
			{
				AddInt8(msg, LED_GREEN_TLV_PARAMID, 0);
			}
			pushMessages(msg);
			FreeMessage(msg);

			Clear_Sys_State(Me3616, SYS_STATE_LWM_NEED_CMD_ACK);
		}
		HAL_Delay(10000);
	}
}


void ME3616_APP(Me3616_DeviceType * Me3616)
{
	HAL_Delay(1000);

	me3616_test_information(Me3616);
    
	me3616_test_lwm(Me3616);

//	me3616_test_easyiot(Me3616);

}


