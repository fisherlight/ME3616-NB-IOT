/*************************************************************************
    > File Name:    easyiot.c
    > Author:       Guangdong Research Institute of China Telecom Corporation Ltd.
	> See More:     https://www.easy-iot.cn/
	> Description:  ���ļ�����ʵ���� EasyIoT �ն˲�ӿ�Э�飬ʹ�� Messages ����
                    ������ݵ��ϱ�������ָ���
    > Created Time: 2018/01/01
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "easyiot.h"

static char gl_imei[20];
static char gl_imsi[20];

#define COMMAND_MAX_HANDLER 8
#define STANDARD_IMEI_LENGTH 15
#define STANDARD_IMSI_LENGTH 15
#define EASYIOT_COAP_VERSION 0x01


static TimestampCbFuncPtr gl_timestampcb;
static SignalCbFuncPtr gl_signalcb;
static BatteryCbFuncPtr gl_batterycb;
static CmdHandlerFuncPtr gl_ackhandler;
static OutputFuncPtr gl_nb_out;
static OutputFuncPtr gl_log_out;
static int cmd_handler_count;
static enum LoggingLevel gl_loglevel;


typedef struct {
	int CmdID;
	CmdHandlerFuncPtr ptr;
} cmd_handler_t;
static cmd_handler_t gl_cmd_handlers[COMMAND_MAX_HANDLER];

// ǰ��������

#define nb_htons(_n)  ((uint16_t)((((_n) & 0xff) << 8) | (((_n) >> 8) & 0xff)))
#define nb_ntohs(_n)  ((uint16_t)((((_n) & 0xff) << 8) | (((_n) >> 8) & 0xff)))
#define nb_htonl(_n)  ((uint32_t)( (((_n) & 0xff) << 24) | (((_n) & 0xff00) << 8) | (((_n) >> 8)  & 0xff00) | (((_n) >> 24) & 0xff) ))
#define nb_ntohl(_n)  ((uint32_t)( (((_n) & 0xff) << 24) | (((_n) & 0xff00) << 8) | (((_n) >> 8)  & 0xff00) | (((_n) >> 24) & 0xff) ))

float host2NetFloat(float value);
double host2NetDouble(double value);
int8_t host2NetInt8(int8_t value);
int16_t host2NetInt16(int16_t value);
int32_t host2NetInt32(int32_t value);
int64_t  host2NetInt64(int64_t value);

float net2HostFloat(float value);
double net2HostDouble(double value);
int8_t net2HostInt8(int8_t value);
int16_t net2HostInt16(int16_t value);
int32_t net2HostInt32(int32_t value);
int64_t   net2HostInt64(int64_t value);

int AddBuffer(struct Messages* msg, uint8_t type, uint8_t* v, uint16_t length, uint8_t vformat);

// ��ʼ����ʹ��IMEI��IMSI��ʼ��������ȫ�ֱ����ÿ�
void EasyIotInit(const char* imei, const char* imsi)
{
	Logging(LOG_TRACE, "EasyIoT version %s\n", EASY_IOT_VERSION);
	
	memset(gl_imei, 0, sizeof(gl_imei));
	memset(gl_imsi, 0, sizeof(gl_imsi));
	
	gl_timestampcb = NULL;
	gl_signalcb = NULL;
	gl_batterycb = NULL;
	gl_ackhandler = NULL;
	memset(gl_cmd_handlers, 0, sizeof(gl_cmd_handlers));
	cmd_handler_count = 0;
	gl_loglevel = LOG_TRACE;
	
	if (strlen(imei) != STANDARD_IMEI_LENGTH) {
		Logging(LOG_WARNING, "IMEI %s length not equal %d\n", imei, STANDARD_IMEI_LENGTH);
	} else {
		strcpy(gl_imei, imei);
	}

	if (strlen(imsi) != STANDARD_IMSI_LENGTH) {
		Logging(LOG_WARNING, "IMSI %s length not equal %d\n", imsi, STANDARD_IMSI_LENGTH);
	} else {
		strcpy(gl_imsi, imsi);
	}

	Logging(LOG_TRACE, "EasyIoT Initialize finished.\n");
}


// ���� TIMESTAMP �ص�����
void setsTimestampCb(TimestampCbFuncPtr func)
{
	Logging(LOG_TRACE, "set timestamp callback to 0x%p\n", func);
	gl_timestampcb = func;
}


// ���� �ź�ǿ�� �ص�����
void setSignalCb(SignalCbFuncPtr func)
{
	Logging(LOG_TRACE, "set signal callback to 0x%p\n", func);
	gl_signalcb = func;
}


// ���� ��ص��� �ص�����
void setBatteryCb(BatteryCbFuncPtr func)
{
	Logging(LOG_TRACE, "set battery callback to 0x%p\n", func);
	gl_batterycb = func;
}


// ��������ָ���ص�������ָ��ID��EasyIoTƽ̨��Ԥ����
int setCmdHandler(int cmdid, CmdHandlerFuncPtr func)
{
	Logging(LOG_TRACE, "add cmd %d handler callback to 0x%p\n", cmdid, func);
	if (cmd_handler_count >= COMMAND_MAX_HANDLER) {
		Logging(LOG_ERROR, "COMMAND_MAX_HANDLER count %d\n", COMMAND_MAX_HANDLER);
		return -1;
	}
	
	gl_cmd_handlers[cmd_handler_count].CmdID = cmdid;
	gl_cmd_handlers[cmd_handler_count].ptr = func;
	cmd_handler_count += 1;
	Logging(LOG_TRACE, "add command %d processer 0x%p, current handler count %d.\r\n", cmdid, func, cmd_handler_count);

	return 0;
}


// ���� ����ACK ����ص��������ɹ��������ݺ����ACKȷ��֪ͨ
void setAckHandler(CmdHandlerFuncPtr func)
{
	Logging(LOG_TRACE, "set ack handler to 0x%p\n", func);
	gl_ackhandler = func;
}


// ������NBģ��ͨѶ�Ļص�����
void setNbSerialOutputCb(OutputFuncPtr func)
{
	Logging(LOG_TRACE, "set nb serial output function to 0x%p\n", func);
	gl_nb_out = func;
}


// ������־����Ļص������������ý���������κ���־����־��ʹ��setLogLevel��������ȼ�
void setLogSerialOutputCb(OutputFuncPtr func)
{
	Logging(LOG_TRACE, "set logging serial output function to 0x%p\n", func);
	gl_log_out = func;
}


// Message �ṹ���ʼ��������ʹ�� malloc ��̬�����ڴ�
struct Messages* NewMessage(void)
{
	struct Messages* msg = malloc(sizeof(struct Messages));
	if (!msg) {
		Logging(LOG_WARNING, "new messages, malloc failed.\n");
		return NULL;
	}
	memset(msg, 0, sizeof(struct Messages));
	Logging(LOG_TRACE, "new message from malloc at 0x%p.\r\n", msg);

	return msg;
}


// ����Messages ������Ϣ���� msgid������
void setMessages(struct Messages* msg, enum CoapMessageType type, uint8_t msgid)
{
	Logging(LOG_TRACE, "set msg 0x%p msgid: %d, type: %d.\r\n", msg, msgid, type);

	msg->msgType = type;
	msg->msgid = msgid;
}


// ��̬��ʼ�� Message �ṹ�壬ʹ�ô˺�������� Message ������ addtlv�У��಻�ᶯ̬�����ڴ�
struct Messages* NewMessageStatic(uint8_t* buf, uint16_t inMaxLength)
{
	struct Messages* msg;

	if (!buf) {
		Logging(LOG_WARNING, "new message from static, but buffer is null.\n");
		return NULL;
	}
	if (inMaxLength < sizeof(struct Messages)) {
		Logging(LOG_WARNING, "new message from static, but buffer too small.\n");
		return NULL;
	}

	msg = (struct Messages*)buf;
	memset(msg, 0, sizeof(struct Messages));

	msg->sbuf_use = 1;
	msg->sbuf = buf;
	msg->sbuf_offset = sizeof(struct Messages);
	msg->sbuf_maxlength = inMaxLength;
	Logging(LOG_TRACE, "new message from static buffer at 0x%p.\r\n", msg);

	return msg;
}


// ��Message���ڴ�ռ��ڣ�����һ��ָ����С n ���ڴ�ռ�
// ����Ŀռ�Ϊ 4 �ֽ��Ѷ���
void* MessagesStaticMalloc(struct Messages* msg, uint16_t n)
{
	void* dst;
	uint16_t align_n;

	align_n = n;
	align_n += n % 4 ? 4 - n % 4 : 0;
	if (msg->sbuf_offset + align_n > msg->sbuf_maxlength) {
		Logging(LOG_WARNING, "message static malloc, buffer overloaded, malloc failed.\n");
		return NULL;
	}

	dst = msg->sbuf + msg->sbuf_offset;
	msg->sbuf_offset += align_n;
	return dst;
}


// �ͷ� Message ���ڴ�ռ䣬���ʹ�� NewMessageStatic ���䣬��������ָ���ڴ�����
void FreeMessage(struct Messages* msg)
{
	int i;

	// ���ʹ�þ�̬�ڴ����汾������free
	if (msg->sbuf_use) {
		memset(msg->sbuf, 0, msg->sbuf_maxlength);
		return;
	}

	if (msg == NULL) {
		Logging(LOG_FATAL, "free messages == NULL, aborted.\n");
		return;
	}

	for (i = 0; i != msg->tlv_count; ++i) {
		if (msg->tlvs[i]) {
			FreeTLV(msg->tlvs[i]);
		} else {
			Logging(LOG_WARNING, "tlv ptr == NULL.\n");
		}
	}
	free(msg);
}


// ������� Float
typedef union FLOAT_CONV
{
	float f;
	char c[4];
}floatConv;


// ������� Double
typedef union DOUBLE_CONV
{
	double d;
	char c[8];
}doubleConv;


// �����ֽ���ת�����ֽ��� float �汾
float host2NetFloat(float value)
{
	floatConv f1, f2;
	int size = sizeof(float);
	int i = 0;
	if (4 == size)
	{
		f1.f = value;
		f2.c[0] = f1.c[3];
		f2.c[1] = f1.c[2];
		f2.c[2] = f1.c[1];
		f2.c[3] = f1.c[0];
	}
	else {
		f1.f = value;
		for (; i < size; i++) {
			f2.c[size - 1 - i] = f1.c[i];
		}
	}
	return f2.f;
}


// �����ֽ���ת�����ֽ��� double �汾
double host2NetDouble(double value)
{
	doubleConv d1, d2;
	int size = sizeof(double);
	int i = 0;
	d1.d = value;
	for (; i < size; i++) {
		d2.c[size - 1 - i] = d1.c[i];
	}
	return d2.d;
}


// �����ֽ���ת�����ֽ��� int8 �汾���ᴦ�����/����
int8_t host2NetInt8(int8_t value)
{
	if (value >= 0 || 1)
	{
		return value;
	}
	else
	{
		value = ((~value + 1)) | 0x80;
		return value;
	}
}



// �����ֽ���ת�����ֽ��� int16 �汾���ᴦ�����/����
int16_t host2NetInt16(int16_t value)
{
	if (value >= 0 || 1)
	{
		return ((uint16_t)((((value) & 0xff) << 8) | (((value) >> 8) & 0xff)));
	}
	else
	{
		value = (((~value + 1)) | 0x8000);
		return ((uint16_t)((((value) & 0xff) << 8) | (((value) >> 8) & 0xff)));
	}
}


// �����ֽ���ת�����ֽ��� int32 �汾���ᴦ�����/����
int32_t host2NetInt32(int32_t value)
{
	if (value >= 0 || 1)
	{
		return ((uint32_t)((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) >> 8) & 0xff00) | (((value) >> 24) & 0xff)));
	}
	else
	{
		value = (((~value + 1)) | 0x80000000);
		return ((uint32_t)((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) >> 8) & 0xff00) | (((value) >> 24) & 0xff)));
	}
}


// �����ֽ���ת�����ֽ��� int64 �汾���ᴦ�����/����
int64_t  host2NetInt64(int64_t value)
{
	if (value >= 0 || 1)
	{
		return (value & 0x00000000000000FF) << 56 | (value & 0x000000000000FF00) << 40 |
			(value & 0x0000000000FF0000) << 24 | (value & 0x00000000FF000000) << 8 |
			(value & 0x000000FF00000000) >> 8 | (value & 0x0000FF0000000000) >> 24 |
			(value & 0x00FF000000000000) >> 40 | (value & 0xFF00000000000000) >> 56;
	}
	else
	{
		value = (((~value + 1)) | 0x8000000000000000);
		return  (value & 0x00000000000000FF) << 56 | (value & 0x000000000000FF00) << 40 |
			(value & 0x0000000000FF0000) << 24 | (value & 0x00000000FF000000) << 8 |
			(value & 0x000000FF00000000) >> 8 | (value & 0x0000FF0000000000) >> 24 |
			(value & 0x00FF000000000000) >> 40 | (value & 0xFF00000000000000) >> 56;
	}
}


// �����ֽ���ת�����ֽ��� float �汾
float net2HostFloat(float value)
{
	floatConv f1, f2;
	int size = sizeof(float);
	int i = 0;
	if (4 == size)
	{
		f1.f = value;
		f2.c[0] = f1.c[3];
		f2.c[1] = f1.c[2];
		f2.c[2] = f1.c[1];
		f2.c[3] = f1.c[0];
	}
	else {
		f1.f = value;
		for (; i < size; i++) {
			f2.c[size - 1 - i] = f1.c[i];
		}
	}
	return f2.f;
}


// �����ֽ���ת�����ֽ��� double �汾
double net2HostDouble(double value)
{
	doubleConv d1, d2;
	int size = sizeof(double);
	int i = 0;
	d1.d = value;
	for (; i < size; i++) {
		d2.c[size - 1 - i] = d1.c[i];
	}
	return d2.d;
}


// �����ֽ���ת�����ֽ��� int8 �汾�������й�
int8_t net2HostInt8(int8_t value)
{
	if (value >= 0 || 1)
	{
		return value;
	}
	else
	{
		value = ((~value + 1)) | 0x80;
		return value;
	}
}


// �����ֽ���ת�����ֽ��� Int6 �汾�������й�
int16_t net2HostInt16(int16_t value)
{
	if (value >= 0 || 1)
	{
		return ((uint16_t)((((value) & 0xff) << 8) | (((value) >> 8) & 0xff)));
	}
	else
	{
		value = (((~value + 1)) | 0x8000);
		return ((uint16_t)((((value) & 0xff) << 8) | (((value) >> 8) & 0xff)));
	}
}


// �����ֽ���ת�����ֽ��� int32 �汾�������й�
int32_t net2HostInt32(int32_t value)
{
	if (value >= 0 || 1)
	{
		return ((uint32_t)((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) >> 8) & 0xff00) | (((value) >> 24) & 0xff)));
	}
	else
	{
		value = (((~value + 1)) | 0x80000000);
		return ((uint32_t)((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) >> 8) & 0xff00) | (((value) >> 24) & 0xff)));
	}
}


// �����ֽ���ת�����ֽ��� int64 �汾�������й�
int64_t   net2HostInt64(int64_t value)
{
	if (value >= 0 || 1)
	{
		return (value & 0x00000000000000FF) << 56 | (value & 0x000000000000FF00) << 40 |
			(value & 0x0000000000FF0000) << 24 | (value & 0x00000000FF000000) << 8 |
			(value & 0x000000FF00000000) >> 8 | (value & 0x0000FF0000000000) >> 24 |
			(value & 0x00FF000000000000) >> 40 | (value & 0xFF00000000000000) >> 56;
	}
	else
	{
		value = (((~value + 1)) | 0x8000000000000000);
		return  (value & 0x00000000000000FF) << 56 | (value & 0x000000000000FF00) << 40 |
			(value & 0x0000000000FF0000) << 24 | (value & 0x00000000FF000000) << 8 |
			(value & 0x000000FF00000000) >> 8 | (value & 0x0000FF0000000000) >> 24 |
			(value & 0x00FF000000000000) >> 40 | (value & 0xFF00000000000000) >> 56;
	}
}


// ��һ�� Message �����У�����һ�� TLV��ָ�� type��length���Լ�ֵָ��
// vformat Ϊ tlv �� value �ĸ�ʽ���� int �͡�byte �͵�
// δ֪����ȡ UNKNOWN��ֻ���ڷ����л�ʱ���ֵ
int AddBuffer(struct Messages* msg, uint8_t type, uint8_t* v, uint16_t length, uint8_t vformat)
{
	struct TLV* tlv = NULL;
	if (msg->tlv_count >= MESSAGE_MAX_TLV) {
		Logging(LOG_WARNING, "tlv count max %d\n", MESSAGE_MAX_TLV);
		return -1;
	}

	if (msg->sbuf_use) {
		tlv = NewTLVStatic(msg, type);
	} else {
		tlv = NewTLV(type);
	}
	if (!tlv) {
		Logging(LOG_WARNING, "new tlv, malloc failed.\n");
		return -1;
	}

	if (msg->sbuf_use) {
		tlv->value = (uint8_t*)MessagesStaticMalloc(msg, length);
	}
	else {
		tlv->value = (uint8_t*)malloc(length);
	}
	if (!tlv->value) {
		Logging(LOG_WARNING, "add buffer, new tlv, malloc failed.\n");
		return -1;
	}
	tlv->vformat = vformat;
	tlv->length = length;
	memcpy(tlv->value, v, length);

	msg->tlvs[msg->tlv_count] = tlv;
	++msg->tlv_count;
	return length + 3;
}


// ��Messages�����У�����һ�� int8 ��ʽ�� TLV ����
int AddInt8(struct Messages* msg, uint8_t type, int8_t v)
{
	return AddBuffer(msg, type, (uint8_t*)&v, sizeof(int8_t), TLV_TYPE_BYTE);
}


// ��Messages�����У�����һ�� int16 ��ʽ�� TLV ����
int AddInt16(struct Messages* msg, uint8_t type, int16_t v)
{
	return AddBuffer(msg, type, (uint8_t*)&v, sizeof(v), TLV_TYPE_SHORT);
}


// ��Messages�����У�����һ�� int32 ��ʽ�� TLV ����
int AddInt32(struct Messages* msg, uint8_t type, int32_t v)
{
	return AddBuffer(msg, type, (uint8_t*)&v, sizeof(v), TLV_TYPE_INT32);
}


// ��Messages�����У�����һ�� int64 ��ʽ�� TLV ����
int AddInt64(struct Messages* msg, uint8_t type, int32_t v)
{
	return AddBuffer(msg, type, (uint8_t*)&v, sizeof(v), TLV_TYPE_LONG64);
}


// ��Messages�����У�����һ�� float ��ʽ�� TLV ����
int AddFloat(struct Messages* msg, uint8_t type, float v)
{
	return AddBuffer(msg, type, (uint8_t*)&v, sizeof(v), TLV_TYPE_FLOAT);
}


// ��Messages�����У�����һ�� double ��ʽ�� TLV ����
int AddDouble(struct Messages* msg, uint8_t type, double v)
{
	return AddBuffer(msg, type, (uint8_t*)&v, sizeof(v), TLV_TYPE_DOUBLE);
}


// ��Messages�����У�����һ�� string ��ʽ�� TLV ����
int AddString(struct Messages* msg, uint8_t type, const char* v)
{
	return AddBuffer(msg, type, (uint8_t*)v, (uint16_t)strlen(v), TLV_TYPE_STRING_ISO_8859);
}


// ��Messages�����У�����һ�� �����Ƹ�ʽ�ڴ����� TLV ����
int AddBinary(struct Messages* msg, uint8_t type, const char* v, uint16_t length)
{
	return AddBuffer(msg, type, (uint8_t*)v, length, TLV_TYPE_STRING_HEX);
}


// ��һ��Message�����У���ȡָ��typeֵ��TLV�ṹ�壬����֤ vformat �Ƿ���ȷ
// �������ȡһ�� int8 ��tlv����ʵ�������ݲ����� int8���򷵻�NULL
// ��msg��������ָ��typeֵ��tlv��Ҳ����NULL
const struct TLV* get_tlv_from_msg(const struct Messages* msg, uint8_t type, uint16_t length, uint8_t vformat)
{
	int i;
	const struct TLV* tlv;

	for (i = 0; i != msg->tlv_count; ++i) {
		if (msg->tlvs[i]->type != type) {
			continue;
		}
		
		tlv = msg->tlvs[i];
		if (vformat == TLV_TYPE_STRING_HEX || vformat == TLV_TYPE_STRING_ISO_8859) {
			// ignore length judge.
		} else {
			if (tlv->length != length) {
				Logging(LOG_WARNING, "Get tlv %d, length %d not match %d.\n", type, tlv->length, length);
				return NULL;
			}
		}

		if (tlv->vformat != TLV_TYPE_UNKNOWN && tlv->vformat != vformat) {
			Logging(LOG_WARNING, "Get tlv %d, vformat %d not match %d.\n", type, tlv->vformat, vformat);
			return NULL;
		}

		return tlv;
	}

	return NULL;
}


// ��ȡ Int8 ��TLV���� vformat Ϊ TLV_TYPE_UNKNOWN�����ʾ������δ�����л��ģ�����ж���ת��
int GetInt8(const struct Messages* msg, uint8_t type, int8_t* v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, sizeof(uint8_t), TLV_TYPE_BYTE);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	if (tlv->vformat == TLV_TYPE_UNKNOWN) {
		*v = net2HostInt8(*(int8_t*)tlv->value);
	} else {
		*v = *(int8_t*)tlv->value;
	}

	return sizeof(int8_t);
}


// ��ȡ Int16 ��TLV
int GetInt16(const struct Messages* msg, uint8_t type, int16_t* v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, sizeof(int16_t), TLV_TYPE_SHORT);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	if (tlv->vformat == TLV_TYPE_UNKNOWN) {
		*v = net2HostInt16(*(int16_t*)tlv->value);
	}
	else {
		*v = *(int16_t*)tlv->value;
	}

	return sizeof(int16_t);
}


// ��ȡ Int32 ��TLV
int GetInt32(const struct Messages* msg, uint8_t type, int32_t* v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, sizeof(int32_t), TLV_TYPE_INT32);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	if (tlv->vformat == TLV_TYPE_UNKNOWN) {
		*v = net2HostInt32(*(int32_t*)tlv->value);
	}
	else {
		*v = *(int32_t*)tlv->value;
	}

	return sizeof(int32_t);
}


// ��ȡ Int64 ��TLV
int GetLong64(const struct Messages* msg, uint8_t type, int64_t* v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, sizeof(int64_t), TLV_TYPE_LONG64);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	if (tlv->vformat == TLV_TYPE_UNKNOWN) {
		*v = net2HostInt64(*(int64_t*)tlv->value);
	}
	else {
		*v = *(int64_t*)tlv->value;
	}

	return sizeof(int64_t);
}


// ��ȡ float ��TLV
int GetFloat(const struct Messages* msg, uint8_t type, float* v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, sizeof(float), TLV_TYPE_FLOAT);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	if (tlv->vformat == TLV_TYPE_UNKNOWN) {
		*v = net2HostFloat(*(float*)tlv->value);
	}
	else {
		*v = *(float*)tlv->value;
	}

	return sizeof(float);
}


// ��ȡ double ��TLV
int GetDouble(const struct Messages* msg, uint8_t type, double* v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, sizeof(double), TLV_TYPE_DOUBLE);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	if (tlv->vformat == TLV_TYPE_UNKNOWN) {
		*v = net2HostDouble(*(double*)tlv->value);
	}
	else {
		*v = *(double*)tlv->value;
	}

	return sizeof(double);
}


// ��ȡ string ��TLV
int GetString(const struct Messages* msg, uint8_t type, char** v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, 0, TLV_TYPE_STRING_ISO_8859);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	*v = (char*)tlv->value;
	return tlv->length;
}


// ��ȡ binary �������ڴ����� ��TLV
int GetBinary(const struct Messages* msg, uint8_t type, uint8_t** v)
{
	const struct TLV* tlv;

	tlv = get_tlv_from_msg(msg, type, 0, TLV_TYPE_STRING_HEX);
	if (!tlv) {
		Logging(LOG_WARNING, "cannot found valid tlv %d from msg 0x%p\n", type, msg);
		return -1;
	}

	*v = tlv->value;
	return tlv->length;
}


// malloc �µ�TLV�ռ䣬������TLV�ṹ��� type ֵ
struct TLV* NewTLV(uint8_t type)
{
	struct TLV* tlv = malloc(sizeof(struct TLV));
	if (!tlv) {
		Logging(LOG_WARNING, "new tlv, malloc failed.\n");
		return NULL;
	}
	tlv->length = 0;
	tlv->type = type;

	return tlv;
}


// �� Message ��ʣ��ռ��У�Ϊ TLV �����ڴ�ռ䣬��ʹ��malloc
struct TLV* NewTLVStatic(struct Messages* msg, uint8_t type)
{
	struct TLV* tlv = MessagesStaticMalloc(msg, sizeof(struct TLV));
	if (!tlv) {
		Logging(LOG_WARNING, "new tlv static, malloc failed.\n");
		return NULL;
	}
	tlv->type = type;
	tlv->length = 0;
	tlv->value = NULL;

	return tlv;
}


// �ͷ�TLV�ڴ�ռ�
void FreeTLV(struct TLV* tlv)
{
	if (!tlv) {
		Logging(LOG_WARNING, "free null tlv?\n");
		return;
	} 

	if (tlv->value) {
		free(tlv->value);
	}
	free(tlv);
}


// �ڴ����д uint16
union u16_setter {
	uint16_t val;
	char buf[2];
};


// ʹ�� union ��ʽ�����ڴ����ķ�ʽд uint16 ���ݵ� buffer
void alignment_u16_w(char* buf, uint16_t val)
{
	union u16_setter setter;
	setter.val = val;
	memcpy(buf, setter.buf, sizeof(setter));
}


// �ڴ����д uint32
union u32_setter {
	uint32_t val;
	char buf[4];
};


// ʹ�� union ��ʽ�����ڴ����ķ�ʽд uint32 ���ݵ� buffer
void alignment_u32_w(char* buf, uint32_t val)
{
	union u32_setter setter;
	setter.val = val;
	memcpy(buf, setter.buf, sizeof(setter));
}


// �ڴ����д uint64
union u64_setter {
	uint64_t val;
	char buf[8];
};


// ʹ�� union ��ʽ�����ڴ����ķ�ʽд uint64 ���ݵ� buffer
void alignment_u64_w(char* buf, uint64_t val)
{
	union u64_setter setter;
	setter.val = val;
	memcpy(buf, setter.buf, sizeof(setter));
}


// ʹ�� union ��ʽ�����ڴ����ķ�ʽ��ȡ uint16 ������
uint16_t alignment_u16_r(const char* buf)
{
	union u16_setter reader;
	memcpy(reader.buf, buf, sizeof(reader));
	return reader.val;
}


// ֵ���л� int8
void int8_serialize(int8_t v, char* inBuf)
{
	memcpy(inBuf, &v, sizeof(v));
}


// ֵ���л� int16
void int16_serialize(int16_t v, char* inBuf)
{
	memcpy(inBuf, &v, sizeof(v));
}


// ֵ���л� int32
void int32_serialize(int32_t v, char* inBuf)
{
	memcpy(inBuf, &v, sizeof(v));
}


// ֵ���л� int64
void int64_serialize(int64_t v, char* inBuf)
{
	memcpy(inBuf, &v, sizeof(v));
}


// ֵ���л� float
void float_serialize(float v, char* inBuf)
{
	memcpy(inBuf, &v, sizeof(v));
}


// ֵ���л� double
void double_serialize(double v, char* inBuf)
{
	memcpy(inBuf, &v, sizeof(v));
}


// ֵ���л������� tlv->vformat ѡ��ͬ�����л�����
void value_serialize(struct TLV* tlv, char* inBuf, uint16_t inMaxLength)
{
	switch (tlv->vformat) {
	case TLV_TYPE_BYTE:
	case TLV_TYPE_ENUM:
	case TLV_TYPE_BOOL:
		int8_serialize(host2NetInt8(*(int8_t*)tlv->value), inBuf);
		break;
	case TLV_TYPE_SHORT:
		int16_serialize(host2NetInt16(*(int16_t*)tlv->value), inBuf);
		break;
	case TLV_TYPE_INT32:
		int32_serialize(host2NetInt32(*(int32_t*)tlv->value), inBuf);
		break;
	case TLV_TYPE_LONG64:
		int64_serialize(host2NetInt64(*(int64_t*)tlv->value), inBuf);
		break;
	case TLV_TYPE_FLOAT:
		float_serialize(host2NetFloat(*(float*)tlv->value), inBuf);
		break;
	case TLV_TYPE_DOUBLE:
		double_serialize(host2NetDouble(*(double*)tlv->value), inBuf);
		break;
	default:
		memcpy(inBuf, tlv->value, tlv->length);
		break;
	}
}


// �ɹ��������л���ĳ��ȣ�ʧ�ܷ���-1
int TLVSerialize(struct TLV* tlv, char* inBuf, uint16_t inMaxLength)
{
	if (tlv == NULL) {
		Logging(LOG_WARNING, "tlv serialize failed, tlv == NULL.\n");
		return -1;
	}

	if (inBuf == NULL) {
		Logging(LOG_WARNING, "tlv serialize failed, inBuf == NULL.\n");
		return -1;
	}
	if (inMaxLength < tlv->length + 3) {
		Logging(LOG_TRACE, "tlv serialize buffer is too small, aborted.\n");
		return -1;
	}
	inBuf[0] = tlv->type;
	alignment_u16_w(inBuf + 1, nb_htons(tlv->length));

	value_serialize(tlv, inBuf + 3, inMaxLength - 3);

	return tlv->length + 3;
}


// ���л� Message ����� data ����
int SerializeBody(const struct Messages* msg, char* inBuf, uint16_t inMaxLength)
{
	int i, ret, pos, length;
	inBuf[0] = msg->msgid;

	length = 0;
	pos = 3;
	for (i = 0; i != msg->tlv_count; ++i) {
		length += msg->tlvs[i]->length + 3;
		ret = TLVSerialize(msg->tlvs[i], inBuf + pos, inMaxLength - pos);
		if (ret < 0) {
			Logging(LOG_WARNING, "serialize tlv failed. \n");
			return -1;
		}
		pos += ret;
	}

	alignment_u16_w(inBuf + 1, nb_htons(length));
	return 3 + length;
}


// checksum ���㣬�ۼӺ�
uint8_t CalcCheckSum(const char* buf, uint16_t length)
{
	uint16_t i;
	uint32_t sum = 0;

	for (i = 0; i != length; ++i) {
		sum += (uint8_t)(buf[i]);
	}
	return sum % 256;
}


/*
�û�ָ����Ӧ�������л�
1����������ܳ���
2��������л�tlv�������
*/
int UserCmdRspMsgSerialize(const struct Messages* msg, char* inBuf, uint16_t inMaxLength)
{
	int i, pos, packet_length;

	// ����ж�
	if (msg == NULL) {
		Logging(LOG_WARNING, "tlv messages msg == NULL.\n");
		return -1;
	}
	if (inBuf == NULL) {
		Logging(LOG_WARNING, "serialize buffer == NULL.\n");
		return -1;
	}

	// ��ȥdata��ĳ���
	// 1	1	2	2	1 == 7
	packet_length = 7;
	for (i = 0; i != msg->tlv_count; ++i) {
		// ÿ�� TLV ��ͷ����3�ֽ�
		packet_length += msg->tlvs[i]->length + 3;
	}
	// data ����Ҳ��һ���ܴ��tlv����t + l ����Ϊ3
	packet_length += 3;
	if (inMaxLength < packet_length) {
		Logging(LOG_WARNING, "Messages serialize buffer too small.\n");
		return -1;
	}
	// ��ʼ���л��������ǹ�������
	inBuf[0] = EASYIOT_COAP_VERSION;
	inBuf[1] = CMT_USER_CMD_RSP;
	pos = 2;

	// ����λ���ֶ� 4 ~ 11��Ϊ�ܳ��� - 4
	alignment_u16_w(inBuf + pos, nb_htons(packet_length - 4));
	pos += 2;

	// mid
	alignment_u16_w(inBuf + pos, msg->dtag_mid);
	pos += 2;

	// Ȼ�����л� data ����
	i = SerializeBody(msg, inBuf + pos, inMaxLength - pos);
	if (i < 0) {
		Logging(LOG_WARNING, "messages body serialize failed.\n");
		return -1;
	}
	pos += i;

	// check sum.
	inBuf[pos] = CalcCheckSum(inBuf, pos);

	return pos + 1;
}


