

#include "NTFSTarget.h"
#include "Log.h"
#include "errorcode.h"

#define NTFSIO			"NTFSIO"

using namespace ZQ::common;

#define MOLOG (*_pLog)

#define NtfsIO			"NtfsIO"


namespace ZQTianShan {
	namespace ContentProvision {


NTFSTarget::NTFSTarget()
{
	_nOutputCount = 0;
	_nInputCount = 1;

	for(int i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	_bDriverModule = false;
	_offset = 0;
}

NTFSTarget::~NTFSTarget()
{
	Close();
}

bool NTFSTarget::Init()
{
	_fstrm.open(_strFilename.c_str(), std::ios_base::binary);
	if (_fstrm.fail())
	{
		char tmp[256];
		sprintf(tmp, "Failed to open file %s for write", _strFilename.c_str());
		SetLastError(tmp, ERRCODE_NTFS_CREATEFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(NtfsIO, "Failed to open file %s for write"), _strFilename.c_str());
		return false;
	}

	_llProcBytes = 0;

	MOLOG(Log::L_INFO, CLOGFMT(NtfsIO, "Open file %s for write successful"), _strFilename.c_str());
	return true;
}

void NTFSTarget::Stop()
{
	
	
}

void NTFSTarget::Close()
{
	if (_fstrm.is_open())
	{
		_fstrm.close();	
		MOLOG(Log::L_INFO, CLOGFMT(NtfsIO, "File %s closed"), _strFilename.c_str());
	}	
}

void NTFSTarget::endOfStream()
{
	_fstrm.close();
	GetGraph()->Finish();
}

const char* NTFSTarget::GetName()
{
	return TARGET_TYPE_NTFS;
}

bool NTFSTarget::Receive(MediaSample* pSample, int nInputIndex)
{
	if (nInputIndex)
	{
		return false; 
	}

	unsigned int uLow, uHigh;
	__int64 nOffset=0;
	uLow = pSample->getOffset(&uHigh);
	int nDataLen = pSample->getDataLength();

	if (_offset != uLow)
	{
		_fstrm.seekp(uLow , std::ios_base::beg);
//		MOLOG(Log::L_DEBUG, CLOGFMT(NtfsIO, "[%s] seek to %d"), _strFilename.c_str(), uLow);
	}

	_fstrm.write((char*)pSample->getPointer(), nDataLen);
	_offset = uLow + nDataLen;
	if (_fstrm.fail())
	{
		char tmp[256];
		sprintf(tmp, "Failed to write file %s", _strFilename.c_str());
		SetLastError(tmp, ERRCODE_NTFS_WRITEFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(NtfsIO, "Failed to write file %s"), _strFilename.c_str());
		return false;
	}	

	GetGraph()->freeMediaSample(pSample);
	
	IncProcvBytes(nDataLen);
	return true;
}

}}