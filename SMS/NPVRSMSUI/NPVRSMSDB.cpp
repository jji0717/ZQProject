#include "npvrsmsdb.h"

NPVRSMSDB::NPVRSMSDB()
{
//	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

NPVRSMSDB::~NPVRSMSDB()
{
	if (m_pRecordset != NULL)
	{
		m_pRecordset.Release();
	}
	m_pRecordset = NULL;

	if (m_pConnection)
	{
		if(m_pConnection->State)
		{
			m_pConnection->Close();
			m_pConnection.Release();
		}
	}
	m_pConnection = NULL;

	CoUninitialize();
}

bool NPVRSMSDB::ConnectDB(char* path)
{
	CoInitialize(NULL);

	bool ret = false;

	HRESULT hr;
	
	try{
		hr = m_pConnection.CreateInstance(__uuidof(Connection));
		if (!SUCCEEDED(hr))
		{
			AfxThrowOleException(hr);
		}
	}
	catch(COleException *pOleEx)
	{
		char stError[256];
		memset(stError, 0x00, 256*sizeof(char));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		TRACE("ERROR: COleException <%s>", stError);
		return false;
	}

	try{
		char dbPath[200];
		memset(dbPath, 0x00, 200*sizeof(char));
		sprintf(dbPath, "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s", path);
		m_pConnection->Open(_bstr_t(dbPath),"","",adModeUnknown);
		ret = true;
	}
	catch(_com_error cComErr)
	{
		char error[300];
		memset(error, 0x00, 300*sizeof(char));
		PopulateComError("connectToAdoDB", cComErr, error);
		TRACE("db error <%s>", error);
		return false;
	}

	try{
		hr = m_pRecordset.CreateInstance(__uuidof(Recordset));
	}
	catch(_com_error cComErr)
	{
		char error[300];
		memset(error, 0x00, 300*sizeof(char));
		PopulateComError("connectToAdoDB", cComErr, error);
		TRACE("db error <%s>", error);
		return false;
	}

	return ret;
}

bool NPVRSMSDB::Insert( int		packageLength, 
						int		cmd, 
						int		uid,
						char*	serviceCode,
						char*   callerNumber,
						char*	sendTime,
						char*   SMSContent,
						char*	SendContent,
						char*   leftContent,
						bool	TicpFinished,
						bool	SmsFinished)
{
	WCHAR wcsErr[200];
	memset(wcsErr, 0x00, 200*sizeof(WCHAR));
	
	try
	{
		TRACE("Insert db");
		m_pRecordset->Open("SELECT * FROM NPVRSMSDB",
							m_pConnection.GetInterfacePtr(), // 获取库接库的IDispatch指针
							adOpenDynamic,
							adLockOptimistic,
							adCmdText);
	}
	catch (...) 
	{
		TRACE("GetLastError <%d>", GetLastError());
		return false;
	}
/*	catch(_com_error cComErr)
	{
		PopulateComError(L"connectToAdoDB", cComErr, wcsErr);
		TRACE("db error <%s>", wcsErr);
		return false;
	}
*/	
	m_pRecordset->AddNew();

	m_pRecordset->PutCollect("PackageLength",     long(packageLength));
	m_pRecordset->PutCollect("Cmd",				  long(cmd));
	m_pRecordset->PutCollect("Uid",				  long(uid));
	m_pRecordset->PutCollect("ServiceCode",       _variant_t(serviceCode));
	m_pRecordset->PutCollect("CallerNumber",      _variant_t(callerNumber));
	m_pRecordset->PutCollect("SendTime",		  _variant_t(sendTime));
	m_pRecordset->PutCollect("SMSContent",		  _variant_t(SMSContent));
	m_pRecordset->PutCollect("SendContent",		  _variant_t(SendContent));
	m_pRecordset->PutCollect("LeftContent",		  _variant_t(leftContent));
	m_pRecordset->PutCollect("TicpFinished",	  bool(TicpFinished));
	m_pRecordset->PutCollect("SmsFinished",		  bool(SmsFinished));
	
	m_pRecordset->Update();

	m_pRecordset->MoveLast();

	m_pRecordset->Close();

	return true;
}


bool NPVRSMSDB::UpdateState( int		uid,
							 bool		TicpFinished,
							 bool		SMSFinished,
							 char*		TicpSMSContent)
{
	if (uid < 0)
	{
		TRACE("Uid <%d> must above 0", uid);
		return false;
	}
	
	WCHAR wcsErr[200];
	memset(wcsErr, 0x00, 200*sizeof(WCHAR));

	char strSql[100];
	memset(strSql, 0x00, 100*sizeof(char));
	sprintf(strSql, "SELECT * FROM NPVRSMSDB WHERE Uid = %d", uid);

	try
	{
		TRACE("Update db, <%s>, set TicpFinished <%d>, SMSFinished <%d>", strSql, TicpFinished, SMSFinished);
		m_pRecordset->Open(_variant_t(strSql),
				m_pConnection.GetInterfacePtr(),
				adOpenDynamic,
				adLockOptimistic,
				adCmdText);
	}
	catch (...) 
	{
		TRACE("Select GetLastError <%d>", GetLastError());
		return false;
	}
/*	catch(_com_error cComErr)
	{
		PopulateComError(L"connectToAdoDB", cComErr, wcsErr);
		TRACE("db error <%s>", wcsErr);
		return false;
	}
*/
	m_pRecordset->PutCollect("TicpFinished",		bool(TicpFinished));
//	TRACE("Update db TicpFinished <%d>", TicpFinished);// just for debug
	
	m_pRecordset->PutCollect("SmsFinished",			bool(SMSFinished));
//	TRACE("Update db SMSFinished <%d>", SMSFinished);// just for debug
	
	if (strlen(TicpSMSContent) > 0)
	{
		m_pRecordset->PutCollect("SMSTicpContent",		_variant_t(TicpSMSContent));
		TRACE("TicpSMSContent <%s>", TicpSMSContent);// just for debug
	}
	
	try
	{
		HRESULT hr = m_pRecordset->Update();
		if (!SUCCEEDED(hr))
		{
			AfxThrowOleException(hr);
		}
	}
	catch(COleException *pOleEx)
	{
		char stError[256];
		memset(stError, 0x00, 256*sizeof(char));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		//return false;
		TRACE("Update: ERROR: COleException <%s>", stError);
	}
	catch (...) 
	{
		TRACE("Update GetLastError <%d>", GetLastError());
	}
	
	m_pRecordset->Close();
	
	return true;
}

int NPVRSMSDB::GetUID()
{
	char strSql[] = "SELECT * FROM NPVRSMSDB ORDER BY Uid DESC";
	
	try
	{
		TRACE("Get UID From DB");
		m_pRecordset->Open(_variant_t(strSql),
				m_pConnection.GetInterfacePtr(),
				adOpenDynamic,
				adLockOptimistic,
				adCmdText);
	}
	catch (...) 
	{
		TRACE("GetLastError <%d>", GetLastError());
		return -1;
	}
/*	catch(_com_error *e)
	{
		return -1;
	}
*/	if(m_pRecordset->BOF)
	{
		m_pRecordset->Close();
		return 0;
	}
	m_pRecordset->MoveFirst();

	_variant_t var;
	var = m_pRecordset->GetCollect("Uid");

	int uid = (long)(var) + 1;

	m_pRecordset->Close();

	return uid;	
}


bool NPVRSMSDB::SelectUnfinished(char* time)
{
	char strSql[256];
	memset(strSql, 0x00, 256*sizeof(char));
	sprintf(strSql, "SELECT * FROM NPVRSMSDB WHERE SmsFinished = 0 AND SendTime > #%s#", time);
	
	try
	{
		TRACE("Select unfinished <%s>" ,strSql);
		m_pRecordset->Open( _variant_t(strSql),  
							this->m_pConnection.GetInterfacePtr(), // 获取库接库的IDispatch指针
							adOpenDynamic,
							adLockOptimistic,
							adCmdText);
	}
	catch (COleException *pOleEx)
	{
		char stError[256];
		memset(stError, 0x00, 256*sizeof(char));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		TRACE("ERROR: COleException <%s>", stError);
		m_pRecordset->Close();
		return false;
	}
	
	catch (...) 
	{
		TRACE("GetLastError <%d>", GetLastError());
		return false;
	}

	if(!m_pRecordset->BOF)
	{
		m_pRecordset->MoveFirst();
		return true;
	}
	else
	{
		m_pRecordset->Close();
		return false;
	}
	return false;
}

bool NPVRSMSDB::SelectByTime(char* time)
{
	char strSql[256];
	memset(strSql, 0x00, 256*sizeof(char));
	sprintf(strSql, "SELECT * FROM NPVRSMSDB WHERE SendTime > #%s#", time);
	
	try
	{
		TRACE("<%s>" ,strSql);
		m_pRecordset->Open( _variant_t(strSql),  
							this->m_pConnection.GetInterfacePtr(), // 获取库接库的IDispatch指针
							adOpenDynamic,
							adLockOptimistic,
							adCmdText);
	}
	catch (COleException *pOleEx)
	{
		char stError[256];
		memset(stError, 0x00, 256*sizeof(char));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		TRACE("ERROR: COleException <%s>", stError);
		m_pRecordset->Close();
		return false;
	}
	
	catch (...) 
	{
		TRACE("GetLastError <%d>", GetLastError());
		return false;
	}

	if(!m_pRecordset->BOF)
	{
		m_pRecordset->MoveFirst();
		return true;
	}
	else
	{
		m_pRecordset->Close();
		return false;
	}
	return false;
}

bool NPVRSMSDB::getData(int&		packageLength,
						int&		cmd,
						int&		uid,
						char*		serviceCode,
						char*		callerNumber,
						char*		sendTime,
						char*		SMSContent,
						char*		SendContent,
						char*		leftContent,
						char*		TicpContent,
						bool&		TicpFinished,
						bool&		SmsFinished)
{
	if (m_pRecordset->adoEOF)
	{
		m_pRecordset->Close();
		return false;
	}

	_variant_t var;

	var = m_pRecordset->GetCollect("PackageLength");
	packageLength = (long)(var);
	
	var = m_pRecordset->GetCollect("Uid");
	uid = (long)(var);
	
	var = m_pRecordset->GetCollect("Cmd");
	cmd = (long)(var);

	var = m_pRecordset->GetCollect("ServiceCode");
	if (var.vt != VT_NULL)
	{
		strcpy(serviceCode, (LPSTR)_bstr_t(var));
	}
	
	var = m_pRecordset->GetCollect("CallerNumber");
	if (var.vt != VT_NULL)
	{
		strcpy(callerNumber, (LPSTR)_bstr_t(var));
	}
	
	var = m_pRecordset->GetCollect("SendTime");
	if (var.vt != VT_NULL)
	{
		strcpy(sendTime, (LPSTR)_bstr_t(var));
	}
	
	var = m_pRecordset->GetCollect("SMSContent");
	if (var.vt != VT_NULL)
	{
		strcpy(SMSContent, (LPSTR)_bstr_t(var));
	}

	var = m_pRecordset->GetCollect("SendContent");
	if (var.vt != VT_NULL)
	{
		strcpy(SendContent, (LPSTR)_bstr_t(var));
	}

	var = m_pRecordset->GetCollect("SMSTicpContent");
	if (var.vt != VT_NULL)
	{
		strcpy(TicpContent, (LPSTR)_bstr_t(var));
	}

	var = m_pRecordset->GetCollect("TicpFinished");
	if (var.vt != VT_NULL)
	{
		TicpFinished = (bool)(var);
	}

	var = m_pRecordset->GetCollect("SmsFinished");
	if (var.vt != VT_NULL)
	{
		SmsFinished = (bool)(var);
	}

	var = m_pRecordset->GetCollect("LeftContent");
	if (var.vt != VT_NULL)
	{
		strcpy(leftContent, (LPSTR)_bstr_t(var));
	}
	
	m_pRecordset->MoveNext();
	return true;
}

bool NPVRSMSDB::DeleteOverdueMessage(char* time)
{
	char strSql[256];
	memset(strSql, 0x00, 256*sizeof(char));
	sprintf(strSql, "DELETE FROM NPVRSMSDB WHERE SendTime < #%s#", time);

	try
	{
		TRACE("Delete Overdue Messages <%s>" ,strSql);
		m_pConnection->Execute(_bstr_t(strSql), NULL, adCmdText);
	}
	catch (...) 
	{
		TRACE("GetLastError <%d>", GetLastError());
		return false;
	}
	return true;
}

void
NPVRSMSDB::PopulateComError(const char* sText, _com_error& cComErr, char* sError)
{
	IErrorInfo *pErr = cComErr.ErrorInfo();
	
	if (pErr != NULL)
	{
		BSTR	bstrDescription;
		BSTR	bstrSource;

		pErr->GetDescription(&bstrDescription);
		pErr->GetSource(&bstrSource);
		pErr->Release();
		
		sprintf(sError, "ERROR: %s: ComError() <%s> <%s>", sText, (LPCSTR)bstrDescription, (LPCSTR)bstrSource);

		::SysFreeString(bstrDescription);
		::SysFreeString(bstrSource);
	}
}
