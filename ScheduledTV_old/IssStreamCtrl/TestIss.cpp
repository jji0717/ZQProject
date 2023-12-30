#include "TestIss.h"
#include "itvservicetypes.h"
#include "StreamSession.h"
#include "IdsSession.h"
#include <tchar.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


#define APPTYPE_META	L"ApplicationType" 

BOOL bTerminated = FALSE;
void signal_handler(int signal_value);
bool getMappedAppFromIds(const TYPEINST& TypeInst, const wchar_t *server, const wchar_t *user, APPNAME *app);
PAELIST getAEListFromMC();

int main(int argc, char *argv[])
{
	pGlog = new ScLog(_T("C:\\ITV\\Log\\StreamSession.log"), Log::L_DEBUG);

	// install Ctrl-C signal
	signal(SIGINT, signal_handler);
	TYPEINST typeInst;
	typeInst.s.dwType = ITV_TYPE_PRIMARY_ZQ_SCHEDULEDTV; //0x7FFFFFFF
	typeInst.s.dwInst = 1;
	

	glog(Log::L_DEBUG, _T("************** TestISS BEGIN ***********"));

	
	APPNAME app;
	//if(getMappedAppFromIds(typeInst, L"10.3.0.23", L"SSession", &app))
	if(getMappedAppFromIds(typeInst, L"10.7.0.23", L"SSession", &app))
	{
		CStreamSession* pSession =  CStreamSession::Instance(typeInst, app.dwUid);
//		CStreamSession* pSession =  CStreamSession::Instance(typeInst, 0x80002);
		if(pSession->Initialize())
		{
			printf("Type Ctrl-C to terminate the program\n");
			while (!bTerminated) 
				Sleep(100);

			printf("end of program\n");

			pSession->UnInitialize();
		}
		CStreamSession::FreeInstance();
	}
	glog(Log::L_DEBUG, _T("************** TestISS END ***********\n"));
	return 0;
}

void signal_handler(int code)
{
	printf("Do you really want to Quit? [y/n] :");
	char c;
	c = getchar();
	if (c == 'y' || c == 'Y')
		bTerminated = TRUE;
}

bool getMappedAppFromIds(const TYPEINST& TypeInst, const wchar_t *server, const wchar_t *user, APPNAME *app)
{
	glog(Log::L_DEBUG, _T("Began IDS Session Initialize, plz wait ..."));
	bool succ = false;
	IdsSession idsSession;
	if (!idsSession.Initialize())
	{
		glog(Log::L_ERROR, _T("IDS Initialize Failed"));
		return false;
	}

	wchar_t *MetaDataName = APPTYPE_META;
	DWORD	tpye = 0;
	glog(Log::L_DEBUG, _T("IDS Session began bind server %s, User %s"), server, user);
	if (idsSession.Bind(server, user))
	{
		IdsSession::apps_t registeredApps;
		if (idsSession.ListApplications(registeredApps))
		{
			glog(Log::L_DEBUG, _T("IDS Session list all the Applications."));
			for (DWORD i = 0; i < registeredApps.size(); i++)
			{
				DWORD type;
				if (idsSession.GetAppMetaData(registeredApps[i].dwUid, MetaDataName, type)
					&& type == TypeInst.s.dwType)
				{
					succ = TRUE;
					glog(Log::L_DEBUG, _T("Find the application Succeed"));
					memcpy(app, &(registeredApps[i]), sizeof(APPNAME));
					break;
				}
			}
		}
		idsSession.UnBind();
	}
	else
	{
		glog(Log::L_ERROR, _T("IDS Session began bind server failed"));
	}

	idsSession.UnInitialize();

	return succ;
}

