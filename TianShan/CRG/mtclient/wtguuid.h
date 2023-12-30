#ifndef __WTGUUID_DEFINE_H__
#define  __WTGUUID_DEFINE_H__
#include <WinSock2.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <errno.h> 
#include <string.h>
#include "wtlog.h"
extern ZQ::common::Log  *PLOG;
class CWTGUUID 
{
public:

public:	
	CWTGUUID();
	CWTGUUID(bool bsuffix,bool bffsession);
	~CWTGUUID();
	static bool HexToString(std::string& outputString , const unsigned char *buf, unsigned short int len);
	bool fabricateTID(UINT32& fTransactionID );//transactionId
	bool decimalSerialN();
	bool decimalSerialN2();
	const char* decimalSerialNX();
	std::string _strdecimalserialn;
	char  _decimalserialn[11];
	void set_suffix(bool bflag);
	bool get_suffix();
	void set_ffsession(bool bflag);
	bool get_ffsession();
protected:
	
private:
	bool  _bflag_ffsession; //create a session ,and first byte is set ff flag.
	bool  _bflag_suffix; //บ๓ืบ
//	char  _decimalserialn[11];
	
};
#endif //__WTGUUID_DEFINE_H__ 

