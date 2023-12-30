#ifndef __ssm_ngod2_d5_update_header_file_h__
#define __ssm_ngod2_d5_update_header_file_h__

#include <VrepSpeaker.h>
#include "NgodEnv.h"
#include "SelectionResourceManager.h"

namespace NGOD
{


class D5Speaker;

class D5StateSinker : public ZQ::Vrep::StateMachine::Monitor
{
public:
	D5StateSinker( NgodEnv& environment , D5Speaker& speaker );
	virtual ~D5StateSinker();

protected:

	virtual void onStateChanged( ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to) ;

	virtual void onEvent( ZQ::Vrep::Event e) ;

private:
	NgodEnv&						mEnv;
	D5Speaker&						mD5Speaker;
	ZQ::Vrep::StateDescriptor		mCurState;
};

class D5Speaker : public ZQ::common::NativeThread , public ZQ::Vrep::Speaker
{
public:

	D5Speaker( NgodEnv& environment , NgodResourceManager&selManager , ZQ::common::NativeThreadPool& pool);

	virtual ~D5Speaker(void);

public:

	bool		start();

	void		stop();	

	void		onConnected( );

	void		onDisconnected( );

	void		onSpigotStateChange( const std::string& netId , bool bUp );
	
	void		onSpigotStateChange(  );

protected:

	int			run( );
	
	
	void		updateBWInfo( );
	

protected:

	bool		initSpeakerConf( );

	void		initOutputPort();

	void		sendServiceState( bool bUp );	

	void		sendBWChangedInfo(  const std::string& sopName , const std::string& sopGroupName ,int32 portId ,  const std::string& sourceIp , int32 totalBW , int32 availBW , int32 cost );

	void		sendVolumesInfo( );

	void		sendMaxStreamsInfo( );

	

	virtual void onStateChanged( ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to);

	std::string	getASopName( ) const;

	std::string getComponentName( ) const;
	

private:

	void		setRouteAndHopServerInfo( const std::string& sopGroupName , ZQ::Vrep::Route& r , ZQ::Vrep::NextHopServer& n );	

	int32		getSopPortId( const std::string& netId) const;

	void		getVolumeInfo( const std::string& sopName , ZQ::Vrep::Volumes& vols ) const;

	bool		checkBwUsage();

	struct BWUsage 
	{
		int64 totalBW;
		int64 availBW;
		BWUsage()
		{
			totalBW = 0;
			availBW = 0;
		}
	};

	bool		isBWChanged( const BWUsage& last , const BWUsage& current );
	bool		isChanged( int64 last , int64 current);

	std::string getGroupNameOfSop( const std::string& sopName ) const;

private:

	//ZQ::Vrep::Speaker									mSpeaker;	
	
	NgodEnv&				mEnv;
	NgodResourceManager&	mSelManager;

	typedef std::map<std::string , int>	PORTIDMAP;
	/*               netid , portId */
	PORTIDMAP			mSopOutputPorts;

	struct VolumeInformation 
	{
		int64		capacity;
		int			portId;
	};
	typedef std::map<std::string,VolumeInformation>	VOLUMECAPACITYMAP;
	/*				volume name , capacity*/
	typedef std::map<std::string,VOLUMECAPACITYMAP> SOPVOLCAPACITYMAP;
	/*				sop name , volume capacity	*/
	
	SOPVOLCAPACITYMAP	mSopVolInfos;

	ZQ::common::Mutex	mMutex;
	ZQ::common::Cond	mCond;
	bool				mbQuit;
	
	typedef std::map<std::string ,  BWUsage> SOPBWUSAGEMAP;
	SOPBWUSAGEMAP		mSopBwUsage;

	std::vector<ZQ::Vrep::Speaker*>	mOtherD5Speakers;
};

}

#endif//__ssm_ngod2_d5_update_header_file_h__

