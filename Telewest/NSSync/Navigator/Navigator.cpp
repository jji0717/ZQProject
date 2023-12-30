// Navigator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Navigator.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "nsBuilder.h"
#include "nsTimer.h"
#include "Log.h"
#include "ScLog.h"

// The one and only application object

CWinApp theApp;

using namespace std;


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	pGlog = new ScLog("C:\\NS\\ns.log", Log::L_DEBUG, 9*1024*1024);

	// initialize MFC and print and error on failure
	//if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	//{
	//	// TODO: change error code to suit your needs
	//	_tprintf(_T("Fatal Error: MFC initialization failed\n"));
	//	nRetCode = 1;
	//}
	//else
	//{
		// TODO: code your application's behavior here.
		nsBuilder* pBuilder= new nsBuilder;
		nsTimer* pTimer= new nsTimer(pBuilder);

		pBuilder->init();
		pTimer->start();
	//}

	delete pGlog;
	return nRetCode;
}
