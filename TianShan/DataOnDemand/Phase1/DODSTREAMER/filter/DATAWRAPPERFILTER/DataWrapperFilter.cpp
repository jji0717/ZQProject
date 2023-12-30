
#include "stdafx.h"

#include <initguid.h>

#include "common.h"
#include "strmif.h"
#include "DataOutputPin.h"
#include "DataInputPin.h"
#include "DataPullPin.h"
#include "ObjectWrapper.h"
#include "DataWrapperFilter.h"
#include "CWrapper.h"

// itv parser
extern "C" {
#include "../itv_parse/parser/inc/parse.h"
}

// {869FEC67-11B1-4387-9F76-60FBDD37659E}
DEFINE_GUID(CLSID_DataWrapper, 
			0x869fec67, 0x11b1, 0x4387, 0x9f, 0x76, 0x60, 0xfb, 0xdd, 0x37, 0x65, 0x9e);

// setup data
long g_lForChannelFilename=1;

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_Stream,       // Major type MEDIASUBTYPE_NULL
		&MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
	{
		L"Input",           // String pin name
			FALSE,              // Is it rendered
			FALSE,              // Is it an output
			FALSE,              // Allowed none
			FALSE,              // Allowed many
			&CLSID_NULL,        // Connects to filter
			L"Output",          // Connects to pin
			1,                  // Number of types
			&sudPinTypes },     // The pin details
		{ L"Output",          // String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&sudPinTypes        // The pin details
		}
};

const AMOVIESETUP_FILTER sudContrast =
{
	&CLSID_DataWrapper,        // Filter CLSID
		L"Data Wrapper",      // Filter name
		MERIT_DO_NOT_USE,       // Its merit
		2,                      // Number of pins
		psudPins                // Pin details
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = 
{
	{ L"Data Wrapper"
		, &CLSID_DataWrapper
		, CDataWrapperFilter::CreateInstance
		, NULL
		, &sudContrast }
	//,
	//{ L"Video Contrast Property Page"
	//, &CLSID_ContrastPropertyPage
	//, CContrastProperties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


CDataWrapperFilter::CDataWrapperFilter(LPUNKNOWN punk,HRESULT *phr) :
CBaseFilter(NAME("Data Wrapper"), punk, &m_csFilter, CLSID_DataWrapper, phr ),
m_pInputPin(NULL),
m_pWrapper(NULL),
m_iPins(0)
//CTransInPlaceFilter( NULL, punk, CLSID_DataWrapper, phr)
{
	ASSERT(phr);
	CAutoLock cAutoLock(&m_csFilter);

	/*
	m_paStreams = (CSourceStream **) new CDataOutputPin*[1];
	if(m_paStreams == NULL)
	{
		if(phr)
			*phr = E_OUTOFMEMORY;

		return;
	}

	m_paStreams[0] = new CDataOutputPin( phr, this, L"Data Output Pin!");
	if(m_paStreams[0] == NULL)
	{
		if(phr)
			*phr = E_OUTOFMEMORY;

		return;
	}
	// when the filter exit, the releasing work of CDataOutputPin and CDataOutputPin will be done by CSource.
	*/
	
	if (SUCCEEDED(*phr))
	{
		m_pOutputPin = new CDataOutputPin(this, &m_csFilter, phr, L"Output Pin" );
		m_iPins++;
	}
	if (SUCCEEDED(*phr))
	{
		m_pInputPin = new CDataInputPin(this, &m_csFilter, phr);
		m_iPins++;
	}


	if (SUCCEEDED(*phr))
	{
		m_pWrapper = new CObjectWrapper();
	}

	m_nBasePID = 0;
	m_ArrangeBuf = NULL;
	m_arrangeBufNumber = 0;
	g_lForChannelFilename = 1;
	m_IsStop = FALSE;
	//zhenan add it :fix bug DSA close service is very slow:
	m_pDeliverBuf = NULL;
} // Contrast

CDataWrapperFilter::~CDataWrapperFilter()
{
	m_IsStop = TRUE;

	char strLog[MAX_PATH];	

	sprintf(strLog,"PID=%d:Enter delete DataWrapperFilter !", m_nBasePID);	LogMyEvent(1,1,strLog);

	if( m_pOutputPin )
	{
		delete m_pOutputPin;
		m_pOutputPin = NULL;
		m_iPins--;
	}

	sprintf(strLog,"PID=%d:delete DataWrapperFilter ok!", m_nBasePID);	LogMyEvent(1,1,strLog);

	if( m_pInputPin )
	{
		delete m_pInputPin;
		m_pInputPin = NULL;
		m_iPins--;
	}
	sprintf(strLog,"PID=%d:delete InputPin ok!", m_nBasePID);	LogMyEvent(1,1,strLog);

	if( m_pWrapper )
	{
		delete m_pWrapper;
		m_pWrapper = NULL;
	}	
	sprintf(strLog,"PID=%d:delete Wrapper ok!", m_nBasePID);	LogMyEvent(1,1,strLog);

	DelArrangeBufContent();

	sprintf(strLog,"PID=%d: DelArrangeBufContent ok!", m_nBasePID);	LogMyEvent(1,1,strLog);

	if( m_ArrangeBuf )
	{
		delete m_ArrangeBuf;
		m_ArrangeBuf = NULL;
	}
	sprintf(strLog,"PID=%d: Del all ArrangeBufContent ok!", m_nBasePID);	LogMyEvent(1,1,strLog);

	if( m_pDeliverBuf )
	{
		delete m_pDeliverBuf;
		m_pDeliverBuf = NULL;
	}
}

//static 
CUnknown* WINAPI 
CDataWrapperFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
	ASSERT(phr);

	CUnknown *punk = new CDataWrapperFilter(pUnk, phr);
	if(punk == NULL)
	{
		if(phr)
			*phr = E_OUTOFMEMORY;
	}
	return punk;
}


//
// Basic COM - used here to reveal our own interfaces
STDMETHODIMP CDataWrapperFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == IID_IWrapperControl) 
	{
		return GetInterface((IWrapperControl *) this, ppv);
	}
	else if (riid == IID_IStatusReport) 
	{
		return GetInterface((IStatusReport *) this, ppv);
	}

#ifndef _NO_FIX_LOG
	else if (riid == IID_IFilterInit) {
		return GetInterface((IFilterInit *) this, ppv);
	}
#endif

	else 
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
} // NonDelegatingQueryInterface

//HRESULT CDataWrapperFilter::CheckInputType(const CMediaType* mtIn)
//{
//	return NOERROR;
//}
//
//HRESULT CDataWrapperFilter::Transform(IMediaSample *pSample)
//{
//	return NOERROR;
//}

