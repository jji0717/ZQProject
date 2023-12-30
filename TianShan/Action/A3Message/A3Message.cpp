// A3Message.cpp : Defines the entry point for the DLL application.
//

#include <Log.h>
#include <FileLog.h>
#include "RuleEngineModule.h"
#include "A3Call.h"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	return TRUE;
}
#endif

using namespace ZQ::common;
//A3Call* action = NULL;
A3TransferContent* tcAction = NULL;
A3ExposeContent* ecAction = NULL;
A3GetContentInfo* gcAction = NULL;
A3DeleteContent* dcAction = NULL;
A3GetVolumeInfo* gvAction = NULL;
A3GetTransferStatus* gtAction = NULL;
A3CancelTransfer* ctAction = NULL;
A3GetContentChecksum* gccAction = NULL;

extern "C"
{

__EXPORT bool Initialize(RuleEngine* engine, ZQ::common::Log* log)
{
//	action = new A3Call(*engine, *log);
	tcAction = new A3TransferContent(*engine, *log);
	ecAction = new A3ExposeContent(*engine, *log);
	gcAction = new A3GetContentInfo(*engine, *log);
	dcAction = new A3DeleteContent(*engine, *log);
	gvAction = new A3GetVolumeInfo(*engine, *log);
	gtAction = new A3GetTransferStatus(*engine, *log);
	ctAction = new A3CancelTransfer(*engine, *log);
	gccAction = new A3GetContentChecksum(*engine, *log);
	return true;
}

__EXPORT bool UnInitialize(void)
{	
	try
	{
		if(tcAction)
		{
			delete tcAction;
			tcAction = NULL;
		}
		if(ecAction)
		{
			delete ecAction;
			ecAction = NULL;
		}
		if(gcAction)
		{
			delete gcAction;
			gcAction = NULL;
		}
		if(dcAction)
		{
			delete dcAction;
			dcAction = NULL;
		}
		if(gvAction)
		{
			delete gvAction;
			gvAction = NULL;
		}
		if(gtAction)
		{
			delete gtAction;
			gtAction = NULL;
		}
		if(ctAction)
		{
			delete ctAction;
			ctAction = NULL;
		}
		if(gccAction)
		{
			delete gccAction;
			gccAction = NULL;
		}
	}
	catch (...)
	{
	}
	return true;
}
};