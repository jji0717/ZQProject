
#include "implementation.h"
#include "implinclude.h"
#include "zqfmtstring.h"
#include "iarmscript.h"
#include "loghandler.h"
#include <exception>

	
TG_BEGIN
	
using std::exception;

Implementation::Implementation(const char* pluginfile, DataSource& datasource)
:m_source(datasource)
{
	if (NULL == pluginfile)
	{
		throw exception("[Implementation::Implementation] pluginfile name is empty");
	}
		
	m_hModule = ::LoadLibrary(pluginfile);
	if (NULL == m_hModule)
	{
		throw exception("[Implementation::Implementation] can not open pluginfile");
	}
		
	m_pInit = ::GetProcAddress(m_hModule, PROC_INIT);
	if (NULL == m_pInit)
	{
		::FreeLibrary(m_hModule);
		throw exception("[Implementation::Implementation] can not get init procduce");
	}
	
	m_pUnInit = ::GetProcAddress(m_hModule, PROC_UNINIT);
	if (NULL == m_pInit)
	{
		::FreeLibrary(m_hModule);
		throw exception("[Implementation::Implementation] can not get uninit procduce");
	}
	
	m_pAction = ::GetProcAddress(m_hModule, PROC_ACTION);
	if (NULL == m_pInit)
	{
		::FreeLibrary(m_hModule);
		throw exception("[Implementation::Implementation] can not get action procduce");
	}
	
	if (!init())
	{
		throw exception("[Implementation::Implementation] can not initialize");
	}
}

Implementation::~Implementation()
{
	uninit();
	::FreeLibrary(m_hModule);
}

bool Implementation::init()
{
	INIT_PROC proc = (INIT_PROC)m_pInit;
	return proc();
}

void Implementation::uninit()
{
	UNINIT_PROC proc = (UNINIT_PROC)m_pUnInit;
	proc();
}

bool Implementation::action(size_t sourceid, size_t syntaxid, const char* line,
							char* const* params, size_t paramcount)
{
	const char* syntax = m_source.listSyntax(sourceid, syntaxid).c_str();

	FormatConverter* converter = new FormatConverter[paramcount];
	for (int i = 0; i < paramcount; ++i)
	{
		_snprintf(converter[i].converter, MAX_CONVERTER_LEN, "$%d", i);
		_snprintf(converter[i].formater, MAX_FORMATER_LEN, "%s", params[i]);
	}

	int datacount = m_source.countData(sourceid, syntaxid);
	char** pstrDataKey = new char*[datacount];
	char** pstrDataValue = new char*[datacount];
	for (i = 0; i < datacount; ++i)
	{
		pstrDataKey[i] = new char[MAX_DATA_LINE_BUFFER_LEN];
		memset(pstrDataKey[i], 0, MAX_DATA_LINE_BUFFER_LEN);

		pstrDataValue[i] = new char[MAX_DATA_LINE_BUFFER_LEN];
		memset(pstrDataValue[i], 0, MAX_DATA_LINE_BUFFER_LEN);

		std::string datakey = m_source.listDataKey(sourceid, syntaxid, i);
		std::string datavalue = m_source.getDataValue(sourceid, syntaxid, datakey.c_str());

		strncpy(pstrDataKey[i], datakey.c_str(), MAX_DATA_LINE_BUFFER_LEN);
		if (NULL == FormatString(datavalue.c_str(), converter, paramcount, pstrDataValue[i], MAX_DATA_LINE_BUFFER_LEN))
		{
			for (int j = 0; j <= i; ++j)
			{
				delete[] pstrDataKey[j];
				delete[] pstrDataValue[j];
			}
			delete[] pstrDataKey;
			delete[] pstrDataValue;
			delete[] converter;
			throw exception("[Implementation::action] can not format string");
		}
	}

	ACTION_PROC proc = (ACTION_PROC)m_pAction;
	bool result = proc(pstrDataKey, pstrDataValue, datacount);

	for (i = 0; i < datacount; ++i)
	{
		delete[] pstrDataKey[i];
		delete[] pstrDataValue[i];
	}
	delete[] pstrDataKey;
	delete[] pstrDataValue;

	delete[] converter;

	return result;
}
	
TG_END
