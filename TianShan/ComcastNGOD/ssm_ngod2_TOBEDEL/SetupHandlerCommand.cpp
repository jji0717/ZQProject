// SetupHandlerCommand.cpp: implementation of the SetupHandlerCommand class.
//
//////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <urlstr.h>
#include "SetupHandlerCommand.h"
#include <stdlib.h>
#include <TianShanIceHelper.h>


#ifdef HANDLERLOG
	#undef HANDLERLOG
    #undef HANDLEREVENTLOG
#endif

#ifdef HANDLERLOGFMT
	#undef HANDLERLOGFMT
#endif

#define HANDLERLOG	_handler._ssmNGODr2c1._fileLog
#define HANDLEREVENTLOG _handler._ssmNGODr2c1._sentryLog
#define HANDLERLOGFMT(_X, _T)	CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s) " _T), _handler._session.c_str(), _handler._sequence.c_str(), _handler._pRequest, _handler._method.c_str()


class MyRandom 
{
public:
	ptrdiff_t operator() (ptrdiff_t max) 
	{
		Ice::Long timeStamp = ZQTianShan::now();		
		unsigned int seed = static_cast<unsigned int>(timeStamp);
		
		srand( seed );

		if( max  > 0 )
			return rand()%max;
		else 
			return 0;
	}
};

/*--------------------------------------------------------------------
PrepareItemInfoCommand
--------------------------------------------------------------------*/
PrepareItemInfoCommand::PrepareItemInfoCommand( SetupHandler& handler )
:SetupHandlerCommand(handler)
{	
}

PrepareItemInfoCommand::~PrepareItemInfoCommand( )
{
}

bool PrepareItemInfoCommand::execute( const std::string& strURL , const std::string& clientId)
{	
	ZQ::common::URLStr uri( strURL.c_str(), true );
	char szItemBuf[256];
	int iItemCount = 0;
	SetupHandler::PidPaidInfoS&	plInfo = _handler.playlistInfo;	
	//SetupHandler::ItemInfoS& infos = _handler._itemInfos;
	const char* pID = NULL;
	do 
	{
		sprintf(szItemBuf,"item%d",iItemCount);		pID	=	uri.getVar(szItemBuf);
		
		sprintf(szItemBuf,"cueIn%d",iItemCount);	const char* pCuein = uri.getVar(szItemBuf);
		
		sprintf(szItemBuf,"cueOut%d",iItemCount);	const char* pCueout = uri.getVar(szItemBuf);
		
		sprintf(szItemBuf, "DisableF%d", iItemCount);const char* pDisableF = uri.getVar(szItemBuf);
		
		sprintf(szItemBuf, "DisableR%d", iItemCount);const char* pDisableR = uri.getVar(szItemBuf);
		
		sprintf(szItemBuf, "DisableP%d", iItemCount);const char* pDisableP = uri.getVar(szItemBuf);
		
		if ( pID && pID[0] != 0 ) 
		{
			SetupHandler::PidPaidInfo info;
			//SetupHandler::PAIDandItemInfo paInfo;
			//SetupHandler::ItemInfo&	infoElement = paInfo.itemInfo;			
			if ( pCuein && strlen(pCuein)>0 )
				info.cuein = atoi(pCuein);
			
			if ( pCuein && strlen(pCueout)>0 ) 
				info.cueout = atoi(pCueout);				
			
			if (pDisableF && atoi(pDisableF) == 1)
				info.ctrlMark += TianShanIce::Streamer::PLISFlagNoFF;
			
			if (pDisableR && atoi(pDisableR) == 1)
				info.ctrlMark += TianShanIce::Streamer::PLISFlagNoRew;
			
			if (pDisableP && atoi(pDisableP) == 1)
				info.ctrlMark += TianShanIce::Streamer::PLISFlagNoPause;
			
			//chop the id into pid and paid
			const char* pDelimiter = strstr( pID , "#");
			if( pDelimiter )
			{
				info.pid.assign( pID , pDelimiter - pID);
				info.paid.assign( pDelimiter+1 );
			}
			else
			{
				info.pid = pID;
				info.paid.clear();				
				//add also log an error here
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1,HANDLERLOGFMT(PrepareItemInfoCommand, "invalid PID PAID[%s]"), pID);
				std::string notice_str;
				notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING);
				_handler.setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
				return	false;
			}	

			info.sid	= clientId;
			//ccMap[ std::string(pID) ] = infoElement;
			plInfo.push_back(info);
		}		
		iItemCount++;		
	} while( pID && strlen(pID)> 0 );
	
	return true;
}

/*-----------------------------------------------------------------------------
SelectStreamerCommand
-----------------------------------------------------------------------------*/
const int32		max_weight	= 10000;
const int32		out_of_service_weight = 0;

