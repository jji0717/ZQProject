
#include "SetupRequest.h"
#include <TimeUtil.h>
#include "LiveChannelConfig.h"
#include "urlstr.h"
#include <string>
#include <string.h>
#include <algorithm>
#include "auth5i.h"

#define STATUS_CODE_WITH_NGOD_EXT

#ifndef ZQ_OS_MSWIN
#include <arpa/inet.h>
#endif
extern ZQ::common::Log* s1log;

namespace LiveChannel
{
	FixupSetup::FixupSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}
	
	FixupSetup::~FixupSetup()
	{
	}

	std::string FixupSetup::calculateGroup(std::string groupExpression, uint64 subnet)
	{
		if (groupExpression.empty())
			return "";

		uint64 subnetIP;
		size_t posH, posT, posV;
		while(true)
		{
			posH = groupExpression.find("${");
			if (std::string::npos != posH)
			{
				posT = groupExpression.find("}", posH + 2);
				if (std::string::npos != posT)
				{
					// find a variable, replace it
					std::string var = groupExpression.substr(posH + 2, posT - posH - 2);

					if ("SUBNET" == var)
					{
						char szBuf[64];
						szBuf[sizeof(szBuf) - 1] = '\0';
						snprintf(szBuf, sizeof(szBuf) - 1, "%lld", subnet);

						groupExpression.replace(posH, posT - posH + 1, szBuf);
						continue;
					}
				}
			}

			// parse macros
			posV = groupExpression.find_last_of("(");
			if (std::string::npos != posV)
			{
				posH = groupExpression.find_last_of("$", posV);
				posT = groupExpression.find(')', posV + 1);

				if (posT > posV && posV >posH)
				{
					std::string funcName = groupExpression.substr(posH+1, posV -posH -1);

					if ("BITSHIFT_R" == funcName || "BITSHIFT_L" == funcName) //right shift or left shift
					{
						uint64 digit = 0, res;
						
						res = sscanf(groupExpression.c_str() + posV + 1, "%lld,%d", &subnetIP, &digit);

						if (subnetIP > 0 && res > 1 && digit >= 0 && digit < sizeof(subnetIP)*8)
						{
							if ("BITSHIFT_R" == funcName)	//right shift
							{
								subnetIP >>= digit;
							} 
							else	//left shift
							{
								subnetIP <<= digit;
							}
							
							char szBuf[64];
							szBuf[sizeof(szBuf) - 1] = '\0';
							snprintf(szBuf, sizeof(szBuf) - 1, "%lld", subnetIP);

							groupExpression.replace(posH, posT - posH + 1, szBuf);
						}
						continue;
					}

					if ("BITAND" == funcName)	//and operation
					{
						int res;
						uint64 digit = 0;

						res = sscanf(groupExpression.c_str() + posV + 1, "%lld,%d", &subnetIP, &digit);
						if (subnetIP > 0 && res > 1)
						{	
							subnetIP &= digit;

							char szBuf[1024];
							szBuf[sizeof(szBuf) - 1] = '\0';
							snprintf(szBuf, sizeof(szBuf) - 1, "%lld", subnetIP);

							groupExpression.replace(posH, posT - posH + 1, szBuf);
						}
						continue;
					}

					if ("ADD" == funcName)	//and operation
					{
						int res;
						uint64 digit = 0;

						res = sscanf(groupExpression.c_str() + posV + 1, "%d, %lld", &digit, &subnetIP);
						if (res > 1 && subnetIP > 0)
						{	
							uint64 groupID = digit + subnetIP;
							
							char szBuf[64];
							szBuf[sizeof(szBuf) - 1] = '\0';
							snprintf(szBuf, sizeof(szBuf) - 1, "%lld", groupID);

							groupExpression.replace(posH, posT - posH + 1, szBuf);
						}
						continue;
					}
					//TODO: more funcs
				}
			}	
			break; // quit the loop if nothing matched
		} //end while
		
		return groupExpression;
	}

	std::string FixupSetup::findServiceGroupByQam(std::string qam)
	{
		std::string serviceGroup;
		for(std::vector<SessionGroupHolder>::iterator iter = _liveChannelConfig._ngods1._sessionGroups.sessionGroups.begin();
			iter != _liveChannelConfig._ngods1._sessionGroups.sessionGroups.end(); iter++ )
		{
			boost::regex qamRegex(iter->expression);
			boost::cmatch result;
			if (!boost::regex_match(qam.c_str(),result, qamRegex))
				continue;

			serviceGroup = iter->group; // "1${2}00${2}" group="1000${2}" />
			for (int i =result.size()-1; i>0; i--)
			{
				char macro[10];
				snprintf(macro, sizeof(macro) -2, "${%d}", i);
				std::string macroStr(macro);
				int pos = 0;
				while((pos = serviceGroup.find(macro,pos)) != std::string::npos)//replace all
				{
					serviceGroup.replace(pos,macroStr.length(), result.str(i));
					pos++;
				}
			}
			break;
		}
		return serviceGroup;
	}

	typedef struct _NamePair {
		char* from;
		char* to;
	} NamePair;

	// it's main task is to change non-TianShan spec request to TianShan spec request
	bool FixupSetup::process()
	{
		std::string strURL = getUrl();
		std::string strPath;

		size_t nStartPath = strURL.find_last_of('/');
		size_t nEndPath	  = strURL.find_first_of('?');
		if (std::string::npos == nEndPath)
			strPath = strURL.substr(nStartPath + 1);
		else
			strPath = strURL.substr(nStartPath + 1, nEndPath - nStartPath - 1);

		// case per UserAgent
		std::string userAgent = getRequestHeader(HeaderUserAgent);
		bool bOCN = (userAgent.find("OCN.RTSP") != std::string::npos);
		bool bVLC = (userAgent.find(VLCFormat) != std::string::npos);

		//rtsp://CatvOfChangNing/60010000?assetUID=TestBcastAsset8
		std::string strReg = _liveChannelConfig._path;
		boost::regex reg(strReg);

		if (!boost::regex_match(strPath, reg))
		{
			SSMLOG(WarningLevel, HandlerFmt(FixupSetup, "failed to recognize the request, url[%s] reg[%s]"), strPath.c_str(), strReg.c_str());
			return false;
		}

		// rewrite app-path as "LiveChannel"
		strURL.replace(nStartPath + 1, strPath.length(), "LiveChannel");

		char protoBuf[16];
		_pRequestWriter->setArgument(_pRequest->getVerb(), strURL.c_str(), _pRequest->getProtocol(protoBuf, sizeof(protoBuf) - 1));

		// step 1. if there's a TianShan-AppData field, treat it as TianShan spec and return directly
		std::string scSrvrDataStr = getRequestHeader(HeaderSeaChangeServerData);
		String::removeChar(scSrvrDataStr, ' ');
		std::string tsAppData = getRequestHeader(HeaderTianShanAppData);
		String::removeChar(tsAppData, ' ');

		std::string require = getRequestHeader(HeaderRequire);
		bool bNGOD_S1 = (require.find("com.comcast.ngod.s1") != std::string::npos);

		std::string hdrServiceGroup = getRequestHeader(HeaderTianShanServiceGroup);

		if (bVLC) //VLC format
		{
			//TODO: translate to tianshan spec
			//step 1: fixup url(for vlc)
			char uriBuf[512];
			uriBuf[511] = '\0';
			const char* pUrl = _pRequest->getUri(uriBuf, sizeof(uriBuf) - 1);
			int urlLen = strlen(pUrl);
			if (uriBuf[urlLen - 1] == '/')
				uriBuf[urlLen - 1] = '\0';
			std::string strUrl = uriBuf;
			_pRequestWriter->setArgument(_pRequest->getVerb(), strUrl.c_str(), _pRequest->getProtocol(uriBuf, sizeof(uriBuf) - 1));

			//step 2: set header type to VLC format
			_pRequestWriter->setHeader(HeaderFormatType, VLCFormat);

			//step 3: copy "Transport" to "TianShan-Transport"
			std::string scTransportStr = getRequestHeader(HeaderTransport);
			String::removeChar(scTransportStr, ' ');
			_pRequestWriter->setHeader(HeaderTianShanTransport, (char*) scTransportStr.c_str());

			//step 4: set virtual service group
			String::removeChar(hdrServiceGroup, ' ');
			if (hdrServiceGroup.empty())
				_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char *)_liveChannelConfig._virtualServiceGroup.group.c_str());			
		}
		else if (bNGOD_S1)
		{
			// NGOD spec
			//step 1: fixup url
			STRINGVECTOR strs;
			
			//step 2: set header type to VLC format
			_pRequestWriter->setHeader(HeaderFormatType, NGODFormat);

			//step 3: set ServiceGroup
			std::string serviceGroup = getRequestHeader(HeaderTianShanServiceGroup);
			String::removeChar(serviceGroup, ' ');
			bool bFound = false;
			std::string transport = getRequestHeader(HeaderTransport);
			if (transport.find("MP2T/DVBC/QAM") != std::string::npos && serviceGroup.empty())
			{
				std::string qam = "";
				String::splitStr(transport, ",", strs);// case: MP2T/DVBC/QAM;unicast;client=00AF123456DE;qam_name=Chicago.Southbend.5,MP2T/DVBC/QAM;unicast;client=00AF123456DE;qam_name=Chicago.Southbend.10
				for (STRINGVECTOR_ITOR strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
				{
					STRINGVECTOR strs2;
					String::splitStr(*strsItor, ";", strs2);
					for (STRINGVECTOR_ITOR strsItor2 = strs2.begin(); strsItor2 != strs2.end(); strsItor2 ++)
					{
						std::string keystr, valstr;
						keystr = String::getLeftStr(*strsItor2, "=", true);
						valstr = String::getRightStr(*strsItor2, "=", true);
						if ("qam_name" == keystr)
						{
							qam = valstr;
							serviceGroup = findServiceGroupByQam(qam);
							break;
						}
					}
					if (!serviceGroup.empty())
					{
						bFound = true;
					}

					if (bFound)
					{	
						_pRequestWriter->setHeader(HeaderTransport, (char *)(*strsItor).c_str());
						break;
					}
				}				
			}
			else if (transport.find("MP2T/AVP/UDP") != std::string::npos && serviceGroup.empty())
			{
				//case: MP2T/AVP/UDP;unicast;service_group=3;destination=192.168.23.23;client_port=1234;bandwidth=3750000
				String::splitStr(transport, ";", strs);
				for (STRINGVECTOR_ITOR strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
				{
					std::string keystr, valstr;
					keystr = String::getLeftStr(*strsItor, "=", true);
					valstr = String::getRightStr(*strsItor, "=", true);
					if ("service_group" == keystr)
					{
						bFound = true;
						serviceGroup = valstr;
						break;
					}
				}
			}
			else if (transport.find("MP2T/AVP/C2") != std::string::npos && serviceGroup.empty())
			{
				composeResponse(461);	//unsupoort transport
				return false;
			}

			if (!serviceGroup.empty())// NGOD spec
			{
				_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char *)serviceGroup.c_str());
			}
			else if (_liveChannelConfig._ngods1._sessionGroups.defaultServiceGroup >= 0)
			{
				serviceGroup = _liveChannelConfig._ngods1._sessionGroups.defaultServiceGroup;
				_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char *)serviceGroup.c_str());
			}
			else
			{
				composeResponse(676);
				return false;
			}
		}
		else if (false == tsAppData.empty())
		{
			// TianShan spec
			_pRequestWriter->setHeader(HeaderFormatType, TianShanFormat);
		}
		else if (false == scSrvrDataStr.empty())
		{
			// SeaChange spec
			_pRequestWriter->setHeader(HeaderFormatType, SeaChangeFormat);

			// SETUP /SeaChange/ITV?00000000.00381b0c RTSP/1.0
			// CSeq:1
			// SeaChange-MayNotify:
			// SeaChange-Mod-Data:billing-id=0000005000;purchase-time=0000000000;time-remaining=0000000000;home-id=0000050000;smartcard-id=0000005000;purchase-id=0000000000;package-id=0000000000
			// SeaChange-Server-Data:node-group-id=0000000001;smartcard-id=0000005000;device-id=01005E000001;supercas-id=0000000100
			// SeaChange-Version:1
			// Transport: MP2T/AVP/UDP;unicast;destination=127.0.0.1;client_port=26151;client_mac=01005e000001
			
			/*
			copy the seachange-data to TianShan-appData;
			if ( there is seachange-mod-data && seachange-data is not empty) 
			append seahange-mod-data to seachange-data
			TianShan-appdata = this merged data
			*/

			// step2. get datas in order to change SeaChange spec to TianShan spec
			STRINGMAP tsAppMap;
			STRINGVECTOR strs;
			STRINGVECTOR_ITOR strsItor;
			String::splitStr(scSrvrDataStr, ";", strs);
			for (strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
			{
				std::string keystr, valstr;
				keystr = String::getLeftStr(*strsItor, "=", true);
				valstr = String::getRightStr(*strsItor, "=", true);
				if (false == keystr.empty() && false == valstr.empty())
					tsAppMap[keystr] = valstr;
			}

			std::string scModDataStr = getRequestHeader(HeaderSeaChangeModData);
			String::removeChar(scModDataStr, ' ');
			String::splitStr(scModDataStr, ";", strs);
			for (strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
			{
				std::string keystr, valstr;
				keystr = String::getLeftStr(*strsItor, "=", true);
				valstr = String::getRightStr(*strsItor, "=", true);
				if (false == keystr.empty() && false == valstr.empty())
					tsAppMap[keystr] = valstr;
			}

			// step3. set TianShan-ServiceGroup header value
			_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char*) tsAppMap[NodeGroupID].c_str());

			// step4. set TianShan-AppData header value
			std::string tsAppDataStr;
			STRINGMAP_ITOR tsAppMapItor;
			for (tsAppMapItor = tsAppMap.begin(); tsAppMapItor != tsAppMap.end(); tsAppMapItor ++)
			{
				tsAppDataStr += tsAppMapItor->first + "=" + tsAppMapItor->second +";";
			}
			{
			::std::string::size_type tmpSize = tsAppDataStr.size();
			if (tmpSize > 0 && tsAppDataStr[tmpSize - 1] == ';')
				tsAppDataStr.resize(tmpSize - 1); // 去除最后的';'
			}
			_pRequestWriter->setHeader(HeaderTianShanAppData, (char*) tsAppDataStr.c_str());

			// DO: copy "SeaChange-Transport" to "TianShan-Transport"
			std::string scTransportStr = getRequestHeader(HeaderSeaChangeTransport);
			String::removeChar(scTransportStr, ' ');
			_pRequestWriter->setHeader(HeaderTianShanTransport, (char*) scTransportStr.c_str());

			// step 5. fixup url in order to support sledge hammer if needed.
			std::string urlStr;
			memset(_szBuf, 0, sizeof(_szBuf));
			const char* pURL = _pRequest->getUri(_szBuf, sizeof(_szBuf) - 1);

			if ( pURL && pURL[0] != 0 )
			{
				//set original url so we can use it in ContentHandler process
				_pRequestWriter->setHeader(OriginalUrl, (char*) pURL );
			}


			//convert Axiom URL format into TianShan Spec
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "Convert Axiom format into TianShan format from url:%s"),pURL );
			std::string strTempUrl = std::string("rtsp://") + pURL;
			ZQ::common::URLStr axiomUrl( strTempUrl.c_str(), false );
			
