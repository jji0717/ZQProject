// VV2Parser.h: interface for the VV2Parser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VV2PARSER_H__6A0DDD6D_6822_4FB7_8F5A_F27A6A82FA91__INCLUDED_)
#define AFX_VV2PARSER_H__6A0DDD6D_6822_4FB7_8F5A_F27A6A82FA91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class VV2Parser  
{
protected:
	struct SubFileInfo {
		char	ext[8];
		long	numerator;
		long	denominator;
		long	direction;				
	};

	typedef std::vector<SubFileInfo> SubFileInfoVec;

	SubFileInfoVec		_subFileInfoVec;

	unsigned long		_bitrate;
	unsigned long		_playTime;
#ifndef NAS
	static HANDLE		_vstrmClass;
#endif

public:
	VV2Parser();
	virtual ~VV2Parser();

#ifndef NAS
	static bool vsmInit(HANDLE vstrmClass);
#endif

	bool parse(const char* fileName, bool fromNtfs = false);

	unsigned long getBitrate() const
	{
		return _bitrate;		
	}

	unsigned long getPlayTime() const
	{
		return _playTime;
	}

	size_t getSubFileCount() const
	{
		return _subFileInfoVec.size();

	}

	bool getSubFileSpeed(size_t index, long& numerator, long& denominator, 
		long& direction)
	{
		if (index >= _subFileInfoVec.size())
			return false;

		SubFileInfo& info = _subFileInfoVec[index];

		numerator = info.numerator;
		denominator = info.denominator;
		direction = info.direction;
		return true;
	}

	size_t getSubFileExtension(size_t index, char* ext, size_t extLen)
	{
		if (index >= _subFileInfoVec.size())
			return false;

		SubFileInfo& info = _subFileInfoVec[index];

		const size_t extMaxLen = sizeof(info.ext) - 1;

		strncpy(ext, info.ext, extLen > extMaxLen ? extMaxLen : extLen);
		ext[extMaxLen] = 0;
		return true;
	}

protected:
	bool readSubFileRecord(const char* linebuf, SubFileInfo& info, int& index);
	
	bool parseFile(FILE* fp);
	bool parseFromNtfs(const char* fileName);
#ifndef NAS 
	bool parseFromVstrm(const char* fileName);
#endif
};

#endif // !defined(AFX_VV2PARSER_H__6A0DDD6D_6822_4FB7_8F5A_F27A6A82FA91__INCLUDED_)
