

#include "checkContent.h"
#include <log.h>


#include <ContentStore.h>
#include <ContentSysMD.h>
#include <vstrmver.h>
#include <TianShanIceHelper.h>
#include <strhelper.h>
#include <TimeUtil.h>


#ifdef _DEBUG
	#include "adebugmem.h"
#endif


#define _NPVR_TEMP_NAME_SOULTION_

namespace ZQ{
namespace StreamSmith{



#define CHECKCTNT(x) "[Playlist] Stream(%s)Thread[%10u][%16s]\t"##x,PlaylistID.c_str(),GetCurrentThreadId(),"CheckContent"
#define CHECKCTNT0(x) "[CheckContent]\t"##x

CheckContent::CheckContent( Ice::CommunicatorPtr& icPtr ,
						   Ice::ObjectPrx objPrx ,
						   VHANDLE vstrmhandle,
						   ZQ::IdxParser::IdxParserEnv* env)
:_icPtr(icPtr)
{
	if( env)
	{
		mIdxParserEnv = env;
		mbOwnEnv = false;
	}
	else
	{
		mIdxParserEnv = new ZQ::IdxParser::IdxParserEnv();
		mIdxParserEnv->InitVstrmEnv();
		mbOwnEnv = true;		
		mIdxParserEnv->AttchLogger(&glog);
		if (gStreamSmithConfig.embededContenStoreCfg.ctntAttr.attrFromVstm != 0)
		{
			mIdxParserEnv->setUseVstrmIndexParseAPI(true);
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(CheckContent, "use vstrm api anyway, due to attrFromVstm is unequal to 0"));
		}
		else
		{
			mIdxParserEnv->setUseVstrmIndexParseAPI( ( gStreamSmithConfig.serverMode == 2 ) );//EdgeServer
		}
		mIdxParserEnv->setUseVsOpenAPI( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.useVsOpenAPI >= 1 );
		mIdxParserEnv->setSkipZeroByteFile( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.skipZeroByteFiles >= 1 );
	}
	mbEdgeServer = (gStreamSmithConfig.serverMode == 2);

#if VER_PRODUCTVERSION_MAJOR >= 6 &&  (( VER_PRODUCTVERSION_MINOR == 0 && VER_PRODUCTBUILD >= 9207 ) || VER_PRODUCTVERSION_MINOR > 0 )
	_strHDot264Driver = "\\vv2\\";
#else
	_strHDot264Driver = "\\tsoverip\\";
#endif

	_bQuit = false;
	_bPrimaryEndpoint = true;
	if( objPrx )
	{
		setContentStoreProxy(objPrx);
	}

}
CheckContent::~CheckContent()
{
	if( mbOwnEnv )
	{
		delete mIdxParserEnv;
	}
}
bool CheckContent::setContentStoreProxy( Ice::ObjectPrx  objPrx  )
{
	
	try
	{
		_ctntStorageServicePrx = TianShanIce::Storage::ContentStorePrx::checkedCast( objPrx );
		if ( !_ctntStorageServicePrx ) 
		{
			glog( ZQ::common::Log::L_INFO, CHECKCTNT0( "can't cast ObjectProxy to ContentStoreProxy" ) );
			return false;
		}
	}
	catch ( const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_INFO,CHECKCTNT0("can't cast ObjectProxy to ContentStoreProxy:%s"),
			ex.ice_name().c_str() );
		return false;
	}
	
	return true;	
}
std::string getContentTypeFromIndexData( const ZQ::IdxParser::IndexData& data )
{
	switch(data.getIndexType())
	{
		case ZQ::IdxParser::IndexData::INDEX_TYPE_VVX:
			return TianShanIce::Storage::subctVVX;
		case ZQ::IdxParser::IndexData::INDEX_TYPE_VV2:
			return TianShanIce::Storage::subctVV2;
		case ZQ::IdxParser::IndexData::INDEX_TYPE_VVC:
			return "vvc";
		default:
			return "";
	}
}
std::string timeNow()
{
	char szBuf[256];
	szBuf[sizeof(szBuf)-1] = 0;
	const char* p = ZQ::common::TimeUtil::TimeToUTC( ZQ::common::now(), szBuf, sizeof(szBuf)-2 );
	return std::string(p);
}

