// lscMsgSniffer.h: interface for the lscMsgSniffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LSCMSGSNIFFER_H__C272B3E1_928B_4B34_B814_559D0214702F__INCLUDED_)
#define AFX_LSCMSGSNIFFER_H__C272B3E1_928B_4B34_B814_559D0214702F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lsc_parser.h"
#include <string>

class lscMsgSniffer  
{
public:
	static void showMsgDetail(lsc::lscMessage* msg,const std::string& strHint);
};

#endif // !defined(AFX_LSCMSGSNIFFER_H__C272B3E1_928B_4B34_B814_559D0214702F__INCLUDED_)
