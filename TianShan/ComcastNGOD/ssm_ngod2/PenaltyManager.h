// PenaltyManager.h: interface for the PenaltyManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PENALTYMANAGER_H__A8DE7513_3AFC_4C9D_A1B4_ED9C5BE655BC__INCLUDED_)
#define AFX_PENALTYMANAGER_H__A8DE7513_3AFC_4C9D_A1B4_ED9C5BE655BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <NativeThreadPool.h>
#include <list>


class NGODEnv;

class PenaltyManager  : public ZQ::common::NativeThread
{
public:

	PenaltyManager(NGODEnv& env);
	
	virtual ~PenaltyManager();

public:
	void		decreasePenalty( const std::string& streamerNetId = "" );
	void		Stop( );
	int			run(void);

private:

	NGODEnv&				_env;
	bool					_bQuit;
	HANDLE					_hEvent;
	ZQ::common::Mutex		_locker;
	typedef std::list<std::string> STREMERS;
	STREMERS				_streamers;

};

#endif // !defined(AFX_PENALTYMANAGER_H__A8DE7513_3AFC_4C9D_A1B4_ED9C5BE655BC__INCLUDED_)
