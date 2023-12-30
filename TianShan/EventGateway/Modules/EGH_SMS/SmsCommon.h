#pragma once
#include <ZQ_common_conf.h>

namespace nsSMS
{

// �û���Ϣ���뷽ʽ
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8

// Ӧ��״̬
#define GSM_WAIT		0		// �ȴ�����ȷ��
#define GSM_OK			1		// OK
#define GSM_ERR			-1		// ERROR

// ����Ϣ�����ṹ������/���빲��
// ���У��ַ�����'\0'��β
typedef struct SmParam{
	char SCA[16];			// ����Ϣ�������ĺ���(SMSC��ַ)
	char TPA[16];			// Ŀ������ظ�����(TP-DA��TP-RA)
	char TP_PID;			// �û���ϢЭ���ʶ(TP-PID)
	char TP_DCS;			// �û���Ϣ���뷽ʽ(TP-DCS)
	char TP_SCTS[16];		// ����ʱ����ַ���(TP_SCTS), ����ʱ�õ�
	char TP_UD[161];		// ԭʼ�û���Ϣ(����ǰ�������TP-UD)
	short index;			// ����Ϣ��ţ��ڶ�ȡʱ�õ�
} SM_PARAM;

// ��ȡӦ��Ļ�����
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
