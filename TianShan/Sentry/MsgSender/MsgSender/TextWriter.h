// TextWriter.h: interface for the TextWriter class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __MSGSENDER_TEXTWRITER_H__
#define __MSGSENDER_TEXTWRITER_H__

#include "BaseSender.h"
#include <NativeThreadPool.h>
#include <deque>
#include <string>
#include <Locks.h>

class TextWriter : public BaseSender  
{
	friend class WriterCmd;
public:
	TextWriter(int poolSize = MSGSENDER_POOLSIZE);
	virtual ~TextWriter();

	virtual bool init(void);
	virtual bool GetParFromFile(const char* pFileName);
	virtual void AddMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx);
	virtual void Close();

protected:
	bool WriteMessage(const MSGSTRUCT& msg);

private:
	ZQ::common::NativeThreadPool	_thPool;
	ZQ::common::Mutex			_lock;	
	
	FILE*						_hFile;
	std::string					_strSaveName;
	bool						_bQuit;
};

#endif //__MSGSENDER_TEXTWRITER_H__
