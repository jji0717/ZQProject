#ifndef __d4_update_header_file_h__
#define __d4_update_header_file_h__

#include "VrepSpeaker.h"
#include <set>
#include "CPCImpl.h"
#include "ContentStore.h"

typedef std::vector<std::string>AdvertiseMethods;
typedef struct 
{
	std::string		listener;
	int32			enableD4;
	int32           advInterval;
	std::string		strA3Interface;
	std::string     strStreamZone;
	std::string		strRouteAddr;
	AdvertiseMethods AdMethod;
	std::string     strVolumeId;
	int32           portId;
	std::string     ServerIp;
	std::string     ServerName;
}D4MessageConfig;

class D4Speaker;
//class ContentStore;

using namespace ::TianShanIce::ContentProvision;

class D4StateSinker : public ZQ::Vrep::StateMachine::Monitor
{
public:
	D4StateSinker( D4Speaker& speaker );
	virtual ~D4StateSinker();

protected:

	virtual void onStateChanged( ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to) ;

	virtual void onEvent( ZQ::Vrep::Event e) ;

private:
	D4Speaker&						mD4Speaker;
	ZQ::Vrep::StateDescriptor		mCurState;
};

struct MethodBWInfo
{
	std::string method;
	int64       totalBw;
	int64       allocBw;
	int64       availBw;
	int32       cost;
	ZQ::Vrep::byte        bcap;
};

class D4Speaker : public ZQ::common::NativeThread
{
public:

	D4Speaker(ZQ::common::Log& log,ZQ::common::NativeThreadPool& pool, ::TianShanIce::Storage::ContentStoreExPrx& csPrx, D4MessageConfig& d4MsgCfg);

	virtual ~D4Speaker(void);

public:

	bool		start();

	void		stop();	

	void		onConnected( );

	void		onDisconnected( );

	ZQ::Vrep::byte        decideCapType(std::string methodType);
	void        onSpigotStateChange(std::map<std::string, CPCImpl::CPEInst>& cpemap, bool bUp);
	void		onSpigotStateChange(const std::vector<CPCImpl::CPEInst>& cpes , bool bUp );

protected:
	D4MessageConfig _d4MsgCfg;

protected:

	int			run( );
	
	bool					mbQuit;

	ZQ::common::Semaphore	mSem;

protected:

	bool		initSpeakerConf( );

	void		sendServiceState( bool bUp );	

	void		sendBWChangedInfo(int32 totalBW , int32 availBW , int32 cost, ZQ::Vrep::byte cap, int32 readBW);
	bool        getVolumeInfo(TianShanIce::Storage::VolumeInfos& volInfos);

private:

	ZQ::Vrep::Speaker									mSpeaker;	
	int64                                               _hisTotalBW;
	int64                                               _hisAllocBW;
	std::string                                         _strNextHop;
	std::string                                         _strListenerIp;
	u_short                                             _ListenerPort;
	std::vector<MethodBWInfo>                           _BWInfoVec;
	::TianShanIce::Storage::ContentStoreExPrx           _csPrx;
	bool                                                _bReportVolYet;
	bool                                                _bStartAlready;
};


#endif//__d4_update_header_file_h__

