#include "SmsCommon.h"

namespace nsSMS{

SmsCommon::SmsCommon(void)
{
}

SmsCommon::~SmsCommon(void)
{
}

// 可打印字符串转换为字节数据
// like:"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
int SmsCommon::String2Bytes(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	for (int i = 0; i < nSrcLength; i += 2)
	{
		// 输出高4位
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}

		pSrc++;

		// 输出低4位
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

	// 返回目标数据长度
	return (nSrcLength / 2);
}

// 字节数据转换为可打印字符串
// like：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
int SmsCommon::Bytes2String(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表

	for (int i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];		// 输出高4位
		*pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
		pSrc++;
	}

	*pDst = '\0';

	// 返回目标字符串长度
	return (nSrcLength * 2);
}

// 7bit编码
int SmsCommon::Encode7bit(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	int nSrc = 0;
	int nDst = 0;
	int nChar = 0;	// 当前正在处理的组内字符字节的序号，范围是0-7
	unsigned char nLeft = 0;	// 上一字节残余的数据


	// 将源串每8个字节分为一组，压缩成7个字节
	// 循环该处理过程，直至源串被处理完
	// 如果分组不到8字节，也能正确处理
	while (nSrc < nSrcLength)
	{
		// 取源字符串的计数值的最低3位
		nChar = nSrc & 7;

		// 处理源串的每个字节
		if(nChar == 0)
		{
			// 组内第一个字节，只是保存起来，待处理下一个字节时使用
			nLeft = *pSrc;
		}
		else
		{
			// 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			// 将该字节剩下的左边部分，作为残余数据保存起来
			nLeft = *pSrc >> nChar;

			// 修改目标串的指针和计数值
			pDst++;
			nDst++;
		}

		// 修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	return nDst;
}

// 7bit解码
int SmsCommon::Decode7bit(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	int nSrc = 0;
	int nDst = 0;
	int nByte = 0;		// 当前正在处理的组内字节的序号，范围是0-6
	unsigned char nLeft = 0;	// 上一字节残余的数据


	// 将源数据每7个字节分为一组，解压缩成8个字节
	// 循环该处理过程，直至源数据被处理完
	// 如果分组不到7字节，也能正确处理
	while(nSrc<nSrcLength)
	{
		// 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

		// 将该字节剩下的左边部分，作为残余数据保存起来
		nLeft = *pSrc >> (7-nByte);

		// 修改目标串的指针和计数值
		pDst++;
		nDst++;

		// 修改字节计数值
		nByte++;

		// 到了一组的最后一个字节
		if(nByte == 7)
		{
			// 额外得到一个目标解码字节
			*pDst = nLeft;

			// 修改目标串的指针和计数值
			pDst++;
			nDst++;

			// 组内字节序号和残余数据初始化
			nByte = 0;
			nLeft = 0;
		}

		// 修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}

	// 输出字符串加个结束符
	*pDst = '\0';

	return nDst;
}

// 8bit编码
int SmsCommon::Encode8bit(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}

// 8bit解码
int SmsCommon::Decode8bit(const unsigned char* pSrc, int nSrcLength, char* pDst)
{

	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}


#ifdef ZQ_OS_MSWIN
// UCS2编码
int SmsCommon::EncodeUcs2(const char* pSrc, int nSrcLength, unsigned char* pDst)
{
	int nDstLength;
	WCHAR wchar[128] = {0};

	// 字符串-->UNICODE串
	nDstLength = MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);

	// 高低字节对调，输出
	for(int i=0; i<nDstLength; i++)
	{
		*pDst++ = wchar[i] >> 8;
		*pDst++ = wchar[i] & 0xff;
	}
	
	*pDst = '\0';

	return nDstLength * 2;
}

// UCS2解码
int SmsCommon::DecodeUcs2(const unsigned char* pSrc, int nSrcLength, char* pDst)
{
	int nDstLength;
	WCHAR wchar[160] = {0};

	// 高低字节对调，拼成UNICODE
	for(int i=0; i<nSrcLength/2; i++)
	{
		wchar[i] = *pSrc++ << 8;
		wchar[i] |= *pSrc++;
	}

	// UNICODE串-->字符串
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

// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
// like："8613851872468" --> "683158812764F8"
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

	// 源串长度是奇数,补 'F'
	if(nSrcLength & 1)
	{
		*(pDst-2) = 'F';
		nDstLength++;
	}

	*pDst = '\0';

	return nDstLength;
}

// 两两颠倒的字符串转换为正常顺序的字符串
// like："683158812764F8" --> "8613851872468"
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

	// 最后的字符是'F'
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;
	}

	*pDst = '\0';

	return nDstLength;
}