//
// GetPinCount
//
// Returns the number of pins this filter has
int CDataWrapperFilter::GetPinCount(void) {

	CAutoLock lock(&m_csFilter);
	return m_iPins;
}


//
// GetPin
//
// Return a non-addref'd pointer to pin n
// needed by CBaseFilter
CBasePin *CDataWrapperFilter::GetPin(int n) {

	CAutoLock lock(&m_csFilter);

	/*
	// n must be in the range 0..m_iPins-1
	// if m_iPins>n  && n>=0 it follows that m_iPins>0
	// which is what used to be checked (i.e. checking that we have a pin)
	if ((n >= 0) && (n < m_iPins-1)) {

		ASSERT(m_paStreams[n]);
		return m_paStreams[n];
	}
	else if( n == m_iPins-1 )
		return m_pInputPin;
		*/
	if( n == 0 )
		return m_pOutputPin;
	else if( n == 1 )
		return m_pInputPin;

	return NULL;
}

HRESULT 
CDataWrapperFilter::CompleteConnect(IPin* pPeer)
{
	// called when input is connected to 
	// validate the input stream, and locate 
	// media type and timing information.

	// we use pull mode
	IAsyncReaderPtr pRdr = pPeer;
	if (pRdr == NULL)
	{
		return E_NOINTERFACE;
	}

	// get total length
	LONGLONG total, available;
	pRdr->Length(&total, &available);
	m_llFileSize = total;

	return S_OK;
}

HRESULT CDataWrapperFilter::SetSampleBlock( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long * outSize, DWS_SubChannelList pSubChannel )
{
	SampBlock tmpSampBlock;
	tmpSampBlock.dwProtocol = 0x01;
	tmpSampBlock.dwVersion = 0x01;
	tmpSampBlock.dwPID = pSubChannel.lChannelPID;
	memcpy( tmpSampBlock.chTag, pSubChannel.sChannelTag, pSubChannel.byTagLength );
	tmpSampBlock.nControlFlag = pSubChannel.byControlFlag;	// start
	tmpSampBlock.dwCount = 1;
	tmpSampBlock.dwLength = htonl( inSize );

	memcpy( pOutBuffer, &tmpSampBlock, sizeof( SampBlock ));
	if( inSize > 0 )
		memcpy( pOutBuffer+sizeof(SampBlock), pInBuffer, inSize );

	*outSize = inSize+sizeof(SampBlock);
	return S_OK;
}

HRESULT CDataWrapperFilter::DeliverBuffer(  BYTE * pInBuffer, long inSize, DWS_SubChannelList pSubChannel ,BOOL bRealDelive)
{
	HRESULT hr;

	BYTE byMode;
	//wsprintf(chLogMsg,"input1 PID=%d, inSize=%ld",pSubChannel.lChannelPID,inSize);LogMyEvent(1,0,chLogMsg);

	GetMode( &byMode );
	if( !(byMode & WRAP_MODE_MULTISTREAM) )
		GetChannelPID( &(pSubChannel.lChannelPID) );
	GetChannelTag( pSubChannel.sChannelTag, &(pSubChannel.byTagLength) );

	//wsprintf(chLogMsg,"input6 PID=%d, inSize=%ld",pSubChannel.lChannelPID,inSize);LogMyEvent(1,0,chLogMsg);

	if (m_arrangeBufNumber>0 && inSize >0 && bRealDelive==FALSE)
	{		
		BOOL PIDFound=FALSE;
		int nPackNumber=inSize/PACKET_UNIT_SIZE;
		int nPID=(pInBuffer[1]& 0x1f)<<8 | pInBuffer[2];	

		//wsprintf(chLogMsg,"input7 PID=%d, nPackNumber=%d",nPID,nPackNumber);LogMyEvent(1,0,chLogMsg);

		for (int i=0;i<m_arrangeBufNumber;i++)
		{
			if (m_IsStop)
			{
				wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::DeliverBuffer:respond stop_command",nPID);				
				LogMyEvent(1,1,chLogMsg);
				return S_OK;
			}

			if (nPID == m_ArrangeBuf[i]->m_nPID)
			{
				//wsprintf(chLogMsg,"PID=%d, m_ArrangeBuf[%d]->m_nPID=%d",nPID, m_ArrangeBuf[i]->m_nPID);LogMyEvent(1,0,chLogMsg);
				PIDFound=TRUE;

				if (m_ArrangeBuf[i]->m_hFile !=INVALID_HANDLE_VALUE)
				{
					//wsprintf(chLogMsg,"PID=%d, m_ArrangeBuf[i]->m_>m_hFile !=INVALID_HANDLE_",nPID);LogMyEvent(1,0,chLogMsg);
					DWORD dwWritten=0;
					if (!WriteFile(m_ArrangeBuf[i]->m_hFile, (PVOID)(pInBuffer), (DWORD)inSize,
						&dwWritten, NULL)) 
					{
						DWORD dwErr = ::GetLastError();
						wsprintf(chLogMsg,"PID=%d::WriteFile(%s) error.errorcode=%d ",nPID,m_ArrangeBuf[i]->m_strFilename,dwErr);				
						LogMyEvent(0,1,chLogMsg);
						CloseHandle(m_ArrangeBuf[i]->m_hFile); 
						m_ArrangeBuf[i]->m_hFile=INVALID_HANDLE_VALUE;
						return  S_FALSE;
					}
					m_ArrangeBuf[i]->m_nTotalPacketNumber +=nPackNumber;
				}
				break;
			}
		}
		if (PIDFound== FALSE)
		{
			wsprintf(chLogMsg,"PID=%d don't found",nPID);LogMyEvent(1,0,chLogMsg);
		}
		return S_OK;
	}

	// get an empty buffer for output.
	IMediaSample * pDesSample = NULL;
	//IMediaSamplePtr pDesSample;
	
	if (m_IsStop)
	{
		wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::GetDeliveryBuffer:respond stop_command",m_nBasePID);				
		LogMyEvent(1,1,chLogMsg);
		return S_OK;
	}

	hr = m_pOutputPin->GetDeliveryBuffer( &pDesSample, NULL, NULL, 0);
	if (FAILED(hr))
	{
		wsprintf(chLogMsg,"PID=%d:In CDataWrapperFilter::DeliverBuffer() - m_pOutputPin->GetDeliveryBuffer() failcode=%ld, no deliver,",m_nBasePID,hr);LogMyEvent(0,0,chLogMsg);
		return hr;
	}
	//wsprintf(chLogMsg,"input3 PID=%d, inSize=%ld",pSubChannel.lChannelPID,inSize);LogMyEvent(1,0,chLogMsg);
	if (pDesSample->GetSize() < inSize)
	{
		wsprintf(chLogMsg,"PID=%d:In CDataWrapperFilter::DeliverBuffer() - pDesSample->GetSize() < inSize no deliver",m_nBasePID);LogMyEvent(0,0,chLogMsg);

		pDesSample->Release();
		return E_INVALIDARG;
	}
	//wsprintf(chLogMsg,"input4 PID=%d, inSize=%ld",pSubChannel.lChannelPID,inSize);LogMyEvent(1,0,chLogMsg);

	BYTE * pOutPointer;
	hr = pDesSample->GetPointer( &pOutPointer );
	if(FAILED(hr))
	{
		wsprintf(chLogMsg,"PID=%d:In CDataWrapperFilter::DeliverBuffer() - pDesSample->GetPointer() fail, no deliver",m_nBasePID);LogMyEvent(0,0,chLogMsg);
		pDesSample->Release();
		return hr;
	}
//	wsprintf(chLogMsg,"input5 PID=%d, inSize=%ld",pSubChannel.lChannelPID,inSize);LogMyEvent(1,0,chLogMsg);

	// for test
	//m_pWrapper->SaveToFile( pInBuffer, inSize );

	long lSampleLength = 0;
	SetSampleBlock( pInBuffer, inSize, pOutPointer, &lSampleLength, pSubChannel );

	pDesSample->SetActualDataLength(lSampleLength);

	hr = m_pOutputPin->Deliver( pDesSample );
	pDesSample->Release();

	if (FAILED(hr)) {
		glog(ISvcLog::L_ERROR, "%s:\tCDataOutputPin::Deliver() failed. ChannelPID = %d",
			__FUNCTION__, pSubChannel.lChannelPID);
		return hr;
	}

	//m_pOutputPin->ReleaseBuffer( pDesSample );

	return S_OK;
}

