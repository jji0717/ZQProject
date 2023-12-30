// TextWriter.h: interface for the TextWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTWRITER_H__FD2C9EC0_9B5B_4DBA_B261_2B152A849897__INCLUDED_)
#define AFX_TEXTWRITER_H__FD2C9EC0_9B5B_4DBA_B261_2B152A849897__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <NativeThread.h>
#include "BaseSender.h"
#include <deque>
#include <string>
#include <Locks.h>

class TextWriter : public ZQ::common::NativeThread,public BaseSender  
{
public:
	TextWriter();
	virtual ~TextWriter();

	virtual bool init(void);
	virtual int run(void);
	
	virtual bool GetParFromFile(const char* pFileName);
	virtual void AddMessage(const MSGSTRUCT& msgStruct);
	virtual void Close();

protected:
	bool WriteMessage(const MSGSTRUCT& msg);

private:
	ZQ::common::Mutex	_lock;	

	bool			_hExit;
	ZQ::common::Semaphore		_hMsgSem;
	std::deque<MSGSTRUCT>		_msgQue;
	
	FILE*                      _hFile;
	std::string					_strSaveName;

};

#endif // !defined(AFX_TEXTWRITER_H__FD2C9EC0_9B5B_4DBA_B261_2B152A849897__INCLUDED_)