int64 timeToInt( const std::string& timeStr )
{
	return ZQ::common::TimeUtil::ISO8601ToTime( timeStr.c_str() );
} 


bool needGetAttributeFromRemote( const TianShanIce::Properties& metaData , const std::string& PlaylistID, bool bPWE = false )
{
	std::string strStampLastUpdate;
	std::string strStampFilledBySS;
	int pwe = 0;
	ZQTianShan::Util::getPropertyDataWithDefault( metaData , METADATA_StampLastUpdated , "", strStampLastUpdate );
	ZQTianShan::Util::getPropertyDataWithDefault( metaData , METADATA_STAMPFilledBySS ,"" , strStampFilledBySS );
	ZQTianShan::Util::getPropertyDataWithDefault( metaData , METADATA_ContentInPWE , 0, pwe );
	int64 stampLastUpdate = 0;
	int64 stampFilledBySS = 0;
	if( !strStampFilledBySS.empty() )
	{
		stampFilledBySS = timeToInt( strStampFilledBySS );
	}
	if( !strStampLastUpdate.empty() )
	{
		stampLastUpdate = timeToInt( strStampLastUpdate );
	}
	int64 cur = ZQ::common::now();

	int64 deltaA = cur - stampFilledBySS;
	int64 deltaB = cur - stampLastUpdate;

	int64 delta = deltaA > deltaB ? deltaB : deltaA;

	int64 confInterval = ( pwe >= 1 ) ? (int64)GAPPLICATIONCONFIGURATION.playlistItemConf.lGetContentAttributeFromRemoteForPWE :
										(int64)GAPPLICATIONCONFIGURATION.playlistItemConf.lGetContentAttributeFromRemoteThreshold;

	if( delta > confInterval )
	{
		glog(ZQ::common::Log::L_DEBUG,CHECKCTNT("stampFilledBySS[%s] stampLastUpdate[%s] now[%s] threashold[%ld],pwe[%s] need get attribute from remote"),
			strStampFilledBySS.c_str(), strStampLastUpdate.c_str() , timeNow().c_str(), confInterval , pwe >= 1 ?"true":"false");
		return true;
	}
	return false;
}

