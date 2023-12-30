
#ifndef OBJECTWRAPPER_H
#define OBJECTWRAPPER_H

#include "scqueue.h"
#include "CWrapper.h"

class CObjectWrapper : public CWrapper
{
public:
	static CObjectWrapper * pObjectWrapper;
	CRITICAL_SECTION m_csUpdateParaSet;
	bool m_bUpdateParaSet;
	
	BYTE m_bEncrypt;	// added in 05-08-08
private:
	DW_ParameterSet m_ParaSet;
	DW_ParameterSet m_tmpParaSet;
	//zhenan add pid for log 20060815
	int m_nPID;
	WORD m_wTmpExt;
	WORD m_wTableIDExt;
public:
	CObjectWrapper();
	~CObjectWrapper();

	bool UpdateParaSet();
	bool SetWrapParaSet( DW_ParameterSet inPara );
	bool GetWrapParaSet( DW_ParameterSet * outPara );
	bool SetChannelPID(long inPID);
	bool GetChannelPID(long * outPID);
	bool GetMode(BYTE * byMode);
	bool SetMode(BYTE byMode);
	bool GetStreamCount( int * nCount );
	bool SetStreamCount( int nCount );
	bool SetChannelTag(const char * inTag, BYTE inLength);
	bool GetChannelTag(char * outTag, BYTE * outLength);
	bool AddVersionNumber();
	bool ResetVersionNumber();
	bool SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion);
	bool GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion);
	bool SetObjectKeyLength(BYTE inLength);
	bool GetObjectKeyLength(BYTE *outLength);
	bool SetIndexDescLength(BYTE inLength);
	bool GetIndexDescLength(BYTE *outLength);	
	bool SetEncrypt( BYTE inEncrypt );
	bool GetEncrypt( BYTE * outEncrypt );
	//bool SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen );
	//bool GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen);
	bool SetMaxSizeOfSample(long inSize);
	bool GetMaxSizeOfSample(long *outSize);	
	bool SetMinSizeOfSample(long inSize);
	bool GetMinSizeOfSample(long *outSize);	

	bool WrapData(  BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel );
private:
	bool ChipTable(  BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel );
};

#endif