/*
�û������ϱ������ݱ������л�
1����������ܳ���
2��������л�tlv�������
*/
int UserUpMsgSerialize(const struct Messages* msg, char* inBuf, uint16_t inMaxLength)
{
	int i, pos, packet_length;
	uint8_t battery;
	int32_t signal;
	uint64_t timestamp;

	// ����ж�
	if (msg == NULL) {
		Logging(LOG_WARNING, "tlv messages msg == NULL.\n");
		return -1;
	}
	if (inBuf == NULL) {
		Logging(LOG_WARNING, "serialize buffer == NULL.\n");
		return -1;
	}

	// ��ȥdata��ĳ���
	// 1	1	2	2	1	4	15	15	8	1 == 50
	packet_length = 50;
	for (i = 0; i != msg->tlv_count; ++i) {
		// ÿ�� TLV ��ͷ����3�ֽ�
		packet_length += msg->tlvs[i]->length + 3;
	}
	// data ����Ҳ��һ���ܴ��tlv����t + l ����Ϊ3
	packet_length += 3;
	if (inMaxLength < packet_length) {
		Logging(LOG_WARNING, "Messages serialize buffer too small.\n");
		return -1;
	}

	// ��� ��ص��� ���ź�ǿ�ȣ�ʱ���
	if (gl_timestampcb) {
		timestamp = gl_timestampcb();
	} else {
		Logging(LOG_WARNING, "timestamp callback empty, ignore and set to 0.\n");
		timestamp = 0;
	}
	if (gl_signalcb) {
		signal = gl_signalcb();
	} else {
		Logging(LOG_WARNING, "signal strength callback empty, ignore and set to 0.\n");
		signal = 0;
	}
	if (gl_batterycb) {
		battery = gl_batterycb();
	} else {
		Logging(LOG_WARNING, "battery status callback empty, ignore and set to 0.\n");
		battery = 0;
	}
	if (battery > 100) {
		Logging(LOG_WARNING, "battery level lager than 100, set to 100.\n");
		battery = 100;
	}

	// ��ʼ���л��������ǹ�������
	inBuf[0] = EASYIOT_COAP_VERSION;
	inBuf[1] = CMT_USER_UP;
	pos = 2;

	// ����λ���ֶ� 4 ~ 11��Ϊ�ܳ��� - 4
	alignment_u16_w(inBuf + pos, nb_htons(packet_length - 4));
	pos += sizeof(uint16_t);

	// dtag.
	alignment_u16_w(inBuf + pos, msg->dtag_mid);
	pos += sizeof(uint16_t);

	// battery.
	inBuf[pos] = battery;
	pos += sizeof(uint8_t);

	// signal strength
	alignment_u32_w(inBuf + pos, host2NetInt32(signal));
	pos += sizeof(uint32_t);

	// imei && imsi;
	memcpy(inBuf + pos, gl_imei, 15);
	pos += 15;
	memcpy(inBuf + pos, gl_imsi, 15);
	pos += 15;

	// timestamp
	alignment_u64_w(inBuf + pos, host2NetInt64(timestamp));
	pos += sizeof(uint64_t);

	// Ȼ�����л� data ����
	i = SerializeBody(msg, inBuf + pos, inMaxLength - pos);
	if (i < 0) {
		Logging(LOG_WARNING, "messages body serialize failed.\n");
		return -1;
	}
	pos += i;

	// check sum.
	inBuf[pos] = CalcCheckSum(inBuf, pos);

	//ȷ���ַ�����0��β��
	inBuf[pos + 1] = '\0';
	
	return pos + 1;
}


