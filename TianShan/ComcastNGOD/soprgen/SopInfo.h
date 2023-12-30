// SopInfo.h: interface for the SopInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOPINFO_H__1253DB80_4711_409B_8337_18838AFEF420__INCLUDED_)
#define AFX_SOPINFO_H__1253DB80_4711_409B_8337_18838AFEF420__INCLUDED_
#pragma warning( disable : 4786)
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include <vector>

class SopInfo  
{
public:
	void parse(int argc, char** argv);
	SopInfo();
	virtual ~SopInfo();
	std::string getSopInfo(const char* pFile, const char* pSopName = "SOP_Name");
	private:
		std::string _strRead;
		std::string _strWrite;
		std::string _strSopName;
	typedef struct tagsop
	{
		std::vector<int>			servgID;
		std::vector<std::string>	streamID;
	}SOPSTRUCT;

};

#endif // !defined(AFX_SOPINFO_H__1253DB80_4711_409B_8337_18838AFEF420__INCLUDED_)
