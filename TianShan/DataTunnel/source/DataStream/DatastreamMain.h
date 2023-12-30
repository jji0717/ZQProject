#pragma  once

#include "BaseZQServiceApplication.h"

class DataStreamShell : public ZQLIB::BaseZQServiceApplication 
{
public:

	DataStreamShell();
	virtual ~DataStreamShell();

public:

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

};
//////////////////////////////////////////////////////////////////////////