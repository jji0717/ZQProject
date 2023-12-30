
#ifndef _ZQ_IMPLEMETATION_H_
#define _ZQ_IMPLEMETATION_H_
	
#include <windows.h>
#include "alarmcommon.h"
#include "datasource.h"

#define MAX_DATA_LINE_COUNT			256
#define MAX_DATA_LINE_BUFFER_LEN	256

TG_BEGIN
	
class Implementation
{
private:
	HMODULE	m_hModule;
	void*	m_pInit;
	void*	m_pUnInit;
	void*	m_pAction;


public:
	DataSource&	m_source;

	Implementation(const char* pluginfile, DataSource& datasource);
	~Implementation();
	
	bool init();
	void uninit();
	bool action(size_t sourceid, size_t syntaxid, const char* line,
		char* const * params, size_t paramcount);
};
	
TG_END
	
#endif//_ZQ_IMPLEMETATION_H_
