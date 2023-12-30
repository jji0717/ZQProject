// FileLogCol.h: interface for the FileLogCol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILELOGCOL_H__FE1E858D_F187_493D_BF8B_3FAEE399985C__INCLUDED_)
#define AFX_FILELOGCOL_H__FE1E858D_F187_493D_BF8B_3FAEE399985C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KeyDefine.h"
#include "BaseInfoCol.h"

class FileLogCol : public BaseInfoCol  
{
public:
	FileLogCol();
	virtual ~FileLogCol();
	static const char* getTypeInfo()
	{
		return KD_KV_SOURCETYPE_FILELOG;
	}

	virtual bool init(InitInfo& initInfo, const char* szSessionName);

	virtual void close();

protected:

	int run();
private:
	
	std::string _filename;
	DWORD  _pos;
	bool   _bQuit;
	
	int nextLogLine(char* buf, char** pline);
};

#endif // !defined(AFX_FILELOGCOL_H__FE1E858D_F187_493D_BF8B_3FAEE399985C__INCLUDED_)
