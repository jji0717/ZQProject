// stdafx.cpp : source file that includes just the standard includes
//	SRM.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#include <atlbase.h>
#include <Lm.h>
//#include <Iads.h>
//#include <Adshlp.h>

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

//FTP server (Sony e-VTR) format is 
//Address:<address>;
//User:[user name];
//Password:[password];
//Port:[port];
//Path:[path];
BOOL ParseFtpServerLocation(char* pServer,
					CString& strAddress, 
					CString& strUser, 
					CString& strPassword, 
					long* lPort, 
					CString& strPath)
{
	char* pBegin;
	char* pEnd;

	////////////////////////////////////////////////////////////
	// find Address.
	pBegin = pServer;
	pEnd = pServer;
	while( *pEnd!='\0'&&*pEnd!=':')
	{
		pEnd++;
	}
	if( *pEnd!=':' )
		return FALSE;
	*pEnd = '\0';
	if( strcmp(pBegin, "Address")!=0 )
		return FALSE;
	*pEnd = ':';

	// find address, such as "192.168.80.45"
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if( *pEnd!=';' )
		return FALSE;
	*pEnd = '\0';
	strAddress = pBegin;
	*pEnd = ';';

	/////////////////////////////////////////////////////////////////
	// find User
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=':' )
	{
		pEnd++;
	}
	if( *pEnd!=':' )
		return FALSE;
	*pEnd = '\0';
	if( strcmp(pBegin, "User")!=0 )
		return FALSE;
	*pEnd=':';

	// find user name
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if(*pEnd!=';')
		return FALSE;
	*pEnd = '\0';
	strUser = pBegin;
	*pEnd = ';';

	//////////////////////////////////////////////////////////////////////
	// find Password
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=':' )
	{
		pEnd++;
	}
	if( *pEnd!=':' )
		return FALSE;
	*pEnd = '\0';
	if( strcmp(pBegin, "Password")!=0 )
		return FALSE;
	*pEnd=':';

	// find password
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if(*pEnd!=';')
		return FALSE;
	*pEnd = '\0';
	strPassword = pBegin;
	*pEnd = ';';	

	//////////////////////////////////////////////////////////////////////
	// find Port
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=':' )
	{
		pEnd++;
	}
	if( *pEnd!=':' )
		return FALSE;
	*pEnd = '\0';
	if( strcmp(pBegin, "Port")!=0 )
		return FALSE;
	*pEnd=':';

	// find port
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if(*pEnd!=';')
		return FALSE;
	*pEnd = '\0';
	*lPort = atoi(pBegin);
	*pEnd = ';';

	//////////////////////////////////////////////////////////////////////
	// find Path
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=':' )
	{
		pEnd++;
	}
	if( *pEnd!=':' )
		return FALSE;
	*pEnd = '\0';
	if( strcmp(pBegin, "Path")!=0 )
		return FALSE;
	*pEnd=':';

	// find path
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if(*pEnd!=';')
		return FALSE;
	*pEnd = '\0';
	strPath = pBegin;
	*pEnd = ';';

	return TRUE;

}

