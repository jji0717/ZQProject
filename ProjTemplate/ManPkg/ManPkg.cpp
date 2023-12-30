// #include <afx.h>
#include <windows.h>
//#include <winnt.h>
//#include <malloc.h>

// #include "AppShell.h"
#include "ManPkg.h"
#pragma comment (lib, "ManPkgU_d.lib")
#pragma warning (disable:4786)

#include <string>
#include <vector>

#define MANASSERT(_X) if (!_X) throw "oops";
#define NUM_COL 8
#define NUM_ROW 8
#define SIM_VAR 6

// must add the following key
// [HKEY_LOCAL_MACHINE\SOFTWARE\SeaChange\Management\CurrentVersion\Services\TestApp]
// "MgmtPortNumber"=dword:00000fa1

class ManageSession
{
public:
//	manCmplxCallback_t
// Cmd := <MsgType> <tab> <VariableName>
// Cmd:= "\x06\tPortTable" | "\x08\tPortTable"
// MsgType:= MAN_READ_VARS (6) | MAN_RESET_COUNTERS (8)
// 
//
// Response:= ReadVarResponse | ResetCountersResponse
// ReadVarResponse :=  <complex-descriptor> [<scalar-section>] [<vector-section>]
//      <complex-descriptor> := <NumScalars> <tab> <NumVectors> <newline>
//        <scalar-section> := <scalar-descriptor> [<scalar-descriptor>...]
//          <scalar-descriptor := <data-type> <tab> <VarName> <tab> <VarValue> <newline> 
//    <vector-section> := <vector-descriptor> [<vector-descriptor>...]
//      <vector-descriptor> : = <vector-heading> <vector-entry> [<vector-entry>...]
//        <vector-heading> := <data-type> <tab> <DisplayWidth> <tab> <NumVectorEntries> <tab> <VarName> <newline>
//        <vector-entry> := <VarValue><newline>
//
// ResetCountersResponse :=  <ReturnStatus> = MAN_SUCCESS
	typedef MANSTATUS (*manCmplxCallback_t) (WCHAR* pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);

	ManageSession(const WCHAR* serviceName, DWORD mgmtPort);
	~ManageSession();

	DWORD lastError() const { return _lastError; }
	const WCHAR* name() const { return _name.c_str(); }
	bool  isValid() const { return (_hManPkg != NULL); }

	bool attachVariable(const WCHAR* varName, const WORD type, DWORD referenceAddress, bool readOnly=true);
	
	bool manageInteger(const WCHAR* varName, DWORD& integer, bool readOnly=true);
	bool manageBool(const WCHAR* varName, BOOL& boolean, bool readOnly=true);
	bool manageString(const WCHAR* varName, WCHAR** string, bool readOnly=true);
	bool manageCount(const WCHAR* varName, DWORD& count, bool readOnly=true);
	bool manageComplex(const WCHAR* varName, manCmplxCallback_t callBack, bool readOnly=true);

protected:

	DWORD _mgmtPort;
	DWORD _lastError;
	HANDLE _hManPkg;
	std::wstring _name;

	typedef std::vector< std::wstring > varList_t;
	varList_t _varList;
};

ManageSession::ManageSession(const WCHAR* serviceName, DWORD mgmtPort)
:_lastError(0), _mgmtPort(mgmtPort), _hManPkg(NULL)
{
	if (serviceName == NULL || *serviceName == 0x0000)
		return;

	_varList.clear();

	_name = serviceName;

	MANSTATUS manRet = ManOpenSession(serviceName, _mgmtPort, &_hManPkg, &_lastError);

	if ( manRet != MAN_SUCCESS )
		_hManPkg = NULL;
}

ManageSession::~ManageSession()
{
	if (isValid())
	{
		MANSTATUS manRet;

		for (varList_t::iterator i = _varList.begin(); i< _varList.end(); i++)
		{
			try
			{
				manRet = ManUnmanageVar(_hManPkg, i->c_str(), &_lastError);
			}
			catch(...) {}
		}
		_varList.clear();

		manRet = ManCloseSession (_hManPkg, &_lastError);
		_hManPkg = NULL;
	}
}

