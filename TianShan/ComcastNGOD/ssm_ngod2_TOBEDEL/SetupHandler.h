#ifndef __SetupHandler_H__
#define __SetupHandler_H__

#include <vector>
#include <string>
#include <set>
#include "RequestHandler.h"
#include "SelectionCommand.h"


#define	OPERATION_OK						1

#define OPERATION_ERROR						-1

#define OPERATION_NOT_ACCEPTABLE			-406
#define OPERATION_BAD_PARAMETER				-451
#define OPERATION_NOT_ENOUGH_BW				-453
#define OPERATION_SESS_NOT_FOUND			-454
#define OPERATION_INVALID_RANGE				-457

#define OPERATION_SETUP_NO_RESPONSE			-770
#define OPERATION_SETUP_ASSET_NOT_FOUND		-771
#define OPERATION_SETUP_SOP_NA				-772
#define OPERATION_SETUP_UNKNOWN_SOPGROUP	-773
#define OPERATION_SETUP_UNKNONW_SOPNAME		-774
#define OPERATION_SETUP_INSUFFICIENT_VOLBW	-775
#define OPERATION_SETUP_INFUFFICIENT_NETBW	-776
#define OPERATION_SETUP_INVALID_REQUEST		-777


class SetupHandler : public RequestHandler
{
	friend class SelectStreamerCommand;
	friend class RenderPlaylistCommand;
	friend class PrepareItemInfoCommand;
	friend class PrepareEncryptionData;
	friend class GetReferrenceAEListCommand;
	
public:
	typedef IceUtil::Handle<SetupHandler> Ptr;

	SetupHandler( NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq );
	virtual ~SetupHandler();
	virtual	RequestProcessResult		process();
	/*void    updateFailSessions();*/
	
protected:	

	void			composeResource(const std::string& strKey , const std::string& value , TianShanIce::SRM::Resource& res);
	void			composeResource(const std::string& strKey , long value , TianShanIce::SRM::Resource& res);
	void			composeResource(const std::string& strKey , int value , TianShanIce::SRM::Resource& res);
	
	void			updateResourceMap( const TianShanIce::SRM::ResourceType& type ,  const TianShanIce::SRM::Resource& res , TianShanIce::SRM::ResourceMap& resMap);						

	void			getTransportDetail( const std::string& strTransportdata,
								std::string& strSop , long& bandwidth , int& clientPort , 
								std::string& destIp , std::string& destMac , int& sourcePort );

	std::string		prepareNatInfo( const std::string& pokeHoleId );

	void			updateCounters( );

private:

	TianShanIce::Streamer::StreamSmithAdminPrx	connectStreamSmith(const std::string& strEndpoint);

	bool										prepareAssetInfo( const std::string& url , const std::string& clientId );

	bool										prepareEncryptionData( const char* pContent );

	bool										createStream( );

	bool										renderPlaylist( NGODr2c1::PlaylistItemSetupInfos& setupInfos );
	
	void										fillEncryptionData(	const std::string&	volumeName,	const std::string&	 pid, const std::string&	paid,	
																	::TianShanIce::ValueMap&	infoPrivateData );

public:
	
	typedef struct _tagECMData
	{
		int							iProgNum;
		int							iFrq1;
		std::vector<int>			vecKeyOffset;
		std::vector<std::string>	vecKeys;
	}ECMData;

	typedef std::map<std::string,ECMData>	ECMDataMAP;	

	ECMDataMAP								mEcmDatas;
	TianShanIce::SRM::ResourceMap			mResMap;

protected:
	
	SelectionIntention			mIntention;
	StreamerSelection			mSelStreamer;

	std::string					mFirstPaid;
	bool						mbRemoteSession;
};


#endif // __SetupHandler_H__