// PDU编码，用于编制、发送短消息
int SmsCommon::EncodePdu(const SM_PARAM* pSrc, int msgLT, char* pDst)
{
	int nLength;
	int nDstLength;
	unsigned char buf[1024] = {0};

	// SMSC地址信息段
	nLength = (int)strlen(pSrc->SCA);	// SMSC地址字符串的长度	
	if(nLength == 0)//not set center number
	{
		buf[0] = 0;
		nDstLength = Bytes2String(buf,1,pDst);
	}
	else
	{
		buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC地址信息长度
		buf[1] = 0x91;		// 固定: 用国际格式号码
		nDstLength = Bytes2String(buf, 2, pDst);		// 转换2个字节到目标PDU串
		nDstLength += InvertNumbers(pSrc->SCA, nLength, &pDst[nDstLength]);	// 转换SMSC号码到目标PDU串
	}


	// TPDU段基本参数、目标地址等
	nLength = (int)strlen(pSrc->TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x11;					// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)nLength;			// 目标地址数字个数(TP-DA地址字符串真实长度)
	buf[3] = 0x91;					// 固定: 用国际格式号码
	nDstLength += Bytes2String(buf, 4, &pDst[nDstLength]);		// 转换4个字节到目标PDU串
	nDstLength += InvertNumbers(pSrc->TPA, nLength, &pDst[nDstLength]);	// 转换TP-DA到目标PDU串

	// TPDU段协议标识、编码方式、用户信息等
	nLength = (int)strlen(pSrc->TP_UD);	// 用户信息字符串的长度
	buf[0] = pSrc->TP_PID;			// 协议标识(TP-PID)
	buf[1] = pSrc->TP_DCS;			// 用户信息编码方式(TP-DCS)
	buf[2] = msgLT/5 - 1;			// 有效期(TP-VP)
	
	if(pSrc->TP_DCS == GSM_7BIT)// 7-bit编码方式	
	{		
		buf[3] = nLength;
		nLength = Encode7bit(pSrc->TP_UD, nLength+1, &buf[4]) + 4;
	}
	else if(pSrc->TP_DCS == GSM_UCS2)// UCS2编码方式
	{
		buf[3] = EncodeUcs2(pSrc->TP_UD, nLength, &buf[4]);
		nLength = buf[3] + 4;
	}
	else// 8-bit编码方式
	{		
		buf[3] = Encode8bit(pSrc->TP_UD, nLength, &buf[4]);	
		nLength = buf[3] + 4;
	}
	nDstLength += Bytes2String(buf, nLength, &pDst[nDstLength]);

	return nDstLength;
}

// PDU解码，用于接收、阅读短消息
int SmsCommon::DecodePdu(const char* pSrc, SM_PARAM* pDst)
{
	int nDstLength;
	unsigned char tmp;
	unsigned char buf[256];

	// SMSC地址信息段
	String2Bytes(pSrc, 2, &tmp);
	tmp = (tmp - 1) * 2;
	pSrc += 4;		
	SerializeNumbers(pSrc, tmp, pDst->SCA);	
	pSrc += tmp;

	// TPDU段基本参数
	String2Bytes(pSrc, 2, &tmp);
	pSrc += 2;

	// 取回复号码
	String2Bytes(pSrc, 2, &tmp);
	if(tmp & 1) tmp += 1;	// 调整奇偶性
	pSrc += 4;			// 指针后移，忽略了回复地址(TP-RA)格式
	SerializeNumbers(pSrc, tmp, pDst->TPA);	// 取TP-RA号码
	pSrc += tmp;	

	// TPDU段协议标识、编码方式、用户信息等
	String2Bytes(pSrc, 2, (unsigned char*)&pDst->TP_PID);	// 取协议标识(TP-PID)
	pSrc += 2;		
	String2Bytes(pSrc, 2, (unsigned char*)&pDst->TP_DCS);	// 取编码方式(TP-DCS)
	pSrc += 2;		
	SerializeNumbers(pSrc, 14, pDst->TP_SCTS);		// 服务时间戳字符串(TP_SCTS) 
	pSrc += 14;		
	String2Bytes(pSrc, 2, &tmp);	// 用户信息长度(TP-UDL)
	pSrc += 2;
	if(pDst->TP_DCS == GSM_7BIT)// 7-bit解码	
	{		
		nDstLength = String2Bytes(pSrc, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4, buf);	// 格式转换
		Decode7bit(buf, nDstLength, pDst->TP_UD);	// 转换到TP-DU
		nDstLength = tmp;
	}
	else if(pDst->TP_DCS == GSM_UCS2)// UCS2解码
	{
		nDstLength = String2Bytes(pSrc, tmp * 2, buf);
		nDstLength = DecodeUcs2(buf, nDstLength, pDst->TP_UD);
	}
	else// 8-bit解码
	{		
		nDstLength = String2Bytes(pSrc, tmp * 2, buf);	
		nDstLength = Decode8bit(buf, nDstLength, pDst->TP_UD);
	}

	return nDstLength;
}

}

