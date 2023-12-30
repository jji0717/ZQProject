
#ifndef DATAWRAPPERFILTER_H
#define DATAWRAPPERFILTER_H

#include "common.h"
#include "IObjectControl.h"
#include "IStatusReport.h"
#include "StatusReport.h"
#include "arrangebuf.h"

#define MAX_OBJECT_LENGTH 64*1024*16*4

#define NEW_BUFFER_SIZE  (PACKET_UNIT_SIZE*1000)
#define DELIVER_BUFFER_SIZE  (PACKET_UNIT_SIZE*500)
//////////////////////////////////////////////////////////////////////////
#include <boost/shared_ptr.hpp>

class CMemoryBlock
{
public:
	unsigned char * m_szBuffer;
	int  m_nSize;
public:
	CMemoryBlock( int nSize ) : m_nSize(nSize)
	{
		m_szBuffer = (unsigned char*)malloc(nSize);
		memset( m_szBuffer, 0, nSize );
	}
	~CMemoryBlock()
	{
		free( m_szBuffer );
	}
};

typedef boost::shared_ptr<CMemoryBlock> MemoryBlockPtr;

//////////////////////////////////////////////////////////////////////////

class CDataInputPin;
class CDataOutputPin;
class CWrapper;

#ifndef _NO_FIX_LOG
#include "fltinit.h"
#endif

//DECLSPEC_UUID("869FEC67-11B1-4387-9F76-60FBDD37659E")
class CDataWrapperFilter : public CBaseFilter, 
					 public IWrapperControl, 
#ifndef _NO_FIX_LOG
					 public IFilterInit, 
#endif
					 public IStatusReport
{
public:
	CDataOutputPin * m_pOutputPin;
	CDataInputPin * m_pInputPin;
	int	m_iPins;

	//zhenan 20060816 add it for respond stop_command; 
	BOOL m_IsStop;

	CWrapper * m_pWrapper;
	CStatusReport m_StatusReport;
	LONGLONG m_llFileSize;
	
	SubChannelVector m_SubChannelList;
	CArrangeBuf **m_ArrangeBuf;
	int m_arrangeBufNumber;

	int m_nBasePID;
	char chLogMsg[MAX_PATH];
	BYTE *m_pDeliverBuf;
	unsigned char output_p[MAX_OBJECT_LENGTH];        // MAX_OBJECT_LENGTH 为定义的输出区的长度  
private:
	// Constructor
	CDataWrapperFilter( LPUNKNOWN punk, HRESULT *phr);
	~CDataWrapperFilter();
public:
	CCritSec m_csFilter;

	int       GetPinCount(void);
	CBasePin *GetPin(int n);	// constructor method used by class factory
	int DelArrangeBufContent();
	int DelArrangeBufFile();
	int RealSortAndDelive();
	int RealInitPinSortBuffer();

	static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);
	DECLARE_IUNKNOWN;

	// Reveals IContrast & ISpecifyPropertyPages
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// check if you can support mtIn
	STDMETHODIMP Stop(); // PURE
	//virtual HRESULT Transform(IMediaSample *pSample); // PURE

	HRESULT SetSubChannelList( SubChannelVector &pSubChannellist );

	bool DiscardPrefixOfFilename( LPTSTR lpszFileName );

	// change the extension name of certain file to .dv
	// e.g. "temp.dv.pd" -> "temp.dv"
	bool DiscardFileExtName( LPTSTR lpszFileName );

	HRESULT SendIndexTable();

	HRESULT SendFlag( BYTE byFlag );

	long CalculateWrapSize( long cBytes );

	HRESULT Receive(IMediaSample* pSample, DWS_SubChannelList pSubChannel);
	HRESULT CompleteConnect(IPin* pPeer);

	HRESULT SetSampleBlock( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long * outSize, DWS_SubChannelList pSubChannel );
	HRESULT DeliverBuffer(  BYTE * pInBuffer, long inSize, DWS_SubChannelList pSubChannel,BOOL bRealDelive=FALSE );

	// interface from IWrapperControl
	STDMETHODIMP UpdateParaSet();
	STDMETHODIMP SetWrapParaSet( DW_ParameterSet inPara );
	STDMETHODIMP GetWrapParaSet( DW_ParameterSet * outPara );
	STDMETHODIMP SetChannelPID(long inPID);
	STDMETHODIMP GetMode(BYTE * byMode);
	STDMETHODIMP SetMode(BYTE byMode);
	STDMETHODIMP GetStreamCount(int * nCount);
	STDMETHODIMP SetStreamCount(int nCount,char *pcharPath);
	STDMETHODIMP GetChannelPID(long * outPID);
	STDMETHODIMP SetChannelTag(const char * inTag, BYTE inLength);
	STDMETHODIMP GetChannelTag(char * outTag, BYTE * outLength);
	STDMETHODIMP AddVersionNumber();
	STDMETHODIMP ResetVersionNumber();
	STDMETHODIMP SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion);
	STDMETHODIMP GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion);
	STDMETHODIMP SetObjectKeyLength(BYTE inLength);
	STDMETHODIMP GetObjectKeyLength(BYTE *outLength);
	STDMETHODIMP SetIndexDescLength(BYTE inLength);
	STDMETHODIMP GetIndexDescLength(BYTE *outLength);	
	STDMETHODIMP SetEncrypt(BYTE bEncrypt);
	STDMETHODIMP GetEncrypt(BYTE * bEncrypt);	
	STDMETHODIMP SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen );
	STDMETHODIMP GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen);
	STDMETHODIMP SetMaxSizeOfSample(long inSize);
	STDMETHODIMP GetMaxSizeOfSample(long *outSize);	
	STDMETHODIMP SetMinSizeOfSample(long inSize);
	STDMETHODIMP GetMinSizeOfSample(long *outSize);	

	// interface from IStatusReport
	STDMETHODIMP GetState( long * outState );
	STDMETHODIMP GetLastError( long * outError );
	STDMETHODIMP GetErrorMsg( char * outMsg, BYTE * outLength );

	// added by Cary
#ifndef _NO_FIX_LOG
	virtual void initLog(ISvcLog* log)
	{
		extern ISvcLog* service_log;
		service_log = log;
	}

protected:

	inline HRESULT itvConv(LPBYTE& pData, long& cRealBytes);
#endif

};

#endif
