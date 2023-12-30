// handler.h: interface for the handler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HANDLER_H__553DEFAC_60E6_4C88_9E1B_EED1C7C93A25__INCLUDED_)
#define AFX_HANDLER_H__553DEFAC_60E6_4C88_9E1B_EED1C7C93A25__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

//////////////////////////////////////////////////////////////////////////

int initHandlers(Commander& cmder);

//////////////////////////////////////////////////////////////////////////

class PingCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class DestroyStreamCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class ListStreamsCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class CreateStreamCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class PlayCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class PauseCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class ResumeCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

class DumpStreamCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);

protected:
	int dumpStream(Commander& cmder, const std::string& strmName);
	std::string getStateName(int state);
	
protected:
	int		_rateCount;
};

//////////////////////////////////////////////////////////////////////////

class AddMuxItemCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class RemoveMuxItemCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class NotifyCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class DumpMuxItemCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);

protected:
	const char* getEncryptName(int encryptMode);
	
	int dumpMuxItem(Commander& cmder, const std::string& strmName, 
		const std::string& muxItemName);

protected:

	int		_rateCount;
};

//////////////////////////////////////////////////////////////////////////

class ListMuxItemCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

//////////////////////////////////////////////////////////////////////////

class TotalMuxItem: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

#endif // !defined(AFX_HANDLER_H__553DEFAC_60E6_4C88_9E1B_EED1C7C93A25__INCLUDED_)