TianShanIce::Streamer::StreamSmithAdminPrx SelectStreamerCommand::getStreamerFromSop(  int32 requestBandwidth , NGOD2::SOPRestriction::SopHolder& sop ,  SetupHandler::VolumeInfoToBuildList& vi )
{
	//step 1
	//collect streamer by volume name
	//NGOD2::Sop::StreamerHolder
	std::vector<std::vector<NGOD2::Sop::StreamerHolder>::iterator> selectedStreamers;
	std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = sop._streamerDatas.begin();
	
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT( SelectStreamerCommand, "selecting streamers by storage[%s]" ), vi.netId.c_str() );

	for( ; itStreamer != sop._streamerDatas.end() ; itStreamer ++ )
	{
#pragma message(__MSGLOC__"TODO: if the streamer not supported by all volumes under the Storage ??? ")
		if( itStreamer->_storageNetId == vi.netId )
		{
			// add by zjm
			if( (itStreamer->_penalty > 0) || (itStreamer->_enabled <= 0) || (!itStreamer->_bReplicaStatus) || (!itStreamer->_streamServicePrx) )
			{
				HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SelectStreamerCommand,"skip streamer[%s] with storage[%s] due to it's penalty[%d], status[%s] connected[%s]"),
					itStreamer->_netId.c_str(), vi.netId.c_str(),
					itStreamer->_penalty ,
					(itStreamer->_enabled <=0) ? "disabled": ( itStreamer->_bReplicaStatus ? "up":"down"),
					itStreamer->_streamServicePrx ? "true":"false");
			}
			else
			{//				
				selectedStreamers.push_back( itStreamer );
			}
		}
	}
	if( selectedStreamers.size() == 0 )
	{
		
		char szLocalBuf[1024];
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(SelectStreamerCommand,"no available streamer for storage[%s]"), vi.netId.c_str()  );

		HANDLERLOG(ZQ::common::Log::L_WARNING, szLocalBuf	);
		
		std::string notice_str;
		notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING);
		_handler.setResponseString( RESPONSE_SSF_NORESPONSE , notice_str.c_str());
	}


	bool bUseReportBandwidth = _sopConfig._sopRestrict._enableReportedImportChannelBandWidth >= 1;

	std::map< std::string, NGOD2::PassThruStreaming::ImportChannelHolder>::iterator itChannel = _ngodConfig._passThruStreaming._importChannelDatas.end();
	
	std::vector<SelectedStreamerInfo> slStreamerInfos;
	std::vector<std::vector<NGOD2::Sop::StreamerHolder>::iterator>::const_iterator itSelected = selectedStreamers.begin();
	for( ; itSelected != selectedStreamers.end() ; itSelected ++ )
	{
		int32	importChannelWight	= max_weight;
		int32	streamerWeight		= max_weight;
		
		itStreamer = *itSelected;
		
		//calculate the weight of each streamer and maybe import channel will be included
		if( vi.plType != SetupHandler::PLAYLIST_STREAMING_FROM_LOCAL )
		{//if all the content can't stream from LOCAL storage , calculate the import channel weight
			//get the import channel
			std::string channelName = "";
			if( bUseReportBandwidth )
			{
				channelName = itStreamer->_netId;
			}
			else
			{
				channelName	= itStreamer->_importChannel;
			}

			itChannel = getImportChannel( channelName );
			if( !isValid( itChannel ) )
			{
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SelectStreamerCommand, "no such an ImportChannel[%s] is defined in configuration"),
					itStreamer->_importChannel .c_str() );
				HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SelectStreamerCommand, "no such an ImportChannel[%s] is defined in configuration"),
					itStreamer->_importChannel .c_str() );
				continue;
			}
			NGOD2::PassThruStreaming::ImportChannelHolder& imChannel = itChannel->second;

			int64 usedBandWidth = imChannel._usedBandwidth;
			if( (imChannel._reportUsedBandwidth != 0 )&& (usedBandWidth < imChannel._reportUsedBandwidth ) )
			{
				usedBandWidth = imChannel._reportUsedBandwidth;
				HANDLERLOG(ZQ::common::Log::L_INFO,	HANDLERLOGFMT(SelectStreamerCommand, "usedBandwidth of ImportChannel[%s], calculation result [%lld], reported [%lld], taking [%lld]"),
					channelName.c_str(),				
					imChannel._usedBandwidth,
					imChannel._reportUsedBandwidth ,
					usedBandWidth);
			}

			//calculate the import channel weight
			//importChannelWight		= 
			int64 maxBandwidth = imChannel._maxBandwidth;
			if( imChannel._reportTotalBandwidth > 0 )
				maxBandwidth = imChannel._reportTotalBandwidth < maxBandwidth ? maxBandwidth : imChannel._reportTotalBandwidth;
				//take the bigger one
				
			if( ( requestBandwidth + usedBandWidth ) > maxBandwidth )
			{//out of service cost
				HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SelectStreamerCommand,"ImportChannel[%s] has no enough bandwidth to supply the session: maxBandwidth[%lld] usedBandwidth[%lld], reportMaxBW[%lld] reportUsedBW[%lld], requestBandwidth[%d] "),
					imChannel._name.c_str() , 
					imChannel._maxBandwidth,
					imChannel._usedBandwidth,
					imChannel._reportTotalBandwidth,
					imChannel._reportUsedBandwidth,
					requestBandwidth);
				continue;
			}
			if( imChannel._usedImport + 1 > imChannel._maxImport )
			{
				HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SelectStreamerCommand,"ImportChannel[%s] has no enough session quota: maxImport[%d] usedImport[%d]"),
					imChannel._name.c_str() , 
					imChannel._maxImport,
					imChannel._usedImport);
				continue;
			}
			
			

			int32 bwWeight		= static_cast<int32>(( ( maxBandwidth -  usedBandWidth ) * max_weight  ) / maxBandwidth);
			int32 countWeight	= static_cast<int32>(( ( imChannel._maxImport - imChannel._usedImport ) * max_weight ) / imChannel._maxImport);
			importChannelWight	= bwWeight > countWeight ? countWeight : bwWeight ;
			HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SelectStreamerCommand,
				"got ImportChannel[%s] weight[%d] with maxBW[%lld] usedBW[%lld],localMaxBW[%lld] localUsedBW[%lld], reportMaxBW[%lld] reportUsedBW[%lld], totalImportCount[%d] usedImportCount[%d] requestBW[%d]"),
				imChannel._name.c_str() , 
				importChannelWight,
				maxBandwidth,
				usedBandWidth,
				imChannel._maxBandwidth,
				imChannel._usedBandwidth,
				imChannel._reportTotalBandwidth,
				imChannel._reportUsedBandwidth,
				imChannel._maxImport,
				imChannel._usedImport,
				requestBandwidth);
		}
		else
		{
			HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(SelectStreamerCommand,"no importing is needed as having local asset replica, refer to select this by highering its weight"));
		}
		//calculate the streamer weight
		if( ( itStreamer->_usedBandwidth + requestBandwidth ) > itStreamer->_totalBandwidth || itStreamer->_usedStream + 1 > itStreamer->_maxStream)
		{
			HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SelectStreamerCommand,"streamer[%s] is not able to host the session, totalBW[%lld] usedBW[%lld] requestBW[%d] totalStreams[%d] usedStreams[%d]"),
				itStreamer->_netId.c_str(),
				itStreamer->_totalBandwidth,
				itStreamer->_usedBandwidth,
				requestBandwidth,
				itStreamer->_maxStream,
				itStreamer->_usedStream);
			continue;
		}
// 		if( itStreamer->_usedStream + 1 > itStreamer->_maxStream )
// 		{
// 			HANDLERLOG(ZQ::common::Log::L_WARNING,
// 				HANDLERLOGFMT(SelectStreamerCommand,"not enough Streamer[%s] can't perform more stream,totalStreams[%d] usedStreams[%d]"),
// 				itStreamer->_netId.c_str(),
// 				itStreamer->_maxStream,
// 				itStreamer->_usedStream);
// 			continue;
// 		}
		int32 bwWeight		= static_cast<int32>( ( itStreamer->_totalBandwidth - itStreamer->_usedBandwidth ) *max_weight / itStreamer->_totalBandwidth );
		int32 countWeight	= static_cast<int32>( (itStreamer->_maxStream - itStreamer->_usedStream ) * max_weight / itStreamer->_maxStream );
		streamerWeight		= bwWeight > countWeight ? countWeight : bwWeight;
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SelectStreamerCommand,"got streamer[%s], weight[%d] totalBW[%lld] usedBW[%lld] totalStream[%d] usedStream[%d] requestBW[%d]"),
			itStreamer->_netId.c_str() ,
			streamerWeight,
			itStreamer->_totalBandwidth,
			itStreamer->_usedBandwidth,
			itStreamer->_maxStream,
			itStreamer->_usedStream,
			requestBandwidth);

		SelectedStreamerInfo info;
		info.weight = streamerWeight > importChannelWight ? importChannelWight : streamerWeight;
		info.itStreamer		= itStreamer;
		info.itChannel		= itChannel;
		slStreamerInfos.push_back(info);
	}
	if( slStreamerInfos.size() == 0 )
	{
		char szLocalBuf[1024];
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1,
		HANDLERLOGFMT(SelectStreamerCommand,"failed to select streamer by storage[%s]"), 
		vi.netId.c_str() );
		HANDLERLOG(ZQ::common::Log::L_WARNING, szLocalBuf);
		std::string notice_str;
		notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING );
		_handler.setResponseString( RESPONSE_SSF_NO_NETWORKBANDWIDTH , notice_str.c_str());
		return NULL;
	}
	
	std::random_shuffle(slStreamerInfos.begin() ,slStreamerInfos.end(),MyRandom() );
	std::sort(slStreamerInfos.begin() , slStreamerInfos.end(),SelectedStreamerSort() );
	itStreamer	= slStreamerInfos.begin()->itStreamer;
	itChannel	= slStreamerInfos.begin()->itChannel;
	{
		_selectedSop				=	sop._name;
		_selectedEndpoint			=	itStreamer->_serviceEndpoint;
		_selectedStreamerNetId		=	itStreamer->_netId;
		_selectedImportChannelName	=	 isValid(itChannel) ?  itChannel->second._name : "";

		_requestBandwidth			=	requestBandwidth;

		itStreamer->_usedBandwidth += requestBandwidth;
		itStreamer->_usedStream ++;

		//add the remote session count record
		if( isValid(itChannel))
		{
			itChannel->second._usedBandwidth += requestBandwidth;
			itChannel->second._usedImport++;
		}
	}
	HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(SelectStreamerCommand,"selected streamer[%s], weight[%d]"), itStreamer->_netId.c_str() , slStreamerInfos.begin()->weight );
	return itStreamer->_streamServicePrx;
}
std::map<std::string , NGOD2::PassThruStreaming::ImportChannelHolder>::iterator SelectStreamerCommand::getImportChannel( 
																							const std::string& strChannelName )
{	
	std::string channelName = strChannelName;
	std::string::size_type pos = channelName.find("/");
	if( pos != std::string::npos )
	{
		channelName = channelName.substr( 0, pos );
	}
	std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder>& icHolder = _ngodConfig._passThruStreaming._importChannelDatas;
	std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder>::iterator it = icHolder.find(channelName);	
	return it;
}

