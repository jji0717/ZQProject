#ifndef __COMMAND_DEFINE_H__
#define  __COMMAND_DEFINE_H__
#include <stdio.h>
#include <utility>
#include <fcntl.h>
#include <time.h>
#include <string>
#include <errno.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "TianShanDefines.h"
#include <TianShanIceHelper.h>
#include <TianShanIce.h>
#include <list>
#include <map>
#include <iomanip>
#include "wtconfig.h"
#include "wtsocket.h"
#include "wtguuid.h"
#include "tlock.h"
#include "wtusocket.h"
#define  MAXLINEBUFFER 1024

typedef unsigned char       BYTE;
extern ZQ::common::Log  *PLOG;

class C_COMMANDS 
{
public:

public:	
	C_COMMANDS();
	~C_COMMANDS();
	friend class TUSOCKET;
	friend class SocketClient;
	
	bool cSutUp(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);
	bool cSutUp(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);

	bool cPlay(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);
	bool cPlay(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);

	bool cstatus(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);
	bool cstatus(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);

	bool cpause(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);
	bool cpause(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);

	bool cClose(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);
	bool cClose(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle);

	char* getg_pmv_resourse();
	inline bool get_setupflag()
	{
		return bflag;
	}
	PROPERTIESMAP::iterator proitor;
private:
	bool  bflag; //setUp boolen flag
	bool  b_initsocket1_flag;
	bool  b_initsocket2_flag;

	struct timeval _cmtime;
	unsigned int _StreamHandle ;
	std::list<std::string> CommnadLines;	
	std::list<SCOMMANDS>::iterator itor;
	
	char arrg_pmv_resourse[100];//assetUID++
	CritSect _gcrits;

	static int gindexCount;
};
#endif //__COMMAND_DEFINE_H__ 

