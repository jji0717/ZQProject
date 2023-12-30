// WeiwooAdmin.cpp: implementation of the WeiwooAdmin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WeiwooAdmin.h"

#include <conio.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define CHECKVAR1(var,x1,tips)	 if( !(var.size()==x1) ){log(#tips);return OPERATION_INVALIDPARAMETER;}

#define CHECKVAR2(var,x1,x2,tips)	 if( !(var.size()==x1 ||var.size()==x2 ) ){log(#tips);return OPERATION_INVALIDPARAMETER; }

#define CHECKVAR3(var,x1,x2,x3,tips)	 if( !(var.size()==x1 ||var.size()==x2 ||var.size()==x3) ){log(#tips);return OPERATION_INVALIDPARAMETER; }

#define CHECKSESSMAN()				if(!m_sessManager){log("connect to server first");return OPERATION_FAIL;}

#define	RECORDTIMESTART()			DWORD dwTestTime=GetTickCount();
#define	RECORDTIMEEND()				dwTestTime=GetTickCount()-dwTestTime;
#define	SHOWTIME(x)					if(m_bShowTimeCount)log("%s using %u ms",#x,dwTestTime);

#define CATCHALL() 	catch (TianShanIce::BaseException& ex)\
{\
	log("TianShan exception:%s",ex.message.c_str());\
	return TIANSHAN_GENERALERROR;\
}\
catch (Ice::Exception& ex)\
{\
	log("Ice exception:%s",ex.ice_name().c_str());\
	return ICEERROR_GENERALERROR;\
}\
catch (...)\
{\
	log("Unknown exception");\
	return OPERATION_FAIL;\
}



#define GETSESSION(total,index)\
if ( var.size() == total) \
{\
m_weiwooSess = GetSession(var[index]);\
}


#define CLIENTREQ(x) "ClientRequest#"##x

std::string	strChannelID ;

WeiwooAdmin::WeiwooAdmin( ::Ice::CommunicatorPtr& ic , SessionManagerPrx& sessManagerPrx ):
m_sessManager(sessManagerPrx),m_Ic(ic)
{
	m_bShowTimeCount = false;
	m_bDestroyCommunicator = false;
}
WeiwooAdmin::WeiwooAdmin()
{
	int i=0;
	m_Ic=Ice::initialize(i,NULL);	
	m_bShowTimeCount=false;
	m_bDestroyCommunicator = true;
}

WeiwooAdmin::~WeiwooAdmin()
{
	try
	{
		if(m_bDestroyCommunicator)
		{
			VARVEC var;
			DisConnect(var);
			m_Ic->destroy();
		}
	}
	catch (...)
	{
	}
}
BEGIN_CMDROUTE(WeiwooAdmin,CCmdParser)
	COMMAND(Help,Help)
	COMMAND(ParseIni,ParseIni)
	COMMAND(Connect,Connect)
	COMMAND(DisConnect,DisConnect)	
	COMMAND(createSession,createSession)
	COMMAND(StartSession,StartSession)
	COMMAND(ShowTimeCount,ShowTimeCount)
	COMMAND(AddResource,AddResource)
	COMMAND(AddPriveteData,AddPriveteData)
	COMMAND(AddPD,AddPD)
	COMMAND(AddRes,AddRes)
	COMMAND(AddNGODResource,AddNGODResource)
	COMMAND(AddDVBCResource,AddDVBCResource)
	COMMAND(AddIPResource,AddIPResource)
	COMMAND(Destroy,Destroy)
	COMMAND(renew,renew)
END_CMDROUTE()


int WeiwooAdmin::Help(VARVEC& var)
{
	log("No help!");
	return OPERATION_SUCCESS;
}

