#pragma once
#include <ZQ_common_conf.h>

namespace nsSMS
{

// 用户信息编码方式
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8

// 应答状态
#define GSM_WAIT		0		// 等待，不确定
#define GSM_OK			1		// OK
#define GSM_ERR			-1		// ERROR

// 短消息参数结构，编码/解码共用
// 其中，字符串以'\0'结尾
typedef struct SmParam{
	char SCA[16];			// 短消息服务中心号码(SMSC地址)
	char TPA[16];			// 目标号码或回复号码(TP-DA或TP-RA)
	char TP_PID;			// 用户信息协议标识(TP-PID)
	char TP_DCS;			// 用户信息编码方式(TP-DCS)
	char TP_SCTS[16];		// 服务时间戳字符串(TP_SCTS), 接收时用到
	char TP_UD[161];		// 原始用户信息(编码前或解码后的TP-UD)
	short index;			// 短消息序号，在读取时用到
} SM_PARAM;

// 读取应答的缓冲区
typedef struct SmBuffer{
	int len;
	char data[16384];
} SM_BUFF;

class SmsCommon
{
public:
	SmsCommon(void);
	~SmsCommon(void);

	int EncodePdu(const SM_PARAM* pSrc, int msgLT, char* pDst);
	int DecodePdu(const char* pSrc, SM_PARAM* pDst);

	int String2Bytes(const char* pSrc, int nSrcLength, unsigned char* pDst);
	int Bytes2String(const unsigned char* pSrc, int nSrcLength, char* pDst);

	int Encode7bit(const char* pSrc, int nSrcLength, unsigned char* pDst);
	int Decode7bit(const unsigned char* pSrc, int nSrcLength, char* pDst);

	int Encode8bit(const char* pSrc, int nSrcLength, unsigned char* pDst);
	int Decode8bit(const unsigned char* pSrc, int nSrcLength, char* pDst);

	int EncodeUcs2(const char* pSrc, int nSrcLength, unsigned char* pDst);
	int DecodeUcs2(const unsigned char* pSrc, int nSrcLength, char* pDst);

	int InvertNumbers(const char* pSrc, int nSrcLength, char* pDst);
	int SerializeNumbers(const char* pSrc, int nSrcLength, char* pDst);
};

}
