
#include "../../implinclude.h"
#include "corbalib/corbalibinclude.h"
#include "loghandler.h"

#include "registryex.h"


#define ISAALARM_REG_PATH		"Software\\ZQ\\ISAAlarm"
#define REG_DOMAIN_KEY			"Domain"
#define REG_NAMINGSERVICE_KEY	"NamingService"
#define REG_EVENTCHANNEL_KEY	"EventChannel"
#define REG_POA_KEY				"Poa"
#define REG_NOTIFYSERVICE_KEY	"NotifyService"
#define REG_ARG_COUNT_KEY		"ArgCount"
#define REG_ARG_ITEM_KEY		"Arg"

#define REG_MAX_ARG_STR_LENGTH	256
#define REG_MAX_ARG_COUNT		256
#define MAX_SERVICE_NAME_LENGTH	256

struct ISAAlarmOption
{
public:
	RegistryEx_T									registry;
	TaoNotifyEventChannel::StructuredPushSupplier	supplier;
	HANDLE											thread_handle;

	TaoServiceBase*									tsb;

	struct Setting
	{
		char	domain_name[MAX_SERVICE_NAME_LENGTH];
		char	namingservice_name[MAX_SERVICE_NAME_LENGTH];
		char	eventchannel_name[MAX_SERVICE_NAME_LENGTH];
		char	poa_name[MAX_SERVICE_NAME_LENGTH];
		char	notifyservice_name[MAX_SERVICE_NAME_LENGTH];

		int		argc;
		char*	argv[REG_MAX_ARG_COUNT];
	} setting;

	ISAAlarmOption();
	~ISAAlarmOption();
};

ISAAlarmOption* g_pOption = NULL;

DWORD WINAPI tao_env(void* param)
{
	if (NULL == g_pOption->tsb)
		g_pOption->tsb = new TaoServiceBase(g_pOption->setting.argc, g_pOption->setting.argv);

	TaoRootPOA trp(g_pOption->tsb, g_pOption->setting.poa_name);
	trp.Run();

	TaoNamingService tns(g_pOption->tsb, g_pOption->setting.namingservice_name);
	TaoNotifyEventChannel tec(&tns, g_pOption->setting.notifyservice_name);

	tec.GetStructuredPushSupplier(&g_pOption->supplier, g_pOption->setting.eventchannel_name);
	g_pOption->supplier.Connect();

	if (!g_pOption->tsb->Run())
	{
		return 1;
	}

	Log(LogHandler::L_ERROR, "[tao_env] can not run TAO instance");
	return 0;
};


bool Init_Proc()
{
	if (NULL == g_pOption)
		g_pOption = new ISAAlarmOption;

	g_pOption->thread_handle = CreateThread(NULL, 0, tao_env, NULL, 0, NULL);

	return NULL != g_pOption->thread_handle;
}

void UnInit_Proc()
{
	g_pOption->tsb->Shutdown();
	g_pOption->tsb->Destroy();

	CloseHandle(g_pOption->thread_handle);
	
	if (NULL != g_pOption->tsb)
		delete g_pOption->tsb;

	if (NULL != g_pOption)
	{
		delete g_pOption;
		g_pOption = NULL;
	}
}


#define FIXED_KEY_EVENT_TYPE "EventType"
#define FIXED_KEY_EVENT_NAME "EventName"

//EVENT_STR_FF_*** defined for Fault and Flow Event Header
#define FIXED_KEY_EVENT_FF_TIMEOF		"TimeOf"
#define FIXED_KEY_EVENT_FF_EVENTCODE	"EventCode"
#define FIXED_KEY_EVENT_FF_SEV			"Sev"
#define FIXED_KEY_EVENT_FF_COMP			"Comp"
#define FIXED_KEY_EVENT_FF_ADDR			"Addr"
#define FIXED_KEY_EVENT_FF_TASK			"Task"
#define FIXED_KEY_EVENT_FF_MP			"Mp"
#define FIXED_KEY_EVENT_FF_TRANS		"Trans"

//EVENT_STR_PERF_*** defined for Performance Event Elements
#define FIXED_KEY_EVENT_PF_ENDTIME		"EndTime"
#define FIXED_KEY_EVENT_PF_EVENTCODE	"EventCode"
#define FIXED_KEY_EVENT_PF_FOR			"For"
#define FIXED_KEY_EVENT_PF_DUR			"Dur"
#define FIXED_KEY_EVENT_PF_END			"End"
#define FIXED_KEY_EVENT_PF_STRT			"Strt"

//EVENT_STR_BE_*** defined for Event Body Elements
#define FIXED_KEY_EVENT_BE_SDESC		"Sdesc"
#define FIXED_KEY_EVENT_BE_OP			"Op"
#define FIXED_KEY_EVENT_BE_DD			"Dd"
#define FIXED_KEY_EVENT_BE_DUR			"Dur"
#define FIXED_KEY_EVENT_BE_XR			"Xr"


