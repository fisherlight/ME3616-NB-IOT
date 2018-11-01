/*************************************************************************
    > File Name:    easyiot.h
    > Author:       Guangdong Research Institute of China Telecom Corporation Ltd.
	> See More:     https://www.easy-iot.cn/
	> Description:  EasyIoT SDK 
    > Created Time: 2018/01/01
 ************************************************************************/

#ifndef _GUARD_H_EASYIOT_H_
#define _GUARD_H_EASYIOT_H_

#include <stdint.h>

#define EASY_IOT_VERSION "0.0.1"


enum TlvValueType {
	TLV_TYPE_BYTE=0x01,
	TLV_TYPE_SHORT,
	TLV_TYPE_INT32,
	TLV_TYPE_LONG64,
	TLV_TYPE_FLOAT,
	TLV_TYPE_DOUBLE,
	TLV_TYPE_BOOL,
	TLV_TYPE_ENUM,
	TLV_TYPE_STRING_ISO_8859,
	TLV_TYPE_STRING_HEX,
	TLV_TYPE_UNKNOWN,
};

enum CoapMessageType {
	CMT_USER_UP = 0xF0,
	CMT_USER_UP_ACK = 0xF1,
	CMT_USER_CMD_REQ = 0xF2,
	CMT_USER_CMD_RSP = 0xF3,
	CMT_SYS_CONF_REQ = 0xF4,
	CMT_SYS_CONF_RSP = 0xF5,
	CMT_SYS_QUERY_REQ = 0xF6,
	CMT_SYS_QUERY_RSP = 0xF7
};

/*
TLV �ṹ��
*/
struct TLV {
	uint16_t length;
	uint8_t type;
	uint8_t vformat;
	uint8_t *value;
};

/*
* Messages��������������� 5�� TLV
*/
#define MESSAGE_MAX_TLV 5
struct Messages {
    
    /* ��̬�ڴ���������� MessageMalloc�����У�ʵ����һ���򵥵ľ�̬�ڴ������ƣ���ֻ���䣬���ͷš� */
	uint8_t * sbuf;
	// dtag_mid��������������ʹ�� dtag ���壬������һ���϶̵�ʱ���ڣ�ȥ���ظ����е�����
	// ������ָ���У�ȡ��mid���壬���е�ָ����Ӧ����ʹ��ͬ����midֵ�����Թ���ָ���ִ�н����
	uint16_t dtag_mid;

	// msgid����ϢID����Ϊ������Ϣ��Type����EasyIoTƽ̨������Ϣʱ����ֵ�ᱻ����
	// �ڵ�����ͷ�ļ��У����д�ֵ�ĺ궨�壬�����н���ʹ�ú궨�塣
	uint8_t msgid;

	// ��ǰMessage�����е�TLV�����ܺ�
	uint8_t tlv_count;

	
    
    // �ڴ�buffer��ʼ��ַ��һ����ֵ�������ٱ䶯�������ڴ�ʹ�ã��� sbuf_offset ֵȷ��
	uint16_t sbuf_offset;
	// ԭʼ sbuf ����󳤶�
	uint16_t sbuf_maxlength;
	// �Ƿ�ʹ�þ�̬�ڴ�������
	uint8_t sbuf_use;
	/* ��̬�ڴ������ */

	// enum TlvValueType ֵ�����������Ǻ�����������
	uint8_t msgType;

	// TLV����ָ������
	struct TLV* tlvs[MESSAGE_MAX_TLV];
};

enum LoggingLevel {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL
};

struct TLV* NewTLV(uint8_t type);
void FreeTLV(struct TLV* tlv);

int TLVSerialize(struct TLV* tlv, char* inBuf, uint16_t inMaxLength);

struct Messages* NewMessage(void);
/* static version */
struct Messages* NewMessageStatic(uint8_t* buf, uint16_t inMaxLength);
struct TLV* NewTLVStatic(struct Messages* msg, uint8_t type);

void FreeMessage(struct Messages* msg);
void setMessages(struct Messages* msg, enum CoapMessageType type, uint8_t msgid);

// ���л��뷴���л�
int MessagesSerialize(const struct Messages* msg, char* inBuf, uint16_t inMaxLength);
int MessagesDeserialize(const char* inBuf, uint16_t inLength, struct Messages* out);

/* Coap ����������� */
int CoapInput(struct Messages* msg, uint8_t *data, uint16_t inLength);
int CoapHexInput(const char* data);
int CoapHexInputStatic(const char* data, uint8_t* inBuf, uint16_t inMaxLength);
/*
* �������ͺ���������EasyIoT ƽ̨����� Messages
* �����ڲ������ CoapOutput ���ָ�Ӳ��
*/
int pushMessages(struct Messages *msg);

/* �����ݼ���message�ṹ�� */
int AddInt8(struct Messages* msg, uint8_t type, int8_t v);
int AddInt16(struct Messages* msg, uint8_t type, int16_t v);
int AddInt32(struct Messages* msg, uint8_t type, int32_t v);
int AddFloat(struct Messages* msg, uint8_t type, float v);
int AddDouble(struct Messages* msg, uint8_t type, double v);
int AddString(struct Messages* msg, uint8_t type, const char* v);
int AddBinary(struct Messages* msg, uint8_t type, const char* v, uint16_t length);

/*
 * ���ѷ����л��Ľṹ���л�ȡֵ
 * ���� TLV �ṹ���У�vformat ӦΪ UNKNOWN����value�ֶ�����δ�������л���
 * �������UNKNOWN�ģ��� vformat Ӧ��ƥ�䣻
 */
int GetInt8(const struct Messages* msg, uint8_t type, int8_t* v);
int GetInt16(const struct Messages* msg, uint8_t type, int16_t* v);
int GetInt32(const struct Messages* msg, uint8_t type, int32_t* v);
int GetLong64(const struct Messages* msg, uint8_t type, int64_t* v);
int GetFloat(const struct Messages* msg, uint8_t type, float* v);
int GetDouble(const struct Messages* msg, uint8_t type, double* v);
int GetString(const struct Messages* msg, uint8_t type, char** v);
int GetBinary(const struct Messages* msg, uint8_t type, uint8_t** v);

/* EasyIoT Initialize */
void EasyIotInit(const char* imei, const char* imsi);

typedef uint64_t(*TimestampCbFuncPtr)(void);
typedef uint8_t(*BatteryCbFuncPtr)(void);
typedef int32_t(*SignalCbFuncPtr)(void);
typedef void(*OutputFuncPtr)(const uint8_t* buf, uint16_t length);
typedef void(*CmdHandlerFuncPtr)(struct Messages* req);

void setsTimestampCb(TimestampCbFuncPtr func);
void setSignalCb(SignalCbFuncPtr func);
void setBatteryCb(BatteryCbFuncPtr func);
void setNbSerialOutputCb(OutputFuncPtr func);
void setLogSerialOutputCb(OutputFuncPtr func);
void setAckHandler(CmdHandlerFuncPtr func);
int setCmdHandler(int cmdid, CmdHandlerFuncPtr func);


/* �ײ�ӿ� ���CoAP ����������ͬ��ģ�飬��Ҫʹ�ò�ͬ��PORTING */
int CoapOutput(uint8_t *inBuf, uint16_t inLength);

void SetLogLevel(enum LoggingLevel level);
int Logging(enum LoggingLevel level, const char* fmt, ...);

#endif /* _GUARD_H_EASYIOT_H_ */