HRESULT CDataWrapperFilter::SendFlag( BYTE byFlag )
{
	HRESULT hr;

	DWS_SubChannelList tmpSubChannel;
	if( byFlag == 0 )
	{	// start		
		tmpSubChannel.byControlFlag = 0;
		m_pWrapper->UpdateParaSet();

		m_StatusReport.SetState( 1 );
	}
	else	
	{   //end
		tmpSubChannel.byControlFlag = 2;

		m_StatusReport.SetState( 0 );
	}

	hr = DeliverBuffer( NULL, 0, tmpSubChannel );

	if (FAILED(hr)) {
		glog(ISvcLog::L_ERROR, "%s:\tCDataWrapperFilter::DeliverBuffer() failed.", 
			__FUNCTION__);		
	} else {
		// send index table
		if( byFlag == 0 )
		{
			//TCHAR szTag[MAXTAGLENGTH];
			//BYTE  byLen = MAXTAGLENGTH;
			//m_pWrapper->GetChannelTag( szTag, &byLen );
			BYTE byMode;
			m_pWrapper->GetMode( &byMode );
			if( byMode & WRAP_MODE_WITHINDEXTABLE )
			//if( _tcsicmp( szTag, "msg") != 0 )
			{
				SendIndexTable();
			}
		}
	}

	return hr;
}

static void dumpHexLine(char* buffer, const unsigned char* data, 
						int pos, int size, const char* hint)
{
	int i;
	int bufPos = 0;
	int len;

	if (hint) {
		len = sprintf(&buffer[bufPos], "%s ", hint);
		bufPos += len;
	}

	len = sprintf(&buffer[bufPos], "%08x ", pos);
	bufPos += len;
	for (i = 0; i < 16; i ++) {
		if (i < size) {
			len = sprintf(&buffer[bufPos], "%0.2x ", data[i]);
			bufPos += len;
		}
		else {
			len = sprintf(&buffer[bufPos], "   ");
			bufPos += len;
		}

		if (i == 7) {
			buffer[bufPos ++] = ' ';
		}
	}

	buffer[bufPos ++] = ' ';
	buffer[bufPos ++] = '|';
	for (i = 0; i < 16; i ++) {
		if (i < size) {
			if (isgraph(data[i])) {
				buffer[bufPos ++] = data[i];
			} else {
				buffer[bufPos ++] = '.';
			}
		}
		else
			buffer[bufPos ++] = ' ';
	}

	//buffer[bufPos ++] = '\n';
	buffer[bufPos ++] = '\0';
}