bool ManageSession::attachVariable(const WCHAR* varName, const WORD type, DWORD referenceAddress, bool readOnly /*=true*/)
{
	if (!isValid() || varName == NULL || *varName==0x0000)
		return false;

	try
	{
		MANSTATUS manRet = ManManageVar(_hManPkg, varName, (WORD)type, referenceAddress, (readOnly?TRUE:FALSE), &_lastError);
		
		if (manRet == MAN_SUCCESS)
		{
			_varList.push_back(varName);
			return true;
		}
	}
	catch(...) {}

	return false;
}

bool ManageSession::manageInteger(const WCHAR* varName, DWORD& integer, bool readOnly /*=true*/)
{
	return attachVariable(varName, MAN_INT, (DWORD) &integer, readOnly);
}

bool ManageSession::manageBool(const WCHAR* varName, BOOL& boolean, bool readOnly /*=true*/)
{
	return attachVariable(varName, MAN_BOOL, (DWORD) &boolean, readOnly);
}

bool ManageSession::manageString(const WCHAR* varName, WCHAR** string, bool readOnly /*=true*/)
{
	return attachVariable(varName, MAN_STR, (DWORD) string, readOnly);
}

bool ManageSession::manageCount(const WCHAR* varName, DWORD& count, bool readOnly /*=true*/)
{
	return attachVariable(varName, MAN_COUNT, (DWORD) &count, readOnly);
}

bool ManageSession::manageComplex(const WCHAR* varName, manCmplxCallback_t callBack, bool readOnly /*=true*/)
{
	return attachVariable(varName, MAN_COMPLEX, (DWORD) callBack, readOnly);
}

WCHAR  varString[MAX_PATH]; //MAN_STR
DWORD  varInteger;			// MAN_INT
DWORD  varCount;			// MAN_COUNT
BOOL   varBOOL;				// MAN_BOOL

MANSTATUS ManTblCB(WCHAR* pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);
BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

int main()
{
	ManageSession msession(L"TestApp", 4001);

	// Manage all configuration variable we've read so far as well
	// as any global statistic numbers
	msession.manageString(L"varString", (WCHAR**) &varString);
	msession.manageInteger(L"varInteger", varInteger);
	msession.manageCount(L"varInteger", varCount);
	msession.manageBool(L"varBOOL", varBOOL);
	msession.manageComplex(L"varGrid", ManTblCB);
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
		return -1;
	}
	printf("\"Ctrl-C\" at any time to exit the program.\n");

	while (!bQuit)
	{
		::Sleep(500);
	}

	return 0;

}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
        break;
    }
    return TRUE;
}
MANSTATUS ManTblCB(WCHAR* pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	static WCHAR respBuffer[4096];
	WCHAR* wptr = respBuffer;
	*ppwszBuffer= respBuffer;
    *pdwLength = 0;

	static int c=0;
	swprintf(varString, L"Grid was visited %d times", c++);

	// Parse out command byte.
	//
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);
	// Dispatch based on requested operation
	switch (wCommand)
	{
		case MAN_READ_VARS:
		{
			int i;

			// Output buffer header to tell ManUtil how many simple variables (1)
			// are being provide and number of table columns
			wptr += swprintf(wptr, L"%d\t%d\n", SIM_VAR, NUM_COL);

			for (i=0; i< SIM_VAR; i++)
			{
				// Put in total count
				WCHAR varname[32];
				swprintf(varname, L"SimpleVariable%d", i);
				wptr += swprintf (wptr, L"%d\t%s\t%d\n", MAN_INT, varname, 0);
			}

			for (i=0; i< NUM_COL; i++)
			{
				WCHAR colname[32];
				swprintf(colname, L"Col%d", i);

			// Column Header: Client Id
			// <Type><tab><Width><tab><RowCount><tab><Name><newline>
			wptr += swprintf (wptr, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, NUM_ROW, colname);

			// fill in the cells of this column
			for ( int j =0; j< NUM_ROW; j++)
			{
				WCHAR cell[32];
				swprintf(cell, L"Cell %d-%d", j, i);
				wptr += swprintf (wptr, L"Cell %d-%d\n", j, i);
			}

			}

            *pdwLength = wcslen(*ppwszBuffer);
			break;
		}
		case MAN_FREE:
			// Free the allocated response buffer
			if (*ppwszBuffer != NULL)
			{
				*ppwszBuffer = NULL;
			}
			break;

		default:
			return MAN_BAD_PARAM;
	}

	
	return MAN_SUCCESS;
}