/*
���ݷ����л��������л�Message��data����
1��������TLV�ṹ��
������ inLength >= 3
length + 3 >= inLength
*/
int MessageDeserializeBodyData(const char* inBuf, uint16_t inLength, struct Messages* out)
{
	uint8_t type;
	uint16_t length;

	if (inLength < 3) {
		Logging(LOG_WARNING, "message deserialize body data failed, too small.\n");
		return -1;
	}

	type = inBuf[0];
	length = nb_htons(alignment_u16_r(inBuf + 1));
	if (length + 3 > inLength) {
		Logging(LOG_WARNING, "message deserialize body data failed, length not match.\n");
		return -1;
	}

	AddBuffer(out, type, (uint8_t*)(inBuf + 3), length, TLV_TYPE_UNKNOWN);

	// �����õ��˶����ֽ�
	return length + 3;
}


// ACK�ķ����л���δʵ��
int UserUpAckMsgDeserialize(const char* inBuf, uint16_t inLength, struct Messages* out)
{
	return 0;
}


// �û�ָ��ķ����л�
int UserCmdReqMsgDeserialize(const char* inBuf, uint16_t inLength, struct Messages* out)
{
	int rsp, pos, left;
	uint16_t length;
	// dtag / mid ����Ҫ���д�С��ת��
	out->dtag_mid = alignment_u16_r(inBuf + 4);

	/*
	2����� data ���tlv ��� length �Ƿ���ȷ
	3����������л� tlv
	*/
	out->msgid = inBuf[6];
	// ���length�Ƿ���ȷ, 7 �� ָ��body��length��ƫ�ƣ�����ĵ�
	length = nb_htons(alignment_u16_r(inBuf + 7));
	// 10 �� ���������У���ȥָ��tlv�����⣬�޹��ֽ���
	if (length != inLength - 10) {
		Logging(LOG_WARNING, "tlv body deserialize failed, length not match.\n");
		return -1;
	}

	// ��������л�
	pos = 9;
	do {
		// left == inLength - pos - 1, -1 ����ΪҪȥ�� checksum
		rsp = MessageDeserializeBodyData(inBuf + pos, inLength - pos - 1, out);
		if (rsp < 0) {
			Logging(LOG_WARNING, "message deserialize body data failed, left %d\n", rsp);
			return -1;
		}
		pos += rsp;
		left = inLength - pos - 1;
	} while (left > 0);

	return rsp;
}


