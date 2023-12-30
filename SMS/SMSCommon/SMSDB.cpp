#include "smsdb.h"

#include "Log.h"
#include "ScLog.h"
using namespace ZQ::common;

SMSDB::SMSDB()
{
}

SMSDB::~SMSDB()
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

bool SMSDB::ConnectDB(char* path, WCHAR* error)
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
		WCHAR stError[256];
		memset(stError, 0x00, 256*sizeof(WCHAR));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		swprintf(error, L"ERROR: COleException <%s>", stError);
		glog(Log::L_DEBUG, L"Connection CreateInstance <%s> ", error);
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
		PopulateComError(L"ConnectToAdoDB", cComErr, error);
		glog(Log::L_DEBUG, L"Connection Open Error <%s> ", error);
		return false;
	}

	try{
		hr = m_pRecordset.CreateInstance(__uuidof(Recordset));
	}
	catch(_com_error cComErr)
	{
		PopulateComError(L"connectToAdoDB", cComErr, error);
		glog(Log::L_DEBUG, L"Recordset CreateInstance Error <%s> ", error);
		return false;
	}

	return ret;
}

void
SMSDB::PopulateComError(const WCHAR* sText, _com_error& cComErr, WCHAR* sError)
{
	IErrorInfo *pErr = cComErr.ErrorInfo();
	if (pErr != NULL)
	{
		BSTR	bstrDescription;
		BSTR	bstrSource;

		pErr->GetDescription(&bstrDescription);
		pErr->GetSource(&bstrSource);
		pErr->Release();
		
		swprintf(sError, L"ERROR: %s: ComError() <%s> <%s>", sText, (LPCWSTR)bstrDescription, (LPCWSTR)bstrSource);

		::SysFreeString(bstrDescription);
		::SysFreeString(bstrSource);
	}
}

bool SMSDB::Insert( int		packageLength, 
					int		cmd, 
					int		uid,
					char*	serviceCode,
					char*   callerNumber,
					char*	sendTime,
					char*   SMSContent,
					char*	SendContent,
					bool	TicpFinished,
					bool	SmsFinished,
					char*   leftContent /* = NULL */)
{
	WCHAR wcsErr[200];
	memset(wcsErr, 0x00, 200*sizeof(WCHAR));
	
	try
	{
		glog(Log::L_DEBUG, "插入数据库, UID 是 %d", uid);
		m_pRecordset->Open("SELECT * FROM NPVRSMSDB",
							m_pConnection.GetInterfacePtr(), // 获取库接库的IDispatch指针
							adOpenDynamic,
							adLockOptimistic,
							adCmdText);
	}
	catch (...) 
	{
		glog(Log::L_DEBUG, L"GetLastError <%d>", GetLastError());
		return false;
	}

	try
	{
		m_pRecordset->AddNew();
	}
	catch (...)
	{
		glog(Log::L_DEBUG, L"GetLastError <%d>", GetLastError());
		return false;
	}
	
	glog(Log::L_DEBUG, "(插入数据库) 记录集 AddNew UID is %d", uid);
	
	m_pRecordset->PutCollect("PackageLength",     long(packageLength));
	glog(Log::L_DEBUG, "(插入数据库) PackageLen   is %d", packageLength);
	
	m_pRecordset->PutCollect("Cmd",				  long(cmd));
	glog(Log::L_DEBUG, "(插入数据库) CMD          is %d", cmd);
	
	m_pRecordset->PutCollect("Uid",				  long(uid));
	glog(Log::L_DEBUG, "(插入数据库) UID          is %d", uid);
	
	m_pRecordset->PutCollect("ServiceCode",       _variant_t(serviceCode));
	glog(Log::L_DEBUG, "(插入数据库) ServiceCode  is %s", serviceCode);
	
	m_pRecordset->PutCollect("CallerNumber",      _variant_t(callerNumber));
	glog(Log::L_DEBUG, "(插入数据库) CallerNumber is %s", callerNumber);

	m_pRecordset->PutCollect("SendTime",		  _variant_t(sendTime));
	glog(Log::L_DEBUG, "(插入数据库) SendTime     is %s", sendTime);

	m_pRecordset->PutCollect("SMSContent",		  _variant_t(SMSContent));
	glog(Log::L_DEBUG, "(插入数据库) SMSContent   is %s", SMSContent);

	m_pRecordset->PutCollect("SendContent",		  _variant_t(SendContent));
	glog(Log::L_DEBUG, "(插入数据库) SendContent  is %s", SendContent);

	if (leftContent)
	{
		m_pRecordset->PutCollect("LeftContent",		  _variant_t(leftContent));
		glog(Log::L_DEBUG, "(插入数据库) LeftContent  is %s", leftContent);
	}

	m_pRecordset->PutCollect("TicpFinished",	  bool(TicpFinished));
	glog(Log::L_DEBUG, "(插入数据库) TicpFinished is %d", TicpFinished);

	m_pRecordset->PutCollect("SmsFinished",		  bool(SmsFinished));
	glog(Log::L_DEBUG, "(插入数据库) SmsFinished  is %d", SmsFinished);
	
	try
	{
		m_pRecordset->Update();
	}
	catch(...)
	{
		glog(Log::L_DEBUG, "GetLastError <%d>", GetLastError());
		return false;
	}

	m_pRecordset->MoveLast();

	m_pRecordset->Close();

	return true;
}