//Format is Address:<address>;Port:<port>;Path:[path];
BOOL ParseBMSServerLocation(char* pServer,
					CString& strAddress, 
					long* lPort, 
					CString& strPath)
{
	char* pBegin;
	char* pEnd;

	////////////////////////////////////////////////////////////
	Clog(0,"[ParseBMSServerLocation]ParseBMSServerLocation() start.");
	Clog(0,"[ParseBMSServerLocation]%s",pServer);
	// find Address.
	pBegin = pServer;
	pEnd = pServer;
	while( *pEnd!='\0'&&*pEnd!=':')
	{
		pEnd++;
	}
	if( *pEnd!=':' )
	{
		Clog(0,"[ParseBMSServerLocation]*pEnd!=':' find Address.");
		return FALSE;
	}
	*pEnd = '\0';
	if( strcmp(pBegin, "Address")!=0 )//modified by Rose : "Addess" should be "Address"
	{
		Clog(0,"[ParseBMSServerLocation]strcmp(pBegin, Addess)!=0 ");
		return FALSE;
	}
	*pEnd = ':';

	// find address, such as "192.168.12.45"
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if( *pEnd!=';' )
	{
		Clog(0,"[ParseBMSServerLocation]*pEnd!=';' find address, such as 192.168.12.45");
		return FALSE;
	}
	*pEnd = '\0';
	strAddress = pBegin;
	*pEnd = ';';


	//////////////////////////////////////////////////////////////////////
	// find Port
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=':' )
	{
		pEnd++;
	}
	if( *pEnd!=':' )
	{
		Clog(0,"[ParseBMSServerLocation]*pEnd!=';'  find Port");
		return FALSE;
	}
	*pEnd = '\0'; 
	Clog(0,"[ParseBMSServerLocation]pBegin=%s",pBegin);
	if( strcmp(pBegin, "Port")!=0 )
	{
		Clog(0,"[ParseBMSServerLocation]strcmp(pBegin, Port)!=0");
		return FALSE;
	}
	*pEnd=':';

	// find port
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if(*pEnd!=';')
	{
		Clog(0,"[ParseBMSServerLocation]*pEnd!=';'  find Port2");
		return FALSE;
	}
	*pEnd = '\0';
	*lPort = atoi(pBegin);
	*pEnd = ';';

	//////////////////////////////////////////////////////////////////////
	// find Path
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=':' )
	{
		pEnd++;
	}
	if( *pEnd!=':' )
	{
		Clog(0,"[ParseBMSServerLocation]*pEnd!=';'  find Path");
		return FALSE;
	}
	*pEnd = '\0'; 
	
	if( strcmp(pBegin, "Path")!=0 )
	{
		Clog(0,"[ParseBMSServerLocation]strcmp(pBegin, Path)!=0");
		return FALSE;
	}
	*pEnd=':';

	// find path
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if(*pEnd!=';')
	{
		Clog(0,"[ParseBMSServerLocation]*pEnd!=';' find path2");
		return FALSE;
	}
	*pEnd = '\0';
	strPath = pBegin;
	*pEnd = ';';

	return TRUE;

}

//FS Format is Address:<address>;
BOOL ParseFSLocation(char* pServer,
					 CString& strAddress)
{
	char* pBegin;
	char* pEnd;

	////////////////////////////////////////////////////////////
	// find Address.
	pBegin = pServer;
	pEnd = pServer;
	while( *pEnd!='\0'&&*pEnd!=':')
	{
		pEnd++;
	}
	if( *pEnd!=':' )
	{
		Clog(0,"[ParseFSLocation]*pEnd!=':' find Address.");
		return FALSE;
	}
	*pEnd = '\0';
	if( strcmp(pBegin, "Address")!=0 )//modified by Rose : "Addess" should be "Address"
	{
		Clog(0,"[ParseFSLocation]strcmp(pBegin, Address)!=0");
		return FALSE;
	}
	*pEnd = ':';

	// find address, such as "192.168.12.45"
	pEnd++;
	pBegin = pEnd;
	while( *pEnd!='\0'&&*pEnd!=';' )
	{
		pEnd++;
	}
	if( *pEnd!=';' )
	{
		Clog(0,"[ParseFSLocation]*pEnd!=';' find address, such as 192.168.12.45 ");
		return FALSE;
	}
	*pEnd = '\0';
	strAddress = pBegin;
	*pEnd = ';';

	return TRUE;
}

// get SRM path.
BOOL GetControllerPath(CString& strController)
{
	//char strDrive[4];
	char strDrive[MAX_PATH];//modified by Rose
	char strDir[MAX_PATH];
	char strFile[MAX_PATH];
	//char strExt[4];
	char strExt[MAX_PATH];//modified by Rose
	char strModuleFile[MAX_PATH];
	DWORD dwSize = GetModuleFileName(NULL, strModuleFile, MAX_PATH-1);
	if( dwSize==0 )
	{
		DWORD dwError = GetLastError();
		return FALSE;
	}
	strModuleFile[dwSize]='\0';

	_splitpath( strModuleFile, strDrive, strDir, strFile, strExt);
	strController.Format("%s%s",strDrive,strDir);
	
	return TRUE;
}