bool CheckContent::updateContentAttribute( const std::string& strFullContentName ,const ZQ::IdxParser::IndexData& data ,const std::string& PlaylistID)
{
	std::vector<std::string> pathInfo;
	ZQ::common::stringHelper::SplitString(strFullContentName,pathInfo,"/","/");
	if( pathInfo.size() < 2 )
	{
		glog(ZQ::common::Log::L_WARNING,CHECKCTNT("invalid content name [%s]"),strFullContentName.c_str());
		return false;
	}
	//open volume 
	TianShanIce::Storage::FolderPrx volPrx = TianShanIce::Storage::FolderPrx::uncheckedCast( _ctntStorageServicePrx );	
	for( size_t idx  = 0 ; idx < ( pathInfo.size() - 1 ) ; idx ++ )
	{
		try
		{
			volPrx = volPrx->openSubFolder( pathInfo[idx] , false , 0 );
			if(!volPrx)
			{
				glog(ZQ::common::Log::L_WARNING,CHECKCTNT("failed to open volume[%s] for content[%s] "), pathInfo[idx].c_str() , strFullContentName.c_str() );
				return false;
			}
		}
		catch( const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_WARNING,CHECKCTNT("failed to open volume[%s] for content[%s] due to TianShanIce exception[%s]"),
				pathInfo[idx].c_str() , strFullContentName.c_str() , ex.message.c_str() );
			return false;
		}
		catch( const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING,CHECKCTNT("failed to open volume[%s] for content[%s] due to  exception[%s]"),
				pathInfo[idx].c_str() , strFullContentName.c_str() , ex.ice_name().c_str() );
			return false;
		}
	}
	
	TianShanIce::Storage::UnivContentPrx contentPrx = NULL;
	try
	{
		contentPrx = TianShanIce::Storage::UnivContentPrx::uncheckedCast( volPrx->openContent( pathInfo[pathInfo.size() - 1] , "", true ) );
	}
	catch( const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_WARNING, CHECKCTNT("failed to create [%s] for content[%s] due to [%s]"),
			pathInfo[pathInfo.size() - 1].c_str() , strFullContentName.c_str() , ex.message.c_str());
		return false;
	}
	catch( const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_WARNING, CHECKCTNT("failed to create [%s] for content[%s] due to [%s]"),
			pathInfo[pathInfo.size() - 1].c_str() , strFullContentName.c_str() , ex.ice_name().c_str());
		return false;
	}

	try
	{

		TianShanIce::Properties props;
		std::string strTimeNow = timeNow();
		ZQTianShan::Util::updatePropertyData( props , METADATA_PlayTime , data.getPlayTime() );
		ZQTianShan::Util::updatePropertyData( props , METADATA_BitRate , data.getMuxBitrate() );
		ZQTianShan::Util::updatePropertyData( props , METADATA_SubType , getContentTypeFromIndexData(data) );
		ZQTianShan::Util::updatePropertyData( props , METADATA_STAMPFilledBySS , strTimeNow );
		ZQTianShan::Util::updatePropertyData( props , METADATA_ContentInPWE , data.isPWE() ? 1 : 0 );

		contentPrx->setMetaData( props );
		glog(ZQ::common::Log::L_DEBUG,CHECKCTNT("update content attribute:playtime[%lu] bitrate[%lu] time[%s] pwe[%s]"),
			data.getPlayTime() , data.getMuxBitrate(), strTimeNow.c_str(), data.isPWE() ? "true":"false");
	}
	catch( const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_WARNING, CHECKCTNT("failed to update properties for content[%s] due to [%s]"),
			strFullContentName.c_str() , ex.message.c_str());
		return false;
	}
	catch( const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_WARNING, CHECKCTNT("failed to update properties for content[%s] due to [%s]"),
			strFullContentName.c_str() , ex.ice_name().c_str());
		return false;
	}
	return true;
}

bool CheckContent::getItemTypeFromRemote( const std::string& strItemName,
											  std::string& strFullName,
											  const std::string& PlaylistID,
											  ULONG* fileFlag ,
											  bool bLocalContent )
{
	std::string strItem = strItemName;
	if( mbEdgeServer )
	{
		std::string::size_type pos = strItem.rfind('/');
		if( pos != std::string::npos )
		{
			strItem = strItem.substr( pos + 1 );
		}
	}

	using namespace ZQ::IdxParser;
	IndexData idxData;
	IndexFileParser parser(*mIdxParserEnv);
	
	glog(ZQ::common::Log::L_INFO,CHECKCTNT("try to get asset information from Remote for content[%s]"), strItem.c_str() );
	if( !parser.ParseIndexFileFromVstrm( strItem , idxData, false, "", bLocalContent ,bLocalContent ) )
	{
		mLastError = parser.getLastError();
		glog(ZQ::common::Log::L_ERROR,
			CHECKCTNT("can't get item information for [%s]") , strItem.c_str() );
		return false;
	}
	glog(ZQ::common::Log::L_DEBUG,CHECKCTNT("now we get asset information from Remote for content[%s]"), strItem.c_str() );

	if( idxData.getIndexType() == IndexData::INDEX_TYPE_VVX)
	{
		strFullName = "\\vod\\" + strItem;
	}
	else if( idxData.getIndexType() == IndexData::INDEX_TYPE_VV2 )
	{
		strFullName = _strHDot264Driver + strItem;
	}
	else if( idxData.getIndexType() == IndexData::INDEX_TYPE_VVC )
	{
		strFullName = "\\vvc\\" + strItem;
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,CHECKCTNT("unkown item index type"));
		return false;
	}
	updateContentAttribute( strItemName , idxData , PlaylistID );
	return true;
}