bool SMSDB::UpdateState( int		uid,
						 bool		TicpFinished,
						 bool		SMSFinished,
						 char*		TicpSMSContent)
{
	if (uid < 0)
	{
		glog(Log::L_DEBUG, L"UID<%d> 必须大于 0", uid);
		return false;
	}
	
//	WCHAR wcsErr[200];
//	memset(wcsErr, 0x00, 200*sizeof(WCHAR));

	char strSql[100];
	memset(strSql, 0x00, 100*sizeof(char));
	sprintf(strSql, "SELECT * FROM NPVRSMSDB WHERE Uid = %d", uid);

	try
	{
//		TRACE(L"Update db, <%d>, set TicpFinished <%d>, SMSFinished <%d>", uid, TicpFinished, SMSFinished);
		glog(Log::L_DEBUG, "(更新数据库) UID 是 %d, ");
		m_pRecordset->Open(_variant_t(strSql),
				m_pConnection.GetInterfacePtr(),
				adOpenDynamic,
				adLockOptimistic,
				adCmdText);
	}
	catch (...) 
	{
		glog(Log::L_DEBUG, "GetLastError <%d>", GetLastError());
		return false;
	}
/*	catch(_com_error cComErr)
	{
		PopulateComError(L"connectToAdoDB", cComErr, wcsErr);
		TRACE(L"db error <%s>", wcsErr);
		return false;
	}
*/
	if(m_pRecordset->BOF)
	{
		m_pRecordset->Close();
		return false;
	}
	
	m_pRecordset->PutCollect("TicpFinished",		bool(TicpFinished));
	glog(Log::L_DEBUG, "(更新数据库) TicpFinished     is %d", TicpFinished);
	
	m_pRecordset->PutCollect("SmsFinished",			bool(SMSFinished));
	glog(Log::L_DEBUG, "(更新数据库) SMSFinished      is %d", SMSFinished);
	
	if (strlen(TicpSMSContent) > 0)
	{
		m_pRecordset->PutCollect("SMSTicpContent",		_variant_t(TicpSMSContent));
		glog(Log::L_DEBUG, "(更新数据库) TicpSMSContent   is %s", TicpSMSContent);
	}
	
	try
	{
//		TRACE(L"Recordset Update");
		HRESULT hr = m_pRecordset->Update();
		if (!SUCCEEDED(hr))
		{
			AfxThrowOleException(hr);
		}
	}
/*	catch(COleException *pOleEx)
	{
		WCHAR stError[256];
		memset(stError, 0x00, 256*sizeof(WCHAR));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		//return false;
		TRACE(L"Update: ERROR: COleException <%s>", stError);
	}
*/	catch (...) 
	{
		glog(Log::L_DEBUG, "(更新数据库) 记录集更新失败 %d", GetLastError());
		return false;
	}
	
	m_pRecordset->Close();
	
	return true;
}