/*
   ���ݷ����л�
1���汾���Ƿ�ƥ��
2��У����Ƿ���ȷ
3�������Ƿ�ƥ��
4�������ͽ��з����л�
*/
int MessagesDeserialize(const char* inBuf, uint16_t inLength, struct Messages* out)
{
	uint8_t version, checksum, rchecksum;
	uint16_t packet_length;
	int rsp;

	rsp = -1;
	Logging(LOG_TRACE, "prepare deserialize buffer 0x%p, length: %d\n", inBuf, inLength);

	version = inBuf[0];
	if (version != EASYIOT_COAP_VERSION) {
		Logging(LOG_WARNING, "message deserialize failed, version not match.");
		return -1;
	}

	checksum = CalcCheckSum(inBuf, inLength - 1);
	rchecksum = inBuf[inLength - 1];
	if (checksum != rchecksum) {
		Logging(LOG_WARNING, "message checksum failed, expected [%d], but get [%d].\n", checksum, inBuf[inLength - 1]);
		return -1;
	}

	packet_length = nb_htons(alignment_u16_r(inBuf + 2));
	if (packet_length + 4 != inLength) {
		Logging(LOG_WARNING, "packet length not match.\n");
		return -1;
	}

	out->msgType = ((unsigned char*)inBuf)[1];
	switch (out->msgType) {
	case CMT_USER_CMD_REQ:
		rsp = UserCmdReqMsgDeserialize(inBuf, inLength, out);
		break;
	case CMT_SYS_CONF_REQ:
		break;
	case CMT_USER_UP_ACK:
		rsp = UserUpAckMsgDeserialize(inBuf, inLength, out);
		break;
	case CMT_SYS_QUERY_REQ:
		break;
	default:
		break;
	}

	return rsp;
}