bool SelectStreamerCommand::isValid( std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder >::iterator it )
{
	std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder>& icHolder = _ngodConfig._passThruStreaming._importChannelDatas;
	return it != icHolder.end();
}

TianShanIce::Streamer::StreamSmithAdminPrx SelectStreamerCommand::execute( const std::string& sopName , int32 requestBandwidth , SetupHandler::VolumeInfoToBuildList& vi )
{	
	ZQ::common::MutexGuard gd( _handler._ssmNGODr2c1._lockSopMap );
	ZQ::common::Config::Holder<NGOD2::SOPRestriction>&	sopRes = _sopConfig._sopRestrict;
	std::map<std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop = sopRes._sopDatas.find( sopName );
	if( itSop == sopRes._sopDatas.end() )
	{
		char szLocalBuf[1024];
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(SelectStreamerCommand,"unknown SOP[%s] is requested"),
			sopName.c_str() );

		HANDLERLOG(ZQ::common::Log::L_ERROR, szLocalBuf	);
		std::string notice_str;
		notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING);
		_handler.setResponseString( RESPONSE_SSF_UKNOWN_SOPNAMES , notice_str.c_str() );

		return NULL;
	}
	
	clearSelectedInfo();

	return getStreamerFromSop( requestBandwidth, itSop->second , vi  );
}

void SelectStreamerCommand::clearSelectedInfo( )
{
	_selectedEndpoint			=	"";
	_selectedSop				=	"";
	_selectedStreamerNetId		=	"";
	_selectedImportChannelName	=	"";
	_requestBandwidth			=	0;
}

SelectStreamerCommand::SelectStreamerCommand( SetupHandler& handler)
:SetupHandlerCommand(handler)
{
	clearSelectedInfo();
}

SelectStreamerCommand::~SelectStreamerCommand( )
{
}

void SelectStreamerCommand::releaseAllocatedResource( )
{
	if( _selectedImportChannelName.empty() ) 
		return;	
	std::map<std::string,NGOD2::PassThruStreaming::ImportChannelHolder>::iterator it = getImportChannel(_selectedImportChannelName);
	if(isValid(it))
	{
		NGOD2::PassThruStreaming::ImportChannelHolder& imChannel = it->second;
		imChannel._usedImport --;
		if( imChannel._usedImport < 0 )
			imChannel._usedImport = 0;
		imChannel._usedBandwidth -= _requestBandwidth;
		if( imChannel._usedBandwidth <0 )
			 imChannel._usedBandwidth = 0 ;
		HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(SelectStreamerCommand,"released ImportChannel[%s] resource with bw[%d], now usedBw[%lld], usedCount[%d]"),
			imChannel._name.c_str() ,_requestBandwidth, imChannel._usedBandwidth , imChannel._usedImport);
	}
}

void SelectStreamerCommand::releaseStreamer( bool addPenalty /* = false  */)
{
	if (  _handler._streamPrx != NULL ) 
	{
		//must destroy stream if it exist
		try
		{
			_handler._streamPrx->destroy();
		}
		catch(...)
		{

		}
		//actually , this behavior is very danger because if connection is tempariry lost the target stream can't be destroyed
		_handler._streamPrx = NULL;
	}	
	
	ZQ::common::MutexGuard gd( _handler._ssmNGODr2c1._lockSopMap );

	releaseAllocatedResource( );
	
	
	ZQ::common::Config::Holder<NGOD2::SOPRestriction>&	sopRes = _sopConfig._sopRestrict;
	std::map<std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop = sopRes._sopDatas.find( _selectedSop );
	if( itSop != sopRes._sopDatas.end() )
	{
		NGOD2::SOPRestriction::SopHolder& sop = itSop->second;
		std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = sop._streamerDatas.begin();
		for( ; itStreamer != sop._streamerDatas.end() ; itStreamer++ )
		{
			if( _selectedStreamerNetId == itStreamer->_netId )
			{
				itStreamer->_usedBandwidth	-= _requestBandwidth;
				if( itStreamer->_usedBandwidth < 0 )
					itStreamer->_usedBandwidth = 0;
				itStreamer->_usedStream --;
				if( itStreamer->_usedStream < 0 )
					itStreamer->_usedStream = 0;
				HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SelectStreamerCommand,"released selected streamer[%s] resource with bw[%d] and now usedBw[%lld] usedCount[%d]"),
					itStreamer->_netId.c_str() ,
					_requestBandwidth ,
					itStreamer->_usedBandwidth ,
					itStreamer->_usedStream);
				if( addPenalty )
				{
					HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(SelectStreamerCommand,"added penalty [%d] onto streamer[%s]"),
						_sopConfig._sopRestrict._maxPenaltyValue,
						itStreamer->_netId.c_str() );
					itStreamer->_penalty = _sopConfig._sopRestrict._maxPenaltyValue;
				}

				// add by zjm to support counter for fail session on streamers
				if (itStreamer->_failedSession < LLONG_MAX)
				{
					itStreamer->_failedSession++;
				}
				else
				{
					itStreamer->_failedSession = 1;
					itStreamer->_failedSession = 1;
				}
			}
		}
	}
	clearSelectedInfo();
}


/*-----------------------------------------------------------------------------
PrepareEncryptionData
-----------------------------------------------------------------------------*/
PrepareEncryptionData::PrepareEncryptionData( SetupHandler& handler )
:SetupHandlerCommand(handler)
{
}
PrepareEncryptionData::~PrepareEncryptionData( )
{
}
bool PrepareEncryptionData::execute( const char* pContent , SetupHandler::ECMDataMAP& ecmDatas )
{	
	ecmDatas.clear( );
	if ( !pContent || pContent[0] == 0 )
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PrepareEncryptionData,"no encryption data is specified"));
		return true;
	}
	
	std::string notice_str;
	notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_UNABLE_ENCRPT, NGOD_ANNOUNCE_UNABLE_ENCRPT_STRING);
	const char*		next_content = NULL;
	while ( pContent != NULL )
	{
		pContent += strlen("a=X-motorola-ecm:");
		next_content = pContent;
		std::vector<std::string> temp_strs;
		ZQ::StringOperation::splitStr(pContent, " \r\n\t", temp_strs);		
		if (temp_strs.size() >= 5 
			&& ZQ::StringOperation::isInt(temp_strs[2].c_str())
			&& ZQ::StringOperation::isInt(temp_strs[3].c_str())
			&& ZQ::StringOperation::isInt(temp_strs[4].c_str()))
		{
			SetupHandler::ECMData ecm;
			ecm.vecKeyOffset.clear();
			ecm.vecKeys.clear ();
			
			std::string	providerID;
			std::string	assetID;
			
			providerID = temp_strs[0];
			assetID = temp_strs[1];
			ecm.iProgNum = atoi(temp_strs[2].c_str());
			ecm.iFrq1 = atoi(temp_strs[3].c_str());
			int keyNum = atoi(temp_strs[4].c_str());
			if (keyNum > 0 && (int)temp_strs.size() >= 5 + keyNum * 2)
			{
				for (int tCur = 0; tCur < keyNum; tCur ++)
				{
					int tv = atoi(temp_strs[5 + tCur * 2].c_str());					
					ecm.vecKeyOffset.push_back (tv);
					
					std::string tmpStr = temp_strs[6 + tCur * 2];					
					ecm.vecKeys.push_back (tmpStr);
				}
			}
			else 
			{
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
				HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
				_handler.setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
				return false;
			}
			
			ecmDatas.insert(SetupHandler::ECMDataMAP::value_type(providerID+"#"+assetID,ecm));
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PrepareEncryptionData,"encryption data parsed and applied"));
		}
		else 
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
			_handler.setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
			return false;
		}
		
		pContent = strstr(next_content, "a=X-motorola-ecm:");
	}
	return true;
}

