#ifndef XMLPROC_H
#define XMLPROC_H

#include "xmlpreference.h"

using namespace ZQ::common;

class XMLProc
{
public:
	XMLProc():init(NULL){};

	virtual ~XMLProc() {};

	// Com initialize
	void CoInit()
	{
		if (!init)
		{
			init = new ZQ::common::ComInitializer;
		}
	}


	// Com uninitialize
	void CoUnInit()
	{
		if (init)
		{
			delete init;
			init = NULL;
		}
	}


	bool GetParameters(char*		xml,
					   int&			id,
					   int&			type,
					   wchar_t*		path);

	
	void GetBackupFile(char*		xmlfile,
					   int*         id,
					   int*         type,
					   wchar_t**    path,
					   int*         count);

	void DeleteNode   (char*        xmlfile,
					   int          id);

	void AddNode      (char*        xmlfile,
					   int          id,
					   int          type,
					   wchar_t*     path);

protected:
	ZQ::common::ComInitializer* init;
};

#endif