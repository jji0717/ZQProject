#include "wtguuid.h"

CWTGUUID::CWTGUUID()
{
};
CWTGUUID::CWTGUUID(bool bsuffix,bool bffsession):_bflag_suffix(bsuffix),_bflag_ffsession(bffsession)
{};
CWTGUUID::~CWTGUUID()
{
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(CWTGUUID,"CWTGUUID beginning to de-construction OK"));
};

bool CWTGUUID::HexToString(std::string& outputString ,const unsigned char* buf, unsigned short int len)
{
	if(NULL == buf || len < 1)
		return false;
	char temp[4096] = "";
	for(unsigned short int i = 0 ; i < len; i++)
	{
		//	itoa(*(buf + i), temp + i * 2, 16);
		sprintf(temp + i * 2, "%02x",*(buf + i));
	}	
	outputString = (temp);
	return true;
}
bool CWTGUUID::fabricateTID(UINT32& fTransactionID )
{
	DWORD handlepid = GetCurrentThreadId();//GetThreadId(GetCurrentProcess());//;
	UINT32  loc_pid=(UINT32)handlepid;
	time_t lt;
	time(&lt);
	srand((unsigned int)(time(NULL)));
	int loc_rand=rand();//random number
	fTransactionID|=(loc_pid<<22);
	fTransactionID|=((lt&0x3FFFF)<<4);
	fTransactionID|=(loc_rand&0xF);
	return true;
}
bool CWTGUUID::decimalSerialN()
{
	memset(_decimalserialn,0x00,sizeof(_decimalserialn));
	UINT32 dsn1,dsn2,dsn3;
	fabricateTID(dsn1);
	fabricateTID(dsn2);
	fabricateTID(dsn3);
	BYTE* pbyte=(BYTE*)_decimalserialn;
	//_decimalserialn
	memcpy(pbyte,&dsn1,sizeof(UINT32));
	pbyte += sizeof(unsigned int);
	memcpy(pbyte,&dsn2,sizeof(UINT32));
	pbyte += sizeof(unsigned int);
	memcpy(pbyte,&dsn3,2);
	return true;
}
bool CWTGUUID::decimalSerialN2()
{
	//Sleep(1000);
	memset(_decimalserialn,0x00,sizeof(_decimalserialn));
	DWORD handlepid=GetCurrentProcessId(); 
	DWORD handletid = GetCurrentThreadId();
	time_t lt;
	time(&lt);
	uint8 uff=0xff;
	BYTE* pbyte=(BYTE*)_decimalserialn;
	if(!get_ffsession())
	{
		memcpy(pbyte,&uff,1);//0xFF  创建session序列 第一个字节是否为FF标志
		pbyte+=1;
		memcpy(pbyte,&handlepid,1);
		pbyte+=1;
	}
	else
	{
		memcpy(pbyte,&handlepid,2);
		pbyte+=2;
	}
	memcpy(pbyte,&handletid,2);
	pbyte+=2;
	memcpy(pbyte,&lt,4);
	srand((unsigned int)(time(NULL)));
	int loc_rand=rand();//random number
	pbyte+=4;
	memcpy(pbyte,&loc_rand,2);
	HexToString(_strdecimalserialn,(BYTE*)_decimalserialn,10);
	return true;
}
const char* CWTGUUID::decimalSerialNX()
{
	CoInitialize(NULL);
	static char buf[64] = {0};
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf(buf, sizeof(buf)
			, "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
		, guid.Data4[6], guid.Data4[7]
		);
	}
	memset(_decimalserialn,0x00,sizeof(_decimalserialn));
	memcpy(_decimalserialn,buf,10);
	HexToString(_strdecimalserialn,(BYTE*)_decimalserialn,10);
	CoUninitialize();
	return (const char*)buf;
}
void CWTGUUID::set_suffix(bool bflag)
{
	_bflag_suffix=bflag;
}
bool CWTGUUID::get_suffix()
{
	return _bflag_suffix;
}
void CWTGUUID::set_ffsession(bool bflag)
{
	_bflag_ffsession = bflag;
}
bool CWTGUUID::get_ffsession()
{
	return _bflag_ffsession;
}