/*-----------------------------------------------------------------------------
RenderPlaylistCommand
-----------------------------------------------------------------------------*/
RenderPlaylistCommand::RenderPlaylistCommand( SetupHandler& handler )
:SetupHandlerCommand(handler)
{
}
RenderPlaylistCommand::~RenderPlaylistCommand()
{
}

bool RenderPlaylistCommand::execute(	TianShanIce::Streamer::PlaylistPrx streamPrx ,  
										const SetupHandler::VolumeInfoToBuildList& volumeInfo ,
										NGODr2c1::PlaylistItemSetupInfos& setupInfos )
{
	setupInfos.clear();

	_handler._bRemoteSession = false;

	//check the volume information
	//playlistInfo
	std::string		streamPrxStr = _handler._ssmNGODr2c1._pCommunicator->proxyToString(streamPrx);
	HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(RenderPlaylistCommand,"rendering playlist items onto streamer[%s]"),streamPrxStr.c_str() );
	SetupHandler::PidPaidInfoS plInfos = _handler.playlistInfo;
	if( plInfos.size() == 0 )
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(RenderPlaylistCommand,"quit rendering for empty playlist"));
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(RenderPlaylistCommand,"quit rendering for empty playlist"));
		std::string notice_str;
		notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING);
		_handler.setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
		return false;
	}
	int32 iTotalElementCount = 0;
	SetupHandler::PidPaidInfoS::const_iterator itPlInfo = plInfos.begin();
	for( ; itPlInfo != plInfos.end() ; itPlInfo ++ )
	{
		iTotalElementCount += static_cast<int32>( itPlInfo->elements.size() );
	}

	int32			currentElementIndex = 1; //used as CtrlNum based on 1

	std::string notice_str;
	notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);


	SetupHandler::ECMDataMAP& ecmDatas = _handler._ecmDatas;
	for( itPlInfo = plInfos.begin() ; itPlInfo != plInfos.end() ; itPlInfo ++ )
	{
		TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
		setupInfo.privateData.clear();
		fillEncryptionData( volumeInfo.netId , itPlInfo->pid , itPlInfo->paid, setupInfo.privateData , ecmDatas );
		const SetupHandler::ElementInfoS& elements = itPlInfo->elements;
		SetupHandler::ElementInfoS::const_iterator itEle = elements.begin();
		for( ; itEle != elements.end() ; itEle ++ )
		{
			const SetupHandler::AeVolumeInfoSet& volumeList = itEle->volumeList ;
			if(volumeList.size() > 0 )
			{
				const std::string& volumeName = volumeList.begin()->volumeName;
				if( !volumeName.empty() )
				{
					setupInfo.contentName			=	"/" + volumeList.begin()->volumeName + "/" + itEle->contentName;
				}
				else
				{
					//default volume
					setupInfo.contentName			=	 "/$/" + itEle->contentName;
				}
			}
			else
			{
				//default volume
				setupInfo.contentName			=	"/$/" + itEle->contentName;
			}
			setupInfo.criticalStart			=	0;
			setupInfo.inTimeOffset			=	itEle->cuein;
			setupInfo.outTimeOffset			=	itEle->cueout;
			setupInfo.flags					=	itPlInfo->ctrlMark;
			setupInfo.spliceIn				=	false;
			setupInfo.spliceOut				=	false;
			setupInfo.forceNormal			=	false;
			::TianShanIce::ValueMap& infoPD = setupInfo.privateData;
			infoPD.clear();

			TianShanIce::Variant varProviderId;
			varProviderId.type = TianShanIce::vtStrings;
			varProviderId.strs.clear();
			varProviderId.strs.push_back( itPlInfo->pid );
			infoPD["providerId"]			= varProviderId;

			TianShanIce::Variant varProviderAssetId;
			varProviderAssetId.type = TianShanIce::vtStrings;
			varProviderAssetId.strs.clear();
			varProviderAssetId.strs.push_back( itPlInfo->paid );
			infoPD["providerAssetId"]		= varProviderAssetId;
			if( volumeInfo.supportNasStreaming  >= 1 )
			{
				const StringCollection& urls = itEle->nasurl;
				StringCollection::const_iterator itURL = urls.begin() ;
				TianShanIce::Variant varURL;
				varURL.type = TianShanIce::vtStrings;
				varURL.strs.clear( );
				for ( ; itURL != urls.end() ; itURL ++ ) 
				{
					if( !itURL->empty() )
					{
						_handler._bRemoteSession = true;
						varURL.strs.push_back(*itURL);
						HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand,"add url[%s] to item[%s][%d]"),
							itURL->c_str() , 
							setupInfo.contentName.c_str(),
							currentElementIndex );
					}
				}
				infoPD["storageLibraryUrl"]		= varURL;				
			}
			else
			{
				HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand,"do not add url to item[%s][%d] per supportNasStream[%d]"),					
					setupInfo.contentName.c_str(),
					currentElementIndex ,
					volumeInfo.supportNasStreaming );
				infoPD.erase("storageLibraryUrl");
			}
			if ( iTotalElementCount == currentElementIndex + 1 )
			{
				setupInfo.privateData.erase("TianShan-flag-pauselastuntilnext");
			}

			try
			{
				streamPrx->pushBack( currentElementIndex , setupInfo);
				setupInfos.push_back( setupInfo ); //
			}
			catch( const TianShanIce::InvalidParameter& ex )
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback items onto playlist[%s]" ),
					ex.message.c_str() ,
					streamPrxStr.c_str() );
				HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
				_handler.setResponseString( RESPONSE_SSF_ASSET_NOT_FOUND , notice_str.c_str());
				return false;
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand, "caught exception:[%s] when pushback items onto playlist[%s]" ),
					ex.message.c_str() ,
					streamPrxStr.c_str() );
				HANDLERLOG(ZQ::common::Log::L_ERROR, szLocalBuf );
				_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());
				return false;
			}
			catch( const Ice::ConnectFailedException& ex)
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback item onto playlist[%s]" ),
					ex.ice_name().c_str() ,
					streamPrxStr.c_str()  );
				HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
				
				_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());

				_handler._ssmNGODr2c1._streamerQuerier->pushStreamer( _handler._strSelectedSopName ,
					_handler._strSelectedStreamNetID,
					_handler._strSelectedEndpoint );
				return false;
			}
			catch( const Ice::ConnectionLostException& ex)
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback items onto playlist[%s]" ),
					ex.ice_name().c_str() ,
					streamPrxStr.c_str()  );
				HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );

				_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());

				_handler._ssmNGODr2c1._streamerQuerier->pushStreamer( _handler._strSelectedSopName ,
					_handler._strSelectedStreamNetID,
					_handler._strSelectedEndpoint );				
				return false;
			}
			catch( const Ice::ConnectTimeoutException& ex)
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback item onto playlist[%s]" ),
					ex.ice_name().c_str() ,
					streamPrxStr.c_str()  );
				HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
				_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());			
				
				_handler._ssmNGODr2c1._streamerQuerier->pushStreamer( _handler._strSelectedSopName ,
					_handler._strSelectedStreamNetID,
					_handler._strSelectedEndpoint );
				return false;
			}
			catch (const Ice::Exception& ex) 
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback item onto playlist[%s]" ),
					ex.ice_name().c_str() ,
					streamPrxStr.c_str()  );
				HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
				_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());

				HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
				return false;
			}
			catch (...) 
			{
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1,HANDLERLOGFMT(RenderPlaylistCommand , "caught unknown exception when pushback item into playlist[%s]" ),					
					streamPrxStr.c_str() );
				HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
				_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str() );

				return false;
			}
			++currentElementIndex;
		}
	}
	
	//bool bEnableEOT	= _handler._ssmNGODr2c1._playlistControlEnableEOT;
	bool bEnableEOT		=	_ngodConfig._PlaylistControl.enableEOT >= 1;
	try
	{
		streamPrx->enableEoT( bEnableEOT );

		if (!bEnableEOT)
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand, "disabled EOT onto playlist"));

		streamPrx->commit( );		
	}
	catch( const TianShanIce::BaseException& ex )
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.message.c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);

		if( ex.errorCode == EXT_ERRCODE_BANDWIDTH_EXCEEDED )
		{
			_handler.setResponseString( RESPONSE_SSF_NO_NETWORKBANDWIDTH , szLocalBuf );
		}
		else
		{
			_handler.setResponseString( RESPONSE_INTERNAL_ERROR , szLocalBuf );
		}

		return false;
	}
	catch( const Ice::ConnectFailedException& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());

		_handler._ssmNGODr2c1._streamerQuerier->pushStreamer( _handler._strSelectedSopName ,
			_handler._strSelectedStreamNetID,
			_handler._strSelectedEndpoint );
	}
	catch( const Ice::ConnectionLostException& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
		_handler._ssmNGODr2c1._streamerQuerier->pushStreamer( _handler._strSelectedSopName ,
			_handler._strSelectedStreamNetID,
			_handler._strSelectedEndpoint );

	}
	catch( const Ice::ConnectTimeoutException& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , 
			HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
		_handler._ssmNGODr2c1._streamerQuerier->pushStreamer( _handler._strSelectedSopName ,
			_handler._strSelectedStreamNetID,
			_handler._strSelectedEndpoint );
	}
	catch(const Ice::Exception& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , 
			HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
		return false;
	}
	catch(...)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , 
			HANDLERLOGFMT(RenderPlaylistCommand,"caught unknown exception when invoke enableEOT/commit"));
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());		
		return false;
	}
	
	return static_cast<bool>( currentElementIndex > 0 );
}

