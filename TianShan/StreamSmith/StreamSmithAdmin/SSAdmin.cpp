// SSAdmin.cpp: implementation of the SSAdmin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSAdmin.h"
#include <TianShanDefines.h>
#include <TsTransport.h>
#include <ostream>
#include <time.h>


#define CHECKVAR1(var,x1,tips)	 if( !(var.size()==x1) ){log(#tips);return OPERATION_INVALIDPARAMETER;}

#define CHECKVAR2(var,x1,x2,tips)	 if( !(var.size()==x1 ||var.size()==x2 ) ){log(#tips);return OPERATION_INVALIDPARAMETER; }

#define CHECKVAR3(var,x1,x2,x3,tips)	 if( !(var.size()==x1 ||var.size()==x2 ||var.size()==x3) ){log(#tips);return OPERATION_INVALIDPARAMETER; }




using namespace std;


SSAdmin::SSAdmin()
{
	int i=0;
	m_Ic=Ice::initialize(i,NULL);
	m_bShowDetail=false;
	m_bDestroyCommunicator = true;
}
SSAdmin::SSAdmin(::Ice::CommunicatorPtr ic)
{
	m_Ic = ic;
	m_bShowDetail = false;
	m_bDestroyCommunicator = false;
}

SSAdmin::~SSAdmin()
{
	m_AdminPrx=NULL;
	if (m_bDestroyCommunicator) 
	{
		m_Ic->destroy();
	}	
}

BEGIN_CMDROUTE(SSAdmin,CCmdParser)
	COMMAND(Help,Help)
	COMMAND(Connect,Connect)
	COMMAND(Play,play)
	COMMAND(Disconnect,Disconnect)
	COMMAND(PushItem,PushItem)
	COMMAND(CreatePlaylist,CreatePlaylist)
	COMMAND(ListPlaylist,ListPlaylist)
	COMMAND(showDetail,showDetail)
	COMMAND(destroy,destroy)
	COMMAND(seek,seek)
	COMMAND(setspeed,setspeed)
	COMMAND(pause,pause)
	COMMAND(getState,getState)
	COMMAND(resume,resume)
	COMMAND(listsize,listsize)
	COMMAND(listcurrent,listcurrent)
	COMMAND(sysPause,sysPause)
	COMMAND(UseQam,UseQam)
	COMMAND(GetInfo,GetInfo)
	COMMAND(ListItem,ListItem)
	COMMAND(CreateByRes,CreateByRes)
	COMMAND(listStreamer,listStreamer)
	COMMAND(getNetID,getNetID)
	COMMAND(SeekStream,SeekStream)
	COMMAND(SetRate,SetRate)
	COMMAND(ShowMem,ShowMem)
	COMMAND(erase,erase)
	COMMAND(setdestMac,setdestMac)
	COMMAND(skipToItem,skipToItem)
	COMMAND(PushItemWithPID,PushItemWithPID)
	COMMAND(PushItemWithCS,PushItemWithCS)
	COMMAND(PrepareContext,PrepareContext)
	COMMAND(Version,Version)
	COMMAND(SetPID,SetPID)
	COMMAND(PushNASItem,PushNASItem)	
	COMMAND(playEx,PlayEx)
	COMMAND(PauseEx,PauseEx)
	COMMAND(getReplicas,getReplicas)
	COMMAND(PushItemR,PushItemR)
	///for embed content store
	COMMAND(ConnectCS,ConnectCS)
	COMMAND(openContent,openContent)
	COMMAND(openVolume,openVolume)
	COMMAND(getCapacity,getCapacity)
	COMMAND(listContents,listContents)
	COMMAND(getVolumeName,getVolumeName)
	COMMAND(openSubVolume,openSubVolume)
	COMMAND(getParentVolume,getParentVolume)
	COMMAND(destroyVolume,destroyVolume)
	COMMAND(getCSType,getCSType)
	COMMAND(listVolumes,listVolumes)
	COMMAND(openContentByFullname,openContentByFullname)
	COMMAND(getContentName,getContentName)
	COMMAND(getContentMetaData,getContentMetaData)
	COMMAND(getContentState,getContentState)
	COMMAND(getContentProvisionTime,getContentProvisionTime)
END_CMDROUTE()
int SSAdmin::Version (VARVEC& var)
{
	log("ZQ TianShan StreamSmith Admin utility");
	log("Version:1.0.2.1");
	log("for more detail,please use help command");
	return OPERATION_SUCCESS;
}
int SSAdmin::Help(VARVEC& var)
{
	log("================================================================");	
	log("connect   \tconnect to streamsmith");
	log("Play      \tplay a specified playlist,last playlist is a default");
	log("DisConnect\tDisconnect from the streamsmith");
	log("PushItem  \tpushitem into playlist");
	log("PushNASItem\tpushitem with nas url");
	log("CreatePlaylist\tcrate a playlist");
	log("setdestMac\tset destination macaddress");
	log("ListPlaylist\tshow the playlists of the streamsmith");
	log("destroy   \tdestroy specified or current playlist");
	log("erase     \terase a specified item with ctrlNum");
	log("seek      \tseek to a spcified position with control number and offset");
	log("SeekStream\tSeek in the stream wide,you should provide offset and start position");
	log("setspeed  \tset current stream's speed");
	log("pause     \tpause current stream");
	log("getState  \tshow playlist's state");
	log("resume    \tresume stream");
	log("listSize  \tshow playlist's item count");
	log("listCurrent\tshow current item Control Number of specified playlist");
	log("syspause  \tpause a while");
	log("useqam    \tallocate resource with Qam Resource Manager");
	log("GetInfo   \tget stream information");
	log("ListItem  \tlist playlist's item");
	log("SkipToItem\tskip to specified item with ctrlNum");
	log("CreateByRes\tI forget the usage of this command ^_^");
	log("listStreamer\tlist streamers of the machine which streamsmith runs on");
	log("getNetId  \tget streamsmith net identity");
	log("DestroyAll\tdestroy all playlists runs on streamsmith service");
	log("parseini  \tuse a ini file to command the streamsmithAdmin tool");
	log("PlayEx \tuse new play method");
	log("PauseEx \tuse new pause method");
	log("quit      \tquit this tool");
	log("================================================================");
	return OPERATION_SUCCESS;
}