TianShanIce::Storage::UnivContentPrx CheckContent::getContentProxy( const std::string& itemfullname )
{
	TianShanIce::Storage::UnivContentPrx contentPrx = NULL;
	try
	{
		// modify for ticket 16053
		contentPrx = TianShanIce::Storage::UnivContentPrx::uncheckedCast( _ctntStorageServicePrx->openContentByFullname( itemfullname) );
		
		//if( mbEdgeServer)//full auto sync is turned on
		//{
		//	contentPrx = TianShanIce::Storage::UnivContentPrx::uncheckedCast( _ctntStorageServicePrx->openContentByFullname( itemfullname) );
		//}
		//else
		//{
		//	std::vector<std::string> paths;
		//	ZQ::common::stringHelper::SplitString( itemfullname ,paths , "/" ,"/" );
		//	if( paths.size() >= 2)
		//	{
		//		TianShanIce::Storage::VolumePrx volume = _ctntStorageServicePrx->openVolume( paths[0] );
		//		if( volume )
		//			contentPrx =  TianShanIce::Storage::UnivContentPrx::uncheckedCast( volume->openContent( paths[1] , "" , false ) );
		//	}
		//}
	}
	catch( const Ice::ObjectNotExistException& )
	{
		contentPrx = NULL;
	}
	return contentPrx;
}

bool CheckContent::GetItemType(const std::string& strItem,
							   std::string& strFullName,
							   const std::string& PlaylistID,
							   ULONG*	fileFlag)
{
	if ( strItem.empty() )
	{
		glog(ZQ::common::Log::L_ERROR,CHECKCTNT("no item name passed in when invoking GetItemType"));
		return false;
	}
	strFullName="";
	//if( gStreamSmithConfig.lEnableQueryFromContentStore >= 1)
	{
		try
		{
			DWORD dwStart = GetTickCount();
			
			TianShanIce::Storage::UnivContentPrx prx = getContentProxy(strItem);

			if( !prx )
			{
				if(mbEdgeServer)
				{
					return getItemTypeFromRemote(strItem,strFullName , PlaylistID ,fileFlag ,false );
				}
				else
				{
					glog(ZQ::common::Log::L_ERROR,CHECKCTNT("getItemType() failed to find content [%s]"),strItem.c_str());
					return false;
				}
			}
			if ( mbEdgeServer )
			{
				TianShanIce::Storage::ContentState ctntState = prx->getState();
				TianShanIce::Properties metaData = prx->getMetaData();
				Ice::Int  bLocalContentForVstrm = 0;
				ZQTianShan::Util::getPropertyDataWithDefault( metaData , SYS_PROP(ContentInLocalFileSystem) , 0 , bLocalContentForVstrm );
				if( (TianShanIce::Storage::csInService != ctntState) && needGetAttributeFromRemote(metaData , PlaylistID) )
				{
					glog(ZQ::common::Log::L_DEBUG, CHECKCTNT("asset in local DB is not at InService state, qeury from VstrmAPI for content[%s]"), strItem.c_str() );
					return getItemTypeFromRemote(strItem,strFullName , PlaylistID ,fileFlag , bLocalContentForVstrm != 0 );//bLocalContentForVstrm != 0 means content in local file system					
				}
			}

			std::string strItemRealName = prx->getMainFilePathname();
			std::string	strType = prx->getSubtype();
			//std::string	strType = prx->getSubtype2(); // temporary to call getSubtype2() instead, in order not to break TianShanIce.dll and others' dependencies
			if(strType.empty())
			{
				glog(ZQ::common::Log::L_ERROR,CHECKCTNT("getItemType() can't get item sub type "));
				return false;
			}
			else
			{
				if(strType == TianShanIce::Storage::subctVVX)
				{
					strFullName ="\\vod\\";
				}
				else if( strType == TianShanIce::Storage::subctVV2 )
				{
					strFullName = _strHDot264Driver;
				}
				else if( stricmp(strType.c_str() , "vvc" ) == 0 )
				{
					strFullName ="\\vvc\\";
				}
				else
				{
					glog(ZQ::common::Log::L_ERROR,CHECKCTNT("getItemType() not supported type[%s] for content[%s]"), strType.c_str() , strItem.c_str());
					return false;
				}

				TianShanIce::Properties props = prx->getMetaData();
				TianShanIce::Properties::const_iterator itProp = props.find(METADATA_nPVRLeadCopy);
				if( itProp != props.end())
				{
					if(itProp->second.length() > 0 )
					{
						if (fileFlag != NULL )
						{							
							(*fileFlag) |= STREAMSMITH_FILE_FLAG_NPVR;
						}
#ifdef _NPVR_TEMP_NAME_SOULTION_
						std::string::size_type pos = strItemRealName.find("\\");
						if( pos != std::string::npos )
						{
							strItemRealName = strItemRealName.substr( pos + 1 );
							glog(ZQ::common::Log::L_INFO, CHECKCTNT("getItemType() temp solution for nPVR realName [%s]"),
								strItemRealName.c_str() );
						}
#endif _NPVR_TEMP_NAME_SOULTION_

						glog(ZQ::common::Log::L_INFO, CHECKCTNT("getItemType() item[%s] is a npvr item"),
							strItem.c_str() );
					}
				}

				strFullName += strItemRealName;
				glog(ZQ::common::Log::L_DEBUG,CHECKCTNT("getItemType() get type[%s] for content[%s]"), strType.c_str(), strItem.c_str() );
			}
			
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,CHECKCTNT("getItemType() caugt TianShanIce expcetion with content[%s], exception [%s]"), strItem.c_str() ,ex.message.c_str());
			return false;
		}		
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR,CHECKCTNT("GetItemType() caught Ice exception with content[%s], exception [%s]"), strItem.c_str(),ex.ice_name().c_str());
			//SetEvent(_hEvent);
			return false;
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR,CHECKCTNT("GetItemType() caught unexpect error for content %s"), strItem.c_str());
			//SetEvent(_hEvent);
			return false;
		}
	}
	return true;
}