int SMSDB::GetUID()
{
	char strSql[] = "SELECT * FROM NPVRSMSDB ORDER BY Uid DESC";
	
	try
	{
//		TRACE(L"Get UID From DB");
		m_pRecordset->Open(_variant_t(strSql),
				m_pConnection.GetInterfacePtr(),
				adOpenDynamic,
				adLockOptimistic,
				adCmdText);
	}
	catch (...) 
	{
		glog(Log::L_DEBUG, "(获取UID) 记录集打开失败 %d", GetLastError());
		return 0;
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


bool SMSDB::SelectUnfinished(char* time, WCHAR* sErr)
{
	char strSql[256];
	memset(strSql, 0x00, 256*sizeof(char));
	sprintf(strSql, "SELECT * FROM NPVRSMSDB WHERE SmsFinished = 0 AND SendTime > #%s#", time);
	
	try
	{
//		TRACE(L"Select unfinished <%s>" ,strSql);
		m_pRecordset->Open( _variant_t(strSql),  
							this->m_pConnection.GetInterfacePtr(), // 获取库接库的IDispatch指针
							adOpenDynamic,
							adLockOptimistic,
							adCmdText);
	}
	catch (COleException *pOleEx)
	{
		WCHAR stError[256];
		memset(stError, 0x00, 256*sizeof(WCHAR));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		swprintf(sErr, L"ERROR: COleException <%s>", stError);
		m_pRecordset->Close();
		return false;
	}
	
	catch (...) 
	{
//		TRACE(L"GetLastError <%d>", GetLastError());
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

bool SMSDB::getData(int&		packageLength,
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

	if (serviceCode)
	{
		var = m_pRecordset->GetCollect("ServiceCode");
		if (var.vt != VT_NULL)
		{
			strcpy(serviceCode, (LPSTR)_bstr_t(var));
		}
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

	if (TicpContent)
	{
		var = m_pRecordset->GetCollect("SMSTicpContent");
		if (var.vt != VT_NULL)
		{
			strcpy(TicpContent, (LPSTR)_bstr_t(var));
		}
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

	if (leftContent)
	{
		var = m_pRecordset->GetCollect("LeftContent");
		if (var.vt != VT_NULL)
		{
			strcpy(leftContent, (LPSTR)_bstr_t(var));
		}
	}
	
	m_pRecordset->MoveNext();
	return true;
}

bool SMSDB::DeleteOverdueMessage(char* time)
{
	char strSql[256];
	memset(strSql, 0x00, 256*sizeof(char));
	sprintf(strSql, "DELETE FROM NPVRSMSDB WHERE SendTime < #%s#", time);

	try
	{
//		TRACE(L"Delete Overdue Messages <%s>" ,strSql);
		m_pConnection->Execute(_bstr_t(strSql), NULL, adCmdText);
	}
	catch (...) 
	{
//		TRACE(L"GetLastError <%d>", GetLastError());
		return false;
	}
	return true;
}

bool SMSDB::SelectPart(char* time, WCHAR* sErr)
{
	char strSql[256];
	memset(strSql, 0x00, 256*sizeof(char));
	sprintf(strSql, "SELECT * FROM NPVRSMSDB WHERE SendTime > #%s#", time);
	
	try
	{
//		TRACE(L"Select unfinished <%s>" ,strSql);
		m_pRecordset->Open( _variant_t(strSql),  
							this->m_pConnection.GetInterfacePtr(), // 获取库接库的IDispatch指针
							adOpenDynamic,
							adLockOptimistic,
							adCmdText);
	}
	catch (COleException *pOleEx)
	{
		WCHAR stError[256];
		memset(stError, 0x00, 256*sizeof(WCHAR));
		pOleEx->GetErrorMessage(stError, 256);
		pOleEx->Delete();
		swprintf(sErr, L"ERROR: COleException <%s>", stError);
		m_pRecordset->Close();
		return false;
	}
	
	catch (...) 
	{
//		TRACE(L"GetLastError <%d>", GetLastError());
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
