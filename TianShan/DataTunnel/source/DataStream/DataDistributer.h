// DataDistributer.h: interface for the DataDistributer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATADISTRIBUTER_H__484D3D38_D71C_4FDD_B6BC_49D40BBF7A6D__INCLUDED_)
#define AFX_DATADISTRIBUTER_H__484D3D38_D71C_4FDD_B6BC_49D40BBF7A6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SyncUtil.h"

namespace DataStream {

class DistItem {
public:
	virtual void distribute(void* param) = 0;
};

class DataDistributer  
{
public:
	DataDistributer(unsigned int period, DistItem* item, bool once = false);
	virtual ~DataDistributer();
	bool start(void* param = NULL);
	void stop();

	unsigned int getPeriod() const
	{
		return _period;
	}
	
	bool isRunning()
	{
		return _timerId != 0;
	}

protected:
	static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, 
		DWORD dw1, DWORD dw2);

protected:
	unsigned int	_period;
	DistItem*		_item;
	bool			_once;
	unsigned int	_timerId;
	void*			_param;
};

} // namespace DataStream {

#endif // !defined(AFX_DATADISTRIBUTER_H__484D3D38_D71C_4FDD_B6BC_49D40BBF7A6D__INCLUDED_)