void RenderPlaylistCommand::fillEncryptionData(	const std::string&			volumeName,
											   const std::string&			strProviderId,
											   const std::string&			strAssetId,
											   ::TianShanIce::ValueMap&	infoPrivateData , 
											   SetupHandler::ECMDataMAP&	ecmDatas )
{
	
	//prepare encryption data from each item
	std::string strProviderAssetId = strProviderId + "#" + strAssetId;
	SetupHandler::ECMDataMAP::iterator itEcm = ecmDatas.find ( strProviderAssetId );
	if (itEcm != ecmDatas.end ())
	{		
		std::string keyStr;
		keyStr = "Tianshan-ecm-data:programNumber";			
		::TianShanIce::Variant var;
		var.type = ::TianShanIce::vtInts;
		var.bRange = false;
		var.ints.clear();
		int tmpInt = itEcm->second.iProgNum; /*atoi(temp_strs[2].c_str());*/
		var.ints.push_back(tmpInt);
		infoPrivateData[keyStr] = var;
		
		keyStr = "Tianshan-ecm-data:Freq_1";			
		var.ints.clear();
		tmpInt = itEcm->second.iFrq1; //atoi(temp_strs[3].c_str());
		var.ints.push_back(tmpInt);
		infoPrivateData[keyStr] = var;
		
		int keyNum =  (int)itEcm->second.vecKeyOffset.size();//atoi(temp_strs[4].c_str());
		if (keyNum > 0/* && (int)temp_strs.size() >= 5 + keyNum * 2*/)
		{
			::TianShanIce::Variant intVar, strVar;
			intVar.type = ::TianShanIce::vtInts;
			intVar.bRange = false;
			intVar.ints.clear();
			
			strVar.type = ::TianShanIce::vtStrings;
			strVar.bRange = false;
			strVar.strs.clear();
			
			for (int tCur = 0; tCur < keyNum; tCur ++)
			{
				int tv = itEcm->second.vecKeyOffset[tCur];//atoi(temp_strs[5 + tCur * 2].c_str());
				intVar.ints.push_back(tv);
				
				std::string& tmpStr = itEcm->second.vecKeys[tCur];/*temp_strs[6 + tCur * 2];*/
				strVar.strs.push_back(tmpStr);
			}
			
			keyStr = "Tianshan-ecm-data:keyoffsets";
			infoPrivateData[keyStr] = intVar;
			
			keyStr = "Tianshan-ecm-data:keys";				
			infoPrivateData[keyStr] = strVar;
			
			TianShanIce::Variant varEnablePreEncryption ;
			varEnablePreEncryption.type = TianShanIce::vtInts;
			varEnablePreEncryption.ints.push_back(1);				
			
			infoPrivateData["Tianshan-ecm-data:preEncryption-Enable"] = varEnablePreEncryption;
			
			TianShanIce::Variant varPauseLastUntilNext ;
			varPauseLastUntilNext.ints.clear();
			varPauseLastUntilNext.type = TianShanIce::vtInts;
			varPauseLastUntilNext.ints.push_back(1);
			
			infoPrivateData["TianShan-flag-pauselastuntilnext"] = varPauseLastUntilNext;

			{
				TianShanIce::Variant varVol, varBaseUrl;
				varVol.type = TianShanIce::vtStrings;
				varVol.bRange = false;
				varVol.strs.clear();
				varVol.strs.push_back(volumeName);
				infoPrivateData["library"] = varVol;
				
				varBaseUrl.type = TianShanIce::vtStrings;
				varBaseUrl.bRange = false;
				varBaseUrl.strs.clear();
				varBaseUrl.strs.push_back(_ngodConfig._library._urlTemplate);
				infoPrivateData["url"] = varBaseUrl;
			}
		}
		else 
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RenderPlaylistCommand, "content format error for encription"));
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RenderPlaylistCommand, "content format error for encription"));
		}		
	}
	else 
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand, "no Encryption data was found with [%s]"),
			strProviderAssetId.c_str());
	}
}

/*-----------------------------------------------------------------------------
GetReferrenceAEListCommand
-----------------------------------------------------------------------------*/
static bool fixpath(std::string& path, bool bIsLocal = true)
{
	char* pathbuf = new char[path.length() +2];
	if (NULL ==pathbuf)
		return false;
	strcpy(pathbuf, path.c_str());
	pathbuf[path.length()] = '\0';
	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
			*p = FNSEPC;
	}
	if (!bIsLocal && ':' == pathbuf[1])
		pathbuf[1] = '$';
	else if (bIsLocal && '$' == pathbuf[1])
		pathbuf[1] = ':';
	static const char* szProtocol[] = 
	{
		"file:",
		"cifs:",
		"nfs:"
	};
	static int nProto = sizeof(szProtocol)/sizeof(const char*);
	int i;
	for( i=0; i<nProto; i++ )
	{
		if (!strnicmp(pathbuf, szProtocol[i], strlen(szProtocol[i])))
		{
			path = pathbuf + strlen(szProtocol[i]);
			break;
		}
	}
	if ( i >= nProto )
	{
		//not found
		path = pathbuf;
	}	
	
	return true;
}



