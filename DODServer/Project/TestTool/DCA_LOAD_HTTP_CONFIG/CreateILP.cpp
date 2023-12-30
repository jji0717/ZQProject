#include "StdAfx.h"
#include ".\createilp.h"
#using <mscorlib.dll>
#include "messagemacro.h"

CCreateILP::CCreateILP(void)
{
}

CCreateILP::~CCreateILP(void)
{
}
CString CCreateILP::CreateILPFile(CString sFilename,int nDatyType)
{
	if (nDatyType <=0)
	{
		Clog(LOG_DEBUG,"CCreateILP ::CreateILPFile ,nDatyType value(%d) is error",nDatyType);
		return "";
	}

	int nILPType=0;
	switch(nDatyType) {
	case DATATYPE_NAVIGATION:
		nILPType=DATATYPE_ILP_NAVIGATION;
		break;
	case DATATYPE_PORTAL:
		nILPType=DATATYPE_ILP_PORTAL;
		break;
	case DATATYPE_POSTER:
		nILPType=DATATYPE_ILP_POSTER;
		break;
	default:
		Clog(LOG_DEBUG,"CCreateILP ::CreateILPFile ,nDatyType value(%d) is other stream datetype, error",nDatyType);
		return "";
	}

	SYSTEMTIME time; 
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d-%02d-%02d %02d:%02d:%02d|%4d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond,nILPType);

	CString strTmp;
	strTmp.Format("1,2.0|1101,5,0|%s||||",sTime);

	return strTmp;		 
}
