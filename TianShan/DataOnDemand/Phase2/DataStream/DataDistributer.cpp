// DataDistributer.cpp: implementation of the DataDistributer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDistributer.h"
#include "Mmsystem.h"

namespace DataStream {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataDistributer::DataDistributer(unsigned int period, 
								 DistItem* item, bool once):
	_period(period), _item(item), _once(once)
{
	assert(_item);
	_param = NULL;
	_timerId = 0;
}

DataDistributer::~DataDistributer()
{
	stop();
}
	
bool DataDistributer::start(void* param)
{
	assert(_timerId == 0);

	_param = param;
	_timerId = timeSetEvent(_period, 1, (LPTIMECALLBACK)TimeProc, (DWORD_PTR)this, 
		_once ? TIME_ONESHOT : TIME_PERIODIC);
	if (_timerId == 0)
		return false;

	return true;
}

void DataDistributer::stop()
{
	if (_timerId == 0)
		return;

	timeKillEvent(_timerId);
	_timerId = 0;
}

void CALLBACK DataDistributer::TimeProc(UINT uID, UINT uMsg, DWORD dwUser, 
	DWORD dw1, DWORD dw2)
{
	DataDistributer* dist = (DataDistributer* )dwUser;	
	dist->_item->distribute(dist->_param);
}

} // namespace DataStream {