bool CheckContent::GetItemAttributeFromRemote(const std::string& strItemName ,  long& lPlayTime , 
											  long& lBitRate, bool& bPWE,
											  const std::string& PlaylistID,
											  bool bLocalContent )
{	
	std::string strItem = strItemName;
	if( mbEdgeServer )
	{
		std::string::size_type pos = strItem.rfind('/');
		if( pos != std::string::npos )
		{
			strItem = strItem.substr( pos + 1 );
		}
	}

	using namespace ZQ::IdxParser;
	IndexData idxData;
	IndexFileParser parser(*mIdxParserEnv);
	//glog(ZQ::common::Log::L_DEBUG,CHECKCTNT("try to get asset information from remote for content[%s]"), strItem.c_str() );
	if( !parser.ParseIndexFileFromVstrm( strItem , idxData ,false ,"", bLocalContent , bLocalContent ) )
	{
		mLastError = parser.getLastError();
		glog(ZQ::common::Log::L_ERROR,	CHECKCTNT("can't get item information for [%s] from remote") , strItem.c_str() );
		return false;
	}
	lPlayTime	= static_cast<long>( idxData.getPlayTime() );
	lBitRate	= static_cast<long>( idxData.getMuxBitrate() );
	bPWE		= idxData.isPWE();

	glog(ZQ::common::Log::L_DEBUG,CHECKCTNT("got asset information from Remote for content[%s]: playtime[%ld] bitrate[%ld] pwe[%s]"), 
		strItem.c_str(), lPlayTime , lBitRate , bPWE ? "true":"false" );

	
	updateContentAttribute( strItemName , idxData , PlaylistID );
	return true;
}