GetReferrenceAEListCommand::GetReferrenceAEListCommand( SetupHandler& handler )
:SetupHandlerCommand(handler)
{
}
GetReferrenceAEListCommand::~GetReferrenceAEListCommand( )
{
}

LAMFacadePrx GetReferrenceAEListCommand::getLAMServer( )
{
	std::string notice_str;
	notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	NGOD2::LAMServer::LAMServerHolder&	lam	=	_ngodConfig._lam._lamServer;	
	if(! lam._lamPrx)
	{
		ZQ::common::MutexGuard gd( _handler._ssmNGODr2c1._lockLAMServer);		
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(GetReferrenceAEListCommand,"LAM[%s] is not connected, retry to connect"),
			lam._endpoint.c_str() );
		try
		{
			lam._lamPrx = LAMFacadePrx::checkedCast( _handler._ssmNGODr2c1._pCommunicator->stringToProxy( lam._endpoint ) );
		}
		catch( const Ice::Exception& ex )
		{			
			lam._lamPrx = NULL;	

			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(GetReferrenceAEListCommand,"caught exception[%s] when connect to LAM[%s]"),
				ex.ice_name().c_str() ,
				lam._endpoint.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
			_handler.setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
			return NULL;
		}
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(GetReferrenceAEListCommand,"successfully connected to LAM [%s]"),			
			lam._endpoint.c_str() );
		return lam._lamPrx;
	}	
	return lam._lamPrx;
}
AEInfo3Collection GetReferrenceAEListCommand::dummy( )
{
	AEInfo3Collection result;
	//always get the first one
	
	NGOD2::LAMTestMode::LAMSubTestSetHolderS& tests = _ngodConfig._lam._lamTestMode.subTests;
	if( tests.size() == 0 )
		return result;
	NGOD2::LAMTestMode::LAMSubTestSetHolderS::const_iterator it = tests.begin();
	for(  ; it != tests.end() ; it ++ )
	{
		AEInfo3 info;
		info.name			=	it->contentName;
		info.bandWidth		=	it->bandwidth;
		info.cueIn			=	it->cueIn;
		info.cueOut			=	it->cueOut;
		const std::vector<std::string>& urls = it->urls;
		std::vector<std::string>::const_iterator itUrl = urls.begin();
		for( ; itUrl != urls.end() ; itUrl ++ )
			info.nasUrls.push_back( *itUrl );

		const std::vector<std::string>& volumes = it->volumeList;
		std::vector<std::string>::const_iterator itVol = volumes.begin();
		for( ; itVol != volumes.end() ; itVol ++ )
			info.volumeList.push_back(*itVol);
		result.push_back( info );
	}
	return result;
}

void GetReferrenceAEListCommand::getCandidateVolumes( const  std::string& sopName , std::set<std::string>& volumes )
{
	volumes.clear();
	ZQ::common::MutexGuard gd( _handler._ssmNGODr2c1._lockSopMap );
	ZQ::common::Config::Holder<NGOD2::SOPRestriction>&	sopRes = _sopConfig._sopRestrict;
	std::map<std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop = sopRes._sopDatas.find( sopName );
	if( itSop == sopRes._sopDatas.end()  )
		return;

	const std::vector< NGOD2::Streamer::StreamerHolder >& streamers = itSop->second._streamerDatas;
	std::vector< NGOD2::Streamer::StreamerHolder >::const_iterator it = streamers.begin();
	for( ; it != streamers.end() ; it ++ )
	{
		volumes.insert( it->_storageNetId );
	}
}