// added by Cary
inline HRESULT CDataWrapperFilter::itvConv(LPBYTE& pData, long& cRealBytes)
{
	BYTE encryptMode  = 0;
	m_pWrapper->GetEncrypt( &encryptMode );
    
	if (encryptMode == 0) {
		return S_OK;
	}

	glog(ISvcLog::L_DEBUG_DETAIL, "%s:\tpid: %d, encrypt mode: %d, data length: %d", 
		__FUNCTION__, m_nBasePID, encryptMode, cRealBytes);

	char buffer[1024];
	dumpHexLine(buffer, pData, 0, 16, "incoming data: ");
	glog(ISvcLog::L_DEBUG_DETAIL, "%s:\tpid: %d, %s", __FUNCTION__, m_nBasePID, buffer);

	BYTE compress = (encryptMode & 0x4) >> 2;
	encryptMode &= 0x03;

	ITV_DWORD itv_err;
	WI_ParseInfo data_p;
	WI_ParseInfo *parser_p;
	ITV_UBYTE *getoutput_p = pData;			// 其中该指针指向你们的数据区
	ITV_DWORD getlength_p = cRealBytes;		// 该长度为你们数据区的长度

	if( getoutput_p[0] == 0xef && getoutput_p[1] == 0xbb && 
		getoutput_p[2] == 0xbf ) {

		getoutput_p += 3;
	}

	if( getoutput_p[0] == 0xff && getoutput_p[1] == 0xfe ) {

		getoutput_p += 2;
	}

	memset( output_p, 0, sizeof(output_p) );

	// MAX_OBJECT_LENGTH 为定义的输出区的长度  
	// ITV_UBYTE output_p[MAX_OBJECT_LENGTH] = {0};
	// the 8 was asked by ZheXiang.Yin in 2006-2-21
	// the 16*4 was asked by ZheXiang.Yin in 2006-3-22

	parser_p = &data_p;
	data_p.input = getoutput_p;
	data_p.input_size = cRealBytes;
	data_p.output = output_p;
	data_p.output_size = MAX_OBJECT_LENGTH;

	glog(ISvcLog::L_DEBUG,"%s\tpid: %d, calling WI_Parse_parse() - "
		"encryptMode = %d, compression = %d, input = %p, input_size = %d", 
		__FUNCTION__, m_nBasePID, encryptMode, compress, getoutput_p, 
		getlength_p);

	itv_err = WI_Parse_init(parser_p);

	if(itv_err != 0) {

		glog(ISvcLog::L_DEBUG_DETAIL, 
			"%s\tpid: %d:WI_Parse_init() failed, itv_err = %d!", 
			__FUNCTION__, m_nBasePID, itv_err);

		return S_FALSE;

	}

	switch (encryptMode) {

	case ENCRYPT_MODE_NONE:

		if (compress == ENCRYPT_MODE_COMPRESS_SIMPLE) {
			parser_p->real_outsize = parser_p->output_size;
			glog(ISvcLog::L_DEBUG_DETAIL, "%s\tpid: %d, %s calling", 
				__FUNCTION__, m_nBasePID, "ItvParse_Compress");
			itv_err = ItvParse_Compress(data_p.output, &data_p.real_outsize, 
				data_p.input, data_p.input_size);
			
		} else {
			assert(false);
		}

		break;

	case ENCRYPT_MODE_ILP_SIMPLE:

		if (compress == ENCRYPT_MODE_COMPRESS_SIMPLE) {

			glog(ISvcLog::L_DEBUG_DETAIL, "%s\tpid: %d, %s calling", 
				__FUNCTION__, m_nBasePID, "ItvParse_SimpleParseAndCompress");

			itv_err = ItvParse_SimpleParseAndCompress(parser_p);			
		} else {

			glog(ISvcLog::L_DEBUG_DETAIL, "%s\tpid: %d, %s calling", 
				__FUNCTION__, m_nBasePID, "Itv_Parse_SimpleParse");

			itv_err = Itv_Parse_SimpleParse(parser_p);			
		}

		break;

	case ENCRYPT_MODE_ILP_NORMAL:

		if (compress == ENCRYPT_MODE_COMPRESS_SIMPLE) {

			glog(ISvcLog::L_DEBUG_DETAIL, "%s\tpid: %d, %s calling", 
				__FUNCTION__, m_nBasePID, "ItvParse_ParseAndCompress");

			itv_err = ItvParse_ParseAndCompress(parser_p);		
		} else {

			glog(ISvcLog::L_DEBUG_DETAIL, "%s\tpid: %d, %s calling", 
				__FUNCTION__, m_nBasePID, "WI_Parse_parse");

			itv_err = WI_Parse_parse(parser_p);			
		}

		break;

	default:
		glog(ISvcLog::L_ERROR, "PID = %d:invalid encrypt mode (%x)", 
			m_nBasePID, encryptMode);
		assert(false);
		return S_FALSE;
	}

	if(itv_err != 0) {

		glog(ISvcLog::L_ERROR, "PID=%d:WI_Parse_parse fail, error code: %d",
			m_nBasePID, itv_err);
		return S_FALSE;
	}

	glog(ISvcLog::L_DEBUG, "PID=%d::Receive() - WI_Parse_parse() successful", 
		m_nBasePID);

	pData = output_p;
	cRealBytes = parser_p->real_outsize;

	glog(ISvcLog::L_DEBUG_DETAIL, "%s:\tpid: %d, outgo length: %d", 
		__FUNCTION__, m_nBasePID, cRealBytes);
	dumpHexLine(buffer, pData, 0, 16, "outgo data: ");
	glog(ISvcLog::L_DEBUG_DETAIL, "%s:\tpid: %d, %s", __FUNCTION__, 
		m_nBasePID, buffer);

	return S_OK;
}

HRESULT CDataWrapperFilter::Receive(IMediaSample* pSample, DWS_SubChannelList pSubChannel )
{
	HRESULT hr;
	char chLogMsg[MAX_PATH];

	if (m_IsStop)
	{
		wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::Receive:respond stop_command",m_nBasePID);				
		LogMyEvent(1,1,chLogMsg);
		return S_OK;
	}

	if( pSample != NULL )
	{
		// append data to parser buffer
		BYTE* pData;
		pSample->GetPointer(&pData);
		long cRealBytes = pSample->GetActualDataLength();
		if( cRealBytes < 1 )
		{
			wsprintf(chLogMsg,"PID=%d:CDataWrapperFilter::Receive() - cRealBytes < 1.",m_nBasePID);LogMyEvent(0,0,chLogMsg);
			return S_FALSE;
		}


		//////////////////////////////////////////////////////////////////////////
		// modified by Cary
		hr = itvConv(pData, cRealBytes);
		if (FAILED(hr)) {
			glog(ISvcLog::L_DEBUG, "%s\titvConv() failed(hr = %x).", __FUNCTION__, hr);
		}

		//////////////////////////////////////////////////////////////////////////
		
		//TCHAR szTag[MAXTAGLENGTH] = {0};
		//BYTE  byLen = MAXTAGLENGTH;
		//m_pWrapper->GetChannelTag( szTag, &byLen );
		//if( _tcsicmp( szTag, "nav") == 0 
		// || _tcsicmp( szTag, "chn") == 0 
		// || _tcsicmp( szTag, "po") == 0 )


		// for testing.
		//m_pWrapper->SaveToFile( pData, cBytes );

		// temp limit: a sample pass the data volume of one object-in-a-table per time
		//if( cBytes > OBJECTPAYLOADMAXLEN )
		//	return S_FALSE;

		long lTableCount = cRealBytes/OBJECTPAYLOADMAXLEN; //+ cBytes%OBJECTPAYLOADMAXLEN?1:0 )*TABLEMAPMAXLEN;

		long cBytes = (lTableCount*TABLEPAYLOADMAXLEN) + (cRealBytes%OBJECTPAYLOADMAXLEN) ? (cRealBytes%OBJECTPAYLOADMAXLEN+sizeof(ObjectHeader)) : 0;

		long llength = CalculateWrapSize( cBytes );

		BYTE * pOutBuffer = new BYTE[llength];

		// wrap the data.
		wsprintf(chLogMsg,"PID=%d:Receive() - Call WrapData()  cRealBytes: %d",m_nBasePID,cRealBytes);LogMyEvent(2,0,chLogMsg);

		long lOutLength = 0;
		m_pWrapper->WrapData( pData, cRealBytes, pOutBuffer, llength, &lOutLength, pSubChannel );

		wsprintf(chLogMsg,"PID=%d:Receive() - Call DeliverBuffer().lOutLength=%ld",m_nBasePID,lOutLength);LogMyEvent(2,0,chLogMsg);

		hr = DeliverBuffer( pOutBuffer, lOutLength, pSubChannel );

		wsprintf(chLogMsg,"PID=%d::Receive() end",m_nBasePID);LogMyEvent(1,0,chLogMsg);

		delete [] pOutBuffer;
		pOutBuffer = NULL;

		if (FAILED(hr)) {
			glog(ISvcLog::L_ERROR, "%s:\tCDataWrapperFilter::DeliverBuffer() failed.", 
				__FUNCTION__);
			return hr;
		}
	}

	return S_OK;
}