// Message �������л�
int MessagesSerialize(const struct Messages* msg, char* inBuf, uint16_t inMaxLength)
{
	int rsp;

	rsp = -1;
	Logging(LOG_TRACE, "prepare serialize message 0x%p, sensor count: %d\n", msg, msg->tlv_count);
	switch (msg->msgType) {
	case CMT_USER_UP:
		Logging(LOG_TRACE, "message type is CMT_USER_UP.\r\n");
		rsp = UserUpMsgSerialize(msg, inBuf, inMaxLength);
		break;
	case CMT_USER_CMD_RSP:
		Logging(LOG_TRACE, "message type is CMT_USER_CMD_RSP.\r\n");
		rsp = UserCmdRspMsgSerialize(msg, inBuf, inMaxLength);
		break;
	default:
		Logging(LOG_WARNING, "unknown message type.\n");
		break;
	}

	return rsp;
}


// ֱ��Ԥ����һ���ϴ�Ļ������������������л���Coap���ݷ���
int pushMessageStackedBuffer(struct Messages* msg)
{
	// ���л�
	int rsp, length;
	char buf[512];

	length = MessagesSerialize(msg, buf, sizeof(buf));
	if (length < 0) {
		Logging(LOG_WARNING, "message serialize failed.\n");
		return -1;
	}

	// ���ͳ�ȥ
	rsp = CoapOutput((uint8_t*)buf, length);
	if (rsp < 0) {
		Logging(LOG_WARNING, "coap output failed.\n");
		return -1;
	}
	return -1;
}