int WeiwooAdmin::ShowTimeCount(VARVEC& var)
{
	CHECKVAR1(var,1,"usage:ShowTimeCount on/off");
	if(stricmp("on",var[0].c_str())==0)
	{
		m_bShowTimeCount=true;
	}
	else if(stricmp("off",var[0].c_str())==0)
	{
		m_bShowTimeCount=false;
	}
	else
	{
		log("invalid parameter");
	}
	return OPERATION_SUCCESS;
}
int WeiwooAdmin::Connect(VARVEC& var)
{
	CHECKVAR1(var,1,"Usage:Connect sessManagerEndpoint ");
	std::string sessEndPoint=SERVICE_NAME_SessionManager":";
	const char* p = var[0].c_str();
	if (var[0].find(":") != std::string::npos) 
	{
		sessEndPoint = var[0];
	}
	else
	{
		sessEndPoint = sessEndPoint + var[0];
	}
	
	try
	{
		RECORDTIMESTART();
		DisConnect(var);

		m_sessManager=SessionManagerPrx::checkedCast(m_Ic->stringToProxy(sessEndPoint));
		
		RECORDTIMEEND();
		SHOWTIME(Connect);
	}
	CATCHALL();

	return OPERATION_SUCCESS;
}
int WeiwooAdmin::DisConnect(VARVEC& var)
{

	if(m_sessManager)
	{
		m_sessManager=NULL;
	}
	return OPERATION_SUCCESS;
}

int WeiwooAdmin::ParseIni(VARVEC& var)
{
	CHECKVAR3(var,2,3,4,"usage:parseini inifilepath servicegroup [basePort] [SopName]");
	CHECKSESSMAN();
	InitInfo ini;
	if(!ini.init(var[0].c_str()))
	{
		log("open file %s fail",var[0].c_str());
		return OPERATION_FAIL;
	}
	std::string strURI;//=ComposeUri(ini);
	std::string	strSopName="";
	ini.setCurrent("conf");
	int basePort=1000;
	int svcGroupID=atoi(var[1].c_str());

	if(var.size()==3)
	{
		basePort=atoi(var[2].c_str());
	}
	if(var.size()==4)
	{
		basePort=atoi(var[2].c_str());
		strSopName = var[3];
	}

	
	int iteration=0;
	int sessCount=0;
	int timewait=0;
	int iterwait=0;
	int interval = 1000;
	std::string targetIP;	
	if(!ini.getValue("iterator",iteration))
	{
		log("can't get value of iteration");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("playlistCount",sessCount))
	{
		log("can't get playlistCount");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("channelID",strChannelID))
	{
		log("Can't get channelID");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("timewait",timewait))
	{
		log("can't get timewait");
		return OPERATION_FAIL;
	}

	if(!ini.getValue("target",targetIP))
	{
		log("can't get target");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("interval",interval))
	{
		log("can't get interval");
		return OPERATION_FAIL;
	}

	ini.setCurrent("uri");
	if(!ini.getValue("uri",strURI))
	{
		log("can't get uri");
		return OPERATION_FAIL;
	}
	char szCmd[1024];
	std::string	strCmd;
	int err=0,totalErr=0;
	std::vector<std::string>	vecSess;
	for(int iTempIteration=0;iTempIteration<iteration;iTempIteration++)
	{
		if((kbhit() && (getch()=='q')))
			break;	
		err=0;
		vecSess.clear();
		DWORD	dwMaxTime = 0;
		std::string maxTimeSess;
		for(int isessCount=0;isessCount<sessCount;isessCount++)
		{
			if((kbhit() && (getch()=='q')))
				break;	
			DWORD dwStartTimeCount =GetTickCount();
			strCmd="CreateSession ";
			strCmd+=strURI;
			if(ParseCommand(strCmd)!=OPERATION_SUCCESS)
			{
				err++;
				continue;
			}
			strCmd="setdest ";
			strCmd+=targetIP;
			sprintf(szCmd," %d %d %s",basePort+isessCount,svcGroupID,strSopName);
			strCmd+=szCmd;
			if(ParseCommand(strCmd)!=OPERATION_SUCCESS)
			{
				err++;
				continue;
			}
			strCmd="StartSess";
			if(ParseCommand(strCmd)!=OPERATION_SUCCESS)
			{
				err++;
				continue;
			}
			//std::string sessString=m_Ic->proxyToString(m_weiwooSess);
			std::string sessString=m_weiwooSess->getId();
			vecSess.push_back(sessString);
			DWORD useTimeCount = GetTickCount()-dwStartTimeCount;
			log("session %s is ok with TimeCount [%u]",sessString.c_str(),useTimeCount);
			if ( useTimeCount > dwMaxTime ) 
			{
				maxTimeSess = sessString;
				dwMaxTime = useTimeCount;
			}
			for(int i=0;i<interval;i++)
				Sleep(100);
		}
		log("%d session (%d/%d) and %d err max time[%u] session[%s]",
				sessCount,iTempIteration+1,iteration,err,dwMaxTime,maxTimeSess.c_str());
		for(int iPause=0;iPause<timewait;iPause++)
		{
			if((kbhit() && (getch()=='q')))
				break;	
			Sleep(1000);
		}
		log("destroy session");
		for(int i=0;i<(int)vecSess.size();i++)
		{
			strCmd="stopsess ";
			strCmd+=vecSess[i];
			ParseCommand(strCmd);
		}
		log("destroy ok");
		if((kbhit() && (getch()=='q')))
			break;	
	}
	for(int i=0;i<(int)vecSess.size();i++)
	{
		strCmd="stopsess ";
		strCmd+=vecSess[i];
		ParseCommand(strCmd);
	}	
	
	return OPERATION_SUCCESS;
}