BOOL MOGRegistry()
{
	CRegKey keySoftware;
	CRegKey keyMOGSolutions;
	CRegKey key16;
	CRegKey key1;
	CRegKey keyPayLicense;
	try
	{
		// Do with HKEY_CURRENT_USER\Software\MOG Solutions.
		//open HKEY_CURRENT_USER\Software.
		if( keySoftware.Open(HKEY_CURRENT_USER, "Software")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to open HKEY_CURRENT_USER\\Software");
			return FALSE;
		}
		//create HKEY_CURRENT_USER\Software\MOG Solutions.
		if( keySoftware.Create( keySoftware, "MOG Solutions", REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WRITE|KEY_READ)!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to create HKEY_CURRENT_USER\\Software\\MOG Solutions");
			keySoftware.Close();
			return FALSE;
		}
		keySoftware.Close();

		//Do with HKEY_CURRENT_USER\Software\MOG Solutions\16.
		//open HKEY_CURRENT_USER\Software\MOG Solutions.
		if( keyMOGSolutions.Open(HKEY_CURRENT_USER, "Software\\MOG Solutions")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to open HKEY_CURRENT_USER\\Software\\MOG Solutions");
			return FALSE;
		}
		//create HKEY_CURRENT_USER\Software\MOG Solutions\16.
		if( keyMOGSolutions.Create( keyMOGSolutions, "16", REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WRITE|KEY_READ )!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to create HKEY_CURRENT_USER\\Software\\MOG Solutions\\16");
			keyMOGSolutions.Close();
			return FALSE;
		}
		keyMOGSolutions.Close();
		//Open HKEY_CURRENT_USER\Software\MOG Solutions\16.
		if( key16.Open(HKEY_CURRENT_USER, "Software\\MOG Solutions\\16")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to open HKEY_CURRENT_USER\\Software\\MOG Solutions\\16");
			return FALSE;
		}
		//Set product name.
		if( key16.SetValue("MXF_SDK", "Product Name")!=ERROR_SUCCESS )
		{
			key16.Close();
			Clog(0, "[Registry]Failed to set value of ProductName");
			return FALSE;
		}
		//create it sub key 1.
		if( key16.Create( key16, "1", REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WRITE|KEY_READ)!=ERROR_SUCCESS )
		{
			key16.Close();
			Clog(0, "[Registry]Failed to create HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1");
			return FALSE;
		}
		key16.Close();

		//Do with HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1.
		//Open HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1.
		if( key1.Open(HKEY_CURRENT_USER, "Software\\MOG Solutions\\16\\1")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to open HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1");
			return FALSE;
		}
		if( key1.SetValue("v2.4.0", "Version Name")!=ERROR_SUCCESS )
		{
			key1.Close();
			Clog(0, "[Registry]Failed to set Version Name value.");
			return FALSE;
		}
		if( key1.Create(key1, "Pay License", REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WRITE|KEY_READ)!=ERROR_SUCCESS )
		{
			key1.Close();
			Clog(0, "[Registry]Failed to create HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1\\Pay License");
			return FALSE;
		}
		key1.Close();

		//Do with Pay License.
		//Open HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1\\Pay License
		if( keyPayLicense.Open( HKEY_CURRENT_USER, "Software\\MOG Solutions\\16\\1\\Pay License")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to open HKEY_CURRENT_USER\\Software\\MOG Solutions\\16\\1\\Pay License");
			return FALSE;
		}
		if( keyPayLicense.SetValue("Kaity Yu","User Name")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to set User Name value.");
			keyPayLicense.Close();
			return FALSE;
		}
		if( keyPayLicense.SetValue("ZQ Interactive","Company Name")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to set Company Name value.");
			keyPayLicense.Close();
			return FALSE;
		}
		if( keyPayLicense.SetValue("RZAVC-E78E2-I52FV-B9I3C-YLR52","License Key")!=ERROR_SUCCESS )
		{
			Clog(0, "[Registry]Failed to set License Key value.");
			keyPayLicense.Close();
			return FALSE;
		}
		keyPayLicense.Close();
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}


BOOL IsAdministrator(char* username)
{
	DWORD dwLevel = 1;
	LPUSER_INFO_1 pBuf = NULL;
	NET_API_STATUS nStatus;
	BOOL bAdministrator = FALSE;

	USES_CONVERSION;
	nStatus = NetUserGetInfo( NULL,A2W(username),dwLevel,(LPBYTE *)&pBuf);
	if (nStatus == NERR_Success)
	{
		if (pBuf != NULL)
		{
			if( pBuf->usri1_priv==USER_PRIV_ADMIN)
			{
				Clog(0, "[IsAdministrator]%s is administrator.", username);
				bAdministrator = TRUE;
			}
			else if( pBuf->usri1_priv==USER_PRIV_USER)
			{
				Clog(0, "[IsAdministrator]%s is an user.", username);
			}
			else if( pBuf->usri1_priv==USER_PRIV_GUEST)
			{
				Clog(0, "[IsAdministrator]%s is a guest.", username);
			}
			else
			{
				Clog(0, "[IsAdministrator]%s is an unknown user.", username);
			}
		}
	}
	else
	{
		if( nStatus==ERROR_ACCESS_DENIED)
		{
			Clog(0, "[IsAdministrator] ERROR ACCESS DENIED.");
		}
		Clog(0, "[IsAdministrator]Failed to check if this user is administrator. status is %d. errorcode is %d.", nStatus, ::GetLastError());
	}

	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}

	return bAdministrator;
}
BOOL CheckSystem()
{
	
	//BOOL bRet  = FALSE;
	//NET_API_STATUS nStatus = 0;
	//PNET_DISPLAY_USER pNetDisplayUser = 0;
	//DWORD dwEntryReturn = 0;

	////USES_CONVERSION;
	//int i = 0;
	//do 
	//{	
	//	nStatus = NetQueryDisplayInformation(NULL, 1, i, 1, MAX_PREFERRED_LENGTH, &dwEntryReturn, (LPVOID *)&pNetDisplayUser);
	//	if( nStatus!=NERR_Success && nStatus!=ERROR_MORE_DATA)
	//	{
	//		Clog(0, "[CheckSystem]Failed to call NetQueryDisplayInformation. return value is %d.", nStatus);
	//		return FALSE;
	//	}
	//	NetApiBufferFree( pNetDisplayUser );
	//	i++;
	//} while( nStatus==ERROR_MORE_DATA );
	//Clog(0, "[CheckSystem]successful to call NetQueryDisplayInformation." );

	//wchar_t* pTmp = W2A(pNetDisplayUser->usri1_name);
	//bRet = IsAdministrator( pTmp );

	//free( pTmp );

	//NetApiBufferFree( pNetDisplayUser );

	return TRUE;
}

BOOL CheckUser()
{
	char pUser[32];
	DWORD dwLen = 32;
	if( !GetUserName( pUser, &dwLen ) )
	{
		Clog(0, "[CheckUser]Failed to get log on user. Error code is %d.", GetLastError() );
		return FALSE;
	}
	else
	{
		Clog(0, "[CheckUser]Successful to get log on user. User name is %s.", pUser);
		if( strcmp(pUser, "SYSTEM")==0 )
		{
			Clog(0, "[checkUser]Service Logon user is \"LocalSystem\", it is not permitted.");
			return FALSE;
			//if( CheckSystem() )
			//{
			//	Clog(0, "[CheckUser]Successful to check user.User is %s.", pUser);
			//	return TRUE;
			//}
			//else
			//{
			//	Clog(0, "[CheckUser]Failed to check user. User is %s.", pUser);
			//	return FALSE;
			//}
		}
		else
		{
			if( IsAdministrator(pUser) )
			{
				Clog(0,"[CheckUser]Successful to check user. User is %s.", pUser);
				return TRUE;
			}
			else
			{
				Clog(0, "[CheckUser]Failed to check user.");
				return FALSE;
			}
		}
	}
}