// ʹ��Message�����ڵ�ʣ���ڴ�ռ䣬����������л���CoAP ���ݷ���
// ��ɺ󷵻��ⲿ���ڴ�ռ䣬�� msg->sbuf_offset ��������������
int pushMessageStatic(struct Messages* msg)
{
	int rsp, length, left;
	char* buf;

	buf = (char*)msg->sbuf + msg->sbuf_offset;
	left = msg->sbuf_maxlength - msg->sbuf_offset;
	length = MessagesSerialize(msg, buf, left);
	if (length < 0) {
		Logging(LOG_WARNING, "message serialize failed.\n");
		return -1;
	}

	// ���ͳ�ȥ
	rsp = CoapOutput((uint8_t*)buf, length);
	if (rsp < 0) {
		Logging(LOG_WARNING, "coap output failed.\n");
		return -1;
	}

	memset(buf, 0, left);
	return -1;
}


// ��Message�������͵�EasyIoTƽ̨
int pushMessages(struct Messages *msg)
{
	if (msg->sbuf_use) {
		Logging(LOG_TRACE, "message using static buffer, using static push messages.\r\n");
		return pushMessageStatic(msg);
	} else {
		return pushMessageStackedBuffer(msg);
	}
}


/*
   CoAP��������������
1�������л�
2������input�����ͣ���Ӧ���ûص���
*/
int CoapInput(struct Messages* msg, uint8_t *data, uint16_t inLength)
{
	int ret, i, found;
	CmdHandlerFuncPtr ptr;
	
	ret = MessagesDeserialize((const char*)data, inLength, msg);
	if (ret < 0) {
		Logging(LOG_WARNING, "message deserialize failed.\n");
		return -1;
	}
	Logging(LOG_INFO, "message deserialize succ, tlv count: %d\n", msg->tlv_count);

	switch (msg->msgType) {
	case CMT_USER_UP_ACK: {
		Logging(LOG_INFO, "recv ack.\n");
		if (!gl_ackhandler) {
			Logging(LOG_INFO, "ack handler == null, ignore ack.\r\n");
		} else {
			Logging(LOG_INFO, "found ack handler at 0x%p, process it.\r\n", gl_ackhandler);
			gl_ackhandler(msg);
		}
		break;
	}
	case CMT_USER_CMD_REQ: {
		Logging(LOG_INFO, "recv cmd.\n");
		// ����cmdid���зַ�handler
		found = 0;
		for (i = 0; i != cmd_handler_count; ++i){
			if (msg->msgid == gl_cmd_handlers[i].CmdID) {
				found = 1;
				ptr = gl_cmd_handlers[i].ptr;
				if (!ptr) {
					Logging(LOG_WARNING, "msg %d cmd handler is null.\n", msg->msgid);
				} else {
					Logging(LOG_INFO, "found cmdid %d handler at 0x%p, process it.\r\n", msg->msgid, ptr);
					ptr(msg);
					break;
				}
			}
		}
		if (!found) {
			Logging(LOG_WARNING, "cannot found cmdhandler for %d \n", msg->msgid);
		}
		break;
	}
	default:
		break;
	}

	FreeMessage(msg);
	return ret;
}