long CDataWrapperFilter::CalculateWrapSize( long cBytes )
{
	char chLogMsg[MAX_PATH]; 

	if (m_IsStop)
	{
		wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::CalculateWrapSize:respond stop_command",m_nBasePID);				
		LogMyEvent(1,1,chLogMsg);
		return 0;
	}
	//wsprintf(chLogMsg,"PID=%d:Enter  CDataWrapperFilter::CalculateWrapSize cBytes=%ld",m_nBasePID,cBytes);LogMyEvent(1,0,chLogMsg);

	// cBytes = 131600
	// the count the section needed, left integer.
	long lSectionCount = cBytes / SECTION_PAYLOAD_SIZE; // + cBytes % SECTION_PAYLOAD_SIZE ? 1:0;
	// 32
	// bytes of the last section.
	long lLeftBytes = (cBytes % SECTION_PAYLOAD_SIZE) ? ( cBytes % SECTION_PAYLOAD_SIZE + SECTION_HEADER_SIZE ) : 0;
	// 925
	// the count of total packets needed.
	long lPacketCount = (lSectionCount * PACKETS_IN_SECTION) + (lLeftBytes / PACKET_PAYLOAD_SIZE) + ((lLeftBytes % PACKET_PAYLOAD_SIZE) ? 1 : 0);

	// 742 = 736 + 5 + 1
	long llength = lPacketCount * PACKET_UNIT_SIZE;
	// llength = 139496
	//wsprintf(chLogMsg,"PID=%d:Exit  CDataWrapperFilter::CalculateWrapSize llength=%ld",m_nBasePID,llength);LogMyEvent(1,0,chLogMsg);

	return llength;
}

HRESULT CDataWrapperFilter::SetSubChannelList( SubChannelVector &pSubChannellist )
{
	char chLogMsg[MAX_PATH]; 

	if (m_IsStop)
	{
		wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::SetSubChannelList:respond stop_command",m_nBasePID);				
		LogMyEvent(1,1,chLogMsg);
		return S_OK;
	}

	m_SubChannelList.clear();
	m_SubChannelList = pSubChannellist;

	wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::SetSubChannelList:save subChannellist to dataWrapper list",m_nBasePID);				
	LogMyEvent(1,1,chLogMsg);

	return S_OK;
}

// discard prefix of filename, i.e. "\DVL\050025" => "050025"
bool CDataWrapperFilter::DiscardPrefixOfFilename( LPTSTR lpszFileName )
{
	char chLogMsg[MAX_PATH]; 

	if (m_IsStop)
	{
		wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::DiscardPrefixOfFilename:respond stop_command",m_nBasePID);				
		LogMyEvent(1,1,chLogMsg);
		return true;
	}

	int iLen = (int)strlen( lpszFileName );//_tcslen( lpszFileName );

	TCHAR szTmpName[MAX_PATH] = { 0 };

	int i=iLen-2;
	for( ; i>=0; i-- )
	{
		if( lpszFileName[i] == '\\' )
		{
			strcpy( szTmpName, &lpszFileName[i+1] );//_tcscpy( szTmpName, &lpszFileName[i+1] );
			strcpy( lpszFileName, szTmpName ); //_tcscpy( lpszFileName, szTmpName );
			break;
		}
	}

	return true;
}
// change the extension name of certain file to .dv
// e.g. "temp.dv.pd" -> "temp.dv"
bool CDataWrapperFilter::DiscardFileExtName( LPTSTR lpszFileName )
{
	char chLogMsg[MAX_PATH]; 

	if (m_IsStop)
	{
		wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::DiscardFileExtName:respond stop_command",m_nBasePID);				
		LogMyEvent(1,1,chLogMsg);
		return true;
	}

	int iLen = (int)_tcslen( lpszFileName );

	//bool bHaveExt = false;
	int i=iLen-1;
	for( ; i>=0; i-- )
	{
		if( lpszFileName[i] == '.' )
		{
			lpszFileName[i] = 0;
			//bHaveExt = true;
			break;
		}
	}

	return true;
}