bool GetReferrenceAEListCommand::getAeInfo(  LAMFacadePrx lamPrx ,const std::string& sopName, SetupHandler::PidPaidInfo& info )
{
	mbAssetCanStreamingFromLocal = mbAssetCanStreamingFromRemote = true;
	std::string notice_str;
	notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);
	AEInfo3Collection	aeinfos;
	try
	{//get ae information from LAM
		HANDLERLOG(ZQ::common::Log::L_INFO, CLOGFMT(GetReferrenceAEListCommand,"querying AElist for PID[%s] PAID[%s] SID[%s]"),
			info.pid.c_str() , info.paid.c_str() , info.sid.c_str()	);
		if( _ngodConfig._lam._lamTestMode._enabled >= 1 )
		{
			HANDLERLOG(ZQ::common::Log::L_WARNING,HANDLERLOGFMT(GetReferrenceAEListCommand,"Enter Test Mode"));
			aeinfos = dummy( );
		}
		else
		{
			aeinfos = lamPrx->getAEListByPIdPAIdSId( info.pid , info.paid , info.sid );
		}

		HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(GetReferrenceAEListCommand,"got [%d] elements in the AElist"),
			static_cast<int>( aeinfos.size() ));
	}
	catch( const LogicError& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0 ;
		snprintf(szLocalBuf,sizeof(szLocalBuf), HANDLERLOGFMT(GetReferrenceAEListCommand,"caught LAM logicError[%s] when query AElist info with PID[%s] PAID[%s] SID[%s]"),
			ex.errorMessage.c_str() ,info.pid.c_str() , info.paid.c_str() , info.sid.c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());

		return false;
	}
	catch( const Ice::Exception& ex )
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0 ;
		snprintf(szLocalBuf,sizeof(szLocalBuf), HANDLERLOGFMT(GetReferrenceAEListCommand,"caught exception[%s] when query LAM for AElist by PID[%s] PAID[%s] SID[%s]"),
			ex.ice_name().c_str() ,  info.pid.c_str() , info.paid.c_str()  , info.sid.c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());		
		return false;
	}

	std::set<std::string> candidateVolumes;
	getCandidateVolumes( sopName , candidateVolumes );

	bool bFirst = true;
	//adjust ae information result
	AEInfo3Collection::const_iterator itAe = aeinfos.begin();
	
	std::vector<std::string> specifiedVolumes;
	std::vector<std::string> tmpSpecifiedVolumes;

	for( ; itAe != aeinfos.end() ; itAe ++ )
	{
		if( itAe->nasUrls.size( ) <= 0 && itAe->volumeList.size() <= 0 )
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0 ;
			snprintf(szLocalBuf,sizeof(szLocalBuf), HANDLERLOGFMT(GetReferrenceAEListCommand, "illegal AElist returned from LAM: content[%s] has no local replica and no url to library"),
				itAe->name.c_str() );

			HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
			std::string notice_str;
			notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING);
			_handler.setResponseString( RESPONSE_SSF_INVALID_REQUEST , notice_str.c_str());	
			return false;
		}

		SetupHandler::ElementInfo		eleInfo;
		eleInfo.contentName		=	itAe->name;
		eleInfo.bandwidth		=	itAe->bandWidth;	//this is only for recording
		_maxBandwidth			=	_maxBandwidth > eleInfo.bandwidth ? _maxBandwidth : eleInfo.bandwidth ;
		eleInfo.cuein			=	info.cuein > itAe->cueIn ? info.cuein : itAe->cueIn;
		eleInfo.cueout			=	itAe->cueOut == 0 ? ( info.cueout  ) : ( info.cueout > itAe->cueOut ? itAe->cueOut : info.cueout );
		eleInfo.nasurl			=	itAe->nasUrls;


		if( itAe->nasUrls.size() <= 0 )
		{
			mbAssetCanStreamingFromRemote = false;//no url means that the whole playlist can't be streamed from remote
		}

		if( itAe->volumeList.size() > 0 )
		{
			tmpSpecifiedVolumes.clear();

			StringCollection::const_iterator itVolume = itAe->volumeList.begin();
			for( ; itVolume != itAe->volumeList.end() ; itVolume ++ )
			{
				const char* pVolume		=	itVolume->c_str();
				const char* pBackSlash	=	strstr( pVolume , "/" );
				SetupHandler::AeVolumeInfo	vi;

				//TODO: maybe there is no slash in volume list content 
				if( pBackSlash )
				{
					vi.netId.assign( pVolume , pBackSlash - pVolume );
					vi.volumeName.assign( pBackSlash + 1 );
				}
				else			
				{
					vi.netId	= *itVolume;
					vi.volumeName = "";
				}
				

				//find the net-id from configuration
				//NOTE: ContentVolume configuration is read only , so no mutex guard is needed
				const NGOD2::LAMServer::ContentVolumeHolderS& ctntVolumes = _ngodConfig._lam._lamServer.contentVolumes;
				NGOD2::LAMServer::ContentVolumeHolderS::const_iterator itVolumeConf = ctntVolumes.find( vi.netId );
				if( itVolumeConf == ctntVolumes.end() || 
					(!itVolumeConf->second.bAllVolumeAvailable && itVolumeConf->second.volumeName.find( vi.volumeName ) == itVolumeConf->second.volumeName.end() ) )
				{
					HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetReferrenceAEListCommand,"unkown volume[%s/%s] found in AElist, ignored"),
						vi.netId.c_str() ,
						vi.volumeName.c_str() );
					HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetReferrenceAEListCommand,"unkown volume[%s/%s] found in AElist, ignored"),
						vi.netId.c_str() ,
						vi.volumeName.c_str() );
					continue;
				}
				
				if( candidateVolumes.find(vi.netId) == candidateVolumes.end() )
				{
					HANDLERLOG(ZQ::common::Log::L_WARNING,HANDLERLOGFMT(GetReferrenceAEListCommand,"unkown volume[%s] found in AElist, no streamers associated with this volume in sop [%s], ignored"),
						vi.netId.c_str() ,sopName.c_str() );
					continue;
				}

				vi.cacehLevel				= itVolumeConf->second.cacheLevel;
				vi.supportNasStreaming		= itVolumeConf->second.supportNasStreaming;
				vi.supportCache				= itVolumeConf->second.cache;
				vi.specifiedByAsset			= true;
				
				tmpSpecifiedVolumes.push_back( vi.netId );

				eleInfo.volumeList.insert( vi );
			}
			//record volumes specified by asset info
			if( bFirst )
			{
				specifiedVolumes = tmpSpecifiedVolumes;
				bFirst = false;

			}
			else
			{
				std::vector<std::string> tmp;
				std::set_intersection( specifiedVolumes.begin() , specifiedVolumes.end() , 
										tmpSpecifiedVolumes.begin(), tmpSpecifiedVolumes.end() , 
										std::inserter< std::vector<std::string> >( tmp, tmp.begin() ) );
				specifiedVolumes = tmp;
			}
		}
		else
		{
			mbAssetCanStreamingFromLocal = false;//because volume list is empty, mark that the whole playlist can't be streamed from local cache
		}


		//TODO: make copy of volumes which support NAS streaming, this can help us to reduce CPU consumption
		const NGOD2::LAMServer::ContentVolumeHolderS& cv = _ngodConfig._lam._lamServer.contentVolumes;
		NGOD2::LAMServer::ContentVolumeHolderS::const_iterator itVol = cv.begin();
		SetupHandler::AeVolumeInfoSet& vlmList = eleInfo.volumeList;				
		for( ; itVol != cv.end(); itVol ++ )
		{
			if ( itVol->second.supportNasStreaming <= 0 )
				continue;
			if( candidateVolumes.find( itVol->second.netId ) == candidateVolumes.end() )
			{//the volume is not available because no streamer associated with this volume 
				continue;
			}
			SetupHandler::AeVolumeInfo avi;						
			avi.volumeName					= "";
			avi.netId						= itVol->second.netId;
			avi.supportCache				= itVol->second.cache;
			avi.cacehLevel					= itVol->second.cacheLevel;;
			avi.supportNasStreaming			= itVol->second.supportNasStreaming;

			vlmList.insert(avi);
		}

		HANDLERLOG(ZQ::common::Log::L_INFO,	HANDLERLOGFMT(GetReferrenceAEListCommand, "adjusted cueIn/cueOut of content[%s]:  SETUP[%d/%d] LAM[%d/%d], taking cueIn[%d] cueOut[%d], bandwidth[%d] nasUrl[%s] volumeList[%s]"),
			eleInfo.contentName.c_str() , eleInfo.cuein, eleInfo.cueout, info.cuein, info.cueout, itAe->cueIn, itAe->cueOut,eleInfo.bandwidth,
			logVectorString( itAe->nasUrls ).c_str(), logVectorString( itAe->volumeList ).c_str() );
		
		info.elements.push_back(eleInfo);
	}
	
	if( info.elements.size() == 0 )
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0 ;
		snprintf(szLocalBuf,sizeof(szLocalBuf), HANDLERLOGFMT(GetReferrenceAEListCommand,"no element is returned from LAM with PID[%s] PAID[%s] SID[%s]"),
			info.pid.c_str() ,info.paid.c_str() , info.sid.c_str() );	
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		_handler.setResponseString( RESPONSE_SSF_ASSET_NOT_FOUND , szLocalBuf);	
	}

	ZQTianShan::Util::copyVectorToSet<std::string>( specifiedVolumes, mSpecifiedVolumes );

	return info.elements.size() > 0 ;
}


void  GetReferrenceAEListCommand::randomStoragelist( SetupHandler::VolumeInfoToBuildListS& storList )
{
	{//re-sort the storage list according to 
		int32 lastCacheLevel = -99999999;
		std::vector<SetupHandler::VolumeInfoToBuildListS*> tmpList;
		SetupHandler::VolumeInfoToBuildListS* pTmp = NULL;
		SetupHandler::VolumeInfoToBuildListS::const_iterator it = storList.begin();
		for( ; it != storList.end() ; it ++ )
		{
			if( pTmp == NULL )
			{
				pTmp = new SetupHandler::VolumeInfoToBuildListS;
			}
			else if ( it->cacheLevel != lastCacheLevel )
			{	
				if( pTmp)
				{
					std::random_shuffle( pTmp->begin() , pTmp->end(),MyRandom() );
					tmpList.push_back(pTmp);				
				}
				pTmp = new SetupHandler::VolumeInfoToBuildListS;

			}			
			pTmp->push_back(*it);

			lastCacheLevel = it->cacheLevel;
		}
		if( pTmp && pTmp->size() > 0 )
		{
			std::random_shuffle( pTmp->begin() , pTmp->end(),MyRandom() );
			tmpList.push_back(pTmp);
		}
		storList.clear();
		std::vector<SetupHandler::VolumeInfoToBuildListS*>::const_iterator itTmp = tmpList.begin();
		for( ; itTmp != tmpList.end() ; itTmp ++ )
		{
			//storageList += *(*itTmp);
			const SetupHandler::VolumeInfoToBuildListS& stolist = *(*itTmp);
			SetupHandler::VolumeInfoToBuildListS::const_iterator itList = stolist.begin();
			for( ; itList != stolist.end() ; itList ++ )
			{
				storList.push_back(*itList);
			}
			delete *itTmp;
		}
	}
}