#define		VALIDSTR(x) (x&&x[0]!=0)
			//check if it is old axiom format or not
			const char* pTempVarValue = axiomUrl.getVar( axiomUrl.getVarname(0) ) ;
			const char* pTempVarName = axiomUrl.getVarname(0);
			
			if ( VALIDSTR(pTempVarName))
			{
				if ( VALIDSTR(pTempVarValue) )
				{//new axiom format					
#define		AXIOMAPPNAME			"appname"
#define		AXIOMAPPUID				"appuid"
#define		AXIOMPID				"providerid"
#define		AXIOMPAID				"providerassetid"
#define		AXIOMASSETUID			"assetuid"
#define		AXIOMNODEGROUPID		"nodegroupid"
					//step 1. check 'AppName' or 'AppUid'
					const char* pAppName	= axiomUrl.getVar(AXIOMAPPNAME);
					const char* pAppUID		= axiomUrl.getVar(AXIOMAPPUID);
					const char* pPID		= axiomUrl.getVar(AXIOMPID);
					const char* pPAID		= axiomUrl.getVar(AXIOMPAID);
					const char* pAssetUID	= axiomUrl.getVar(AXIOMASSETUID);
					const char* pNodeGroupId= axiomUrl.getVar(AXIOMNODEGROUPID);

					if ( VALIDSTR(pNodeGroupId) )
					{//replace ndoeGroupId with this one
						SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "replace %s with NodeGroupId in uri [%s]"),
							HeaderTianShanServiceGroup,
							pNodeGroupId);
						_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char*)pNodeGroupId );
					}

					const char* pPath = VALIDSTR(pAppName) ? pAppName : pAppUID;
					if ( ! VALIDSTR(pPath) )
					{//set a default value
						pPath = _liveChannelConfig._defaultParams._axiomMsgDefaultPath.c_str();
						SSMLOG(InfoLevel, HandlerFmt(HandleSetup,"No AppNam or AppUID is found, use default value :%s"),pPath ) ;
					}
					if ( !( ( VALIDSTR(pPID) && VALIDSTR(pPAID) ) || VALIDSTR(pAssetUID) ) )
					{
						SSMLOG(WarningLevel,HandlerFmt(HandleSetup,"Invalid Axiom url format, no PID+PAID or ASSETUID in uri:%s"), pURL );
					}
					else
					{
						if ( ( VALIDSTR(pPID) && VALIDSTR(pPAID) ) && VALIDSTR(pAssetUID) )
						{
							SSMLOG(WarningLevel, HandlerFmt(HandleSetup,"Both PID+PAID and ASSETUID are present, take PID+PAID set"));
							urlStr = std::string("rtsp://") + axiomUrl.getHost() + "/" + pPath + "?asset=" + pPID+"#"+ pPAID;
						}
						else
						{
							if ( VALIDSTR(pAssetUID) )
							{
								urlStr = std::string("rtsp://") + axiomUrl.getHost() + "/" + pPath + "?assetUid=" + pAssetUID;
							}
							else
							{
								urlStr = std::string("rtsp://") + axiomUrl.getHost() + "/" + pPath + "?asset=" + pPID+"#"+ pPAID;
							}

						}
					}

				}
				else
				{//old axiom format
					const char* pDot = strstr( pTempVarName, "." );
					if ( VALIDSTR(pDot) )
					{
						std::string		strAppId;
						std::string		strAssetId;

						strAppId.assign( pTempVarName, pDot - pTempVarName );
						strAssetId = ( pDot + 1 );
						urlStr = std::string("rtsp://") + axiomUrl.getHost() + "/" + strAppId + "?assetUid=" + strAssetId;
					}
					else
					{//can't find dot, treat it as invalid old axiom uri format
						SSMLOG(WarningLevel, HandlerFmt(HandleSetup,"Invalid old axiom uri format:%s"),pURL);
					}
				}
			}
			else
			{
				SSMLOG(WarningLevel,HandlerFmt(HandleSetup,"Invalid url, no uri content is found"));
			}

			// if urlStr is not empty, means that he has been modified.
			if (false == urlStr.empty())
				_pRequestWriter->setArgument(_pRequest->getVerb(), urlStr.c_str(), _pRequest->getProtocol(_szBuf, sizeof(_szBuf) - 1));
		}
		else // no HeaderSeaChangeServerData and TianShan-AppData, unexpect request
		{
			SSMLOG(ErrorLevel, HandlerFmt(FixupSetup, "not a acceptable request"));
			_pResponse->printf_preheader(ResponseUnexpectClientError);
			return false;
		}

		return true;
	}

	HandleSetup::HandleSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse), _sessMgrPrx(NULL),
		mbOpSuccess(false),
		mTimestampStart(0),
		_bNatOpen(false),
		_bQam(false),
		_bIP(false),
		_bC2(false),
		_lBandWidth(-1),
		_destIP(""),
		_destPort(""),
		_destMac("")
	{
		char szBuf[256] = {0};
		szBuf[0] = 0 ;
		uint16 bufSize = sizeof(szBuf);
		const char* p = pReq->getHeader(KEY_MESSAGE_RECVTIME,szBuf,&bufSize);
		if ( p && p[0] )
		{
			sscanf( p, "%lld", &mTimestampStart );
		}
	}
	
	HandleSetup::~HandleSetup()
	{
		if ( !mbOpSuccess && _srvrSessPrx != NULL )
		{
			try
			{
				//destroy srm session
				if ( mErrMsg.length() <= 0 )
				{
					mSrmSess->destroy();
				}
				else
				{
					Ice::Context iceCtx;
					iceCtx["caller"] = mErrMsg;
					iceCtx["caller_type"] = "rtsp_server_destroy"; // don't modify this, weiwoo will refer to it.
					_srvrSessPrx->destroy(iceCtx);			
					
					SSMLOG(ErrorLevel,HandlerFmt(HandleSetup,"setup failed, destroy session due to [%s] "),mErrMsg.c_str() );
				}
			}
			catch(...)
			{
			}			
		}
	}
	bool HandleSetup::checkTimeStampAndConnection()
	{
		if ( !_pRequest->checkConnection() )
		{
			SSMLOG(ErrorLevel,HandlerFmt(HandleSetup,"connection for this request has been disconnected") );
			setErrMsg("212011 connection for this request has been disconnected");			
			return false;
		}

		if ( mTimestampStart == 0 )
		{
			return true;
		}
		
		int64 cur = ZQ::common::now();
		
		//TODO: should be a configuration 
		unsigned int delta =  _liveChannelConfig._rtspSession._setupMsgTimeoutInterval ;
		delta = MAX( delta, 500 );

		if ( cur > ( mTimestampStart + delta ) )
		{			
			SSMLOG(ErrorLevel,HandlerFmt(HandleSetup,"timed out in processing setup command, %u]"), delta);
			setErrMsg("212012 timeout due to timed out in processing setup command, %u]",delta);			

			//response with error code 503 
			composeResponse( 503, "timed out in message processing" );
			return false;
		}
		return true;
	}
	
	void HandleSetup::setErrMsg( const char* fmt, ... )
	{
		//notice error message should never exceed 2048 byte
		char msg[2048];
		va_list args;

		va_start(args, fmt);
		int nCount = vsnprintf(msg, 2047, fmt, args);
		va_end(args);
		if ( nCount == -1 )
		{
			msg[2047] = '\0';
		}
		else
		{
			msg[nCount] = '\0';
		}
		mErrMsg = msg;
	}

	void HandleSetup::confirm( )
	{
		mbOpSuccess = true;
	}

	bool HandleSetup::process()
	{
		_cltSessCtx.announceSeq = 0;
		_cltSessCtx.url = getUrl();

		MAPSET(TianShanIce::Properties, _eventParams, "url", _cltSessCtx.url);
		std::string appdata = getRequestHeader(HeaderTianShanAppData);
		String::removeChar(appdata, ' ');
		STRINGVECTOR strs;
		String::splitStr(appdata, ";", strs);
		for (STRINGVECTOR_ITOR strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
		{
			std::string keystr, valstr;
			keystr = String::getLeftStr(*strsItor, "=", true);
			valstr = String::getRightStr(*strsItor, "=", true);
			if (keystr.empty())
				continue;

			MAPSET(TianShanIce::Properties, _eventParams, keystr, valstr);
		}

		// User-Agent: OCN.RTSP
		std::string userAgent = getRequestHeader(HeaderUserAgent);
		MAPSET(TianShanIce::Properties, _eventParams, "ua", userAgent);
		bool bOCN = (userAgent.find("OCN.RTSP") != std::string::npos);

		_cltSessCtx.rangePrefix = "npt";
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), NGODFormat))
			_cltSessCtx.requestType = SPEC_NGOD_S1; // NGOD spec
		else if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
			_cltSessCtx.requestType = SPEC_NGOD_SeaChange; // SeaChange
		else 
		{
			_bNeedModifyResponse = false;
			_cltSessCtx.requestType = SPEC_NGOD_TianShan; // TianShan
		}

		// Parse "Transport" head
		std::string hdrTransport = getRequestHeader(HeaderTransport);
		String::removeChar(hdrTransport, ' ');
		::TianShanIce::Properties propsTransport;

		{
			STRINGVECTOR tknsTransport;
			String::splitStr(hdrTransport, ";", tknsTransport);
			for (STRINGVECTOR_ITOR tknItTransport = tknsTransport.begin(); tknItTransport != tknsTransport.end(); tknItTransport ++)
			{
				std::string key   = String::getLeftStr(*tknItTransport, "=", true);
				std::string value = String::getRightStr(*tknItTransport, "=", true);
				MAPSET(::TianShanIce::Properties, propsTransport, key, value);
			}
		}

		::TianShanIce::Properties::iterator itProp = propsTransport.find(BandWidth);
		if (propsTransport.end() != itProp)
			_lBandWidth = atol(itProp->second.c_str());

		itProp = propsTransport.find(Destination);
		if (propsTransport.end() != itProp)
			_destIP = itProp->second.c_str();

		itProp = propsTransport.find(ClientPort);
		if (propsTransport.end() != itProp)
			_destPort = itProp->second.c_str();

		itProp = propsTransport.find("client");
		if (propsTransport.end() != itProp)
			_destMac = itProp->second.c_str();

		if (NULL != strstr(hdrTransport.c_str(), "/DVBC/") || NULL != strstr(hdrTransport.c_str(), "/QAM"))
		{// QAM mode(MP2T/DVBC/QAM) 
			_bQam = true;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is QAM mode"));
		}
		//add "|| NULL != strstr(hdrTransport.c_str(), HeaderVLCTransport)" by lxm at 2008.12.22 to support VLC
		else if (NULL != strstr(hdrTransport.c_str(), "/UDP") || NULL != strstr(hdrTransport.c_str(), HeaderVLCTransport))
		{
			// IP mode(MP2T/AVP/UDP)
			_bIP = true;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is IP mode"));
		}
		else 
		{
			// 461 Unsupported Transport
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s not supported", hdrTransport.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			// _error_statusline = ResponseUnsupportedTransport;
			//composeErrorResponse();
			_statusCode = 461;
			composeResponse(_statusCode);
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		// 1. get sessionid from request
		// 2. find sessionid in RtspProxy, if not found, ignore.
		// 3. if session already setuped, return true with the statusline "RTSP/1.0 455 Method Not Valid in This State"
		// notice here, you can not return false, because the upcall function will destroy the session
		// but why application destroy the session when setup failed is that application can make sure there is no leak in resource.
		//////////////////////////////////////////////////////////////////////////
		_session = getRequestHeader(HeaderSession, -1);
		if (!bOCN && _pSite->findClientSession(_session.c_str()))
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "session already setuped");
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 455;
			composeResponse(_statusCode);

			//_ok_statusline = ResponseMethodNotValidInThisState;
			//composeRightResponse();
			return true; // notice here, must return true;
		}

		std::string assetID, svcGroup, name;
		if (bOCN && _session.empty())
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "OCN client takes the last session: %s"), _dummyLastSessId.c_str());
			_session = _dummyLastSessId;
		}
		
		_statusCode = 503; // start _statusCode with 503 about initialization
		if (!_session.empty())
		{
			Ice::Identity ident;
			ident.name = _session;
			ident.category = ServantType;
			if (!openSessionCtx(ident))
			{
				_statusCode = 455;
				composeResponse(_statusCode);
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "failed to open sess record[%s]"), ident.name.c_str());
				return false;
			}
		}
		else
		{
			// create rtsp client session
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do createRtspSession()"));
			IClientSession* pRtspSession = _pSite->createClientSession(NULL, _cltSessCtx.url.c_str());
			if (NULL == pRtspSession || NULL == pRtspSession->getSessionID())
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "SiteError[SetupHandle:0300] create client session failed");
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}

			_pRequestWriter->setHeader(HeaderSession, (char*) pRtspSession->getSessionID());
			_session = pRtspSession->getSessionID();
			_cltSessCtx.ident.name = _session;
			_cltSessCtx.ident.category = ServantType;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "createRtspSession() successfully"));
		}

		if ( !checkTimeStampAndConnection() )
		{
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "connection lost"));
			return false;
		}

		// create weiwoo session
		::TianShanIce::Application::Broadcast::BcastPublisherPrx bcastPublishPrx;
		try {
			assetID = getAssetID();
			if (assetID.empty())
			{
				if (bOCN)
				{
					if (_cltSessCtx.props.end() != _cltSessCtx.props.find("x-ServiceCode.desc"))
					{
						assetID = _cltSessCtx.props["x-ServiceCode.desc"];
						SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "asset taking x-ServiceCode[%s]"), assetID.c_str());
					}

					if (assetID.empty())
						assetID="41";
				}
				else
				{
					_statusCode = 400;
					composeResponse(_statusCode);
					SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "no asset specified"));
					return false;
				}
			}

			svcGroup = getRequestHeader(HeaderTianShanServiceGroup);
			if (bOCN && svcGroup.empty())
			{
				svcGroup=_cltSessCtx.props["ServiceGroup.desc"];
				SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "OCN client ServiceGroup.desc[%s]/103"), svcGroup.c_str());
				if (atol(svcGroup.c_str()) <=0)
					svcGroup = "103";
			}

			name = assetID + "@" + svcGroup;

			bcastPublishPrx = ::TianShanIce::Application::Broadcast::BcastPublisherPrx::checkedCast(_env._pCommunicator->stringToProxy(_liveChannelConfig._bcastPublish._endpoint));

			TianShanIce::Application::PublishPointPrx publishPointPrx = bcastPublishPrx->open(name);
			_bcastPublishPointPrx = TianShanIce::Application::Broadcast::BcastPublishPointPrx::uncheckedCast(publishPointPrx);
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught when create BcastPublishPointPrx by BcastPublisherPrx.open()"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse( _statusCode );
			return false;
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when create BcastPublishPointPrx by BcastPublisherPrx.open()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);			
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch(  const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when create BcastPublishPointPrx by BcastPublisherPrx.open()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when create BcastPublishPointPrx by BcastPublisherPrx.open()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}

		MAPSET(TianShanIce::Properties, _eventParams, "asset", assetID);
		MAPSET(TianShanIce::Properties, _eventParams, "svcg",  svcGroup);

		try {

			if (NULL == _bcastPublishPointPrx)
			{
				::TianShanIce::Properties props;
				::TianShanIce::SRM::ResourceMap resourceRequirement;
				
				props["soap_AssetID"] = assetID;
				props["Persistent_PublistPoint"] = "0";

				//fill service group
				::TianShanIce::SRM::Resource serviceGroupRC;
				serviceGroupRC.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
				serviceGroupRC.status = TianShanIce::SRM::rsRequested;

				int nSvcGroup = atoi(svcGroup.c_str());
				TianShanIce::Variant vrtServiceGroup;
				vrtServiceGroup.type = TianShanIce::vtInts;
				vrtServiceGroup.bRange = false;
				vrtServiceGroup.ints.clear();
				vrtServiceGroup.ints.push_back(nSvcGroup);
				serviceGroupRC.resourceData["id"] = vrtServiceGroup;

				resourceRequirement[::TianShanIce::SRM::rtServiceGroup] = serviceGroupRC;

				if (_bIP) // fillin parameters for IP mode
				{
					//fill ethernet interface resource for weiwoo	
					::TianShanIce::SRM::Resource ethernetInterfaceRC;
					ethernetInterfaceRC.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
					ethernetInterfaceRC.status = TianShanIce::SRM::rsRequested;
					::TianShanIce::Variant vrtUrl;		
					vrtUrl.bRange = false;
					vrtUrl.type = TianShanIce::vtStrings;
					vrtUrl.strs.clear();
					vrtUrl.strs.push_back(_cltSessCtx.url);
					ethernetInterfaceRC.resourceData = prepareEthernetInterfaceResource();
					resourceRequirement[::TianShanIce::SRM::rtEthernetInterface] = ethernetInterfaceRC;
				}

				_bcastPublishPointPrx = bcastPublishPrx->createBcastPublishPoint(name, resourceRequirement, props, "WeiwooSession");
				
				if (_lBandWidth > 0)
					_bcastPublishPointPrx->setMaxBitrate(_lBandWidth);  // both input and output is in bps

				::TianShanIce::Application::ChannelItem itemCtx;
				itemCtx.contentName = assetID;

				//start time
				int64 stampNow = ZQ::common::now();
				char buf[65]="";
				memset(buf, 0, sizeof(buf));
				ZQ::common::TimeUtil::TimeToUTC(stampNow, buf, sizeof(buf) -2);
				itemCtx.broadcastStart = buf;

				//end time
				memset(buf, 0, sizeof(buf));
				ZQ::common::TimeUtil::TimeToUTC(stampNow + (int64)365*24*3600*1000, buf, sizeof(buf) -2);
				itemCtx.expiration = buf;

				itemCtx.playable = false;
				itemCtx.forceNormalSpeed = false;
				itemCtx.inTimeOffset = 0;
				itemCtx.outTimeOffset = 0;
				itemCtx.spliceIn = false;
				itemCtx.spliceOut = false;
				_bcastPublishPointPrx->appendItem(itemCtx);
			}
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught when create BcastPublishPointPrx by BcastPublisherPrx.createBcastPublishPoint()"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse( _statusCode );
			return false;
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when create BcastPublishPointPrx by BcastPublisherPrx.createBcastPublishPoint()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);			
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch(  const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when create BcastPublishPointPrx by BcastPublisherPrx.createBcastPublishPoint()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when create BcastPublishPointPrx by BcastPublisherPrx.createBcastPublishPoint()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}

		try
		{
			_bcastPublishPointPrx->start();
			_srvrSessPrx = _bcastPublishPointPrx->getSession();
			if (NULL == _srvrSessPrx)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "get session failure by BcastPublishPointPrx.getSession()");
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				_statusCode = 500;
				composeResponse(_statusCode);
			}
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught when get session by BcastPublishPointPrx.getSession()"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse( _statusCode );
			return false;
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when get session by BcastPublishPointPrx.getSession()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);			
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch(  const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when get session by BcastPublishPointPrx.getSession()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when get session by BcastPublishPointPrx.getSession()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}
		
		try
		{	
			_cltSessCtx.bcastPublishPointPrxID = _env._pCommunicator->proxyToString(_bcastPublishPointPrx);
			_cltSessCtx.srvrSessPrxID = _env._pCommunicator->proxyToString(_srvrSessPrx);
			_cltSessCtx.srvrSessID = _srvrSessPrx->getId();
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "weiwoo session(%s) created, and proxy is [%s]"),
				_cltSessCtx.srvrSessID.c_str(), _cltSessCtx.srvrSessPrxID.c_str() );
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught when get weiwoo session id by WeiwooService.getId()"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse( _statusCode );
			return false;
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when get weiwoo session id by WeiwooService.getId()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);			
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch(  const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when get weiwoo session id by WeiwooService.getId()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught when get weiwoo session id by WeiwooService.getId()", 
				ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}

		// flush the private datas to weiwoo session
		//flushPrivateData();

		if ( !checkTimeStampAndConnection() )
			return false;
	
		if (!prepareResponse())
		{
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "failed to prepare response"));
			return false;
		}

		renewSession();

		SessionContextImplPtr pSessionContext = convertSessionData(_cltSessCtx);
		if (bOCN) // this remove-then-save is a temporary step that the existing record only alow to save once
		{
			removeSessionCtx(pSessionContext->ident);
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "OCN client removed Sess.desc[%s]"), pSessionContext->ident.name.c_str());
		}

		if (!saveSessionCtx(pSessionContext))
		{
			_statusCode = 500; // if renewSession() encounter timeout exception
			composeResponse(_statusCode,"failed to add record into database");
			//composeErrorResponse();
			return false;
		}

		_statusCode = 200;
		composeResponse(_statusCode);
		//composeRightResponse();
		confirm();
		return true;
	}

	void HandleSetup::ChangeQamMode(int& code)
	{
		switch (code)
		{
		case 0x06: code = 16; break;
		case 0x07: code = 32; break;
		case 0x08: code = 64; break;
		case 0x0c: code = 128; break;
		case 0x10: code = 256; break;
		default: code = -1; break;
		}
	}

	void HandleSetup::addPrivateData(std::string key, TianShanIce::Variant& var)
	{
		// every key should add a "ClientRequest#" prefix in order to distinguish the private data
		// added by plug-in from other components. andy says
		std::string keyTmp = key;
		key = "ClientRequest#";
		key += keyTmp;
		_pdMap[key] = var;
		if (TianShanIce::vtInts == var.type && var.ints.size() > 0)
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "addPrivateData(%s, %d)"), key.c_str(), var.ints[0]);
		else if (TianShanIce::vtLongs == var.type && var.lints.size() > 0) 
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "addPrivateData(%s, %lld)"), key.c_str(), var.lints[0]);
		else if (TianShanIce::vtStrings == var.type && var.strs.size() > 0)
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "addPrivateData(%s, %s)"), key.c_str(), var.strs[0].c_str());
	}
	
	bool HandleSetup::flushPrivateData()
	{
		SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do flushPrivateData()"));

		try 
		{
			_srvrSessPrx->setPrivateData2(_pdMap);
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).setPrivateData2"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0308] caught by session(%s).setPrivateData2", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			return false;
		}

		SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "flushPrivateData() successfully"));
		
		return true;
	}

	std::string HandleSetup::getAssetID()
	{
		char uriBuf[512] ="";
		std::string struri = _pRequest->getUri(uriBuf, sizeof(uriBuf) - 1);
		if (_cltSessCtx.props.end() != _cltSessCtx.props.find("URI.desc"))
			struri = _cltSessCtx.props["URI.desc"];

		ZQ::common::URLStr uri(struri.c_str());

		std::string asset    = uri.getVar("asset");
		std::string assetUID = uri.getVar("assetuid");
		std::string channel  = uri.getVar("ch");

		if (!asset.empty())
		{
			size_t endAssetPos = asset.find_first_of('#');
			if (std::string::npos == endAssetPos)
                return asset;

			return asset.substr(endAssetPos+1);
		}
		
		if (!assetUID.empty())
			return assetUID;

		return channel;
	}

	TianShanIce::ValueMap HandleSetup::prepareEthernetInterfaceResource()
	{
		std::string peerIP, peerPort; // current connection's peerIP and peerPort.
		{
			// get current connection's "peerPort", "peerIP"
			char addr[64];
			addr[63] = '\0';
			IClientRequest::RemoteInfo info;
			info.size = sizeof(IClientRequest::RemoteInfo);
			info.addrlen = sizeof(addr);
			info.ipaddr = addr;
			if (_pRequest->getRemoteInfo(info))
			{
				peerIP = info.ipaddr;
				char sPort[64];
				sPort[63] = '\0';
				snprintf(sPort, 63, "%d", info.port);
				peerPort = sPort;
			}
		}

		// if the "destPort" gained from "Transport" field is not exist or empty, use the "peerPort".
		if (_destPort.empty())
			_destPort = peerPort;

		// define resources and initialize them.
		TianShanIce::ValueMap srvSessionValueMap;
		srvSessionValueMap.clear();
		TianShanIce::Variant vrtDestIP, vrtDestPort, vrtDestMac;

		vrtDestIP.bRange = false;
		vrtDestIP.type = TianShanIce::vtStrings;
		vrtDestIP.strs.clear();

		vrtDestPort.bRange = false;
		vrtDestPort.type = TianShanIce::vtInts;
		vrtDestPort.ints.clear();
		vrtDestPort.ints.push_back(atoi(_destPort.c_str()));

		vrtDestMac.bRange = false;
		vrtDestMac.type = TianShanIce::vtStrings;
		vrtDestMac.strs.clear();

		if ( !_destPort.empty() )
		{
			srvSessionValueMap["destPort"] = vrtDestPort;
		}

		if ( !_destMac.empty() )
		{
			vrtDestMac.strs.push_back(_destMac);
			srvSessionValueMap["destMac"] = vrtDestMac;			
		}

		if (true == _bNatOpen) // NAT mode
		{
			// according to the document NATPenetratingDesignSpec.doc
			// NAT mode is open, use the "peerIP" gained from current connection rather than "_destIP" stored as "destination" in "Transport" field.
			_destIP = peerIP; // copy the "peerIP" to "_destIP"
		}
		else if (true == _destIP.empty())
		{
			// not NAT mode and "_destIP" stored as "destination" in "Transport" field is empty or not exist.
			// use the "peerIP" gained from current connection. the application will use the current peer ip
			// address as the streaming destination.
			_destIP = peerIP;
		}
		vrtDestIP.strs.push_back(_destIP);
		if (!_destIP.empty())
			srvSessionValueMap["destIP"] = vrtDestIP;

		return srvSessionValueMap;
	}

	std::string HandleSetup::eventParamLine() const
	{
		std::string line;
		for (TianShanIce::Properties::const_iterator it = _eventParams.begin(); it != _eventParams.end(); it++)
			line += it->first + "=" + it->second + "|";

		if (line.empty())
			return "";

		return line.substr(0, line.length() -1);
	}

	bool HandleSetup::prepareResponse()
	{
		std::string hdrTransport = getRequestHeader(HeaderTransport);
		String::removeChar(hdrTransport, ' ');

		if ( !checkTimeStampAndConnection() )
			return false;

		try
		{
			_streamPrx = _srvrSessPrx->getStream();
			if ( _env._iceOverrideTimeout > 0 )
			{
				_streamPrx = TianShanIce::Streamer::StreamPrx::uncheckedCast(_streamPrx->ice_timeout( _env._iceOverrideTimeout ));
			}

			_cltSessCtx.streamPrxID = _env._pCommunicator->proxyToString( _streamPrx );
			_cltSessCtx.streamID = _streamPrx->getIdent().name;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "get stream(%s)"), _cltSessCtx.streamPrxID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getStream"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getStream", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch( const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getStream", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0305] caught by session(%s).getStream", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}

		try
		{
			_purchasePrx = _srvrSessPrx->getPurchase();
			_cltSessCtx.purchasePrxID = _env._pCommunicator->proxyToString(_purchasePrx);
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "purchase(%s)"), _cltSessCtx.purchasePrxID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getPurchase"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0306] caught by session(%s).getPurchase", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}

		// Get server session's resource
		TianShanIce::ValueMap svrSessPD;
		TianShanIce::SRM::ResourceMap rsMap;
		TianShanIce::SRM::ResourceMap::iterator rsMap_itor;
		try
		{
			rsMap     = _srvrSessPrx->getReources();
			svrSessPD = _srvrSessPrx->getPrivateData();
		}
		catch(const TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getReources()",
				ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getReources", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch( const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getReources", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch(Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getReources", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}

		// remember some PD in the local session ctx
		{
			char buf[128];
			//1. save SYS_PROP(primaryStart) as SYS_PROP(primaryItemNPT)
			TianShanIce::ValueMap::iterator itPD = svrSessPD.find(SYS_PROP(primaryStart));
			if (svrSessPD.end() != itPD && TianShanIce::vtLongs == itPD->second.type && itPD->second.lints.size() >0)
			{
				sprintf(buf, "%d.%03d", (int)(itPD->second.lints[0]/1000), (int)(itPD->second.lints[0]%1000));
				MAPSET(::TianShanIce::Properties, _cltSessCtx.props, SYS_PROP(primaryItemNPT), buf);
			}

			//2. save SYS_PROP(primaryEnd) as SYS_PROP(primaryEndNPT)
			itPD = svrSessPD.find(SYS_PROP(primaryEnd));
			if (svrSessPD.end() != itPD && TianShanIce::vtLongs == itPD->second.type && itPD->second.lints.size() >0)
			{
				sprintf(buf, "%d.%03d", (int)(itPD->second.lints[0]/1000), (int)(itPD->second.lints[0]%1000));
				MAPSET(::TianShanIce::Properties, _cltSessCtx.props, SYS_PROP(primaryEndNPT), buf);
			}
		}

		// overwrite SYS_PROP(primaryItemNPT) with that of stream if exists
		try
		{
			TianShanIce::Properties props = _streamPrx->getProperties();
			if (props.end() != props.find(SYS_PROP(primaryItemNPT)))
			{
				SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "get primaryItemNPT[%s] from stream"), props[SYS_PROP(primaryItemNPT)].c_str());
				MAPSET(::TianShanIce::Properties, _cltSessCtx.props, SYS_PROP(primaryItemNPT), props[SYS_PROP(primaryItemNPT)]);
			}
		}
		catch (Ice::Exception& ex)
		{
			// do not return 
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "session(%s) caught %s when get stream properties"), _cltSessCtx.srvrSessID.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			// do not return
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "session(%s) caught unknown exception when get stream properties"), _cltSessCtx.srvrSessID.c_str());
		}

		// determining the value of header "TianShan-Transport" in response, up to different mode(IP or QAM)
		if (_bQam)
		{
			int nProgramNumber = -1, nFrequency = -1, nQamMode = -1, nSymbolRate =-1;
			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtMpegProgram)
				&& rsMap[TianShanIce::SRM::rtMpegProgram].resourceData.end() != rsMap[TianShanIce::SRM::rtMpegProgram].resourceData.find("Id")
				&& rsMap[TianShanIce::SRM::rtMpegProgram].resourceData["Id"].ints.size() > 0)
				nProgramNumber = rsMap[TianShanIce::SRM::rtMpegProgram].resourceData["Id"].ints[0];

			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtPhysicalChannel)
				&& rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData.end() != rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData.find("channelId")
				&& rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData["channelId"].ints.size() > 0)
				nFrequency = rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData["channelId"].ints[0];

			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtAtscModulationMode))
			{
				TianShanIce::ValueMap& resData = rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData;

				if (resData.end() != resData.find("modulationFormat") && resData["modulationFormat"].ints.size() > 0)
					nQamMode = resData["modulationFormat"].ints[0];

				if (resData.end() != resData.find("symbolRate") && resData["symbolRate"].ints.size() > 0)
					nSymbolRate = resData["symbolRate"].ints[0];
			}

			ChangeQamMode(nQamMode);

			// Accordding to the document TianShan Architecture (ZQ) RTSP Specification
			// RTSP head "TianShan-Transport" should be in format of "%s;program-number=%d;frequency=%d;qam-mode=%d"
			// confirmed with andy, '%s' should gained from RTSP "Transport" head which should be lies at before the first char ';'.
			if (SPEC_NGOD_S1 == _cltSessCtx.requestType)	// NGOD spec
			{
				STRINGVECTOR strs;
				String::splitStr(hdrTransport, ";", strs);
				if (strs.size() >= 2)
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;%s;destination=%lld.%d;qam-mode=%d;modulation=qam%d", strs[0].c_str(), strs[1].c_str(), ((int64)nFrequency)*1000, nProgramNumber, nQamMode, nQamMode);
				else
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;destination=%lld.%d;qam-mode=%d;modulation=qam%d", String::getLeftStr(hdrTransport, ";", true).c_str(), ((int64)nFrequency)*1000, nProgramNumber, nQamMode, nQamMode);
				hdrTransport = _szBuf;
			}
			else
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;program-number=%d;frequency=%d;qam-mode=%d", String::getLeftStr(hdrTransport, ";", true).c_str(), nProgramNumber, nFrequency, nQamMode);
				if (nSymbolRate>0)
				{
					char* p = _szBuf + strlen(_szBuf);
					snprintf(p, _szBuf + sizeof(_szBuf) -p -2, ";symbol-rate=%d", nSymbolRate);
				}
			}

			// enh#18041 to Export BO TianShan as EdgeResource Management
			char buf[64];
			uint16 len = sizeof(buf)-2;
			const char* userAgent = _pRequest->getHeader(HeaderUserAgent, buf, &len);
			if (userAgent && strstr(userAgent, "EdgeAllocator"))
			{
				rsMap_itor = rsMap.find(TianShanIce::SRM::rtEthernetInterface);
				if (rsMap.end() == rsMap_itor)
					SSMLOG(WarningLevel, HandlerFmt(HandleSetup, "failed to find res[EthernetInterface] to export as EdgeRM"));
				else
				{
					SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "exporting as EdgeRM per user-agent: %s"), userAgent);
					::TianShanIce::ValueMap& ethData = rsMap_itor->second.resourceData;
					char* p = _szBuf + strlen(_szBuf);
					TianShanIce::ValueMap::iterator vMap_itor = ethData.find("destIP");
					if (ethData.end() != vMap_itor && vMap_itor->second.strs.size() >0)
					{
						snprintf(p, _szBuf + sizeof(_szBuf) -p -2, ";destination=%s", vMap_itor->second.strs[0].c_str());
						p+= strlen(p);
					}

					vMap_itor = ethData.find("destPort");
					if (ethData.end() != vMap_itor && vMap_itor->second.ints.size() >0)
					{
						snprintf(p, _szBuf + sizeof(_szBuf) -p -2, ";client_port=%d", vMap_itor->second.ints[0]);
						p+= strlen(p);
					}

					vMap_itor = ethData.find("destMac");
					if (ethData.end() != vMap_itor && vMap_itor->second.strs.size() >0)
					{
						snprintf(p, _szBuf + sizeof(_szBuf) -p -2, ";client=%d", vMap_itor->second.strs[0].c_str());
						p+= strlen(p);
					}

					SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "adjusted TsTranport[%s] per user-agent: %s"), _szBuf, userAgent);
				}
			}
			// end of enh#18041
		}
		else // non-QAM mode
		{
			TianShanIce::ValueMap::iterator vMap_itor;
			rsMap_itor = rsMap.find(TianShanIce::SRM::rtEthernetInterface);
			std::string srcIP, pokeholeSession;
			Ice::Int srcPort = -1, natPenetrating = -1;

			if (rsMap.end() != rsMap_itor)
			{
				::TianShanIce::ValueMap& ethData = rsMap_itor->second.resourceData;
				vMap_itor = ethData.find("natPenetrating");
				if (ethData.end() != vMap_itor && TianShanIce::vtInts == vMap_itor->second.type && vMap_itor->second.ints.size() > 0)
					natPenetrating =  vMap_itor->second.ints[0];

				vMap_itor = ethData.find("pokeholeSession");
				if (ethData.end() != vMap_itor && TianShanIce::vtStrings == vMap_itor->second.type && vMap_itor->second.strs.size() > 0)
					pokeholeSession = vMap_itor->second.strs[0];

				vMap_itor = ethData.find("srcIP");
				if (ethData.end() != vMap_itor && TianShanIce::vtStrings == vMap_itor->second.type && vMap_itor->second.strs.size() > 0)
					srcIP = vMap_itor->second.strs[0];

				vMap_itor = ethData.find("srcPort");
				if (ethData.end() != vMap_itor && TianShanIce::vtInts == vMap_itor->second.type && vMap_itor->second.ints.size() > 0)
					srcPort = vMap_itor->second.ints[0];
			}

			int nPos = -1;
			if (String::hasChar(hdrTransport, ';', nPos))
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;nat_penetrating=%d;source=%s;server_port=%d;pokehole_session=%s", 
					String::getLeftStr(hdrTransport, ";", true).c_str(), natPenetrating, srcIP.c_str(), srcPort, pokeholeSession.c_str());
			}
			else 
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;nat_penetrating=%d;source=%s;server_port=%d;pokehole_session=%s",
					hdrTransport.c_str(), natPenetrating, srcIP.c_str(), srcPort, pokeholeSession.c_str());
			}

		}

		_pResponse->setHeader(HeaderTianShanTransport, _szBuf); // Set "TianShan-Transport" field

		// 1. Mode is QAM. Just copy the request "Transport" as response "Transport" head.
		// 2. Mode is IP. There is other work to be done. as following
		//		if there is no "client_port" in the request head of "Transport", we should add "client_port=<value>" to response "Transport" head
		//	the <value> is "peerPort" gained from current connection.
		//		if there is no "destination" in the request head of "Transport", we should add "destination=<value>" to response "Transport" head
		//	the <value> is "peerIP" gained from current connection.
		//		if the "destination" is already specified in the request "Transport" head and "nat_penetrating=1" is also specified in "TianShan-Transport".
		//		according to document NATPenetratingDesignSpec.doc, we should use the "peerIP" gained from current connection rather than the specified value
		//		in the request. so at this situation we have to replace the value in the request.
		if (false == _bQam)
		{
			// if there is no "destination" or "client_port" in request "Transport" field, add them.
			{
				::std::string::size_type tmpSize = hdrTransport.size();
				if (tmpSize > 0 && hdrTransport[tmpSize - 1] == ';')
					hdrTransport.resize(tmpSize - 1); // remove the last ';' char.
			}

			const char* pDestination = NULL;
			const char* pTransport = hdrTransport.c_str();
			pDestination = strstr(pTransport, Destination "=");
			if (NULL == pDestination) // if no "destination" specified in the request, use the "peerIP" gained from current connection.
				hdrTransport += ";" Destination "=" + _destIP;
			else if (true == _bNatOpen) // replace the destination ip with the current connection's "peerIP"
			{
				// "posEqualAfterDest" specifies the equal char position after the "destination"
				// Transport: MP2T/AVP/UDP;unicast;destination=10.15.10.34;client_port=1234
				::std::string::size_type posEqualAfterDest = (::std::string::size_type)(pDestination - pTransport) + strlen(Destination);
				std::string leftStr, rightStrTmp, rightStr;
				leftStr = String::nLeftStr(hdrTransport, posEqualAfterDest); // gain the first n chars of a string
				rightStrTmp = String::rightStr(hdrTransport, posEqualAfterDest); // gain the string after the specified position
				int nPos;
				if (String::hasChar(rightStrTmp, ';', nPos))
					rightStr = String::rightStr(rightStrTmp, nPos - 1); // gain the string after the specified ip address including char ';'

				hdrTransport = leftStr + "=" + _destIP + rightStr;
			}

			if (NULL == strstr(hdrTransport.c_str(), ClientPort)) // if no "client_port" specified in the request, use the "peerPort" gained from current connection.
				hdrTransport += ";" ClientPort "=" + _destPort;
		}

		_pResponse->setHeader(HeaderTransport, (char*) hdrTransport.c_str());
		_pResponse->setHeader(HeaderRequire, getRequestHeader(HeaderRequire).c_str());

		snprintf(_szBuf, sizeof(_szBuf) - 1, "%d", _liveChannelConfig._rtspSession._timeout);
		_pResponse->setHeader(HeaderTianShanClientTimeout, _szBuf);

		// prepare the TianShanNoticeParam
		{
			static const NamePair TsNoticeParams[] = {
				{"primaryItemNPT", SYS_PROP(primaryItemNPT)},
				{"primaryEndNPT", SYS_PROP(primaryEndNPT)},
				{NULL, NULL}
			};

			std::string tsNoticeParamStr;
			for (int k=0; NULL !=TsNoticeParams[k].from; k++)
			{
				if (NULL == TsNoticeParams[k].to || _cltSessCtx.props.end() == _cltSessCtx.props.find(TsNoticeParams[k].to))
					continue;

				tsNoticeParamStr += std::string(TsNoticeParams[k].from) + "=" + _cltSessCtx.props[TsNoticeParams[k].to] + ";";
			}

			if (!tsNoticeParamStr.empty())
				_pResponse->setHeader(HeaderTianShanNoticeParam, tsNoticeParamStr.c_str());
		}
		return true;
	}

	SetupResponse::SetupResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}
	
	SetupResponse::~SetupResponse()
	{
	}
	
	bool SetupResponse::process()
	{
		bool bC2 = false;
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			// copy "TianShan-Notice" to "SeaChange-Notice"
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
			// Session: 0000000000021A5
			// TianShan-ClientTimeout: 600000				-		TianShan Format
			// copy to
			// Session: 0000000000021A5;timeout=600000		-		SeaChange Format
			std::string scSession = getResponseHeader(HeaderSession) + ";timeout=" + getResponseHeader(HeaderTianShanClientTimeout);
			_pResponse->setHeader(HeaderSession, scSession.c_str());
			_pResponse->setHeader(HeaderTianShanClientTimeout, ""); // remove "TianShan-ClientTimeout"
			// copy "TianShan-Transport" to "SeaChange-Transport", omit the first part of "TianShan-Transport" like the following.
			// TianShan-Transport: MP2T/DVBC/QAM;program-number=3;frequency=573;qam-mode=64
			// SeaChange-Transport: program-number=3;frequency=573;qam-mode=64
			_pResponse->setHeader(HeaderSeaChangeTransport, String::getRightStr(getResponseHeader(HeaderTianShanTransport), ";", true).c_str());
			_pResponse->setHeader(HeaderTianShanTransport, ""); // remove "TianShan-Transport"
		}
		else if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), VLCFormat))
		{
			//step 1: change "TianShan-Transport:" to "Transport:"
			_pResponse->setHeader(HeaderTransport, String::getRightStr(getResponseHeader(HeaderTianShanTransport), ";", true).c_str());
			_pResponse->setHeader(HeaderTianShanTransport, ""); // remove "TianShan-Transport"
		}
		else if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), NGODFormat))
		{
			if (!getRequestHeader(HeaderOnDemandSessionId).empty())
			{
				_pResponse->setHeader(HeaderOnDemandSessionId, (char *)getRequestHeader(HeaderOnDemandSessionId).c_str());
			}

			if (!getRequestHeader(HeaderClientSessionId).empty())
			{
				_pResponse->setHeader(HeaderClientSessionId, (char *)getRequestHeader(HeaderClientSessionId).c_str());
			}

			if (!getRequestHeader(HeaderContentType).empty())
			{
				_pResponse->setHeader(HeaderContentType, (char *)getRequestHeader(HeaderContentType).c_str());
			}
			else if (strstr(getRequestHeader(HeaderTransport).c_str(), "/C2") != NULL)
			{
				//bC2 = true;
				//_pResponse->setHeader(HeaderContentType, "text/xml");
			}
			else 
			{
				_pResponse->setHeader(HeaderContentType, "application/sdp");
			}
			
			char stampUTP[20];
			memset(stampUTP, 0, 20);
			time_t tnow = time(NULL);
			int64 seconds1900 = int64(tnow) + 2208988800;
			snprintf(stampUTP, 19, "%lld", seconds1900);

			std::string rtspServerIP = getRequestHeader("SYS#LocalServerIP");
			std::string rtspServerPort = getRequestHeader("SYS#LocalServerPort");
			std::string scSession = getResponseHeader(HeaderSession);

			std::string sTransType = "udp MP2T";
			std::string transStr = getRequestHeader(HeaderTransport);
			if (NULL != strstr(transStr.c_str(), "/DVBC/") || NULL != strstr(transStr.c_str(), "/QAM"))
				sTransType = "MP2T/DVBC/QAM";
			else if (NULL != strstr(transStr.c_str(), "AVP/UDP"))
				sTransType = "MP2T/AVP/UDP";

			if (!bC2)
			{
				std::string retContent;
				retContent = "v=0\r\n";
				retContent += std::string("o=- ") + scSession + " " + stampUTP + " IN IP4 " + rtspServerIP + "\r\n";
				retContent += "s=\r\n";
				retContent += "c=IN IP4 " + rtspServerIP +"\r\n";
				retContent += "t=0 0\r\n";	
				retContent += std::string("a=control:rtsp://") + rtspServerIP + ":" + rtspServerPort + "/" + scSession + "\r\n";
				retContent += "m=video 0 " + sTransType + "\r\n";
				_pResponse->printf_postheader(retContent.c_str());
			}
		}
		
		return true;
	}

}