// CoAP ����������˴���ʵ��Ϊ BC95 ������ģ����Ҫ����ATָ�����Ӧ�޸�
int CoapOutput(uint8_t *inBuf, uint16_t inLength)
{
//	int i, all_length;
//	char headbuf[16];

	if (!gl_nb_out) {
		Logging(LOG_WARNING, "nb serial output cb is null, pls use setNbSerialOutputCb set it.\r\n");
		return -1;
	}

//	memset(headbuf, 0, sizeof(headbuf));
//	sprintf(headbuf, "AT+NMGS=%d,", inLength);
//	all_length = strlen(headbuf);

	gl_nb_out((uint8_t*)inBuf, inLength);

//	for (i = 0; i != inLength; ++i) {
//		sprintf(headbuf, "%02X", inBuf[i]);
//		gl_nb_out((uint8_t*)headbuf, 2);
//		all_length += 2;
//	}
//
//	gl_nb_out((uint8_t*)"\r\n", 2);
//	return all_length + 2;
    return inLength;
}


// ������־����ȼ������ڴ˵ȼ��Ľ������
void SetLogLevel(enum LoggingLevel level)
{
	gl_loglevel = level;
}


// ��־�������Ҫʹ�� stdarg �еĺ�����������Ҫ����ȥ��
int Logging(enum LoggingLevel level, const char* fmt, ...)
{
	int ret;
	char buf[128];

	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	if (level >= gl_loglevel) {
		ret = vsprintf(buf, fmt, arg_ptr);
	}
	va_end(arg_ptr);

	if (ret > 0 && gl_log_out) {
		gl_log_out((uint8_t*)buf, ret);
	}
	return -1;
}