HRESULT CDataWrapperFilter::SendIndexTable()
{
	HRESULT hr;
	char chLogMsg[MAX_PATH]; 

	long count = (long) m_SubChannelList.size();
	long cBytes = count*sizeof(ObjectDescriptor);

	// temp limit: a sample pass the data volume of one object-in-a-table per time
	// commented because the request from the Jerroy that also send data though the count of object is Zero, 2006-05-23.
	//if( cBytes > OBJECTPAYLOADMAXLEN || cBytes == 0 )
	//	return S_FALSE;

	BYTE* pData = NULL;
	long llength = 0;
	BYTE * pOutBuffer = NULL;

	if( cBytes != 0 )
	{
		// discard the prefix path with filename.
		SubChannelVector::iterator itrSubChannel;
		for( itrSubChannel = m_SubChannelList.begin(); 
			itrSubChannel != m_SubChannelList.end(); ++itrSubChannel )
		{
			DiscardPrefixOfFilename( (*itrSubChannel).szFileName );
			DiscardFileExtName( (*itrSubChannel).szFileName );
		}
		wsprintf(chLogMsg,"PID=%d:create IndexTable, remove file_path and file_ext of all filename ",m_nBasePID);LogMyEvent(1,0,chLogMsg);

		sort( m_SubChannelList.begin(), m_SubChannelList.end(), FileSort() );

		pData = new BYTE[cBytes];

		ObjectDescriptor tmpObjDescriptor;

		int len=0, length=0;
		count = 0;	
		for( itrSubChannel = m_SubChannelList.begin(); 
			itrSubChannel != m_SubChannelList.end(); ++itrSubChannel )
		{
			if (m_IsStop)
			{
				wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::SendIndexTable:respond stop_command",m_nBasePID);				
				LogMyEvent(1,1,chLogMsg);
				delete [] pData;
				pData = NULL;
				return true;
			}

			tmpObjDescriptor.byTableCount = (*itrSubChannel).byTableCount;
			memcpy( tmpObjDescriptor.pchID, (*itrSubChannel).sObjectKey, DEFAULTOBJECTKEYLENGTH );
			len=(int)strlen((*itrSubChannel).szFileName);
			if (len>DEFAULTINDEXDESCLENGTH) {
				length = DEFAULTINDEXDESCLENGTH;
				len = len - DEFAULTINDEXDESCLENGTH;
			}else{
				length = len;
				len = 0;
			}
			memset( tmpObjDescriptor.pchDesc, 0, DEFAULTINDEXDESCLENGTH );
			memcpy( tmpObjDescriptor.pchDesc, (*itrSubChannel).szFileName+len, length );

			//strncpy( tmpObjDescriptor.pchDesc, (*itrSubChannel).szFileName, DEFAULTINDEXDESCLENGTH );
			//wsprintf(chLogMsg,"In IndexTable, FileName: %s",(*itrSubChannel).szFileName );LogMyEvent(1,0,chLogMsg);
			memcpy( pData+count*sizeof(ObjectDescriptor), &tmpObjDescriptor, sizeof(ObjectDescriptor) );
			count++;
		}	

		llength = ( cBytes/OBJECTPAYLOADMAXLEN + cBytes%OBJECTPAYLOADMAXLEN?1:0 )*TABLEMAPMAXLEN;
		wsprintf(chLogMsg,"PID=%d:create IndexTable,tableLen=%ld ",m_nBasePID,llength);LogMyEvent(1,0,chLogMsg);

	}
	else
	{
		pData = NULL;
		llength = TABLEMAPMAXLEN;
	}

	pOutBuffer = new BYTE[llength];

	DWS_SubChannelList tmpSubChannel;
	tmpSubChannel.wReserve = 1;
	tmpSubChannel.byControlFlag = 1;
	tmpSubChannel.byTableID = (char )0x80;
	tmpSubChannel.wTableIDExtension = 0x0080;
	tmpSubChannel.sObjectKey[1] = (char )0x80;
	tmpSubChannel.sObjectKey[3] = (char )0x80;

	long lPID;
	int	 nStreamCount;
	GetChannelPID( &lPID );
	GetStreamCount( &nStreamCount );
	tmpSubChannel.lChannelPID = lPID+nStreamCount;

	wsprintf(chLogMsg,"PID=%d:In IndexTable, Channel PID: ",m_nBasePID,lPID);LogMyEvent(1,0,chLogMsg);

	// wrap the data.
	long lOutLength = 0;
	m_pWrapper->WrapData( pData, cBytes, pOutBuffer, llength, &lOutLength, tmpSubChannel );

	hr = DeliverBuffer( pOutBuffer, lOutLength, tmpSubChannel );

	delete [] pOutBuffer;
	pOutBuffer = NULL;

	delete [] pData;
	pData = NULL;

	wsprintf(chLogMsg,"PID=%d:SendIndexTable successfully",m_nBasePID);LogMyEvent(1,0,chLogMsg);

	if (FAILED(hr)) {
		glog(ISvcLog::L_ERROR, "%s:\tCDataWrapperFilter::DeliverBuffer() failed.", 
			__FUNCTION__);		
	}

	return hr;
}