bool CreateStructuredEvent(char* const* key, char* const* data,
						   size_t count, CosNotification::StructuredEvent& event)
{
	event.header.fixed_header.event_type.domain_name	=
		CORBA::string_dup(g_pOption->setting.domain_name);

	int nHeaderCount = 0;
	int nBodyCount = 0;
	for (size_t i = 0; i < count; ++i)
	{
		if ((0 == stricmp(key[i], FIXED_KEY_EVENT_FF_TIMEOF))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_EVENTCODE))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_COMP))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_ADDR))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_TASK))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_MP))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_TRANS))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_ENDTIME))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_EVENTCODE))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_FOR))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_END))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_SEV))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_DUR))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_STRT)))
		{
			++nHeaderCount;
		}
		else if ((0 == stricmp(key[i], FIXED_KEY_EVENT_BE_SDESC))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_OP))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_DD))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_DUR))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_XR)))
		{
			++nBodyCount;
		}
	}

	if (0 == nHeaderCount)
		return false;

	event.header.variable_header.length(nHeaderCount);
	event.filterable_data.length(nBodyCount);

	int nCurHeaderPos = 0;
	int nCurBodyPos = 0;
	for (i = 0; i < count; ++i)
	{
		if (0 == stricmp(key[i], FIXED_KEY_EVENT_TYPE))
			event.header.fixed_header.event_type.type_name = CORBA::string_dup(data[i]);
		else if (0 == stricmp(key[i], FIXED_KEY_EVENT_NAME))
			event.header.fixed_header.event_name = CORBA::string_dup(data[i]);
		else if ((0 == stricmp(key[i], FIXED_KEY_EVENT_FF_TIMEOF))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_EVENTCODE))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_COMP))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_ADDR))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_TASK))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_MP))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_FF_TRANS))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_ENDTIME))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_EVENTCODE))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_FOR))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_END))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_STRT)))
		{
			event.header.variable_header[nCurHeaderPos].name = CORBA::string_dup(key[i]);
			event.header.variable_header[nCurHeaderPos++].value <<= CORBA::string_dup(data[i]);
		}
		else if ((0 == stricmp(key[i], FIXED_KEY_EVENT_FF_SEV))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_PF_DUR)))
		{
			event.header.variable_header[nCurHeaderPos].name = CORBA::string_dup(key[i]);
			event.header.variable_header[nCurHeaderPos++].value <<= atoi(data[i]);
		}
		else if ((0 == stricmp(key[i], FIXED_KEY_EVENT_BE_SDESC))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_OP))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_DD))||
			(0 == stricmp(key[i], FIXED_KEY_EVENT_BE_XR)))
		{
			event.filterable_data[nCurBodyPos].name = CORBA::string_dup(key[i]);
			event.filterable_data[nCurBodyPos++].value <<= CORBA::string_dup(data[i]);
		}
		else if (0 == stricmp(key[i], FIXED_KEY_EVENT_BE_DUR))
		{
			event.filterable_data[nCurBodyPos].name = CORBA::string_dup(key[i]);
			event.filterable_data[nCurBodyPos++].value <<= atoi(data[i]);
		}
	}

	return true;
}

bool Action_Proc(char* const* key, char* const* data, size_t count)
{
	//todo: add lock code here

	CosNotification::StructuredEvent event;
	if (!CreateStructuredEvent(key, data, count, event))
	{
		Log(LogHandler::L_ERROR, "[Action_Proc] can not create structured event");
		return false;
	}

	g_pOption->supplier.Push(event);
	//find block in ini file by syntax
	return true;
}


ISAAlarmOption::ISAAlarmOption()
:registry(ISAALARM_REG_PATH)
{
	setting.argc = 0;

	if (!registry.LoadStr(REG_DOMAIN_KEY, setting.domain_name, MAX_SERVICE_NAME_LENGTH))
	{
		Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] can not get domain setting in registry");
	}
	if (!registry.LoadStr(REG_NAMINGSERVICE_KEY, setting.namingservice_name, MAX_SERVICE_NAME_LENGTH))
	{
		Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] can not get naming service setting in registry");
	}
	if (!registry.LoadStr(REG_EVENTCHANNEL_KEY, setting.eventchannel_name, MAX_SERVICE_NAME_LENGTH))
	{
		Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] can not get event channel setting in registry");
	}
	if (!registry.LoadStr(REG_POA_KEY, setting.poa_name, MAX_SERVICE_NAME_LENGTH))
	{
		Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] can not get poa setting in registry");
	}
	if (!registry.LoadStr(REG_NOTIFYSERVICE_KEY, setting.notifyservice_name, MAX_SERVICE_NAME_LENGTH))
	{
		Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] can not get notify service setting in registry");
	}

	DWORD dwArgCount = 0;
	if (registry.LoadDword(REG_ARG_COUNT_KEY, dwArgCount))
	{
		setting.argc = dwArgCount;
		if (setting.argc > REG_MAX_ARG_COUNT)
		{
			Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] arguments overflow");
			setting.argc = 0;
		}
		for (int i = 0; i < setting.argc; ++i)
		{
			setting.argv[i] = new char[REG_MAX_ARG_STR_LENGTH];
			memset(setting.argv[i], 0, REG_MAX_ARG_STR_LENGTH);
			char temp[REG_MAX_ARG_STR_LENGTH] = {0};
			_snprintf(temp, REG_MAX_ARG_STR_LENGTH, "%s_%d", REG_ARG_ITEM_KEY, i);
			if (!registry.LoadStr(temp, setting.argv[i], REG_MAX_ARG_STR_LENGTH))
			{
				Log(LogHandler::L_ERROR, "[ISAAlarmOption::ISAAlarmOption] can not get argument service setting in registry");
			}
		}
	}
}

ISAAlarmOption::~ISAAlarmOption()
{
	for (int i = 0; i < setting.argc; ++i)
	{
		if (NULL != setting.argv[i])
			delete[] setting.argv[i];
	}
}