bool CheckContent::GetItemAttribute(const	std::string& strItem , long&	lPlayTime ,long&	lBitRate, long&	lTotalTime, bool& bPWE,
									const	std::string& PlaylistID,
									bool	bCuein, bool	bCueout,int		inTimeoffset,int		outTimeoffset)
{
	//glog(ZQ::common::Log::L_DEBUG , CHECKCTNT("Start query contentStore with item[%s] "),strItem.c_str() );
	if( strItem.empty() )
	{
		glog(ZQ::common::Log::L_ERROR,CHECKCTNT("GetItemAttribute() no item name passed in,return with nothing available"));
		return false;
	}

	lPlayTime	=	-1;
	lBitRate	=	0;
	//if( gStreamSmithConfig.lEnableQueryFromContentStore >= 1)
	{
		try
		{
			DWORD dwStart = GetTickCount();
			TianShanIce::Storage::UnivContentPrx prx = getContentProxy(strItem);
			if( !prx )
			{
				if(mbEdgeServer)
				{
					if( !GetItemAttributeFromRemote(strItem,lPlayTime,lBitRate,bPWE,PlaylistID , false ))
						return false;			
				}
				else
				{
					glog(ZQ::common::Log::L_ERROR, CHECKCTNT("GetItemAttribute() failed to find content [%s]"),strItem.c_str() );
					return false;
				}
			}
			else
			{
				TianShanIce::Properties metaData = prx->getMetaData();
				Ice::Int	pweMode = 0;
				ZQTianShan::Util::getPropertyDataWithDefault( metaData , METADATA_ContentInPWE , 0 , pweMode );
				bPWE = pweMode >= 1;
				if ( mbEdgeServer )
				{
					TianShanIce::Storage::ContentState ctntState = prx->getState();
					
					Ice::Int  bLocalContentForVstrm = 0;
					
					ZQTianShan::Util::getPropertyDataWithDefault( metaData , SYS_PROP(ContentInLocalFileSystem) , 0 , bLocalContentForVstrm );
					if( ctntState < TianShanIce::Storage::csInService && ctntState >= TianShanIce::Storage::csProvisioning)
						bPWE = true;
					
					if( (TianShanIce::Storage::csInService != ctntState) && needGetAttributeFromRemote(metaData,PlaylistID) )
					{
						glog(ZQ::common::Log::L_DEBUG, CHECKCTNT("asset in local DB is not at InService state, qeury from VstrmAPI with asset[%s]"),
							strItem.c_str() );
						if( !GetItemAttributeFromRemote(strItem,lPlayTime , lBitRate , bPWE , PlaylistID , bLocalContentForVstrm != 0 ))//bLocalContentForVstrm != 0 means content in local file system
						{
							return false;
						}						
					}
					else
					{
						lBitRate = prx->getBitRate();
						lPlayTime =(long) prx->getPlayTimeEx();
					}
				}
				else	
				{
					lBitRate = prx->getBitRate();
					lPlayTime =(long) prx->getPlayTimeEx();
				}			
				
 				glog(ZQ::common::Log::L_DEBUG, CHECKCTNT("GetItemAttribute() get content[%s] attribute , playtime[%d] bitrate[%d] Pwe[%s]"), 
 					strItem.c_str(), lPlayTime, lBitRate, bPWE ?"true":"false" );
			}
		}
		catch (::TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,CHECKCTNT("GetItemAttribute() caught TianShan exception when getting content[%s] attribute, exception[%s]"),
				strItem.c_str(),ex.message.c_str());
			return false;
		}
		catch( const Ice::ObjectNotExistException& )
		{
			if( !GetItemAttributeFromRemote(strItem,lPlayTime,lBitRate,bPWE,PlaylistID,false))
				return false;
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR,CHECKCTNT("GetItemAttribute() caught ice exception when getting content[%s] attribute,exception[%s]"),
				strItem.c_str(),ex.ice_name().c_str());
			if( gStreamSmithConfig.lUseLocalVvxParser < 1) 			
				return false;
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR,CHECKCTNT("GetItemAttribute() caught unexpect error when getting content[%s] attribute"),strItem.c_str());
			if( gStreamSmithConfig.lUseLocalVvxParser < 1) 			
				return false;
		}
	}	

	long lCueSectionLen = 0;

	if (inTimeoffset != 0) 		
	{			
		lCueSectionLen = inTimeoffset;
	}
	if (outTimeoffset != 0)		
	{			
		lCueSectionLen = lPlayTime -( outTimeoffset - lCueSectionLen );
	}

	lTotalTime	= lPlayTime;
	lPlayTime	-=lCueSectionLen;
	glog(ZQ::common::Log::L_INFO, CHECKCTNT("got [%s]'s attribute: playtime[%ld] bitrate[%ld] pwe[%s] according to "
		"cuein[%d] inTimeOffset[%d] cueout[%d] outTimeOffset[%d]"),
		strItem.c_str(), lPlayTime,lBitRate,bPWE?"true":"false", bCuein,inTimeoffset,bCueout,outTimeoffset);
	return true;
}

}}//namespace ZQ::StreamSmith

