#include "SmsCommon.h"

namespace nsSMS{

SmsCommon::SmsCommon(void)
{
}

SmsCommon::~SmsCommon(void)
{
}

// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
// like:"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
int SmsCommon::String2Bytes(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	for (int i = 0; i < nSrcLength; i += 2)
	{
		// �����4λ
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}

		pSrc++;

		// �����4λ
		if ((*pSrc>='0') && (*pSrc<='9'))
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}

		pSrc++;
		pDst++;
	}

	*pDst = '\0';

	// ����Ŀ�����ݳ���
	return (nSrcLength / 2);
}

// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
// like��{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
int SmsCommon::Bytes2String(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	const char tab[]="0123456789ABCDEF";	// 0x0-0xf���ַ����ұ�

	for (int i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];		// �����4λ
		*pDst++ = tab[*pSrc & 0x0f];	// �����4λ
		pSrc++;
	}

	*pDst = '\0';

	// ����Ŀ���ַ�������
	return (nSrcLength * 2);
}

// 7bit����
int SmsCommon::Encode7bit(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	int nSrc = 0;
	int nDst = 0;
	int nChar = 0;	// ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	unsigned char nLeft = 0;	// ��һ�ֽڲ��������


	// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô�����̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	while (nSrc < nSrcLength)
	{
		// ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc & 7;

		// ����Դ����ÿ���ֽ�
		if(nChar == 0)
		{
			// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			nLeft = *pSrc;
		}
		else
		{
			// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = *pSrc >> nChar;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	return nDst;
}

// 7bit����
int SmsCommon::Decode7bit(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	int nSrc = 0;
	int nDst = 0;
	int nByte = 0;		// ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	unsigned char nLeft = 0;	// ��һ�ֽڲ��������


	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

		// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte);

		// �޸�Ŀ�괮��ָ��ͼ���ֵ
		pDst++;
		nDst++;

		// �޸��ֽڼ���ֵ
		nByte++;

		// ����һ������һ���ֽ�
		if(nByte == 7)
		{
			// ����õ�һ��Ŀ������ֽ�
			*pDst = nLeft;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;

			// �����ֽ���źͲ������ݳ�ʼ��
			nByte = 0;
			nLeft = 0;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	return nDst;
}

// 8bit����
int SmsCommon::Encode8bit(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}

// 8bit����
int SmsCommon::Decode8bit(const unsigned char* pSrc, int nSrcLength, char* pDst)
{

	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}


#ifdef ZQ_OS_MSWIN
// UCS2����
int SmsCommon::EncodeUcs2(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	int nDstLength;
	WCHAR wchar[128] = {0};

	// �ַ���-->UNICODE��
	nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	// �ߵ��ֽڶԵ������
	for(int i=0; i<nDstLength; i++)
	{
		*pDst++ = wchar[i] >> 8;
		*pDst++ = wchar[i] & 0xff;
	}
	
	*pDst = '\0';

	return nDstLength * 2;
}

// UCS2����
int SmsCommon::DecodeUcs2(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	int nDstLength;
	WCHAR wchar[160] = {0};

	// �ߵ��ֽڶԵ���ƴ��UNICODE
	for(int i=0; i<nSrcLength/2; i++)
	{
		wchar[i] = *pSrc++ << 8;
		wchar[i] |= *pSrc++;
	}

	// UNICODE��-->�ַ���
	nDstLength = WideCharToMultiByte(CP_ACP, 0, wchar, nSrcLength/2, pDst, 160, NULL, NULL);

	pDst[nDstLength] = '\0';

	return nDstLength;
}

#else
#include <iconv.h>

bool code_convert(const char* fromcode, const char* tocode, const char* inbuf, size_t inlen, char* outbuf, size_t& outlen )
{
	char* pinbuf = (char*)inbuf;
	char* poutbuf = outbuf;
	size_t ninlen = inlen;
	size_t noutlen = outlen;

	iconv_t ic;
	ic = iconv_open(tocode,fromcode);
	if(ic == (iconv_t)-1)
		return false;

	size_t nre = iconv(ic, &pinbuf,&ninlen, &poutbuf, &noutlen);
	if(nre == (size_t)-1)
	{
		iconv_close(ic);
		return false;
	}
	
	outlen = outlen - noutlen;	
	iconv_close(ic);
	return true;	
}

int SmsCommon::EncodeUcs2(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	char outbuf[512] = {0};
	size_t outlen = 512;
	
	bool rt = code_convert("ASCII","UNICODEBIG", pSrc, (size_t)nSrcLength, outbuf,outlen);
	if(!rt)
		return 0;

	memcpy(pDst,outbuf,outlen);
	*(pDst+outlen) = '\0';

	return outlen;
}

int SmsCommon::DecodeUcs2(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	char outbuf[512] = {0};
	size_t outlen = 512;
	
	bool rt = code_convert("UNICODEBIG", "ASCII", (const char*)pSrc, nSrcLength, outbuf, outlen);
	if(!rt)
		return 0;
	
	memcpy(pDst, outbuf, outlen);
	*(pDst+outlen) = '\0';
	
	return outlen;
}
#endif

// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// like��"8613851872468" --> "683158812764F8"
int SmsCommon::InvertNumbers(const char* pSrc, int nSrcLength, char* pDst)
{
	int nDstLength = nSrcLength;
	char ch;

	for(int i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;	
		*pDst++ = *pSrc++;
		*pDst++ = ch;
	}

	// Դ������������,�� 'F'
	if(nSrcLength & 1)
	{
		*(pDst-2) = 'F';
		nDstLength++;
	}

	*pDst = '\0';

	return nDstLength;
}

// �����ߵ����ַ���ת��Ϊ����˳����ַ���
// like��"683158812764F8" --> "8613851872468"
int SmsCommon::SerializeNumbers(const char* pSrc, int nSrcLength, char* pDst)
{
	int nDstLength = nSrcLength;
	char ch;


	for(int i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;	
		*pDst++ = *pSrc++;
		*pDst++ = ch;	
	}

	// �����ַ���'F'
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;
	}

	*pDst = '\0';

	return nDstLength;
}

// PDU���룬���ڱ��ơ����Ͷ���Ϣ
int SmsCommon::EncodePdu(const SM_PARAM* pSrc, int msgLT, char* pDst)
{
	int nLength;
	int nDstLength;
	unsigned char buf[1024] = {0};

	// SMSC��ַ��Ϣ��
	nLength = (int)strlen(pSrc->SCA);	// SMSC��ַ�ַ����ĳ���	
	if(nLength == 0)//not set center number
	{
		buf[0] = 0;
		nDstLength = Bytes2String(buf,1,pDst);
	}
	else
	{
		buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC��ַ��Ϣ����
		buf[1] = 0x91;		// �̶�: �ù��ʸ�ʽ����
		nDstLength = Bytes2String(buf, 2, pDst);		// ת��2���ֽڵ�Ŀ��PDU��
		nDstLength += InvertNumbers(pSrc->SCA, nLength, &pDst[nDstLength]);	// ת��SMSC���뵽Ŀ��PDU��
	}


	// TPDU�λ���������Ŀ���ַ��
	nLength = (int)strlen(pSrc->TPA);	// TP-DA��ַ�ַ����ĳ���
	buf[0] = 0x11;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)nLength;			// Ŀ���ַ���ָ���(TP-DA��ַ�ַ�����ʵ����)
	buf[3] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	nDstLength += Bytes2String(buf, 4, &pDst[nDstLength]);		// ת��4���ֽڵ�Ŀ��PDU��
	nDstLength += InvertNumbers(pSrc->TPA, nLength, &pDst[nDstLength]);	// ת��TP-DA��Ŀ��PDU��

	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
	nLength = (int)strlen(pSrc->TP_UD);	// �û���Ϣ�ַ����ĳ���
	buf[0] = pSrc->TP_PID;			// Э���ʶ(TP-PID)
	buf[1] = pSrc->TP_DCS;			// �û���Ϣ���뷽ʽ(TP-DCS)
	buf[2] = msgLT/5 - 1;			// ��Ч��(TP-VP)
	
	if(pSrc->TP_DCS == GSM_7BIT)// 7-bit���뷽ʽ	
	{		
		buf[3] = nLength;
		nLength = Encode7bit(pSrc->TP_UD, nLength+1, &buf[4]) + 4;
	}
	else if(pSrc->TP_DCS == GSM_UCS2)// UCS2���뷽ʽ
	{
		buf[3] = EncodeUcs2(pSrc->TP_UD, nLength, &buf[4]);
		nLength = buf[3] + 4;
	}
	else// 8-bit���뷽ʽ
	{		
		buf[3] = Encode8bit(pSrc->TP_UD, nLength, &buf[4]);	
		nLength = buf[3] + 4;
	}
	nDstLength += Bytes2String(buf, nLength, &pDst[nDstLength]);

	return nDstLength;
}

// PDU���룬���ڽ��ա��Ķ�����Ϣ
int SmsCommon::DecodePdu(const char* pSrc, SM_PARAM* pDst)
{
	int nDstLength;
	unsigned char tmp;
	unsigned char buf[256];

	// SMSC��ַ��Ϣ��
	String2Bytes(pSrc, 2, &tmp);
	tmp = (tmp - 1) * 2;
	pSrc += 4;		
	SerializeNumbers(pSrc, tmp, pDst->SCA);	
	pSrc += tmp;

	// TPDU�λ�������
	String2Bytes(pSrc, 2, &tmp);
	pSrc += 2;

	// ȡ�ظ�����
	String2Bytes(pSrc, 2, &tmp);
	if(tmp & 1) tmp += 1;	// ������ż��
	pSrc += 4;			// ָ����ƣ������˻ظ���ַ(TP-RA)��ʽ
	SerializeNumbers(pSrc, tmp, pDst->TPA);	// ȡTP-RA����
	pSrc += tmp;	

	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
	String2Bytes(pSrc, 2, (unsigned char*)&pDst->TP_PID);	// ȡЭ���ʶ(TP-PID)
	pSrc += 2;		
	String2Bytes(pSrc, 2, (unsigned char*)&pDst->TP_DCS);	// ȡ���뷽ʽ(TP-DCS)
	pSrc += 2;		
	SerializeNumbers(pSrc, 14, pDst->TP_SCTS);		// ����ʱ����ַ���(TP_SCTS) 
	pSrc += 14;		
	String2Bytes(pSrc, 2, &tmp);	// �û���Ϣ����(TP-UDL)
	pSrc += 2;
	if(pDst->TP_DCS == GSM_7BIT)// 7-bit����	
	{		
		nDstLength = String2Bytes(pSrc, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4, buf);	// ��ʽת��
		Decode7bit(buf, nDstLength, pDst->TP_UD);	// ת����TP-DU
		nDstLength = tmp;
	}
	else if(pDst->TP_DCS == GSM_UCS2)// UCS2����
	{
		nDstLength = String2Bytes(pSrc, tmp * 2, buf);
		nDstLength = DecodeUcs2(buf, nDstLength, pDst->TP_UD);
	}
	else// 8-bit����
	{		
		nDstLength = String2Bytes(pSrc, tmp * 2, buf);	
		nDstLength = Decode8bit(buf, nDstLength, pDst->TP_UD);
	}

	return nDstLength;
}

}

