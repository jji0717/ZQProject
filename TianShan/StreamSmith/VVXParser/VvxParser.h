// VvxParser.h: interface for the VvxParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VVXPARSER_H__2BBD741B_2CD3_428E_915B_60C9E2CCE54A__INCLUDED_)
#define AFX_VVXPARSER_H__2BBD741B_2CD3_428E_915B_60C9E2CCE54A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_SUBFILE		20
#define MAX_TS_INFO		10
#define MAX_ES_INFO		10

#ifndef NAS 
#include "VstrmProc.h"
#endif

#include "string.h"

class VvxParser  
{
public:
#ifndef NAS 
	friend bool vsm_ParseVvxFile(const char* szFileName, VvxParser& parser);
#endif

	VvxParser();
	virtual ~VvxParser();

	enum SubfileType{
		TVVX_SUBFILE_TYPE_PES =0,
		TVVX_SUBFILE_TYPE_SPTS
	};
	
	enum FrameRate
	{
		VMPEG_FRAME_RATE_23_976 = 0x01,
		VMPEG_FRAME_RATE_24,
		VMPEG_FRAME_RATE_25,
		VMPEG_FRAME_RATE_29_97,
		VMPEG_FRAME_RATE_30,
		VMPEG_FRAME_RATE_50,
		VMPEG_FRAME_RATE_59_94,
		VMPEG_FRAME_RATE_60
	};

	FrameRate getFrameRateCode(){ return (FrameRate)_dwFrameRateCode;}

	const char* getFrameRateString(FrameRate frameRateCode)
	{		
		const char * szReturn;
		switch (frameRateCode)
		{
			case VMPEG_FRAME_RATE_23_976:	szReturn = "23.976";									break;
			case VMPEG_FRAME_RATE_24:		szReturn = "24";										break;
			case VMPEG_FRAME_RATE_25:		szReturn = "25";										break;
			case VMPEG_FRAME_RATE_29_97:	szReturn = "29.97";										break;
			case VMPEG_FRAME_RATE_30:		szReturn = "30";										break;
			case VMPEG_FRAME_RATE_50:		szReturn = "50";										break;
			case VMPEG_FRAME_RATE_59_94:	szReturn = "59.94";										break;
			case VMPEG_FRAME_RATE_60:		szReturn = "60";										break;
			default:						szReturn = "Unknown";	break;
		}

		return szReturn;
	}

	enum StreamType{
		MPEG_STREAM_TYPE_UNKNOWN = 0,
		MPEG_STREAM_TYPE_MPEG2_TRANSPORT,
		MPEG_STREAM_TYPE_ES_STREAM,
	};

// 	StreamType getStreamTypeCode(){ return (StreamType)_dwStreamType;	}

	int getVideoHorizontalSize(){ return _dwHorizontalSize;	}
	int getVideoVerticalSize(){ return _dwVerticalSize;	}

	int getSubFileCount() {return _nSubFileCount;	}

//	bool getSubFileSpeed(ULONG nIndex, LONG& numerator, ULONG& denominator)
//	{
//		if (nIndex>=_nSubFileCount)
//			return false;
//
//		numerator = _subFileSpeed_numerator[nIndex];
//		denominator = _subFileSpeed_denominator[nIndex];
//		return true;
//	}

//	bool getSubFileSpeed(ULONG nIndex, ULONG& playTimeinMs)
//	{
//		if (nIndex>=_nSubFileCount)
//			return false;
//
//		playTimeinMs = _subFilePlayTimeInMilliSeconds[nIndex];
//		return true;
//	}

	bool getSubFileExtension(unsigned nIndex, char* szExtension, int size)
	{
		if (size<=0)
			return false;

		if (nIndex>=_nSubFileCount)
			return false;

		strncpy(szExtension, _subFileExtension[nIndex], size-1);
		szExtension[size-1] = '\0';		
		return true;
	}

//	bool getSubFileType(ULONG nIndex, SubfileType& stype)
//	{
//		if (nIndex>=_nSubFileCount)
//			return false;
//
//		stype = (SubfileType)(_subFileType[nIndex]);		
//		return true;
//	}

#if defined(ZQ_OS_MSWIN)
	bool getFileTime(SYSTEMTIME* pST);
#elif defined(ZQ_OS_LINUX)
	bool getFileTime(struct tm* pST);
#endif

	bool parse(const char* szFilename, bool bNTFS = false);

	unsigned GetBitRate(){return _dwBitrate; }
	
	unsigned GetPlayTime(){return _subFilePlayTimeInMilliSeconds[0];}

protected:

	static bool ntfs_ParseVvxFile(const char* szFileName, VvxParser& parser);
private:

#if defined(ZQ_OS_MSWIN)
	FILETIME _fileTime;
#elif defined (ZQ_OS_LINUX)
#endif
	unsigned _dwBitrate;
	unsigned _dwVideoBitrate;
	unsigned _dwHorizontalSize;
	unsigned _dwVerticalSize;
	unsigned _dwStreamType;
	long	 _subFileSpeed_numerator[MAX_SUBFILE];	
	unsigned _subFileSpeed_denominator[MAX_SUBFILE];	
	unsigned _subFilePlayTimeInMilliSeconds[MAX_SUBFILE];
	char	 _subFileExtension[MAX_SUBFILE][8];
	unsigned _subFileType[MAX_SUBFILE];
	unsigned _nSubFileCount;
	unsigned _dwFrameRateCode;

	unsigned _tsInformationCount;	
	
	unsigned _tsProgramNumber[MAX_TS_INFO];
	unsigned _tsPMTPid[MAX_TS_INFO];
	unsigned _tsPCRPid[MAX_TS_INFO];
	unsigned _tsVideoPid[MAX_TS_INFO];

	unsigned _esInformationCount;
	uint8	_esStreamType[MAX_ES_INFO];
	uint8	_esStreamFlags[MAX_ES_INFO];
    unsigned short _esPID[MAX_ES_INFO];
	


};

#endif // !defined(AFX_VVXPARSER_H__2BBD741B_2CD3_428E_915B_60C9E2CCE54A__INCLUDED_)