STDMETHODIMP CDataWrapperFilter::UpdateParaSet()
{
	m_pWrapper->UpdateParaSet();
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetWrapParaSet( DW_ParameterSet inPara )
{
	m_pWrapper->SetWrapParaSet( inPara );

	// because maybe the channel pid modified.
	m_pInputPin->SetChannelPID( inPara.lChannelPID );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetWrapParaSet( DW_ParameterSet * outPara )
{
	m_pWrapper->GetWrapParaSet( outPara );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetChannelPID(long inPID)
{
	char    chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"ChannelPID: %d---------------------------------------------",inPID);LogMyEvent(1,0,chLogMsg);

	m_nBasePID=inPID;
	m_pWrapper->SetChannelPID( inPID );

	m_pInputPin->SetChannelPID( inPID );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetMode(BYTE * byMode)
{
	m_pWrapper->GetMode( byMode );
	return NOERROR;
}


STDMETHODIMP CDataWrapperFilter::SetMode(BYTE byMode)
{
	m_pWrapper->SetMode( byMode );
	m_pInputPin->SetMode( byMode );
	char    chLogMsg[MAX_PATH];

	char szTmp[128] = {0};
	if( byMode & WRAP_MODE_COMMON )
		_tcscat( szTmp, _T("WRAP_MODE_COMMON ") );
	if( byMode & WRAP_MODE_WITHINDEXTABLE )
		_tcscat( szTmp, _T("WRAP_MODE_WITHINDEXTABLE ") );
	if( byMode & WRAP_MODE_TABLEIDFROMFILENAME )
		_tcscat( szTmp, _T("WRAP_MODE_TABLEIDFROMFILENAME ") );
	if( byMode & WRAP_MODE_MULTISTREAM )
		_tcscat( szTmp, _T("WRAP_MODE_MULTISTREAM ") );

	wsprintf(chLogMsg,"PID=%d:SetMode: %s",m_nBasePID,szTmp);LogMyEvent(1,0,chLogMsg);

	long lState;
	m_StatusReport.GetState( &lState );
	if( lState = 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetStreamCount(int * nCount)
{
	m_pWrapper->GetStreamCount( nCount );
	return NOERROR;
}

int CDataWrapperFilter::DelArrangeBufContent()
{
	if (m_arrangeBufNumber==0)	
		return 0;
	// char strLog[MAX_PATH];	

	for(int i=0;i<m_arrangeBufNumber;i++)
	{
		if(m_ArrangeBuf[i])
		{
			sprintf(chLogMsg,"PID=%d:Del Arrange Buf Content index=%d", m_ArrangeBuf[i]->m_nPID,i);			LogMyEvent(1,1,chLogMsg);
			delete m_ArrangeBuf[i];
			m_ArrangeBuf[i] = NULL;
		}		
	}
	return 0;
}

int CDataWrapperFilter::DelArrangeBufFile()
{
	if (m_arrangeBufNumber==0)	
		return 0;

	for(int i=0;i<m_arrangeBufNumber;i++)
	{
		if(m_ArrangeBuf[i])
		{
			if (m_ArrangeBuf[i]->m_hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_ArrangeBuf[i]->m_hFile);
				m_ArrangeBuf[i]->m_hFile=INVALID_HANDLE_VALUE;
			}
		}	
		DeleteFile(m_ArrangeBuf[i]->m_strFilename);	
	}
	return 0;
}
STDMETHODIMP CDataWrapperFilter::SetStreamCount(int nCount,char *pcharPath)
{
	char    chMsg[MAX_PATH*2];
	if (nCount >0 && pcharPath !=NULL)
	{
		m_ArrangeBuf=new PCARRANGEBUF[nCount+1];
		m_arrangeBufNumber=nCount+1;

		m_pDeliverBuf=new BYTE[NEW_BUFFER_SIZE+1];
		for (int i=0;i<m_arrangeBufNumber;i++)
		{
			m_ArrangeBuf[i]=new CArrangeBuf;
			m_ArrangeBuf[i]->m_nPID=m_nBasePID+i;

			SYSTEMTIME		St;
			GetLocalTime(&St);
			wsprintf(chMsg,"%s\\PID%d%02d%02d%02d%02d%02d%03d%ld",pcharPath,m_ArrangeBuf[i]->m_nPID,St.wMonth,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds,g_lForChannelFilename++);
			strcpy(m_ArrangeBuf[i]->m_strFilename,chMsg);

			if (g_lForChannelFilename >= 0xfffffff)
				g_lForChannelFilename=1;
		}
	}

	m_pWrapper->SetStreamCount( nCount );
	m_pInputPin->SetStreamCount( nCount,m_nBasePID);

	wsprintf(chLogMsg,"PID=%d:SetStreamCount, nCount: %d",m_nBasePID,nCount);LogMyEvent(1,0,chLogMsg);

	long lState;
	m_StatusReport.GetState( &lState );
	if( lState = 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetChannelPID(long * outPID)
{
	m_pWrapper->GetChannelPID( outPID );
	return NOERROR;
}


STDMETHODIMP CDataWrapperFilter::SetChannelTag(const char * inTag, BYTE inLength)
{
	m_pWrapper->SetChannelTag( inTag, inLength );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetChannelTag(char * outTag, BYTE * outLength)
{
	m_pWrapper->GetChannelTag( outTag, outLength );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::AddVersionNumber()
{
	m_pWrapper->AddVersionNumber();
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::ResetVersionNumber()
{
	m_pWrapper->ResetVersionNumber();
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion)
{
	m_pWrapper->SetVersionNumber( inObjectKey, inObjectKeyLen, inVersion );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion)
{
	m_pWrapper->GetVersionNumber( inObjectKey, inObjectKeyLen, outVersion );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetObjectKeyLength(BYTE inLength)
{
	m_pWrapper->SetObjectKeyLength( inLength );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetObjectKeyLength(BYTE *outLength)
{
	m_pWrapper->GetObjectKeyLength( outLength );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetIndexDescLength(BYTE inLength)
{
	m_pWrapper->SetIndexDescLength( inLength );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetIndexDescLength(BYTE *outLength)	
{
	m_pWrapper->GetIndexDescLength( outLength );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetEncrypt(BYTE bEncrypt)
{
	m_pWrapper->SetEncrypt( bEncrypt );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetEncrypt(BYTE * bEncrypt)
{
	m_pWrapper->GetEncrypt( bEncrypt );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen )
{
	m_pWrapper->SetObjectKey( inObjectName, inObjectNameLen, inObjectKey, inObjectKeyLen );

	// no run
	long lState;
	m_StatusReport.GetState( &lState );
	if( lState == 0 )
		m_pWrapper->UpdateParaSet();

	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen)
{
	m_pWrapper->GetObjectKey( inObjectName, inObjectNameLen, outObjectKey, outObjectKeyLen );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetMaxSizeOfSample(long inSize)
{
	m_pWrapper->SetMaxSizeOfSample( inSize );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetMaxSizeOfSample(long *outSize)
{
	m_pWrapper->GetMaxSizeOfSample( outSize );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::SetMinSizeOfSample(long inSize)
{
	m_pWrapper->SetMinSizeOfSample( inSize );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetMinSizeOfSample(long *outSize)
{
	m_pWrapper->GetMinSizeOfSample( outSize );
	return NOERROR;
}

	// interface from IStatusReport
STDMETHODIMP CDataWrapperFilter::GetState( long * outState )
{
	m_StatusReport.GetState( outState );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetLastError( long * outError )
{
	m_StatusReport.GetLastError( outError );
	return NOERROR;
}

STDMETHODIMP CDataWrapperFilter::GetErrorMsg( char * outMsg, BYTE * outLength )
{
	m_StatusReport.GetErrorMsg( outMsg, outLength );
	return NOERROR;
}
int CDataWrapperFilter::RealInitPinSortBuffer()
{
	if (m_arrangeBufNumber==0)	
		return 0;

	for (int i=0;i<m_arrangeBufNumber;i++)
	{
		m_ArrangeBuf[i]->ReNewOpenFile();				
	}
	return 0;
}
int CDataWrapperFilter::RealSortAndDelive()
{
	if (m_arrangeBufNumber==0)	
		return 0;
	char chLogMsg[MAX_PATH]; 

	//Find min packet len about PID file.
	int nMinPacklen=m_ArrangeBuf[0]->m_nTotalPacketNumber;
	for (int i=0;i<m_arrangeBufNumber;i++)
	{
		if (m_IsStop)
		{
			wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::RealSortAndDelive findout the least:respond stop_command",m_nBasePID);				
			LogMyEvent(1,1,chLogMsg);
			return 0;
		}
		if (m_ArrangeBuf[i]->m_hFile ==INVALID_HANDLE_VALUE)
		{
			sprintf(chLogMsg,"PID=%d:RealSortAndDelive PID=%d:filehandle is error",m_nBasePID,m_ArrangeBuf[i]->m_nPID);
			LogMyEvent(1,1,chLogMsg);
			return 1;
		}

		if (nMinPacklen > m_ArrangeBuf[i]->m_nTotalPacketNumber)
			nMinPacklen = m_ArrangeBuf[i]->m_nTotalPacketNumber;	

		sprintf(chLogMsg,"PID=%d:RealSortAndDelive PID=%d:TotalPacketNumber=%d",m_nBasePID,m_ArrangeBuf[i]->m_nPID,m_ArrangeBuf[i]->m_nTotalPacketNumber);
		LogMyEvent(1,1,chLogMsg);

		CloseHandle(m_ArrangeBuf[i]->m_hFile);
		m_ArrangeBuf[i]->m_hFile = CreateFile(m_ArrangeBuf[i]->m_strFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		if (m_ArrangeBuf[i]->m_hFile == INVALID_HANDLE_VALUE) 
		{			
			DWORD dwErr = ::GetLastError();
			wsprintf(chLogMsg,"PID=%d:CreateFile error strfilename=(%s) errorcode=%d",m_ArrangeBuf[i]->m_nPID,m_ArrangeBuf[i]->m_strFilename,dwErr);		LogMyEvent(1,0,chLogMsg);	
			m_ArrangeBuf[i]->m_hFile = INVALID_HANDLE_VALUE;
			return 1;
		}
	}
	//get packet rate :for example 1.2X ,2.3X
	
	int nTotalPackNumber=0;

	for (int i=0;i<m_arrangeBufNumber;i++)
	{
		if (m_IsStop)
		{
			wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::RealSortAndDelive get packet rate:respond stop_command",m_nBasePID);				
			LogMyEvent(1,1,chLogMsg);
			return 0;
		}

		m_ArrangeBuf[i]->m_nFirstInsetPad=1;
// ------------------------------------------------------ Modified by zhenan_ji at 2006年6月29日 20:11:41
		if (nMinPacklen <=0 || m_ArrangeBuf[i]->m_nTotalPacketNumber <=0)
			continue;
		if (m_ArrangeBuf[i]->m_nTotalPacketNumber > nMinPacklen)
		{
			m_ArrangeBuf[i]->m_nFirstInsetPad=m_ArrangeBuf[i]->m_nTotalPacketNumber / nMinPacklen;
			if (m_ArrangeBuf[i]->m_nTotalPacketNumber % nMinPacklen!=0)
				m_ArrangeBuf[i]->m_nFirstInsetPad++;							 
		}
		nTotalPackNumber+=m_ArrangeBuf[i]->m_nFirstInsetPad;
		sprintf(chLogMsg,"PID=%d:StreamPID=%d:TotalPacketNumber(%d),FirstInset(%d)",m_nBasePID,m_ArrangeBuf[i]->m_nPID,m_ArrangeBuf[i]->m_nTotalPacketNumber,m_ArrangeBuf[i]->m_nFirstInsetPad);
		LogMyEvent(1,1,chLogMsg);
	}
	//this content comes from tmpSubChannel in a function named "sendIndextable"  
	DWS_SubChannelList tmpSubChannel;
	tmpSubChannel.wReserve = 1;
	tmpSubChannel.byControlFlag = 1;
	tmpSubChannel.byTableID = 0x80;
	tmpSubChannel.wTableIDExtension = 0x0080;
	tmpSubChannel.sObjectKey[1] = (char )0x80;
	tmpSubChannel.sObjectKey[3] = (char )0x80;

	int nCurBufSize=0;
	BOOL AllFileSendOK=FALSE;
	int tmpPacketNumber=0;
	while (AllFileSendOK==FALSE)
	{
		if (m_IsStop)
		{
			wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::RealSortAndDelive(check all files were sent) respond stop_command",m_nBasePID);				
			LogMyEvent(1,1,chLogMsg);
			return 0;
		}
		int nRealInstertPadding=1;
		DWORD nRead=0;
		for (int i=0;i<m_arrangeBufNumber;i++)
		{
			if (m_IsStop)
			{
				wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::RealSortAndDelive(get insert packer number) respond stop_command",m_nBasePID);				
				LogMyEvent(1,1,chLogMsg);
				return 0;
			}
			//check file hand is or not invalid .
			if (m_ArrangeBuf[i]->m_hFile ==INVALID_HANDLE_VALUE)
			{
				sprintf(chLogMsg,"PID=%d:RealSortAndDelive PID=%d:filehandle is error",m_nBasePID,m_ArrangeBuf[i]->m_nPID);
				LogMyEvent(1,1,chLogMsg);
				return 1;
			}

			//get current real insert packet number

			if (m_ArrangeBuf[i]->m_bIsFirstPadding)
				nRealInstertPadding=m_ArrangeBuf[i]->m_nFirstInsetPad;
			else
				nRealInstertPadding=m_ArrangeBuf[i]->m_nFirstInsetPad-1;

			m_ArrangeBuf[i]->m_bIsFirstPadding=!m_ArrangeBuf[i]->m_bIsFirstPadding;


			if (nRealInstertPadding <=0)
				nRealInstertPadding =1;
			nRealInstertPadding=nRealInstertPadding * 188;

			//zhenan add 20060810
			int nLeftdData = nRealInstertPadding;
			int nWillRead = nRealInstertPadding;

			if (nWillRead > DELIVER_BUFFER_SIZE)
			{
				nWillRead = DELIVER_BUFFER_SIZE;
			}

			while (nWillRead)
			{
				if (m_IsStop)
				{
					wsprintf(chLogMsg,"PID=%d::CDataWrapperFilter::RealSortAndDelive(read data to Delivebuffer) respond stop_command",m_nBasePID);				
					LogMyEvent(1,1,chLogMsg);
					return 0;
				}

				if(!(ReadFile(m_ArrangeBuf[i]->m_hFile, m_pDeliverBuf + nCurBufSize, nWillRead, &nRead, NULL)))
				{
					sprintf(chLogMsg,"PID=%d:RealSortAndDelive PID=%d:ReadFile is error code=%d",m_nBasePID,m_ArrangeBuf[i]->m_nPID, ::GetLastError());
					LogMyEvent(1,1,chLogMsg);
					return 1;
				}
				if (nRead <=0)
				{
					m_ArrangeBuf[i]->m_bFileOver = TRUE;
					break;
				}
				else
				{
					nCurBufSize += nRead;
					if(nCurBufSize >= DELIVER_BUFFER_SIZE)
					{
						DeliverBuffer(m_pDeliverBuf,nCurBufSize,tmpSubChannel,TRUE);
						nCurBufSize=0;
					}
				}

				nLeftdData -= nRead;
				if (nLeftdData > DELIVER_BUFFER_SIZE)
					nWillRead = DELIVER_BUFFER_SIZE;
				else
					nWillRead = nLeftdData;
			}
		}
	
		//check all file send successed;
		AllFileSendOK=TRUE;		
		for (int i=0;i<m_arrangeBufNumber;i++)
		{
			if (m_ArrangeBuf[i]->m_bFileOver ==FALSE)
				AllFileSendOK=FALSE;
		}

		//deliver to zabroadcastfileter;
		if (nCurBufSize >= DELIVER_BUFFER_SIZE)
		{
			DeliverBuffer(m_pDeliverBuf,nCurBufSize,tmpSubChannel,TRUE);
			nCurBufSize=0;
		}
		else if (AllFileSendOK)			
		{
			DeliverBuffer(m_pDeliverBuf,nCurBufSize,tmpSubChannel,TRUE);
		}		
	}
	
	sprintf(chLogMsg,"PID=%d:CDataWrapperFilter::RealSortAndDelive Sort and deliver end!", m_nBasePID);
	LogMyEvent(1,1,chLogMsg);

	return 0;
}

STDMETHODIMP CDataWrapperFilter::Stop()
{
	// char strLog[MAX_PATH];	
	sprintf(chLogMsg,"PID=%d:CDataWrapperFilter stop!", m_nBasePID);
	LogMyEvent(1,1,chLogMsg);
	m_IsStop = TRUE;
	CBaseFilter::Stop();
	return S_OK;
}