::TianShanIce::SRM::SessionPrx	WeiwooAdmin::GetSession(const std::string& sessID)
{
	TianShanIce::SRM::SessionPrx prx= TianShanIce::SRM::SessionPrx::uncheckedCast(m_sessManager->openSession(sessID));
	return prx;
}
bool  WeiwooAdmin::ConvertStringToVariant(const std::string& strType,
										  const std::string& strValue1, 
										  const std::string& strValue2,
										  TianShanIce::Variant& varOut)
{
	if ( strValue1.empty() && strValue2.empty()) 
	{
		log("no value passed in");
		return false;
	}
	
	bool bRange = ( !strValue2.empty() ) && (!strValue2.empty());

	TianShanIce::ValueType varType ;
	//step 1,check the variant type
	if ( stricmp( "int" , strType.c_str() ) == 0 ) 
	{
		varType =  TianShanIce::vtInts;
	}
	else if ( stricmp( "long" , strType.c_str() ) == 0 )
	{
		varType =  TianShanIce::vtLongs;
	}
	else if ( stricmp( "string" , strType.c_str() ) == 0 ) 
	{
		varType =  TianShanIce::vtStrings;
	}
	else if ( stricmp( "float" ,strType.c_str() ) == 0 ) 
	{
		varType =   TianShanIce::vtFloats;
	}
	else
	{
		log("Not supported type [%s]",strType.c_str());
		return false;
	}

	varOut.bRange = bRange;
	varOut.bin.clear();
	varOut.strs.clear();
	varOut.ints.clear();
	varOut.lints.clear();

	varOut.type = varType;

	switch( varType ) 
	{
	case TianShanIce::vtStrings:
		{
			if (bRange) 
			{
				varOut.strs.push_back(strValue1);
				varOut.strs.push_back(strValue2);
			}
			else
			{
				varOut.strs.push_back(strValue1);
			}
		}
		break;
	case TianShanIce::vtInts:
		{
			Ice::Int a = atoi(strValue1.c_str());			
			varOut.ints.push_back(a);				
			if ( bRange )
			{
				Ice::Int b = atoi(strValue2.c_str());
				varOut.ints.push_back(b);				
			}
		}
		break;
	case TianShanIce::vtLongs:
		{
			Ice::Long a ;
			sscanf(strValue1.c_str(),"%lld",&a);
			varOut.lints.push_back(a);
			if ( bRange )  
			{
				Ice::Long b ;
				sscanf(strValue2.c_str(),"%lld",&b);
				varOut.lints.push_back(b);
			}
		}
		break;
	case TianShanIce::vtFloats:
		{
			Ice::Float a;
			a = (float)atof(strValue1.c_str());
			varOut.floats.push_back(a);
			if ( bRange ) 
			{
				Ice::Float b;
				b = (float)atof( strValue1.c_str() );
				varOut.floats.push_back( b );
			}
		}
	default:
		{
			log("Invalid TianShan Variant Type");
			return false;
		}
	}
	return true;
}
bool WeiwooAdmin::ConvertStringToValuemap( const std::string& varKey , const TianShanIce::Variant& var , TianShanIce::ValueMap& vMap )
{
	vMap[varKey] = var;
	return true;
}
bool WeiwooAdmin::ConvertStringToResourceType(const std::string& strType , TianShanIce::SRM::ResourceType& resType)
{
	if (stricmp("URI" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtURI;
	}
	else if (stricmp("Storage" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtStorage;
	}
	else if (stricmp("Streamer" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtStreamer;
	}
	else if (stricmp("ServiceGroup" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtServiceGroup;
	}
	else if (stricmp("MpegProgram" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtMpegProgram;
	}
	else if (stricmp("TsDownstreamBandwidth" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtTsDownstreamBandwidth;
	}
	else if (stricmp("IP" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtIP;
	}
	else if (stricmp("EthernetInterface" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtEthernetInterface;
	}
	else if (stricmp("PhysicalChannel" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtPhysicalChannel;
	}
	else if (stricmp("AtscModulationMode" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtAtscModulationMode;
	}
	else if (stricmp("HeadendId" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtHeadendId;
	}
	else if (stricmp("ClientConditionalAccess" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtClientConditionalAccess;
	}
	else if (stricmp("ServerConditionalAccess" , strType.c_str()) == 0) 
	{
		resType = TianShanIce::SRM::rtServerConditionalAccess;
	}
	else
	{
		return false;		
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int WeiwooAdmin::AddRes(VARVEC& var)
{
	return AddResource(var);
}
int WeiwooAdmin::AddResource(VARVEC& var)
{
	CHECKVAR2(var,4,5,"usage:AddResource ResourceType  VarKey VarValueType VarValue1 [VarValue2]");
	CHECKSESSMAN();
	//at first convert the string type to tianshanice resource type
	TianShanIce::SRM::ResourceType resType;
	if (!ConvertStringToResourceType(var[0],resType)) 
	{
		log("Can't convert [%s] to TianShanIce SRM resource type",var[0].c_str());
		return OPERATION_INVALIDPARAMETER;
	}
	TianShanIce::Variant	variant;
	TianShanIce::ValueMap	vMap;

	if (var.size() == 5) 
	{
		if ( !ConvertStringToVariant( var[2],var[3],var[4],variant ) ) 
		{
			log( "Can't convert to variant" );
			return OPERATION_FAIL;
		}
	}
	else
	{
		if ( !ConvertStringToVariant( var[2],var[3],"",variant ) ) 
		{
			log( "Can't convert to variant" );
			return OPERATION_FAIL;
		}
	}
	if ((!ConvertStringToValuemap(var[1],variant,vMap))) 
	{
		log("Can't convert to valuemap");
		return OPERATION_FAIL;
	}	

	try
	{
		m_weiwooSess->addResource( resType , TianShanIce::SRM::raMandatoryNonNegotiable , vMap );
	}
	CATCHALL()

	return OPERATION_SUCCESS;
}
int WeiwooAdmin::AddPD(VARVEC& var)
{
	return AddPriveteData(var);
}
int WeiwooAdmin::AddPriveteData(VARVEC& var)
{
	CHECKVAR2(var,3,4,"Usage:AddPrivateData VarKey VarValueType VarValue1 [VarValue2]");
	CHECKSESSMAN();
	TianShanIce::Variant variant;
	if ( var.size() == 4 ) 
	{
		if(!ConvertStringToVariant(var[1] ,var[2],var[3],variant ))
		{
			log("Can't convert to Variant");
			return OPERATION_FAIL;
		}
	}
	else
	{
		if(!ConvertStringToVariant(var[1] ,var[2],"",variant ))
		{
			log("Can't convert to Variant");
			return OPERATION_FAIL;
		}
	}
	try
	{
		m_weiwooSess->setPrivateData(var[0],variant);
	}
	CATCHALL()
		
	return OPERATION_SUCCESS;
}
int WeiwooAdmin::renew(VARVEC& var)
{
	CHECKVAR2(var,1,2,"usage:renew time[ms] sessID");
	CHECKSESSMAN();
	Ice::Long ttl ;
	sscanf(var[0].c_str() , "%lld" , &ttl);

	try
	{
		RECORDTIMESTART();
		GETSESSION(2,1);
		m_weiwooSess->renew(ttl);
		RECORDTIMEEND();
		SHOWTIME(createSession);
	}
	CATCHALL();
	return OPERATION_SUCCESS;
}

int WeiwooAdmin::createSession(VARVEC& svar)
{
	CHECKVAR1(svar,1,"Usage:CreateSession URI");
	CHECKSESSMAN();
	try
	{
		RECORDTIMESTART();
		::TianShanIce::SRM::Resource clientResource;
		clientResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
		clientResource.status = TianShanIce::SRM::rsRequested;		
		//push client request URI
		::TianShanIce::Variant var;
		var.bRange = false;
		var.type = TianShanIce::vtStrings;
		var.strs.push_back(svar[0]);

		::TianShanIce::SRM::SessionPrx weiwooSess;

		clientResource.resourceData["uri"] = var;
		weiwooSess=m_sessManager->createSession(clientResource);
		if(!weiwooSess)
		{
			log("createsession with uri = %s fail",svar[0].c_str());
			return OPERATION_FAIL;
		}
		m_weiwooSess=weiwooSess;
		RECORDTIMEEND();
		SHOWTIME(createSession);
		return OPERATION_SUCCESS;
	}
	CATCHALL();
	return OPERATION_SUCCESS;	
}
int WeiwooAdmin::StartSession(VARVEC& var)
{
	CHECKVAR2(var,0,1,"Usage:StartSession [sessId]");
	CHECKSESSMAN();
	try
	{
		RECORDTIMESTART();
		GETSESSION(1,0);
		m_weiwooSess->provision();
		m_weiwooSess->serve();
		RECORDTIMEEND();
		SHOWTIME(StartSession);
	}
	CATCHALL();
	return OPERATION_SUCCESS;
}
int WeiwooAdmin::Destroy(VARVEC& var)
{
	CHECKVAR2(var,0,1,"Usage:Destroy [sessId]");
	CHECKSESSMAN();
	try
	{
		RECORDTIMESTART();
		GETSESSION(1,0);
		m_weiwooSess->destroy();
		RECORDTIMEEND();
		SHOWTIME(destroy);
	}
	catch(...)
	{
	}
	//CATCHALL();
	return OPERATION_SUCCESS;
}
int WeiwooAdmin::AddDVBCResource(VARVEC& var)
{
	CHECKVAR2(var,1,2,"usage:AddDVBCResource groupID [sessId]");
	CHECKSESSMAN();
	try
	{
		RECORDTIMESTART();
		GETSESSION(3,2);
		TianShanIce::Variant variant;
		TianShanIce::ValueMap vMap;
		ConvertStringToVariant("int",var[0],"",variant);
		ConvertStringToValuemap("id",variant,vMap);
		m_weiwooSess->addResource(::TianShanIce::SRM::rtServiceGroup,
									TianShanIce::SRM::raMandatoryNonNegotiable,
									vMap);
		ConvertStringToVariant("string",var[0],"",variant);
		m_weiwooSess->setPrivateData(CLIENTREQ("node-group-id"),variant);
		m_weiwooSess->setPrivateData(("node-group-id"),variant);


		RECORDTIMEEND();
		SHOWTIME(AddDVBCResource)
	}
	CATCHALL();
	return OPERATION_SUCCESS;
}

int WeiwooAdmin::AddIPResource(VARVEC& var)
{
	CHECKVAR3(var,3,4,5,"Usage:AddIPResource groupID destIP destPort [destmac] [sessId]");
	CHECKSESSMAN();
	try
	{
		RECORDTIMESTART();
		GETSESSION(5,4);
		TianShanIce::Variant variant;
		TianShanIce::ValueMap vMap;
		if ( var.size() >= 4 )
		{
			
			ConvertStringToVariant("string",var[3],"",variant);
			ConvertStringToValuemap("destMac",variant,vMap);
		}

		ConvertStringToVariant("string",var[1],"",variant);
		ConvertStringToValuemap("destIP",variant,vMap);

		ConvertStringToVariant("int",var[2],"",variant);
		ConvertStringToValuemap("destPort",variant,vMap);
		m_weiwooSess->addResource(::TianShanIce::SRM::rtEthernetInterface,TianShanIce::SRM::raMandatoryNonNegotiable,vMap);		

		vMap.clear();
		ConvertStringToVariant("int",var[0],"",variant);
		ConvertStringToValuemap("id",variant,vMap);
		m_weiwooSess->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth,
									TianShanIce::SRM::raMandatoryNonNegotiable,
									vMap);
		//node-group-id
		ConvertStringToVariant("string",var[0],"",variant);
		m_weiwooSess->setPrivateData(CLIENTREQ("node-group-id"),variant);
		m_weiwooSess->setPrivateData(("node-group-id"),variant);

		if(var.size() >= 4)
		{
			ConvertStringToVariant("string",var[3],"",variant);
			m_weiwooSess->setPrivateData(("device-id"),variant);
		}


		RECORDTIMEEND();
		SHOWTIME(AddIPResource);
	}
	CATCHALL();
	return OPERATION_SUCCESS;
}
#define NGOD_RES_PREFIX(X)	"NGOD.R2."#X
int WeiwooAdmin::AddNGODResource(VARVEC& var)
{
	CHECKVAR1(var,1,"Usage:AddNGODResource sopName destIP destPort ");
	CHECKSESSMAN();
		TianShanIce::Variant varDesPort;
		varDesPort.ints.clear();
		varDesPort.ints.push_back( atoi( var[2].c_str() ) );
		varDesPort.type = TianShanIce::vtInts;
		varDesPort.bRange = false;
		m_weiwooSess->setPrivateData( NGOD_RES_PREFIX(client_port) , varDesPort);

// 		TianShanIce::Variant varSourceIP;
// 		varSourceIP.strs.clear();
// 		varSourceIP.strs.push_back("10.15.10.252");
// 		varSourceIP.type = TianShanIce::vtStrings;
// 		varSourceIP.bRange = false;
// 		m_weiwooSess->setPrivateData( NGOD_RES_PREFIX(source),varSourceIP);
		
		TianShanIce::Variant varDestIP;
		varDestIP.strs.clear();
		varDestIP.strs.push_back( var[1] );
		varDestIP.type = TianShanIce::vtStrings;
		varDestIP.bRange = false;
		m_weiwooSess->setPrivateData( NGOD_RES_PREFIX(destination), varDestIP);

		

		TianShanIce::Variant varSopName;
		varSopName.strs.clear();
		varSopName.strs.push_back( var[0] );
		varSopName.type = TianShanIce::vtStrings;
		varSopName.bRange = false;
		m_weiwooSess->setPrivateData( NGOD_RES_PREFIX(sop_name), varSopName);

	return OPERATION_SUCCESS ;
}