int SSAdmin::Connect(VARVEC& var)
{
	//I need 2 parameter at most for this function 
	if(!( var.size()==1 ||var.size()==2 ))
	{//invalid parameter
		return OPERATION_INVALIDPARAMETER;
	}
	m_AdminPrx=NULL;
	std::string	strServer;
	std::string	strLocal;
	if(var.size()==1)
	{
		strServer=var[0];
		strLocal="default -p 5566";
	}
	else
	{
		strServer=var[0];
		strLocal=var[1];
	}
	if (strServer.find(":")==std::string::npos) 
	{
		std::string strTemp = "StreamSmith:";
		strServer = strTemp + strServer;
	}
	try
	{//try to get streamsmith object proxy
		m_AdminPrx=StreamSmithAdminPrx::checkedCast(m_Ic->stringToProxy(strServer));		
	}
	catch(TianShanIce::BaseException& ex)
	{
		err("TianShan Exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (Ice::ConnectionTimeoutException& ) 
	{
		err("ice exception");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (Ice::Exception& ex)
	{
		err("ice exception :%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...)
	{
		err("unexpect error");
		return OPERATION_FAIL;
	}
	if(!m_AdminPrx)
	{
		err("can't get streamsmith");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::Disconnect(VARVEC& var)
{
	ADMINCHECK();
	m_AdminPrx=NULL;
	return OPERATION_SUCCESS;
}
int SSAdmin::getNetID(VARVEC& var)
{
	ADMINCHECK();
	log("%s",m_AdminPrx->getNetId().c_str());
	return OPERATION_SUCCESS;
}
int SSAdmin::listStreamer(VARVEC& var)
{
	ADMINCHECK();	
	TianShanIce::Streamer::StreamerDescriptors strmers=m_AdminPrx->listStreamers();
	for(int i=0;i<(int)strmers.size();i++)
	{
		log("streamer %s",strmers[i].deviceId.c_str());
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::skipToItem(VARVEC& var)
{
	ADMINCHECK();
	if( !(var.size()==1 || var.size()==2 ) )
	{
		log("usage:skipToItem CtrlNum [guid] ");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string	strGuid ;
	int ctlNum = atoi(var[0].c_str());
	if(var.size()==2)
	{
		strGuid = var[1];
		m_lastEffectGuid = strGuid;
	}	
	try
	{
		DWORD	dwTime=GetTickCount();	
		std::vector<int> vec;
		PlaylistExPrx	prx=PlaylistExPrx::uncheckedCast(m_AdminPrx->openPlaylist(strGuid,vec,false));
		if(!prx)
		{
			err("Can't find playlist through guid %s",strGuid.c_str());
			return OPERATION_FAIL;
		}
		prx->skipToItem(ctlNum,true);
		dwTime=GetTickCount()-dwTime;		
		if(m_bShowDetail)
		{
			log("play ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
const char* dumpStreamState( ::TianShanIce::Streamer::StreamState state)
{
	switch( state )
	{
	case TianShanIce::Streamer::stsSetup:
		return "TianShanIce::Streamer::stsSetup";
	case TianShanIce::Streamer::stsStreaming:
		return "TianShanIce::Streamer::stsStreaming";
	case TianShanIce::Streamer::stsPause:
		return "TianShanIce::Streamer::stsPause";
	case TianShanIce::Streamer::stsStop:
		return "TianShanIce::Streamer::stsStop";
	default:
		return "unknown state";
	}
}
void	SSAdmin::dumpStreamInfo( const TianShanIce::Streamer::StreamInfo& info )
{
	log("ID\t\t:\t%s",info.ident.name.c_str() );
	log("State\t\t:\t%s",dumpStreamState(info.state) );
	const TianShanIce::Properties& prop = info.props;
	TianShanIce::Properties::const_iterator it = prop.begin();
	for( ; it != prop.end() ; it ++)
	{
		log("%s\t\t:\t%s",it->first.c_str() , it->second.c_str());
	}
}

int SSAdmin::PauseEx(VARVEC& var)
{
	ADMINCHECK();
	CHECKVAR2( var , 0 , 1 , "Usage:PauseEx [guid]" );
	std::string strGuid = m_lastEffectGuid;
	bool bGet = false;
	if( var.size() == 1 )
	{
		strGuid = var[0];
		bGet = true;
	}
	TianShanIce::StrValues expectValues;
	expectValues.push_back("CURRENTPOS");
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");

	TianShanIce::Streamer::StreamInfo infoRet ;

	try
	{
		DWORD dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		infoRet = m_plPrx->pauseEx( expectValues);
		dumpStreamInfo(infoRet);

	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::PlayEx(VARVEC& var)
{
	ADMINCHECK();
	CHECKVAR2( var , 3 , 4 , "Usage:playEx from offset speed [guid]" );
	
	std::string strGuid = m_lastEffectGuid;
	bool bGet = false;
	if( var.size() == 4 )
	{
		strGuid = var[3];
		bGet = true;
	}
	Ice::Short	from = static_cast<Ice::Short>( atoi(var[0].c_str()) );
	Ice::Long	offset;
	sscanf(var[1].c_str(),"%lld",&offset);
	Ice::Float	speed = static_cast<Ice::Float>( atof(var[2].c_str()) );

	TianShanIce::StrValues expectValues;
	
	expectValues.push_back("CURRENTPOS");
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");
	expectValues.push_back("ITEM_CURRENTPOS");
	expectValues.push_back("ITEM_TOTALPOS");


	TianShanIce::Streamer::StreamInfo infoRet ;
	try
	{
		DWORD dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		infoRet = m_plPrx->playEx( speed , offset , from ,expectValues);
		dumpStreamInfo(infoRet);
		
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::play(VARVEC& var)
{
	ADMINCHECK();
	if( !(var.size()==0 ||var.size()==1 ||var.size()==2) )
	{
		log("usage:play [bandwidth] [guid] ");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string	strGuid;	
	long	lBandWidth =5000*1000;
	if(var.size()==2)
	{
		strGuid=var[1];
		m_lastEffectGuid=strGuid;
		lBandWidth=atoi(var[0].c_str());
	}
	else if(var.size()==1)
	{
		lBandWidth=atoi(var[0].c_str());
		strGuid=m_lastEffectGuid;
	}
	else 
	{	
		strGuid=m_lastEffectGuid;
	}
	
	try
	{
		DWORD	dwTime=GetTickCount();
		std::vector<int> vec;
		PlaylistExPrx	prx=PlaylistExPrx::uncheckedCast(m_AdminPrx->openPlaylist(strGuid,vec,false));
		if(!prx)
		{
			err("Can't find playlist through guid %s",strGuid.c_str());
			return OPERATION_FAIL;
		}
		prx->commit();
		prx->setMuxRate(0,lBandWidth,0);
		if(!prx->play())
		{
			err("PLay fail");
			return OPERATION_FAIL;
		}
		dwTime=GetTickCount()-dwTime;		
		if(m_bShowDetail)
		{
			log("play ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}


int SSAdmin::PushItemWithCS(VARVEC& var)
{
	ADMINCHECK();
	if(!( var.size()==2 ||  var.size()==3 ||var.size()==4) )
	{
		log("use like this:PushItemWithPID ItemName CtrlNum CRTime [GUID]");
		return OPERATION_INVALIDPARAMETER;
	}
	int cNum = 0;
	int PID = 0;
	
	std::string	strGuid;
	std::string	strName = var[0];
	bool bGet =false;
	cNum=atoi(var[1].c_str());	
	int crTime = time(NULL);

	if (var.size()==3) 
	{
		crTime += atoi(var[2].c_str());
	}
	else if (var.size()==4)
	{
		crTime += atoi(var[2].c_str());
		
		strGuid = var[3];
		m_lastEffectGuid =strGuid;
		bGet = true;
	}
	else
	{
		strGuid = m_lastEffectGuid;
	}
	try
	{
		DWORD dwTime=GetTickCount();
		
			PLCHECK(strGuid,bGet);
			PlaylistItemSetupInfo	info;
			info.contentName=strName;
			info.criticalStart=crTime;
			info.forceNormal=false;
			info.inTimeOffset=0;
			info.outTimeOffset=0;
			info.spliceIn=false;
			info.spliceOut=false;
			info.flags=0;		
			m_plPrx->pushBack(cNum,info);
		
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("pushitem ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;	
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::PushItemWithPID(VARVEC& var)
{
	ADMINCHECK();
	if(!( var.size()==2 ||  var.size()==3 ||var.size()==4) )
	{
		log("use like this:PushItemWithPID ItemName CtrlNum [PID] [GUID]");
		return OPERATION_INVALIDPARAMETER;
	}
	int cNum = 0;
	int PID = 0;
	bool bHasPID = false;
	std::string	strGuid;
	std::string	strName = var[0];
	bool bGet =false;
	cNum=atoi(var[1].c_str());
	if (var.size()==3) 
	{
		PID = atoi(var[2].c_str());
		bHasPID =true;
	}
	else if (var.size()==4)
	{
		bHasPID = true;
		PID = atoi(var[2].c_str());
		strGuid = var[3];
		m_lastEffectGuid =strGuid;
		bGet = true;
	}
	else
	{
		strGuid = m_lastEffectGuid;
	}
	try
	{
		DWORD dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		PlaylistItemSetupInfo	info;
		if (bHasPID) 
		{
			TianShanIce::Variant  varPID;
			varPID.type = TianShanIce::vtInts;
			varPID.ints.clear();
			varPID.ints.push_back(PID);
			info.privateData["Item_PID"]=varPID;			
		}
		else
		{
			
		}
		info.contentName=strName;
		info.criticalStart=0;
		info.forceNormal=false;
		info.inTimeOffset=0;
		info.outTimeOffset=0;
		info.spliceIn=false;
		info.spliceOut=false;
		info.flags=0;		
		m_plPrx->pushBack(cNum,info);
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("pushitem ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;	
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}

int SSAdmin::PushNASItem(VARVEC& var)
{
	ADMINCHECK();
	if(!( var.size()==2 ||  var.size()==3||var.size()==4 ||var.size()==5 || var.size() == 6 ) )
	{
		log("use like this:PushItem NasItemName CtrlNum [rawName] [cuein] [cueout] [guid]");
		return OPERATION_INVALIDPARAMETER;
	}	
	int		CtrlNum = 0;
	std::string	strGuid;
	std::string	strName;
	bool		bGet = false;
	strName = var[0];
	CtrlNum = atoi( var[1].c_str() );
	int cuein = 0;
	int cueout = 0;
	bool bEnableCuein = false;
	bool bEnableCueout = false;
	std::string rawName = "DumMyName";
	if( var.size() == 2 )
	{
		strGuid =m_lastEffectGuid;		
	}
	else if(var.size() == 3 )
	{
		rawName = var[2];
		strGuid = m_lastEffectGuid;		
	}
	else if(var.size() == 4)
	{
		bEnableCuein = true;
		cuein=atoi(var[3].c_str());
		strGuid =m_lastEffectGuid;		
	}
	else if (var.size() == 5)
	{
		bEnableCuein = true;
		bEnableCueout = true;
		cuein=atoi(var[3].c_str());
		cueout=atoi(var[4].c_str());
		strGuid =m_lastEffectGuid;
	}
	else
	{
		bEnableCuein = true;
		bEnableCueout = true;
		cuein=atoi(var[3].c_str());
		cueout=atoi(var[4].c_str());	
		strGuid=var[5];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	
	try
	{
		//strName="\\vod\\"+strName;
		DWORD dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		PlaylistItemSetupInfo	info;
		info.contentName		= rawName;
		info.criticalStart		=0;
		info.forceNormal		=false;
		info.inTimeOffset		=cuein;
		info.outTimeOffset		=cueout;
		info.spliceIn			= bEnableCuein ;
		info.spliceOut			= bEnableCueout;
		info.flags=0;		

		::TianShanIce::ValueMap& valMap =info.privateData;
		TianShanIce::Variant var;
		if (strName.find("\\\\")!=0) 
		{
			strName = "\\\\" + strName;
		}
		var.strs.clear();
		var.strs.push_back(strName);
		var.type = TianShanIce::vtStrings;
		valMap["storageLibraryUrl"] = var;

		m_plPrx->pushBack(CtrlNum,info);
		dwTime = GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("pushitem ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}

int SSAdmin::PushItemR(VARVEC& var)
{
	ADMINCHECK();
	if( !( var.size()==2 ||  var.size()==3 ) )
	{
		log("use like this:PushItem ItemName CtrlNum RESTRICTION[FRP]");
		return OPERATION_INVALIDPARAMETER;
	}

	int		CtrlNum=0;
	std::string	strGuid;
	std::string	strName;
	bool bGet=false;
	strName = var[0];
	CtrlNum=atoi(var[1].c_str());
	Ice::Long flag = 0;
	if(var.size()==2)
	{
		strGuid =m_lastEffectGuid;		
	}
	else if(var.size()==3)
	{	
		if( strstr(var[2].c_str(),"F") )
		{
			flag |= TianShanIce::Streamer::PLISFlagNoFF;
		}
		if( strstr(var[2].c_str(),"R") )
		{
			flag |= TianShanIce::Streamer::PLISFlagNoRew;
		}
		if( strstr(var[2].c_str(),"P") )
		{
			flag |= TianShanIce::Streamer::PLISFlagNoPause;
		}
	}
	

	try
	{	
		DWORD dwTime=GetTickCount();
		for(int i = 0 ;i < 1 ;i ++ )
		{
			PLCHECK(strGuid,bGet);
			PlaylistItemSetupInfo	info;
			info.contentName		=	strName;
			info.criticalStart		=	0;
			info.forceNormal		=	false;
			info.inTimeOffset		=	0;
			info.outTimeOffset		=	0;
			info.spliceIn			=	false;
			info.spliceOut			=	false;
			info.flags				=	flag;

			m_plPrx->pushBack(CtrlNum,info);
		}
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("pushitem ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}

int SSAdmin::PushItem(VARVEC& var)
{
	ADMINCHECK();
	if(!( var.size()==2 ||  var.size()==3||var.size()==4 ||var.size()==5) )
	{
		log("use like this:PushItem ItemName CtrlNum [cuein] [cueout] [guid]");
		return OPERATION_INVALIDPARAMETER;
	}	
	int		CtrlNum=0;
	std::string	strGuid;
	std::string	strName;
	bool bGet=false;
	strName=var[0];
	CtrlNum=atoi(var[1].c_str());
	int cuein=0;
	int cueout=0;
	bool bEnableCuein = false;
	bool bEnableCueout = false;
	if(var.size()==2)
	{
		strGuid =m_lastEffectGuid;		
	}
	else if(var.size()==3)
	{	
		cuein=atoi(var[2].c_str());
	}
	else if (var.size()==4)
	{		
		cuein=atoi(var[2].c_str());
		cueout=atoi(var[3].c_str());
	}
	else
	{
		cuein=atoi(var[2].c_str());
		cueout=atoi(var[3].c_str());	
		strGuid=var[4];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	
	try
	{	
		DWORD dwTime=GetTickCount();
		for(int i = 0 ;i < 1 ;i ++ )
		{
			PLCHECK(strGuid,bGet);
			PlaylistItemSetupInfo	info;
			info.contentName=strName;
			info.criticalStart=0;
			info.forceNormal=false;
			info.inTimeOffset=cuein;
			info.outTimeOffset=cueout;
			info.spliceIn= bEnableCuein ;
			info.spliceOut= bEnableCueout;
			info.flags=0;		
			m_plPrx->pushBack(CtrlNum,info);
		}
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("pushitem ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::CreateByRes(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()== 2 ))
	{
		log("usage:CreateByRes serviceGroupID bitRate");
		return OPERATION_INVALIDPARAMETER;
	}
	int gId;
	int maxBitRate;
	gId=atoi(var[0].c_str());
	maxBitRate=atoi(var[1].c_str());
	try
	{
		SHOWDETAILSTART();
		TianShanIce::SRM::ResourceMap res;
		TianShanIce::SRM::Resource resGourpID;
		TianShanIce::SRM::Resource resBandWidth;
		
		resGourpID.resourceData["servicegroup"].ints.push_back(gId);
		resBandWidth.resourceData["bandwidth"].ints.push_back(maxBitRate);
		res[TianShanIce::SRM::rtServiceGroup]=resGourpID;
		res[TianShanIce::SRM::rtTsDownstreamBandwidth]=resBandWidth;

		m_plPrx=PlaylistExPrx::uncheckedCast(m_AdminPrx->createStreamByResource(res));
		if(!m_plPrx)
		{
			err("create fail");
			return OPERATION_FAIL;
		}
//		m_plPrx->setDestination(strIP,port);
//		m_plPrx->setDestMac("1:2:3:4:5:6");
		SHOWDETAILEND(CreatePlaylist);
		m_lastEffectGuid=m_plPrx->getId();
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}

int SSAdmin::CreatePlaylist(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()== 2 ||var.size()==3))
	{
		log("usage:CreatePlaylist [destIP destPort] [spigots]");
		return OPERATION_INVALIDPARAMETER;
	}
	string strIP;
	int		port;
	int		spigots=-1;
	strIP=var[0];
	port=atoi(var[1].c_str());		

	if(var.size()==3)
	{
		spigots=atoi(var[2].c_str());
	}
	
	try
	{		
		SHOWDETAILSTART();		
		m_plPrx=PlaylistExPrx::uncheckedCast(m_AdminPrx->createStream(NULL,_ctx));
		if(!m_plPrx)
		{
			log("create fail");
			return OPERATION_FAIL;
		}
		m_plPrx->setDestination(strIP,port);
		m_plPrx->setDestMac("11:22:23:a4:45:d6");		
		m_lastEffectGuid=m_plPrx->getId();
		SHOWDETAILEND(CreatePlaylist);
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	
	return OPERATION_SUCCESS;
}
int SSAdmin::ListPlaylist(VARVEC& var)
{
	ADMINCHECK();
	try
	{
		DWORD dwTime=GetTickCount();
		PlaylistIDs ids=m_AdminPrx->listPlaylists();
		int i=1;
		PlaylistExPrx pListPrx;
		int		iSetup=0;
		int		iStreaming=0;
		int		iPause=0;
		int		iStop=0;
		int		IErr=0;
		
		for(PlaylistIDs::iterator it=ids.begin();it!=ids.end();it++)
		{
			
			pListPrx=PlaylistExPrx::uncheckedCast(m_AdminPrx->openPlaylist(it->c_str(),std::vector<int>(),false));
			if(!pListPrx)
			{
				cout<<endl;
			}
			else
			{
				cout<<i++<<"\t"<<it->c_str()<<"\t";
				StreamState st;
				try
				{
					st=pListPrx->getCurrentState();
					
					switch(st)
					{
					case stsSetup:
						{
							/*ZQ::common::MutexGuard gd(outMutex);*/cout<<"Setup"<<endl;
							iSetup++;
						}
						break;
					case stsStreaming:
						{
							iStreaming++;
							/*ZQ::common::MutexGuard gd(outMutex);*/cout<<"Streaming"<<endl;
						}
						break;
					case stsPause:
						{
							iPause++;
							/*ZQ::common::MutexGuard gd(outMutex);*/cout<<"Pause"<<endl;
						}
						break;
					case stsStop:
						{
							iStop++;
							/*ZQ::common::MutexGuard gd(outMutex);*/cout<<"Stop"<<endl;
						}
						break;
					default:
						{
							IErr++;
							/*ZQ::common::MutexGuard gd(outMutex);*/cout<<"Unknown status"<<endl;
						}
						break;
					}
				}
				catch (Ice::ObjectNotExistException& ex) 
				{
					std::ostringstream	os;
					ex.ice_print(os);
					cout<<"object not exist"<<endl;
				}
				catch (char* msg) 
				{
					cout<<endl<<"Error :"<<msg<<endl;					
				}
				catch (...) 
				{
					cout<<endl<<"Unexpect error"<<endl;
				}
			}
		}
		dwTime=GetTickCount()-dwTime;
		if(ids.size()<=0)
		{
			cout<<"no playlist"<<endl;
		}
		if(m_bShowDetail)
		{
			log("listplaylist ok with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::showDetail(VARVEC& var)
{
	if(var.size()!=1)
	{
		log("use like this:showDetail on/off");
		return OPERATION_INVALIDPARAMETER;
	}
	if(stricmp("on",var[0].c_str())==0)
	{
		m_bShowDetail=true;
	}
	else if(stricmp("off",var[0].c_str())==0)
	{
		m_bShowDetail=false;
	}
	else
	{
		err("invalid input");
		return OPERATION_INVALIDPARAMETER;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::setdestMac(VARVEC& var)
{
	ADMINCHECK();
	if(! (var.size()==1||var.size()==2) )
	{
		log("usage:setDestMac macAddress [playlist guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string	strGuid=m_lastEffectGuid;
	bool bGet =false;
	if(var.size()==2)
	{
		strGuid = var[1];
		bGet = true;
	}
	try
	{
		DWORD	dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		m_plPrx->setDestMac(var[0]);
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("setDestmac ok with time count=%d",dwTime);
		}			
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::erase(VARVEC& var)
{
	ADMINCHECK();
	if(!( var.size()==1 ||var.size()==2 ))
	{
		log("usage:erase ctrlNum [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	int ctrlNumber;
	ctrlNumber = atoi( var[0].c_str() );
	std::string	strGuid=m_lastEffectGuid;
	bool bGet = false;
	if(var.size() == 1)
	{
	}
	else
	{
		strGuid=var[2];
		bGet = true;
	}
	try
	{
		DWORD	dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		m_plPrx->erase(ctrlNumber);
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("erase ok with time count=%d",dwTime);
		}			
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}

int SSAdmin::destroy(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==0||var.size()==1))
	{
		log("Usage:destroy [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string strGuid=m_lastEffectGuid;
	bool	bGet=false;		
	if(var.size()==1)
	{
		strGuid=var[0];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	try
	{
		DWORD	dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		m_plPrx->destroy();
		dwTime=GetTickCount()-dwTime;
		if(m_bShowDetail)
		{
			log("destroy ok with time count=%d",dwTime);
		}			
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return OPERATION_SUCCESS;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return OPERATION_SUCCESS;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return OPERATION_SUCCESS;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return OPERATION_SUCCESS;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_SUCCESS;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::SeekStream(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size() == 1 || var.size() == 2 || var.size()==3 ))
	{
		log("Usage:SeekStream offset [startPos 1-begin,2-end] [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string strGuid;
	int			Offset  = 0;
	int			StartPos= 1;
	bool		bGet = false;

	Offset = atoi( var[0].c_str());
	
	if (var.size() == 2)
	{	
		StartPos = atoi (var[1].c_str() );
	}
	else if(var.size() == 3)
	{
		StartPos = atoi( var[1].c_str() );
		strGuid =var[2];
		bGet = true;
	}
	try
	{
		DWORD dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);		
		Ice::Long retOffset = m_plPrx->seekStream(Offset,StartPos);
		dwTime=GetTickCount()-dwTime;
		log("Seek Stream at %lld",retOffset);
		if(m_bShowDetail)
		{
			log("Seek OK with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}


	return OPERATION_SUCCESS;
}
int SSAdmin::seek(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==2 || var.size()==3 ||var.size()==4 || var.size()==5 ) )
	{
		log("usage:seek userCtrlNum timeoffset [startPos 0-cur,1-begin,2-end] [speed] [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string		strGuid;
	int				CtrlNum=0;
	int				offset=0;
	float			newSpeed = 0.0f;

	CtrlNum=atoi(var[0].c_str());
	offset=atoi(var[1].c_str());
	bool	bGet=false;
	int StartPos=0;
		if(var.size()==2)
	{
		strGuid=m_lastEffectGuid;		
	}
	else if(var.size()==3)
	{
		StartPos=atoi(var[2].c_str());		
		strGuid=m_lastEffectGuid;
	}
	else if( var.size() == 4 )
	{
		StartPos=atoi(var[2].c_str());		
		newSpeed = atof( var[3].c_str() );
		strGuid=m_lastEffectGuid;		
	}
	else
	{
		StartPos=atoi(var[2].c_str());
		newSpeed = atof( var[3].c_str() );
		m_lastEffectGuid=var[4];		
		strGuid=m_lastEffectGuid;
		bGet=true;
	}

	TianShanIce::Streamer::StreamInfo infoRet ;
	
	try
	{
		TianShanIce::StrValues expectValues;

		expectValues.push_back("ITEM_CURRENTPOS");
		expectValues.push_back("ITEM_TOTALPOS");
		expectValues.push_back("SPEED");
		expectValues.push_back("STATE");
		expectValues.push_back("USERCTRLNUM");
		DWORD dwTime=GetTickCount();
		PLCHECK(strGuid,bGet);
		infoRet = m_plPrx->playItem(CtrlNum,offset,StartPos,newSpeed,expectValues);
		dwTime=GetTickCount()-dwTime;
		dumpStreamInfo(infoRet);
		if(m_bShowDetail)
		{
			log("Seek OK with time count=%d",dwTime);
		}
		return OPERATION_SUCCESS;
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}

int SSAdmin::setspeed(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==2||var.size()==3))
	{
		log("usage:setspeed a b [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	float speed;
	string	strGuid;
	bool	bGet=false;
	speed=((float)(atoi(var[0].c_str()))) /(float(atoi(var[1].c_str())) );
	if(var.size()==2)
	{
		strGuid=m_lastEffectGuid;		
	}
	else
	{
		strGuid=var[2];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		m_plPrx->setSpeed(speed);
		SHOWDETAILEND(setspeed);
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}


	return OPERATION_SUCCESS;
}
int SSAdmin::pause(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==1 ||var.size()==0))
	{
		log("usage:pause [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	string	strGuid=m_lastEffectGuid;
	bool	bGet=false;
	if(var.size()==1)
	{
		strGuid=var[0];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		m_plPrx->pause();
		SHOWDETAILEND(pause);
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}

	return OPERATION_SUCCESS;
}
int SSAdmin::resume(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==0||var.size()==1))
	{
		log("usage:resume [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	string	strGuid=m_lastEffectGuid;
	bool	bGet=false;
	if(var.size()==1)
	{
		strGuid=var[0];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		m_plPrx->resume();
		SHOWDETAILEND(resume);
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}

int SSAdmin::getState(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==1||var.size()==0))
	{
		log("usage:getsatate [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	string	strGuid;
	bool	bGet=false;
	if(var.size()==1)
	{
		strGuid=var[0];
		m_lastEffectGuid=strGuid;
		bGet=true;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		TianShanIce::Streamer::StreamState state=m_plPrx->getCurrentState();
		switch(state) 
		{
		case TianShanIce::Streamer::stsSetup:
			break;
		case TianShanIce::Streamer::stsStreaming:
			break;
		case TianShanIce::Streamer::stsPause:
			break;
		case TianShanIce::Streamer::stsStop:
			break;
		default:
			break;
		}
		SHOWDETAILEND(getState);		
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::listsize(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==0 || var.size()==1))
	{
		log("usage:listsize [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	string	strGuid=m_lastEffectGuid;
	bool	bGet=false;
	if(var.size()==1)
	{
		bGet=true;
		strGuid=var[0];
		m_lastEffectGuid=strGuid;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);		
		log("there are %d item(s) in playlist %s",m_plPrx->size(),strGuid.c_str());
		SHOWDETAILEND(listsize);
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::listcurrent(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size()==0 || var.size()==1))
	{
		log("usage:listcurrent [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	string	strGuid=m_lastEffectGuid;
	bool	bGet=false;
	if(var.size()==1)
	{
		bGet=true;
		strGuid=var[0];
		m_lastEffectGuid=strGuid;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);		
		log("now item ctrlNum=%d in playlist %s",m_plPrx->current(),strGuid.c_str());
		SHOWDETAILEND(listcurrent);
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::sysPause(VARVEC& var)
{
	if(var.size()!=1)
	{
		log("usage:syspause timecount");
		return OPERATION_INVALIDPARAMETER;
	}
	Sleep(atoi(var[0].c_str()));
	return OPERATION_SUCCESS;
}
int SSAdmin::UseQam(VARVEC& var)
{
	if(!(var.size()==1 || var.size()==2 ||var.size()==3))
	{
		log("Usage:useQam serviceGroupID [BandWidth] [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	ADMINCHECK();
	std::string strGuid=m_lastEffectGuid;
	int		serviceGrpID=0;
	int		BandWidth=4000;
	bool	bGet=false;
	serviceGrpID=atoi(var[0].c_str());
	if(var.size()==2)
	{
		BandWidth=atoi(var[1].c_str());
	}
	else if(var.size()==3)
	{
		BandWidth=atoi(var[1].c_str());
		bGet=true;
		strGuid=var[2];
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		if(!m_plPrx->allocDVBCResource(serviceGrpID,BandWidth))
		{
			log("alloc dvbc resource fail");
			return OPERATION_FAIL;
		}
		TianShanIce::ValueMap var;
		m_plPrx->getInfo(TianShanIce::Streamer::infoDVBCRESOURCE,var);
		std::string destIP=var["DestIP"].strs[0];
		int	 DestPort=var["DestPort"].ints[0];
		m_plPrx->setDestination(destIP,DestPort);
		SHOWDETAILEND(UseQam);
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::GetInfo(VARVEC& var)
{
	if (!(var.size()==1 || var.size()==2))
	{
		log("usage:getinfo mask{dbvc|playpos|npt|source} [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	ADMINCHECK();
	bool bGet=false;
	std::string strGuid=m_lastEffectGuid;
	std::string	strInfo;
	int		mask=TianShanIce::Streamer::infoDVBCRESOURCE;
	if(var.size()==1)
	{
		if(stricmp( "dbvc" , var[0].c_str()) ==0)
		{
			mask=TianShanIce::Streamer::infoDVBCRESOURCE;
		}
		else if(stricmp("playpos",var[0].c_str())==0)
		{
			mask=TianShanIce::Streamer::infoPLAYPOSITION;
		}
		else if (stricmp("npt",var[0].c_str()) ==0)
		{
			mask=TianShanIce::Streamer::infoSTREAMNPTPOS;
		}
		else if( stricmp("source",var[0].c_str()) ==0 )
		{
			mask = TianShanIce::Streamer::infoSTREAMSOURCE;
		}
	}
	else if(var.size()==2)
	{
		strGuid=var[1];
		bGet = true;
	}

	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		TianShanIce::ValueMap var;
		if(!m_plPrx->getInfo(mask,var))
		{
			err("getinfo fail");
		}
		else
		{
			//ZQTianShan::dumpValueMap(var);
			TianShanIce::ValueMap::iterator it = var.begin();
			char szBuf[1024];
			for ( ; it != var.end() ; it ++ ) 
			{				
				std::string strKey = it->first;
				TianShanIce::Variant varValue = it->second;
				switch(varValue.type) 
				{
				case TianShanIce::vtInts:
					if (varValue.bRange) 
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%d]-[%d]",
								strKey.c_str(),"VtInts",varValue.ints[0],varValue.ints[1]);
					}
					else
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%d]",
									strKey.c_str(),"VtInts",varValue.ints[0]);
					}					
					break;
				case TianShanIce::vtLongs:
					if (varValue.bRange) 
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%lld]-[%lld]",
							strKey.c_str(),"vtLongs",varValue.lints[0],varValue.lints[1]);
					}
					else
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%lld]",
							strKey.c_str(),"vtLongs",varValue.lints[0]);
					}					
					break;
				case TianShanIce::vtStrings:
					if (varValue.bRange) 
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%s]-[%s]",
							strKey.c_str(),"vtStrings",varValue.strs[0].c_str(),varValue.strs[1].c_str());
					}
					else
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%s]",
							strKey.c_str(),"vtStrings",varValue.strs[0].c_str());
					}					
					break;
				case TianShanIce::vtFloats:
					if (varValue.bRange) 
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%f]-[%f]",
							strKey.c_str(),"vtFloats",varValue.floats[0],varValue.floats[0]);
					}
					else
					{
						sprintf(szBuf,"Key [%s] Value Type[%s] Value [%f]",
							strKey.c_str(),"vtFloats",varValue.floats[0]);
					}					
					break;
				default:
					{
					}
					break;
				}
				log(szBuf);
			}
		}
		SHOWDETAILEND(GetInfo);
	}
	catch (...)
	{
		err("Unexpect exception");
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::SetRate(VARVEC& var)
{
	ADMINCHECK();
	if(!(var.size() == 1 || var.size()== 2))
	{
		log("Usage:SetRate rate [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	bool bGet = false;
	std::string		strGuid;
	int rate = 0;
	
	rate = atoi(var[0].c_str());	
	if(var.size() == 2)
	{
		strGuid = var[1];
		bGet = true;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		
		m_plPrx->setMuxRate(0,rate,0);
		
		SHOWDETAILEND(UseQam);		
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::PrepareContext(VARVEC& var)
{
	CHECKVAR1(var,1,"Usage:PrepareContext srcPort");
	ADMINCHECK();
	_ctx["SrcPort"] = var[0];
	return OPERATION_SUCCESS;
}

int SSAdmin::SetPID (VARVEC& var)
{
	if ( !( var.size() == 1 ||var.size() ==2 )) 
	{
		log("Usage:SetPID pid [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	int pid = atoi(var[0].c_str());
	bool bGet=false;
	std::string strGuid;
	if (var.size() == 2) 
	{
		bGet = true;
		strGuid = var[1];
		m_lastEffectGuid = strGuid;
	}
	else
	{
		strGuid = m_lastEffectGuid;
	}
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		
		m_plPrx->setPID (pid);
		
		SHOWDETAILEND(SetPID);		
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;

}
int SSAdmin::ListItem(VARVEC& var)
{
	if(!(var.size()==0||var.size()==1))
	{
		log("usage: listItem [guid]");
		return OPERATION_INVALIDPARAMETER;
	}
	ADMINCHECK();
	bool bGet=false;
	std::string strGuid;
	if(var.size()==1)
	{
		bGet=true;
		m_lastEffectGuid=var[0];		
	}
	strGuid=m_lastEffectGuid;
	try
	{
		SHOWDETAILSTART();
		PLCHECK(strGuid,bGet);
		
		TianShanIce::IValues ivalue= m_plPrx->getSequence();
		log("There are %d items in playlist %s",ivalue.size(),m_lastEffectGuid.c_str());
		for(int i=0;i<(int)ivalue.size();i++)
		{
			log("Item %d =%d",i,ivalue[i]);
		}
		
		SHOWDETAILEND(UseQam);		
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getReplicas(VARVEC& var)
{
	ADMINCHECK();
	std::string categpry;
	std::string groupId;
	bool bLocalOnly = true;
	try
	{
		SHOWDETAILSTART();		
		TianShanIce::Replicas reps =  m_AdminPrx->queryReplicas(categpry,groupId,bLocalOnly);		
		SHOWDETAILEND(getReplicas);		
		TianShanIce::Replicas::const_iterator it = reps.begin();
		for( ; it != reps.end() ; it ++ )
		{
			log("replicaId[%s]\tstampKnew[%lld]\tstampUpdated[%lld]\tstatus[%s]",
					it->replicaId.c_str(),
					it->stampBorn,
					it->stampChanged,
					(it->replicaState == TianShanIce::stInService) ? "up":"down" );
		}
	}
	catch(const Ice::Exception& ex)
	{
		log("caught Ice Exception:%s",ex.ice_name().c_str() );
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::ShowMem(VARVEC& var)
{
	try
	{
// 		//std::string str = m_AdminPrx->ShowMemory();
// 		if(!str.empty())
// 			log("%s",str.c_str());
// 		else
// 			log("no more information...");
	}
	catch (const TianShanIce::InvalidParameter& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDPARAMETER;
	}
	catch (const TianShanIce::InvalidStateOfArt& ex) 
	{
		log("%s",ex.message.c_str());
		return TIANSHAN_INVALIDSTATE;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		log("TianShan exception:%s",ex.message.c_str());
		return TIANSHAN_GENERALERROR;
	}
	catch (const Ice::ConnectionTimeoutException&) 
	{
		log("ConnectionTimeoutException");
		return ICEERROR_CONNECTIONTIMEOUT;
	}
	catch (const Ice::TimeoutException& ) 
	{
		log("TimeoutException");
		return ICEERROR_OPERATIONTIMEOUT;
	}
	catch (Ice::Exception& ex) 
	{
		log("Ice exception:%s",ex.ice_name().c_str());
		return ICEERROR_GENERALERROR;
	}
	catch (...) 
	{
		log("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::ConnectCS(VARVEC &var)
{
	if( var.size() <= 0 )
	{
		log("usage: ConnectCS endpoint");
		return OPERATION_INVALIDPARAMETER;
	}
	
	std::string strEndpoint = var[0];
	if( strEndpoint.find(":") == std::string::npos )
	{
		strEndpoint = ADAPTER_NAME_ContentStore":" + var[0];
	}
	try
	{
		m_csPrx = ContentStorePrx::checkedCast(m_Ic->stringToProxy(strEndpoint));
	}
	catch(const Ice::Exception& ex )
	{
		err("can't connect to [%s] with exception [%s]",strEndpoint.c_str() , ex.ice_name().c_str() );
		return OPERATION_FAIL;
	}
	catch( ... )
	{
		err("can't connect to [%s]",strEndpoint.c_str( ) );
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}

int SSAdmin::openVolume(VARVEC &var)
{
	CSCHECK();
	std::string	strVolume = "";
	if( var.size() >= 1)
		strVolume = var[0];
	try
	{
		m_volumePrx = 	m_csPrx->openVolume(strVolume);
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("openVolume [%s] failed with exception[%s]",strVolume.c_str() , ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("openVolume [%s] failed with exception[%s]",strVolume.c_str() , ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::openContent(VARVEC &var)
{
	VOLUMECHECK();
	if ( var.size() < 1 )
	{
		log("usage:getConent name [type]");
		return OPERATION_INVALIDPARAMETER;
	}
	std::string strType = TianShanIce::Storage::ctMPEG2TS;
	if ( var.size() >= 2)
	{
		strType = var[1];
	}
	std::string strName = var[0];
	try
	{
		m_contentPrx = m_volumePrx->openContent( strName , strType ,false );
	}	
	catch( const TianShanIce::BaseException& ex)
	{
		err("getContent [%s] [%s]failed with exception[%s]",strName.c_str() , strType.c_str() , ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getContent [%s][%s] failed with exception[%s]",strName.c_str() , strType.c_str() , ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getCapacity(VARVEC& var)
{
	VOLUMECHECK();
	try
	{
		Ice::Long	freeSpace = 0, totalSpace = 0;
		m_volumePrx->getCapacity( freeSpace ,totalSpace );
		log("get capacity freeSapce[%lld]MB totalSpace[%lld]MB", freeSpace , totalSpace );
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getCapacity failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getCapacity failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::listContents(VARVEC& var)
{
	VOLUMECHECK();
	TianShanIce::StrValues metaNames;
	std::string	startName;
	int maxCount=0;

	try
	{
		//listContents
		ContentInfos infos = m_volumePrx->listContents( metaNames , startName , maxCount );
		log("listContent and get [%d] contents",infos.size() );
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("listContents failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("listContents failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getVolumeName(VARVEC& var)
{
	VOLUMECHECK();
	try
	{
		std::string strName = m_volumePrx->getVolumeName( );
		log("get volume name [%s]",strName.c_str() );
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getVolumeName failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getVolumeName failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
	
}
int SSAdmin::openSubVolume(VARVEC& var)
{
	VOLUMECHECK();
	std::string subName;
	bool bCreate = false;
	Ice::Long	quota = 0;//in MB
	try
	{
		m_volumePrx = m_volumePrx->openSubFolder( subName, bCreate , quota );		
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("openSubVolume failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("openSubVolume failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getParentVolume(VARVEC& var)
{
	VOLUMECHECK();
	try
	{
		m_volumePrx = m_volumePrx->parent( );		
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getParentVolume failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getParentVolume failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::destroyVolume(VARVEC& var)
{
	VOLUMECHECK();
	try
	{
		m_volumePrx->destroy( );
		m_volumePrx = NULL;			 
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("destroyVolume failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("destroyVolume failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getCSType(VARVEC& var)
{
	CSCHECK();
	try
	{
		std::string type =m_csPrx->type( );
		log("cs type [%s]", type.c_str());
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getCSType failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getCSType failed with exception[%s]", ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::listVolumes(VARVEC& var)
{
	CSCHECK();
	std::string	listFrom;
	bool includeVirtual = true;
	if(var.size() == 2 )
	{
		includeVirtual = (bool)(atoi(var[1].c_str()));
		listFrom = var[0];
	}
	else if( var.size() == 1 )
	{
		listFrom = var[0];
	}
	
	try
	{
		VolumeInfos infos =m_csPrx->listVolumes( listFrom, includeVirtual );
		
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("listVolumes [%s][%d] failed with exception[%s]", listFrom.c_str() , includeVirtual , ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("listVolumes [%s][%d] failed with exception[%s]", listFrom.c_str() , includeVirtual , ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::openContentByFullname(VARVEC& var)
{
	CSCHECK();
	std::string fullname;
	if( var.size() < 1 )
	{
		log("usage : openContentByFullname fullname");
		return OPERATION_INVALIDPARAMETER;
	}
	fullname = var[0];
	try
	{
		m_contentPrx = m_csPrx->openContentByFullname( fullname );		

	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("openContentByFullname [%s] failed with exception[%s]", fullname.c_str() ,  ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("openContentByFullname [%s] failed with exception[%s]", fullname.c_str() ,  ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getContentName(VARVEC& var)
{
	CONTENTCHECK();
	try
	{
		std::string name = m_contentPrx->getName( );
		log("content name [%s]",name.c_str() );
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getContentName failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getContentName failed with exception[%s]",   ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getContentMetaData(VARVEC& var)
{
	CONTENTCHECK();
	try
	{
		TianShanIce::Properties props = m_contentPrx->getMetaData( );
		TianShanIce::Properties::const_iterator it = props.begin();
		for( ; it != props.end() ; it ++ )
		{
			log("key[%s]\t\tvalue[%s]",it->first.c_str() , it->second.c_str() );
		}
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getContentMetaData failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getContentMetaData failed with exception[%s]",   ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}

int SSAdmin::getContentState(VARVEC& var)
{
	CONTENTCHECK();
	try
	{
		ContentState state = m_contentPrx->getState( );
		switch(state)
		{
		case csNotProvisioned:
			log("csNotProvisioned");
			break;
		case csProvisioning:
			log("csProvisioning");
			break;
		case csProvisioningStreamable:
			log("csProvisioningStreamable");
			break;
		case csInService:
			log("csInService");
			break;
		case csOutService:
			log("csCleaning");
			break;
		case csCleaning:
			log("csCleaning");
			break;
		default:
			log("unknown state");
			break;
		}
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getContentState failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getContentState failed with exception[%s]",   ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
int SSAdmin::getContentProvisionTime(VARVEC& var)
{
	CONTENTCHECK();
	try
	{
		std::string strTime = m_contentPrx->getProvisionTime( );
		log("content provision time[%s]",strTime.c_str());
	}
	catch( const TianShanIce::BaseException& ex)
	{
		err("getContentProvisionTime failed with exception[%s]", ex.message.c_str() );
		return TIANSHAN_SERVERERROR;
	}
	catch(const Ice::Exception& ex )
	{
		err("getContentProvisionTime failed with exception[%s]",   ex.ice_name().c_str() );
		return TIANSHAN_GENERALERROR;
	}
	return OPERATION_SUCCESS;
}
