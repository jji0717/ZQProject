// FNString.h: interface for the CFNString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FNSTRING_H__40BF3A91_83AA_43EB_83B1_80EDA164E01A__INCLUDED_)
#define AFX_FNSTRING_H__40BF3A91_83AA_43EB_83B1_80EDA164E01A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
using namespace std;
class CFNString  
{
public:
	CFNString(string str)
	{
		m_Data.erase();
		m_Data=str;
	}

	virtual ~CFNString(){m_Data.erase();}

	string GetFileName()
	{
		string strFileName="";
		int nFind=m_Data.find_last_of('\\');
		if(nFind>0)
		{
			strFileName=m_Data.substr(nFind+1);
		}
		return strFileName;
	}

private:
	string m_Data;
};

#endif // !defined(AFX_FNSTRING_H__40BF3A91_83AA_43EB_83B1_80EDA164E01A__INCLUDED_)