// ascii2binary������ÿ���ֽ�ASCII��ʾ��HEX���ݣ�ת����1�ֽڵĶ������ڴ�byte���ݣ���2�ֽڵ��ַ���"AA"����ת����1�ֽڵ�byte���� 0xAA��
int a2b_hex(const char* s, char* out, int inMaxLength)
{
	int i, pos;
	int length = strlen(s);

	if (length % 2) {
		Logging(LOG_WARNING, "input hex data must be even\n");
		return -1;
	}
	if (inMaxLength < length / 2) {
		Logging(LOG_WARNING, "input buffer too small.\n");
		return -1;
	}

	pos = 0;
	for (i = 0; i < length - 1; i += 2) {
		out[pos++] = (s[i] >= 'A' ? s[i] - 'A' + 10 : s[i] - '0') * 16 + (s[i + 1] >= 'A' ? s[i + 1] - 'A' + 10 : s[i + 1] - '0');
	}

	return length / 2;
}


// ASCII HEX��ʽ��CoAP�������봦�����ȵ��� a2b_hex ��Ȼ��ֱ�� CoapInput
int CoapHexInput(const char* data)
{
	int ret;
	char cmdbuf[512];
	struct Messages* msg;

	ret = a2b_hex(data, cmdbuf, sizeof(cmdbuf));
	if (ret < 0) {
		Logging(LOG_WARNING, "ascii to binary hex failed.\n");
		return -1;
	}
	Logging(LOG_TRACE, "coap hex static input %d, to binary %d.\r\n", strlen(data), ret);

	msg = NewMessage();
	ret = CoapInput(msg, (uint8_t*)cmdbuf, ret);
	if (ret < 0) {
		Logging(LOG_WARNING, "coap input process failed.\n");
		return -1;
	}
	Logging(LOG_TRACE, "coap input static process finished, ret %d.\r\n", ret);

	return ret;
}


// ASCII HEX��ʽ��CoAP�������봦�����ȵ��� a2b_hex ��Ȼ��ֱ�� CoapInput
// ��ͬ CoapHexInput��ֻ�ǲ�û���ں�����ֱ��ʹ��һ��ϴ���ڴ�ռ䣬ʹ����ָ����buffer����
int CoapHexInputStatic(const char* data, uint8_t* inBuf, uint16_t inMaxLength)
{
	int ret;
	struct Messages* msg;

	ret = a2b_hex(data, (char*)inBuf, inMaxLength);
	if (ret < 0) {
		Logging(LOG_WARNING, "ascii to binary hex failed.\n");
		return -1;
	}
	Logging(LOG_TRACE, "coap hex input %d, to binary %d.\r\n", strlen(data), ret);

	msg = NewMessageStatic(inBuf + ret, inMaxLength - ret);
	ret = CoapInput(msg, inBuf, ret);
	if (ret < 0) {
		Logging(LOG_WARNING, "coap input process failed.\n");
		return -1;
	}
	Logging(LOG_TRACE, "coap input process finished, ret %d.\r\n", ret);

	return ret;
}
