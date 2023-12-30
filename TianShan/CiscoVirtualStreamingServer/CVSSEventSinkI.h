#ifndef __CVSSEVENTSINKI_H__
#define __CVSSEVENTSINKI_H__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TsStreamer.h>

#include <TianShanDefines.h>
#include <NativeThread.h>
#include "FileLog.h"
#include "Variant.h"

namespace ZQTianShan{
namespace CVSS{

	//some known event field will be passed thru SSMH_EventSink() entry
#define	EventField_SessionId			"sessionID"
#define	EventField_PrevState			"prevState"
#define	EventField_CurrentState			"currentState"
#define	EventField_PrevSpeed			"prevSpeed"
#define	EventField_CurrentSpeed			"currentSpeed"
#define	EventField_PrevTimeOffset		"prevTimeOffset"
#define	EventField_CurrentTimeOffset	"currentTimeOffset"
#define EventField_TotalTimeOffset		"TotalTimeOffset"

#define	EventField_runningByteOffset	"runningBYteOffset"
#define	EventField_TotalbyteOffset		"totalByteOffset"
#define	EventField_currentStep			"playListcurrentStep"
#define	EventField_totalStep			"playlistTotalStep"

	//every speed value contain a denominator and a numerator
#define	EventField_SpeedDenom			"denominator"
#define	EventField_SpeedNumer			"numerator"

#define	EventField_ClientSessId			"ClientSessionID"
#define	EventField_PlaylistGuid			"playlistGuid"
#define EventField_ExiReason			"playlistExitReason"
#define EventField_ExitCode				"PLaylistExitCode"
#define	EventField_UserCtrlNum			"userCtrlNumber"
#define EventField_NextUserCtrlNum		"NextUserCtrlNumber"
#define	EventField_ItemFileName			"ItemFileName"
#define EventField_PlayScale			"Scale"
#define EventField_ItemOtherFileName	"ItemFileName-Other"
#define EventField_CurrentItemTimeOffset "currentItemTimeOffset"
#define EventField_StampUTC				"EventStamplUTC"
#define EventField_EventCSEQ			"EventSequence"

#define ENCRYPTION_ENABLE				"ecmEnable"
#define ENCRYPTION_VENDOR				"ecmVendor"
#define ENCRYPTION_ECM_PID				"ecmPid"
#define ENCRYPTION_CYCLE1				"ecmCycle1"
#define ENCRYPTION_CYCLE2				"ecmCycle2"
#define ENCRYPTION_FREQ1				"ecmFreq1"
#define ENCRYPTION_FREQ2				"ecmFreq2"
#define ENCRYPTION_DATACOUNT			"ecmDataCount"
#define ENCRYPTION_DATAPREFIX			"ecmDataPrefix"
#define ENCRYPTION_PNOFFSETPREFIX		"ecmDataPNoffsetPrefix"

#define	VSTRM_ITEM_PID					"vtrsm_item_PID"					//ushort

#define VSTRM_ITEM_PAUSELASTUTILNEXT	"vstrm_Pause_last_util_next"		//ushort

#define	PLI_CATALOG		"ItemCatalog"
#define PL_CATALOG		"PlayList"

#define EVENTCSEQ		"EventSeq"

typedef enum 
{
	E_NULL					=0,
	//edge events
	E_EDGE_EVENT_LOST		=1<<0,
	E_EDGE_REMOTE_SESSION_AVAILABLE=1<<1,
	E_EDGE_NO_LISTENER_FOUND=1<<2,

	//vod events
	E_VOD_EVENT_LOST		=1<<3,
	E_VOD_FORWARD_SPEED_CHANGES_DISABLED=1<<4,
	E_VOD_FORWARD_SPEED_CHANGES_ENABLED=1<<5,
	E_VOD_SPEED_CHANGE		=1<<6,	

	//events about playlist
	E_PLAYLIST_STARTED			=1<<8,
	E_PLAYLIST_STATECHANGED		=1<<9,		//vstrm session state changed
	E_PLAYLIST_SPEEDCHANGED		=1<<10,		//vstrm port speed changed
	E_PLAYLIST_INPROGRESS		=1<<11,		//item progress
	E_PLAYLIST_ITEMLOADED		=1<<12,		
	E_PLAYLIST_ITEMDONE			=1<<13,		//reach end of item	
	E_PLAYLIST_END				=1<<14,		//reach playlist end
	E_PLAYLIST_BEGIN			=1<<15,		//reach playlist begin
	E_PLAYLIST_SESSEXPIRED		=1<<16,		//vstrm session expire abnormally
	E_PLAYLIST_DESTROYED		=1<<17		//playlist destroyed

} EventType;

typedef struct 
{
	DWORD					type;
	::ZQ::common::Variant	param;
}listmem;

typedef struct CVSSEventList
{
	CVSSEventList()
	{
		InitializeCriticalSection(&m_CS);
		m_EventList.clear();
	}
	~CVSSEventList()
	{
		DeleteCriticalSection(&m_CS);
		m_EventList.clear();
	}

	typedef ::std::deque<listmem> paramlist;
	paramlist			m_EventList;

	CRITICAL_SECTION	m_CS;

	void PushBack(listmem &str)
	{
		//get lock
		EnterCriticalSection(&m_CS);

		m_EventList.push_back(str);

		//release lock
		LeaveCriticalSection(&m_CS);
		
	}

	void PopFront()
	{
		//get lock
		EnterCriticalSection(&m_CS);

		m_EventList.pop_front();

		//release lock
		LeaveCriticalSection(&m_CS);
	}

	listmem First()
	{
		listmem tmp;
		//get lock
		EnterCriticalSection(&m_CS);

		tmp = m_EventList.front();
		m_EventList.pop_front();
		//release lock
		LeaveCriticalSection(&m_CS);
		return tmp;
	}

	bool Empty()
	{
		return m_EventList.empty();
	}
}CVSSEventList;

class CVSSEventSinkI : public ::ZQ::common::NativeThread
{
public:
	CVSSEventSinkI(::ZQ::common::FileLog &fileLog, const char * topicManagerEndpoint);
	CVSSEventSinkI(::ZQ::common::FileLog &fileLog, const char * topicManagerEndpoint, ZQADAPTER_DECLTYPE  adpater);
	~CVSSEventSinkI();
	void	setAdapter(ZQADAPTER_DECLTYPE  adpater){_objAdapter = adpater;}
	void	registerEventSink();
	void	setTopicProxyString(std::string strTopicProxy)
	{
		_strTopicProxy=strTopicProxy; 
		connectToStormService();
	}
	bool	dispatchEvent(DWORD eventType, ::ZQ::common::Variant& params);
	int		run();
	void	NotifyBadConnection();
	bool	SetHandle();
	CVSSEventList _paramsList;

protected:
	bool	connectToStormService();

private:
	::ZQ::common::FileLog		&_fileLog;
	std::string					_strTopicProxy;
	bool						_bNetworkOK;
	bool						_bQuit;
	ZQADAPTER_DECLTYPE			_objAdapter;
	::TianShanIce::Streamer::PlaylistEventSinkPrx		_objPlaylistEventPrx;
	::TianShanIce::Streamer::StreamEventSinkPrx			_objStreamEventPrx;
	::TianShanIce::Streamer::StreamProgressSinkPrx		_objProgressEventPrx;

	::IceStorm::TopicManagerPrx							topicManagerPrx;
	HANDLE												_hBadConn;
};

}//namespace CVSS
}//namespace ZQTianShan

#endif __CVSSEVENTSINKI_H__