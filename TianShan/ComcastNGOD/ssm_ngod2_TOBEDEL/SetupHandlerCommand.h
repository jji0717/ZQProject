// SetupHandlerCommand.h: interface for the SetupHandlerCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SETUPHANDLERCOMMAND_H__3DA45C0D_01F1_4496_AC99_674822D42CC3__INCLUDED_)
#define AFX_SETUPHANDLERCOMMAND_H__3DA45C0D_01F1_4496_AC99_674822D42CC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <set>
#include "RequestHandler.h"
#include "setuphandler.h"


class SetupHandlerCommand
{
public:
	SetupHandlerCommand( SetupHandler& handler );

	typedef std::vector<std::string>		STRVEC;
	typedef std::set<std::string>			STRSET;

	std::string 			logVectorString(const STRVEC& strVec );	

protected:	
	SetupHandler& _handler;
};

class SelectStreamerCommand :public SetupHandlerCommand
{
public:
	SelectStreamerCommand(  SetupHandler& handler );
	~SelectStreamerCommand( );
public:
	//select an available streamer 
	TianShanIce::Streamer::StreamSmithAdminPrx	execute( const std::string& sopName , int32 requestBandwidth , SetupHandler::VolumeInfoToBuildList& vi );

	void										releaseStreamer( bool addPenalty  = false  );

	std::string									getSelectedNetId( ) const
	{
		return _selectedStreamerNetId;
	}
	std::string									getSelectedEndpoint() const
	{
		return _selectedEndpoint;
	}
	std::string									getSelectedImportChannel( ) const
	{
		return _selectedImportChannelName;
	}
	
protected:

	TianShanIce::Streamer::StreamSmithAdminPrx	getStreamerFromSop( int32 bandwidth , NGOD2::SOPRestriction::SopHolder& sop ,  SetupHandler::VolumeInfoToBuildList& vi );


private:

	std::map<std::string,NGOD2::PassThruStreaming::ImportChannelHolder>::iterator getImportChannel( const std::string& channelName );
	void										releaseAllocatedResource( );

	bool										isValid( std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder>::iterator );

	void										clearSelectedInfo( );
	
	typedef struct  _SelectedStreamerInfo
	{
		int32					weight;		//weight of each streamer , high weight has high probability to be selected
		std::vector<NGOD2::Sop::StreamerHolder>::iterator	itStreamer;
		std::map<std::string , NGOD2::PassThruStreaming::ImportChannelHolder>::iterator itChannel;
	}SelectedStreamerInfo;
	struct SelectedStreamerSort
	{
		bool operator()(const SelectedStreamerInfo& rpStart, const SelectedStreamerInfo& rpEnd) const
		{
			return rpStart.weight > rpEnd.weight ;
		}
	};

private:
	std::string						_selectedEndpoint;
	std::string						_selectedSop;
	std::string						_selectedStreamerNetId;
	std::string						_selectedImportChannelName;
	int32							_requestBandwidth;
};


class GetReferrenceAEListCommand :public SetupHandlerCommand
{
public:
	GetReferrenceAEListCommand( SetupHandler& handler );
	~GetReferrenceAEListCommand( );
public:
	///query LAM to get the asset information
	///@return true if successfully , vice versa
	///@param 
	bool					execute( const std::string& sopName, bool maybeNeedCache ,  long& maxBandwidth );

private:

	AEInfo3Collection		dummy( );
	
	LAMFacadePrx			getLAMServer( );

	bool					getAeInfo( LAMFacadePrx lamPrx , const std::string& sopName , SetupHandler::PidPaidInfo& info );

	bool					narrowContenVolume( bool maybeNeedCache ,  SetupHandler::PidPaidInfoS& infos  );

	//bool					intersectForPIDPAID( SetupHandler::PidPaidInfo& info );

	//void					clearRedundancyVolume( SetupHandler::AeVolumeInfoSet& volumeList , std::set<std::string>& commonSet );

	void					randomStoragelist( SetupHandler::VolumeInfoToBuildListS& storageList );

	void					getAvailabeVolumesFromAssetInfos( const SetupHandler::PidPaidInfoS& infos , std::vector<std::string>& volumeSet );

	void					buildVolumeContext( const std::string& volumeName , const SetupHandler::PidPaidInfoS& assetInfos  );

	void					getCandidateVolumes( const  std::string& strSop , std::set<std::string>& volumes );

private:
	struct storageListSort
	{
		bool operator()( const SetupHandler::VolumeInfoToBuildList& v1 , const SetupHandler::VolumeInfoToBuildList& v2 );		
	};

private:


	long					_maxBandwidth;
	std::set<std::string>	mSpecifiedVolumes;//volumes that specified by asset info

	SetupHandler::VolumeInfoToBuildListS mSpecifiedVolumeContexts;
	SetupHandler::VolumeInfoToBuildListS mOtherVolumeContexts;
	bool					mbAssetCanStreamingFromLocal;
	bool					mbAssetCanStreamingFromRemote;
};

class RenderPlaylistCommand :public SetupHandlerCommand
{
public:
	RenderPlaylistCommand( SetupHandler& handler );
	~RenderPlaylistCommand( );

public:
	bool		execute( TianShanIce::Streamer::PlaylistPrx streamPrx , 
						const SetupHandler::VolumeInfoToBuildList& volumeInfo,
						NGODr2c1::PlaylistItemSetupInfos& setupInfos );

protected:

	void		fillEncryptionData( const std::string&			volumeName,
									const std::string&			strProviderId,
									const std::string&			strProviderAssetId,
									::TianShanIce::ValueMap&	infoPrivateData , 
									SetupHandler::ECMDataMAP&	ecmDatas );
};

class PrepareItemInfoCommand :public SetupHandlerCommand
{
public:
	PrepareItemInfoCommand( SetupHandler& handler );
	~PrepareItemInfoCommand( );
public:
	
	bool		execute( const std::string& strURL , const std::string& clientId );
};


class PrepareEncryptionData : public SetupHandlerCommand
{
public:
	PrepareEncryptionData( SetupHandler& handler );
	~PrepareEncryptionData( );
public:

	bool		execute( const char* pContent , SetupHandler::ECMDataMAP& ecmDatas );

};


#endif // !defined(AFX_SETUPHANDLERCOMMAND_H__3DA45C0D_01F1_4496_AC99_674822D42CC3__INCLUDED_)