void collectVolumeNetIdFromAeVolumeInfoSet( const SetupHandler::AeVolumeInfoSet& volumeSet, std::vector<std::string>& netIds )
{
	SetupHandler::AeVolumeInfoSet::const_iterator it = volumeSet.begin();
	for( ; it != volumeSet.end() ; it ++ )
	{
		netIds.push_back( it->netId );
	}
}

void GetReferrenceAEListCommand::getAvailabeVolumesFromAssetInfos( const SetupHandler::PidPaidInfoS& infos , std::vector<std::string>& volumeSet )
{
	bool bFirst = true;
	
	SetupHandler::PidPaidInfoS::const_iterator it = infos.begin();	
	for( ; it != infos.end() ; it ++ )
	{
		const SetupHandler::ElementInfoS& eleInfos = it->elements;
		for( SetupHandler::ElementInfoS::const_iterator itEle = eleInfos.begin() ; itEle != eleInfos.end() ; itEle++ )
		{
			const SetupHandler::AeVolumeInfoSet& volumes = itEle->volumeList;
			if( bFirst )
			{
				collectVolumeNetIdFromAeVolumeInfoSet( volumes, volumeSet );
				bFirst = false;
			}
			else
			{
				std::vector<std::string> tmp;
				std::vector<std::string> tmpResult;
				collectVolumeNetIdFromAeVolumeInfoSet( volumes, tmp );
				std::set_intersection( tmp.begin() , tmp.end() , volumeSet.begin(), volumeSet.end() , std::inserter< std::vector<std::string> >( tmpResult,tmpResult.begin() ) );
				volumeSet = tmpResult;
			}			
		}
	}
}
void GetReferrenceAEListCommand::buildVolumeContext( const std::string& volumeNetId , const SetupHandler::PidPaidInfoS& assetInfos  )
{
	
	const NGOD2::LAMServer::ContentVolumeHolderS& cv			= _ngodConfig._lam._lamServer.contentVolumes;
	
	NGOD2::LAMServer::ContentVolumeHolderS::const_iterator itVolumeConf = cv.find( volumeNetId );
	if( itVolumeConf == cv.end() ) 
		return;

	const NGOD2::ContentVolume::ContentVolumeHolder& volumeConf = itVolumeConf->second;
	
	if( !mbAssetCanStreamingFromLocal && volumeConf.supportNasStreaming <= 0 )
		return;//this volume is not qualified

	SetupHandler::VolumeInfoToBuildList volumeContext;
	volumeContext.netId						=	volumeConf.netId;
	volumeContext.cacheLevel				=	volumeConf.cacheLevel;
	volumeContext.supportCache				=	volumeConf.cache;
	volumeContext.supportNasStreaming		=	volumeConf.supportNasStreaming;		
	volumeContext.bSpecifiedByAsset			=	( mSpecifiedVolumes.find( volumeConf.netId ) != mSpecifiedVolumes.end() );
	
	if( mbAssetCanStreamingFromLocal && mSpecifiedVolumes.find(volumeContext.netId) != mSpecifiedVolumes.end() )
	{
		volumeContext.plType =	SetupHandler::PLAYLIST_STREAMING_FROM_LOCAL;
	}
	else if( mbAssetCanStreamingFromRemote )
	{
		volumeContext.plType = 	SetupHandler::PLAYLIST_STREAMING_FROM_NAS;
	}
	else
	{
		return;
	}
	
	if( volumeContext.bSpecifiedByAsset )
	{
		mSpecifiedVolumeContexts.push_back( volumeContext );
	}
	else
	{	
		mOtherVolumeContexts.push_back( volumeContext );
	}
}

bool GetReferrenceAEListCommand::narrowContenVolume( bool bMayNeedCache , SetupHandler::PidPaidInfoS& infos  )
{

	if( infos.size() == 0 )
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(GetReferrenceAEListCommand,"no PID/PAID is specified"));
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(GetReferrenceAEListCommand,"no PID/PAID is specified"));
		_handler.setResponseString( RESPONSE_SSF_INVALID_REQUEST , "no PID/PAID is specified");	
		return false;
	}

	NGOD2::LAMServer::ContentVolumeHolderS& cv = _ngodConfig._lam._lamServer.contentVolumes;
	
	//对每一个element的volume做一次交集，这样我们就知道哪些volume可以被用来播出这些element
	std::vector<std::string> availVolumes;
	getAvailabeVolumesFromAssetInfos( infos, availVolumes );

	for( std::vector<std::string> ::const_iterator itVolumeNetId = availVolumes.begin() ; itVolumeNetId != availVolumes.end() ; itVolumeNetId ++ )
	{//对于每一个可用的volume, build a volume context
		buildVolumeContext( *itVolumeNetId , infos );
	}

	//sort the storage list
	std::sort( mOtherVolumeContexts.begin() , mOtherVolumeContexts.end() , storageListSort() );
	std::sort( mSpecifiedVolumeContexts.begin() , mSpecifiedVolumeContexts.end() , storageListSort() );

	randomStoragelist( mSpecifiedVolumeContexts );
	randomStoragelist( mOtherVolumeContexts );
	
	
	SetupHandler::VolumeInfoToBuildListS& storageList = _handler.storageList ;
	storageList = mSpecifiedVolumeContexts;	
	SetupHandler::VolumeInfoToBuildListS::const_iterator itTemp = mOtherVolumeContexts.begin();
	for( ; itTemp != mOtherVolumeContexts.end() ; itTemp++ )
	{
		storageList.push_back(*itTemp);
	}
	return true;
}

bool GetReferrenceAEListCommand::storageListSort::operator()( const SetupHandler::VolumeInfoToBuildList& v1 , const SetupHandler::VolumeInfoToBuildList& v2 )
{
	if ( v1.cacheLevel < v2.cacheLevel )
	{
		return true;
	}
	return false;
}



bool GetReferrenceAEListCommand::execute( const std::string& sopName ,  bool maybeNeedCache, long& maxBandwidth )
{
	SetupHandler::PidPaidInfoS&  infos = _handler.playlistInfo;
	

	LAMFacadePrx lamPrx  = NULL;
	//step 1 
	//query LAM and get the element result information
	//before querying LAM , we must assure that the lam is connected
	if(_ngodConfig._lam._lamTestMode._enabled >= 1 )
	{
		//return dummy();
	}
	else
	{
		lamPrx = getLAMServer( ); 
		if( !lamPrx )
			return false;
	}

	//reset max bandwidth to -1000
	_maxBandwidth = -1000;

	mSpecifiedVolumes.clear();

	SetupHandler::PidPaidInfoS::iterator it = infos.begin();
	for( ; it != infos.end() ; it ++ )
	{
		if(!getAeInfo( lamPrx ,sopName, *it ) )
			return false;
	}

	if(! narrowContenVolume( maybeNeedCache, infos ) )
		return false;

	maxBandwidth = _maxBandwidth;

	return true;
}


SetupHandlerCommand::SetupHandlerCommand( SetupHandler& handler )
:_handler(handler)
{

}

std::string SetupHandlerCommand::logVectorString( const std::vector<std::string>& strVec )
{
	char szBuf[1024];
	memset( szBuf , 0 , sizeof(szBuf) );
	std::vector<std::string>::const_iterator it = strVec.begin();
	int iPos = 0;
	for ( ; it != strVec.end() ; it ++ ,iPos <sizeof(szBuf) ) 
	{
		iPos += sprintf(szBuf+iPos , "<%s> ", it->c_str() );
	}
	return std::string(szBuf);
}



