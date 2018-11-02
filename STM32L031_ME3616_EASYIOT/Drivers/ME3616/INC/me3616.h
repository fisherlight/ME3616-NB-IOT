	/**
	  ******************************************************************************
	  * @file	 me3616.h
	  * @author  Simon Luk (simonluk@unidevelop.net)
	  * @brief	 This file is the header of me3616.c 
	  *          provides driver functions to handle internal AT Command for
	  * 		 GOSUNCN ME3616 NB-IoT Module Made by emakerzone
	  *
	  ******************************************************************************
	  * @attention
	  *
	  * <h2><center>&copy; COPYRIGHT(c) 2018 Simon Luk </center></h2>
	  *
	  * Redistribution and use in source and binary forms, with or without modification,
	  * are permitted provided that the following conditions are met:
	  *   1. Redistributions of source code must retain the above copyright notice,
	  * 	 this list of conditions and the following disclaimer.
	  *   2. Redistributions in binary form must reproduce the above copyright notice,
	  * 	 this list of conditions and the following disclaimer in the documentation
	  * 	 and/or other materials provided with the distribution.
	  *   3. Neither the name of Simon Luk nor the names of its contributors
	  * 	 may be used to endorse or promote products derived from this software
	  * 	 without specific prior written permission.
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

#ifndef __ME3616_H__
#define __ME3616_H__

#ifdef __cplusplus
    extern "C" {
#endif 
     
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#include "main.h"
#include "stm32l0xx_hal.h"


//use DBG_Print() to foward Tx and Rx and print inner debug message, using DBG_UART.
#define DEBUG_ME3616
   

#define ME3616_UART                     huart2 
extern UART_HandleTypeDef				ME3616_UART;

#define DBG_UART						hlpuart1
extern UART_HandleTypeDef				DBG_UART;

#define ME3616_Power_Port				POWER_ON_EN_GPIO_Port
#define ME3616_Power_Pin				POWER_ON_EN_Pin

#define ME3616_Reset_Port				NB_RST_EN_GPIO_Port
#define ME3616_Reset_Pin				NB_RST_EN_Pin

//A Buffer for ME3616, from MCU to ME3616
#define ME3616_TX_BUFFER_SIZE 			200

//A Buffer for ME3616, from ME3616 to MCU
#define ME3616_RX_BUFFER_SIZE           200

//A Buffer for DBG, from MCU to PC
#define ME3616_DBG_TX_BUFFER_SIZE 		200

//A Buffer for DBG, From PC to MCU
#define ME3616_DBG_RX_BUFFER_SIZE 		200

//Wait ME3616 boot up
#define ME3616_BOOT_TIMOUT              40000

//Wait until AT_State ready to Send
#define ME3616_SEND_TIMOUT              5000

//Wait ME3616 response OK / ERROR
#define ME3616_RECEIVE_TIMOUT           10000

//Buffer size to store IP address
#define ME3616_IPV4_SIZE                18
#define ME3616_IPV6_SIZE                42


//AT command set.
//Caution! These enum defines below, MUST Be corresponded with AT_CMD_String[] at me3616.c.
//For further AT command, refer to GOSUNCN AT Command Manual.
typedef enum {
    //ģ����Ϣʶ��ָ��
    AT_CMD_MODULE_I,						//��ѯģ��ʶ����Ϣ
	AT_CMD_MODULE_GMI,                      //��ѯ����������
	AT_CMD_MODULE_CGMI,						//��ѯ����������
	AT_CMD_MODULE_GMM,                      //��ѯģ�� ID
	AT_CMD_MODULE_CGMM,						//��ѯģ�� ID
	AT_CMD_MODULE_GMR,                      //��ѯ����汾��
	AT_CMD_MODULE_CGMR,						//��ѯ����汾��
	AT_CMD_MODULE_GSN,                      //��ѯ��Ʒ���к�
	AT_CMD_MODULE_CGSN,						//��ѯ��Ʒ��Ӧ�����б�ʶ
	AT_CMD_MODULE_CIMI,						//��ѯ�����ƶ�̨�豸��ʶ
	AT_CMD_MODULE_ZPCBS,                    //��ѯ PCB ��
	
    //ͨ������
    AT_CMD_COMMON_F,                        //�ָ���������
	AT_CMD_COMMON_V,                        //��ʾ��ǰ����
	AT_CMD_COMMON_ATZ,                      //��λΪȱʡ����
    AT_CMD_COMMON_ATQ,                      //���������
    AT_CMD_COMMON_ATE,                      //��������
    AT_CMD_COMMON_ATV,                      //DCE ���ظ�ʽ
    AT_CMD_COMMON_CFUN,                     //���õ绰����
    AT_CMD_COMMON_CMEE,                     //�ϱ��豸����
    AT_CMD_COMMON_CME,                      //ERROR ME ��������

    //���ڿ���ָ��
    AT_CMD_SERIAL_IPR,                      //�趨���ڲ�����
    AT_CMD_SERIAL_CMUX,                     //���ڶ�·����
    AT_CMD_SERIAL_IFC,                      //DTE-DCE �ı�������
    AT_CMD_SERIAL_ZCOMWRT,                  //��������д�ļ�

    //SIM�������
    AT_CMD_SIM_CLCK,                        //������
    AT_CMD_SIM_CPWD,                        //�ı�������
    AT_CMD_SIM_CPIN,                        //���� PIN ��
    AT_CMD_SIM_CRSM,                        //�����Ƶ� SIM ����
    AT_CMD_SIM_MICCID,                      //��ȡ SIM ���� ICCID

    //��������������
    AT_CMD_NETWORK_CEREG,                   //EPS ����ע��״̬
    AT_CMD_NETWORK_COPS,                    //PLMN ѡ��
    AT_CMD_NETWORK_CESQ,                    //�ź�ǿ�Ȳ�ѯ
    AT_CMD_NETWORK_CSQ,                     //�ź�ǿ�Ȳ�ѯ
    AT_CMD_NETWORK_CTZU,                    //�Զ���ȡ����ʱ�俪��
    AT_CMD_NETWORK_CTZR,                    //ʱ�����濪��
    AT_CMD_NETWORK_CCLK,                    //ʱ�ӹ���
    AT_CMD_NETWORK_MSPCHSC,                 //���������㷨
    AT_CMD_NETWORK_MFRCLLCK,                //��Ƶ��/����С��
    AT_CMD_NETWORK_MBAND,                   //��ѯ��ǰ BAND ֵ
    AT_CMD_NETWORK_MBSC,                    //�� BAND
    AT_CMD_NETWORK_MENGINFO,                //��ѯ��ǰ����״̬��С����Ϣ
    AT_CMD_NETWORK_MNBIOTRAI,               //�����ͷ� RRC ����

    //�͹����������
    AT_CMD_LOWPOWER_CEDRXS,                 //eDRX ����
    AT_CMD_LOWPOWER_CEDRXRDP,               //eDRX ��̬������ȡ
    AT_CMD_LOWPOWER_CPSMS,                  //�ڵ�ģʽ��PSM������
    AT_CMD_LOWPOWER_ZSLR,                   //ϵͳ˯�߿���
    AT_CMD_LOWPOWER_SETWAKETIME,            //����ģ�黽��ʱ��
    AT_CMD_LOWPOWER_MNBIOTEVENT,            //��ֹ/ʹ�� PSM ״̬�����ϱ�
    AT_CMD_LOWPOWER_ESOWKUPDELAY,           //����������ʱ  

    //����������
    AT_CMD_PDN_MCGDEFCONT,                  //����Ĭ�ϵ� PSD ��������
    AT_CMD_PDN_CGCONTRDP,                   //��ȡ PDP �����Ĳ���
    AT_CMD_PDN_IP,                          //�Զ����� IP �ϱ�
    AT_CMD_PDN_EGACT,                       //����/ȥ���� PDN ������

    //Ӳ����ؼ���չAT����
    AT_CMD_HARDWARE_ZADC,                   //��ȡ ADC �ܽ�ֵ
    AT_CMD_HARDWARE_ZRST,                   //ģ�鸴λ
    AT_CMD_HARDWARE_ZTURNOFF,               //�ر�ģ��
    AT_CMD_HARDWARE_ZCONTLED,               //״ָ̬ʾ�źſ��ƹ���
    AT_CMD_HARDWARE_PWRKEYSTA,              //���ô�/�ر� POWERKEY �����͹���

    //�����AT����
    AT_CMD_DNS_EDNS,                        //ͨ��������ȡ IP ��ַ

    //TCP/IP���AT����
    AT_CMD_TCPIP_ESOC,                      //����һ�� TCP/UDP
    AT_CMD_TCPIP_ESOCON,                    //�׽������ӵ�Զ�̵�ַ�Ͷ˿�
    AT_CMD_TCPIP_ESOSEND,                   //��������
    AT_CMD_TCPIP_ESOCL,                     //�ر��׽���
    AT_CMD_TCPIP_ESONMI,                    //�׽�����Ϣ����ָʾ��
    AT_CMD_TCPIP_ESOERR,                    //�׽��ִ���ָʾ��
    AT_CMD_TCPIP_ESOSETRPT,                 //�������ݵ���ʾ��ʽ
    AT_CMD_TCPIP_ESOREADEN,                 //�����������������ϱ�
    AT_CMD_TCPIP_ESODATA,                   //���ݵ��������ϱ�
    AT_CMD_TCPIP_ESOREAD,                   //��ȡ����
    AT_CMD_TCPIP_ESOSENDRAW,                //����ԭʼ����
    AT_CMD_TCPIP_PING,                      //ͨ������Э��ջ ping ������

     //MQTT���AT����
    AT_CMD_MQTT_EMQNEW,                     //�����µ� MQTT
    AT_CMD_MQTT_EMQCON,                     //�� MQTT �������������ӱ���
    AT_CMD_MQTT_EMQDISCON,                  //�Ͽ��� MQTT ������������
    AT_CMD_MQTT_EMQSUB,                     //���� MQTT ���ı���
    AT_CMD_MQTT_EMQUNSUB,                   //���� MQTT ȡ�����ı���
    AT_CMD_MQTT_EMQPUB,                     //���� MQTT ��������

    //CoAP���AT����
    AT_CMD_COAP_ECOAPSTA,                   //����һ�� COAP ������
    AT_CMD_COAP_ECOAPNEW,                   //����һ�� COAP �ͻ���
    AT_CMD_COAP_ECOAPSEND,                  //COAP �ͻ��˷�������
    AT_CMD_COAP_ECOAPDEL,                   //���� CoAP �ͻ���ʵ��
    AT_CMD_COAP_ECOAPNMI,                   //���ط���������Ӧ    

    //HTTP/HTTPS�������
    AT_CMD_HTTP_EHTTPCREATE,                //�����ͻ��� HTTP/HTTPS ʵ��
    AT_CMD_HTTP_EHTTPCON,                   //���� HTTP/HTTPS ����
    AT_CMD_HTTP_EHTTPDISCON,                //�ر� HTTP/HTTPS ����
    AT_CMD_HTTP_EHTTPDESTROY,               //�ͷŴ����� HTTP/HTTPS ����
    AT_CMD_HTTP_EHTTPSEND,                  //���� HTTP/HTTPS ����
    AT_CMD_HTTP_EHTTPNMIH,                  //��������Ӧ��ͷ��Ϣ
    AT_CMD_HTTP_EHTTPNMIC,                  //��������Ӧ��������Ϣ
    AT_CMD_HTTP_EHTTPERR,                   //�ͻ������ӵĴ�����ʾ

    //���� IOT ������� AT ����
    AT_CMD_LWM_M2MCLINEW,                   //LWM2M Client ע����� IOT ƽ̨
    AT_CMD_LWM_M2MCLIDEL,                   //LWM2M Client ȥע����� IOT ƽ̨
    AT_CMD_LWM_M2MCLISEND,                  //LWM2M Client ���ݷ���
    AT_CMD_LWM_M2MCLIRECV,                  //LWM2M Client �����ϱ�
    AT_CMD_LWM_M2MCLICFG,                   //���ݷ��ͺ��ϱ�ģʽ����

    //IPERF �������
    AT_CMD_IPERF_IPERF,                     //IPERF �������

    //FOTA ���ָ��
    AT_CMD_FOTA_FOTATV,                     //���� FOTA ��������
    AT_CMD_FOTA_FOTACTR,                    //���� WeFOTA ����
    AT_CMD_FOTA_FOTAIND,                    //WeFOTA ����״̬����

    //FTP ��� AT ָ��
    AT_CMD_FTP_ZFTPOPEN,                    //�����ļ�����
    AT_CMD_FTP_ZFTPCLOSE,                   //�ر��ļ�����
    AT_CMD_FTP_ZFTPSIZE,                    //��ȡ FTP �ļ���С
    AT_CMD_FTP_ZFTPGET,                     //�ļ�����
    AT_CMD_FTP_ZFTPPUT,                      //�ļ��ϴ�����

    //GPS ���ָ��
    AT_CMD_GPS_ZGMODE,                      //���ö�λģʽ
    AT_CMD_GPS_ZGURL,                       //���� AGPS �������� URL
    AT_CMD_GPS_ZGAUTO,                      //���� AGNSS �����Զ����ع���
    AT_CMD_GPS_ZGDATA,                      //���ػ��ѯ AGNSS ����
    AT_CMD_GPS_ZGRUN,                       //����/�ر� GPS ����
    AT_CMD_GPS_ZGLOCK,                      //���õ��ζ�λ��ʹ������ϵͳ˯��
    AT_CMD_GPS_ZGTMOUT,                     //���õ��ζ�λ��ʱʱ��
    AT_CMD_GPS_ZGRST,                       //���� GPS
    AT_CMD_GPS_ZGPSR,                       //ʹ��/��ֹ+ZGPSR �ϱ�
    AT_CMD_GPS_ZGNMEA,                      //���� GPS ���� NMEA �ϱ���ʽ    

    //�й��ƶ� OneNET ƽ̨������� AT ����
    AT_CMD_MIP_MIPLCREATE,                  //���� OneNET instance
    AT_CMD_MIP_MIPLDELETE,                  //ɾ�� OneNET instance
    AT_CMD_MIP_MIPLOPEN,                    //�豸ע�ᵽ OneNET ƽ̨
    AT_CMD_MIP_MIPLCLOSE,                   //ȥע�� OneNET ƽ̨
    AT_CMD_MIP_MIPLADDOBJ,                  //����һ�� object������
    AT_CMD_MIP_MIPLDELOBJ,                  //ɾ��һ�� object������
    AT_CMD_MIP_MIPLUPDATE,                  //ע���������
    AT_CMD_MIP_MIPLREAD,                    //OneNET ƽ̨��ģ�鷢�� read ����
    AT_CMD_MIP_MIPLREADRSP,                 //ģ����Ӧƽ̨�� READ ����
    AT_CMD_MIP_MIPLWRITE,                   //OneNET ƽ̨��ģ�鷢�� write ����
    AT_CMD_MIP_MIPLWRITERSP,                //ģ����Ӧƽ̨�� WRITE ����
    AT_CMD_MIP_MIPLEXECUTE,                 //OneNET ƽ̨��ģ�鷢�� execute ����
    AT_CMD_MIP_MIPLEXEUTERSP,               //ģ����Ӧƽ̨�� execute ����
    AT_CMD_MIP_MIPLOBSERVE,                 //OneNET ƽ̨��ģ�鷢�� observe ����
    AT_CMD_MIP_MIPLOBSERVERSP,              //ģ����Ӧƽ̨�� observe ����
    AT_CMD_MIP_MIPLDISCOVER,                //OneNET ƽ̨��ģ�鷢�� discover ����
    AT_CMD_MIP_MIPLDISCOVERRSP,             //ģ����Ӧƽ̨�� DISCOVER ����
    AT_CMD_MIP_MIPLPARAMETER,               //OneNET ƽ̨��ģ�鷢������ parameter ����
    AT_CMD_MIP_MIPLPARAMETERRSP,            //ģ����Ӧƽ̨������ paramete ����
    AT_CMD_MIP_MIPLNOTIFY,                  //ģ����ƽ̨����ͬ������
    AT_CMD_MIP_MIPLVER,                     //��ѯ OneNET SDK �汾��
    AT_CMD_MIP_MIPLEVENT,                   //ģ��״̬�ϱ�

	AT_CMD_NONE,
    AT_CMD_IGNORE = 254
}AT_CMD_t;

typedef enum {
    AT_STATE_NONE = 0,                      //δ���͹�����
    AT_STATE_SEND,                          //�ѷ���
    AT_STATE_ATOK,                          //�ظ�OK
    AT_STATE_TIMEOUT,                       //��Ӧ��ʱ
    AT_STATE_ATERR,                         //�ظ�����
    AT_STATE_IGNORE = 254                   //����
}AT_State_t;

typedef enum {
	AT_BASE = 0,		//  
    AT_SET,				//  "="
    AT_READ,			//  "?"
    AT_TEST,			//  "=?"
    AT_ACTION_IGNORE = 254
}AT_Action_t;

typedef struct {
    AT_CMD_t            LastCMDBase;
    AT_Action_t         LastCMDAction; 
    AT_State_t          At_State;   
}AT_Cmd_Info_t;

typedef enum {
    SYS_STATE_POWERON =                 0x00000001,
    SYS_STATE_READY =                   0x00000002,
    SYS_STATE_BUSY =                    0x00000004,
    SYS_STATE_ERR =                     0x00000008,
    SYS_STATE_UNKNOW =                  0x00000010,

	SYS_STATE_MATREADY =                0x00000100,
	SYS_STATE_CPIN =                    0x00000200,
	SYS_STATE_CFUN =                    0x00000400,
	SYS_STATE_IPV4 =                    0x00000800,
	SYS_STATE_IPV6 =                    0x00001000,

	SYS_STATE_LWM_REGISTER_SUCCESS =	0x00010000,
	SYS_STATE_LWM_OBSERVE_SUCCESS =		0x00020000,
	SYS_STATE_LWM_NOTIFY_SUCCESS =		0x00040000,
	SYS_STATE_LWM_NEED_CMD_ACK =		0x00080000,
	
	//...add your Sys_State here

	
	SYS_STATE_INCOMMING_NEW_AT_STRING = 0x40000000
	
}SYS_State_t;

typedef struct __Me3616_DeviceType
{
	AT_Cmd_Info_t       AT_Info;                          		
    SYS_State_t        	Sys_State;

	UART_HandleTypeDef  * UartDevice;
    DMA_HandleTypeDef   * UartDMA_Tx;
    DMA_HandleTypeDef   * UartDMA_Rx;
    
	uint32_t	    	TxDataLastTime;							//SysTick time
	uint32_t 	    	RxDataLastTime;							//SysTick time

 	uint8_t				DBG_TxBuffer[ME3616_DBG_TX_BUFFER_SIZE +1];
 	uint8_t				DBG_RxBuffer[ME3616_DBG_RX_BUFFER_SIZE +1];

 	uint8_t 		    TxBuffer[ME3616_TX_BUFFER_SIZE +1];
 	uint16_t			TxStringLen;

 	uint16_t	    	RxStringBegin;
    uint16_t	    	RxStringEnd;
    uint16_t            RxStringLen;
    uint8_t             RxVaildString[ME3616_RX_BUFFER_SIZE +1];
 	uint8_t 	    	RxBuffer[ME3616_RX_BUFFER_SIZE +1];
       
	uint8_t		    	IPv4[ME3616_IPV4_SIZE];
	uint8_t		    	IPv6[ME3616_IPV6_SIZE];
    
}Me3616_DeviceType;


//ME3616 Instance Declaration
extern Me3616_DeviceType ME3616_Instance;


//Function Declaraion For me3616.h
void ME3616_ErrorHandler(char *file, int line, char * pch);

AT_State_t Get_AT_State(Me3616_DeviceType * Me3616);

AT_Action_t Get_Last_AT_Action(Me3616_DeviceType * Me3616);

AT_CMD_t Get_Last_AT_CMD(Me3616_DeviceType * Me3616);

void Set_Last_AT_CMD_None(Me3616_DeviceType * Me3616);

void Set_AT_Info(Me3616_DeviceType * Me3616, AT_CMD_t at_cmd, AT_Action_t at_action, AT_State_t at_state);

void Set_Sys_State(Me3616_DeviceType * Me3616, SYS_State_t mask);

void Clear_Sys_State(Me3616_DeviceType * Me3616, SYS_State_t mask);

bool Get_Sys_State(Me3616_DeviceType * Me3616, SYS_State_t mask);

void ME3616_PowerOn(Me3616_DeviceType * Me3616, uint32_t delay_ticks);

void ME3616_Reset(Me3616_DeviceType * Me3616, uint32_t	delay_ticks);

bool ME3616_Init(Me3616_DeviceType * Me3616, UART_HandleTypeDef * AT_huart, DMA_HandleTypeDef * DmaTx, DMA_HandleTypeDef * DmaRx);

bool ME3616_Send_AT_Command(Me3616_DeviceType * Me3616,  AT_CMD_t at_cmd, AT_Action_t at_action, bool override, char * pch);

void AT_ResultReport(Me3616_DeviceType * Me3616, bool result);

void Active_Report(Me3616_DeviceType * Me3616, char *pch, uint16_t len);

bool Wait_AT_SendReady(Me3616_DeviceType * Me3616);

bool Wait_AT_Response(Me3616_DeviceType * Me3616);

void RxHandler(Me3616_DeviceType * Me3616, char *p_Buff, uint16_t len);

void ME3616_String_Receive(Me3616_DeviceType * Me3616);

bool Check_Response(Me3616_DeviceType * Me3616, char *pch, uint16_t len);

void Hex2Str(char *sDest, const char *sSrc, int nSrcLen);

bool HexStrToByte(unsigned char* dest, const char* source, int sourceLen);

void Command_Response( Me3616_DeviceType * Me3616, char * pch, uint16_t len);
	
void MATREADY_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void CME_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void CFUN_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void CPIN_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void IP_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void ESONMI_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void ESODATA_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void EMQDISCON_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void EMQPUB_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void ECOAPNMI_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void M2MCLI_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void M2MCLIRECV_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void IPERF_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void ZGPSR_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void MIPLEVENT_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void MIPLREAD_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void MIPLWRITE_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void MIPLOBSERVE_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void MIPLDISCOVER_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void MIPLPARAMETER_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);

void UnknowActiveReport_Callback( Me3616_DeviceType * Me3616, char * pch, uint16_t len);


//Function Declaraion For me3616_if.h
void ME3616_IF_ErrorHandler(char *file, int line, char * pch);

void Init_UART_CM(UART_HandleTypeDef * huart);

bool UART_AT_Send(Me3616_DeviceType * Me3616);

void UART_AT_Receive(Me3616_DeviceType * Me3616);

void DBG_Forward(Me3616_DeviceType * Me3616);


//Function Declaraion For me3616_app.h
void ME3616_APP_ErrorHandler(char *file, int line, char * pch);
void ME3616_APP(Me3616_DeviceType * Me3616);


#ifdef DEBUG_ME3616

typedef enum {
    DBG_DIR_RX = 0,
    DBG_DIR_TX,
    DBG_DIR_AT,
    DBG_DIR_SDK,
    DBG_DIR_APP
}DBG_DIR_t;	
	
void DBG_Print(const char * ch, DBG_DIR_t direction);
void State_Hex2Str(char *sDest, uint32_t uSrc);

#else
#define DBG_Print(...)    UNUSED(0)
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ME3616_H__ */
