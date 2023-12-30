
#include "SetupRequest.h"
#include "TsConfig.h"
#include "urlstr.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{
	FixupSetup::FixupSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}
	
	FixupSetup::~FixupSetup()
	{
	}
	
	// it's main task is to change non-TianShan spec request to TianShan spec request
	bool FixupSetup::process()
	{
		// step 1. if there's a TianShan-AppData field, treat it as TianShan spec and return directly
		std::string scSrvrDataStr = getRequestHeader(HeaderSeaChangeServerData);
		String::removeChar(scSrvrDataStr, ' ');
		std::string tsAppData = getRequestHeader(HeaderTianShanAppData);
		String::removeChar(tsAppData, ' ');
		std::string scVLCAppData = getRequestHeader(HeaderUserAgent);
		std::string require = getRequestHeader(HeaderRequire);

		if (scVLCAppData.find(VLCFormat) != std::string::npos)//VLC format
		{
			//TODO: translate to tianshan spec
			//step 1: fixup url(for vlc)
			char _uriBuf[512];
			_uriBuf[511] = '\0';
			const char* pUrl = _pRequest->getUri(_uriBuf, sizeof(_uriBuf) - 1);
			int urlLen = strlen(pUrl);
			if (_uriBuf[urlLen - 1] == '/')
				_uriBuf[urlLen - 1] = '\0';
			std::string strUrl = _uriBuf;
			_pRequestWriter->setArgument(_pRequest->getVerb(), strUrl.c_str(), _pRequest->getProtocol(_uriBuf, sizeof(_uriBuf) - 1));

			//step 2: set header type to VLC format
			_pRequestWriter->setHeader(HeaderFormatType, VLCFormat);

			//step 3: copy "Transport" to "TianShan-Transport"
			std::string scTransportStr = getRequestHeader(HeaderTransport);
			String::removeChar(scTransportStr, ' ');
			_pRequestWriter->setHeader(HeaderTianShanTransport, (char*) scTransportStr.c_str());

			//step 4: set virtual service group
			std::string strServiceGroup = getRequestHeader(HeaderTianShanServiceGroup);
			String::removeChar(strServiceGroup, ' ');
			if (strServiceGroup.empty())
				_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char *)_tsConfig._virtualServiceGroup.group.c_str());			
		}
		else if(require.find("com.comcast.ngod.s1") != std::string::npos)
		{
			// NGOD spec
			//step 1: fixup url
			char _uriBuf[512];
			_uriBuf[511] = '\0';
			_pRequest->getUri(_uriBuf, sizeof(_uriBuf) - 1);
			std::string strUrl = _uriBuf;
			std::string  strAppID;
			STRINGVECTOR strs;
			String::splitStr(strUrl, ";&?", strs);
			for (STRINGVECTOR_ITOR strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
			{
				std::string keystr, valstr;
				keystr = String::getLeftStr(*strsItor, "=", true);
				valstr = String::getRightStr(*strsItor, "=", true);
				if ("AppPath" == keystr)
				{
					strAppID = valstr;
					break;
				}
			}
			if(strAppID.empty())
				strAppID = _tsConfig._ngods1.defaultAppPath;
			strUrl = "";
			for (STRINGVECTOR_ITOR strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
			{
				if(strs.begin() == strsItor)
				{
					std::string temp = *strsItor;
					if(temp[temp.size()-1] != '/')
						temp += "/";
					strUrl = temp + strAppID + "?";
				}
				else
				{
					if(strs.end()-1 != strsItor)
						strUrl = strUrl + *strsItor + "&";
					else
						strUrl = strUrl + *strsItor;
				}
			}

			_pRequestWriter->setArgument(_pRequest->getVerb(), strUrl.c_str(), _pRequest->getProtocol(_uriBuf, sizeof(_uriBuf) - 1));

			//step 2: set header type to VLC format
			_pRequestWriter->setHeader(HeaderFormatType, NGODFormat);

			//step 3: set ServiceGroup
			std::string serviceGroup = getRequestHeader(HeaderTianShanServiceGroup);
			String::removeChar(serviceGroup, ' ');
			bool bFound = false;
			std::string transport = getRequestHeader(HeaderTransport);
			if (transport.find("MP2T/DVBC/QAM") != std::string::npos && serviceGroup.empty())
			{	
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
							for(std::vector<SessionGroupHolder>::iterator iter = _tsConfig._ngods1._sessionGroups.sessionGroups.begin(); iter != _tsConfig._ngods1._sessionGroups.sessionGroups.end(); iter++ )
							{
								boost::regex qamRegex(iter->expression);
								boost::cmatch result;
								if(!boost::regex_match(valstr.c_str(), qamRegex))
								{
									continue;
								}
								else
								{
									serviceGroup = iter->group;
									bFound = true;
									break;
								}
							}
						}
					}
					if(bFound)
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
			if(bFound)// NGOD spec
			{
				_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char *)serviceGroup.c_str());
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
			ZQ::common::URLStr axiomUrl( strTempUrl.c_str() , false );
			
#define		VALIDSTR(x) (x&&x[0]!=0)
			//check if it is old axiom format or not
			const char* pTempVarValue = axiomUrl.getVar( axiomUrl.getVarname(0) ) ;
			const char* pTempVarName = axiomUrl.getVarname(0);
			
			if( VALIDSTR(pTempVarName))
			{
				if( VALIDSTR(pTempVarValue) )
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

					if( VALIDSTR(pNodeGroupId) )
					{//replace ndoeGroupId with this one
						SSMLOG(InfoLevel , HandlerFmt(HandleSetup ,"replace %s with NodeGroupId in uri [%s]"),
							HeaderTianShanServiceGroup,
							pNodeGroupId);
						_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char*)pNodeGroupId );
					}

					const char* pPath = VALIDSTR(pAppName) ? pAppName : pAppUID;
					if( ! VALIDSTR(pPath) )
					{//set a default value
						pPath = _tsConfig._defaultParams._axiomMsgDefaultPath.c_str();
						SSMLOG(InfoLevel , HandlerFmt(HandleSetup,"No AppNam or AppUID is found , use default value :%s"),pPath ) ;
					}
					if( !( ( VALIDSTR(pPID) && VALIDSTR(pPAID) ) || VALIDSTR(pAssetUID) ) )
					{
						SSMLOG(WarningLevel,HandlerFmt(HandleSetup,"Invalid Axiom url format , no PID+PAID or ASSETUID in uri:%s") , pURL );
					}
					else
					{
						if( ( VALIDSTR(pPID) && VALIDSTR(pPAID) ) && VALIDSTR(pAssetUID) )
						{
							SSMLOG(WarningLevel , HandlerFmt(HandleSetup,"Both PID+PAID and ASSETUID are present ,take PID+PAID set"));
							urlStr = std::string("rtsp://") + axiomUrl.getHost() + "/" + pPath + "?asset=" + pPID+"#"+ pPAID;
						}
						else
						{
							if( VALIDSTR(pAssetUID) )
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
					const char* pDot = strstr( pTempVarName , "." );
					if( VALIDSTR(pDot) )
					{
						std::string		strAppId;
						std::string		strAssetId;

						strAppId.assign( pTempVarName , pDot - pTempVarName );
						strAssetId = ( pDot + 1 );
						urlStr = std::string("rtsp://") + axiomUrl.getHost() + "/" + strAppId + "?assetUid=" + strAssetId;
					}
					else
					{//can't find dot , treat it as invalid old axiom uri format
						SSMLOG(WarningLevel , HandlerFmt(HandleSetup,"Invalid old axiom uri format:%s"),pURL);
					}
				}
			}
			else
			{
				SSMLOG(WarningLevel,HandlerFmt(HandleSetup,"Invalid url , no uri content is found"));
			}

			/*
			char buffSite[MAX_PATH], buffApp[MAX_PATH], buffAsst[MAX_PATH], buffNotUse[MAX_PATH];
			buffSite[sizeof(buffSite) - 1] = '\0';
			buffApp[sizeof(buffApp) - 1] = '\0';
			buffAsst[sizeof(buffAsst) - 1] = '\0';
			buffNotUse[sizeof(buffNotUse) - 1] = '\0';
			if (4 == sscanf(pURL, "rtsp://%[^/]/%[^?]?%[^.].%s", buffSite, buffNotUse, buffApp, buffAsst)
				|| 4 == sscanf(pURL, "/%[^/]/%[^?]?%[^.].%s", buffSite, buffNotUse, buffApp, buffAsst)
				|| 4 == sscanf(pURL, "%[^/]/%[^?]?%[^.].%s", buffSite, buffNotUse, buffApp, buffAsst))
			{
				// treat the url as the format of "[rtsp:/[/]]<site>/<notuse>?<application>.<assetID>"
				// SeaChange Url translation
				//	/<site>/<notUsed>?<path>.<assetID>		->		rtsp://<site>/<path>?assetUid=<assetID>
				//	/SeaChange/ITV?60010000.3B9ACDAA		->		rtsp://SeaChange/60010000?assetUid=3B9ACDAA
				urlStr = std::string("rtsp://") + buffSite + "/" + buffApp + "?assetUid=" + buffAsst;
			}
			else if (3 == sscanf(pURL, "rtsp://%[^?]?%[^.].%s", buffSite, buffApp, buffAsst)
				|| 3 == sscanf(pURL, "/%[^?]?%[^.].%s", buffSite, buffApp, buffAsst)
				|| 3 == sscanf(pURL, "%[^?]?%[^.].%s", buffSite, buffApp, buffAsst))
			{
				// treat the url as the format of "[rtsp:/[/]]<site>?<application>.<assetID>"
				// SeaChange Url translation
				//	/<site>?<path>.<assetID>			->		rtsp://<site>/<path>?assetUid=<assetID>
				//	/SeaChange?60010000.3B9ACDAA		->		rtsp://SeaChange/60010000?assetUid=3B9ACDAA
				urlStr = std::string("rtsp://") + buffSite + "/" + buffApp + "?assetUid=" + buffAsst;
			}
			else if( ( 3 == sscanf(pURL,"rtsp://%[^/]/%[^?]?%s",buffSite , buffApp,buffAsst ) )
				||( 3 == sscanf(pURL,"/%[^/]/%[^?]?%s",buffSite , buffApp,buffAsst ) )
				||( 3 == sscanf(pURL,"%[^/]/%[^?]?%s",buffSite , buffApp,buffAsst ) ) )
			{
				//treat the url as the format of  "[rtsp:/[/]]<site>/<application>?PID=x&PAID=x"
			
				char* pSharp = strstr( buffAsst, "&" );
				if( pSharp )
				{
					char* pProviderId = buffAsst;
					char* pAssetId = pSharp+1;
					*pSharp = 0;
#define PIDSTR "ProviderId="
#define PAIDSTR "ProviderAssetId="
					int iPidStrLen = strlen(PIDSTR);
					int iPaidStrlen = strlen(PAIDSTR);
					if( _strnicmp( pProviderId,PIDSTR, iPidStrLen ) == 0 
						&& _strnicmp( pAssetId ,PAIDSTR , iPaidStrlen ) == 0 )
					{
						pProviderId =strstr( pProviderId,"=" ) + 1;
						pAssetId = strstr (pAssetId,"=") + 1 ;
						urlStr = std::string("rtsp://") + buffSite + "/" + buffApp + "?asset=" + pProviderId+"#"+pAssetId;
					}
				}
			}			
			*/

			// if urlStr is not empty, means that he has been modified.
			if (false == urlStr.empty())
				_pRequestWriter->setArgument(_pRequest->getVerb(), urlStr.c_str(), _pRequest->getProtocol(_szBuf, sizeof(_szBuf) - 1));
		}
		else // no HeaderSeaChangeServerData and TianShan-AppData, maybe GB spec
		{
			char _uriBuf[512];
			_uriBuf[511] = '\0';
			_pRequest->getUri(_uriBuf, sizeof(_uriBuf) - 1);
			std::string strUrl = _uriBuf +_tsConfig._ngods1.defaultAppPath;;	
			_pRequestWriter->setArgument(_pRequest->getVerb(), strUrl.c_str(), _pRequest->getProtocol(_uriBuf, sizeof(_uriBuf) - 1));
			return true;
		}

		return true;
	}
	
	HandleSetup::HandleSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse), _sessMgrPrx(NULL)
	{
	}
	
	HandleSetup::~HandleSetup()
	{
	}
	
	bool HandleSetup::process()
	{
		SessionContextImplPtr pSessionContext = new SessionContextImpl(_env);
		pSessionContext->announceSeq = 0;
		pSessionContext->url = getUrl();
		pSessionContext->rangePrefix = "npt";
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), NGODFormat))
			pSessionContext->requestType = 3; // NGOD spec
		else if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
			pSessionContext->requestType = 1; // SeaChange
		else 
		{
			_bNeedModifyResponse = false;
			pSessionContext->requestType = 2; // TianShan
		}

		//////////////////////////////////////////////////////////////////////////
		// 1. get sessionid from request
		// 2. find sessionid in RtspProxy, if not found, ignore.
		// 3. if session already setuped, return true with the statusline "RTSP/1.0 455 Method Not Valid in This State"
		// notice here, you can not return false, because the upcall function will destroy the session
		// but why application destroy the session when setup failed is that application can make sure there is no leak in resource.
		//////////////////////////////////////////////////////////////////////////
		_session = getRequestHeader(HeaderSession, -1);
		if (_pSite->findClientSession(_session.c_str()))
		{
			_ok_statusline = ResponseMethodNotValidInThisState;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "session already setuped");
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeRightResponse();
			return true; // notice here, must return true;
		}
		
		// create rtsp client session
		SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do createRtspSession()"));
		IClientSession* pRtspSession = _pSite->createClientSession(NULL, pSessionContext->url.c_str());
		if (NULL == pRtspSession || NULL == pRtspSession->getSessionID())
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "SiteError[SetupHandle:0300] create client session failed");
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		_pRequestWriter->setHeader(HeaderSession, (char*) pRtspSession->getSessionID());
		_session = pRtspSession->getSessionID();
		pSessionContext->ident.name = _session;
		pSessionContext->ident.category = ServantType;
		SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "createRtspSession() successfully"));

		// create weiwoo session
		::TianShanIce::SRM::Resource sessRC;
		sessRC.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
		sessRC.status = TianShanIce::SRM::rsRequested;
		::TianShanIce::Variant vrtUrl;		
		vrtUrl.bRange = false;
		vrtUrl.type = TianShanIce::vtStrings;
		vrtUrl.strs.clear();
		vrtUrl.strs.push_back(pSessionContext->url);
		sessRC.resourceData["uri"] = vrtUrl;
		try 
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do weiwooService.createSession"));
			_sessMgrPrx = TianShanIce::SRM::SessionManagerPrx::checkedCast(_env._pCommunicator->stringToProxy(_tsConfig._sessionManager._endpoint));
			_srvrSessPrx = _sessMgrPrx->createSession(sessRC);
			pSessionContext->srvrSessPrxID = _env._pCommunicator->proxyToString(_srvrSessPrx);
printf("prxid: %s\n", pSessionContext->srvrSessPrxID.c_str());
			pSessionContext->srvrSessID = _srvrSessPrx->getId();
printf("srvrSessionId: %s\n", pSessionContext->srvrSessID.c_str());
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "weiwoo session(%s) created"), pSessionContext->srvrSessID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by weiwooService(%s).createSession"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _tsConfig._sessionManager._endpoint.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught by weiwooService(%s).createSession", 
				ex.ice_name().c_str(), _tsConfig._sessionManager._endpoint.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		// DO: add x-userID to private data
		std::string xUserID = getRequestHeader(HeaderXUserID);
		if (!xUserID.empty())
		{
			TianShanIce::Variant vrtXUserID;
			vrtXUserID.bRange = false;
			vrtXUserID.type = TianShanIce::vtStrings;
			vrtXUserID.strs.clear();
			vrtXUserID.strs.push_back(xUserID);
			addPrivateData(HeaderXUserID, vrtXUserID);
		}

		// DO: add EntitlementCode to private data
		std::string strEntitlementCode = getRequestHeader(HeaderEntitlementCode);
		TianShanIce::Variant vrtEntitlementCode;
		vrtEntitlementCode.bRange = false;
		vrtEntitlementCode.type = TianShanIce::vtStrings;
		vrtEntitlementCode.strs.clear();
		vrtEntitlementCode.strs.push_back(strEntitlementCode);
		addPrivateData(HeaderEntitlementCode, vrtEntitlementCode);

		// DO: add Authorization to private data
		std::string strAuthorization = getRequestHeader(HeaderAuthorization);
		TianShanIce::Variant vrtAuthorization;
		vrtAuthorization.bRange = false;
		vrtAuthorization.type = TianShanIce::vtStrings;
		vrtAuthorization.strs.clear();
		vrtAuthorization.strs.push_back(strEntitlementCode);
		addPrivateData(HeaderEntitlementCode, vrtAuthorization);

		// DO: add application-id to private data which gained from url's path
		ZQ::common::URLStr urlStr(pSessionContext->url.c_str(),true);
		const char* pAppID = urlStr.getPath();
		std::string strAppID = (NULL != pAppID) ? pAppID : _tsConfig._ngods1.defaultAppPath;
		//if no AppID, use default defaultAppPath in config
		TianShanIce::Variant vrtAppID;
		vrtAppID.bRange = false;
		vrtAppID.type = TianShanIce::vtStrings;
		vrtAppID.strs.clear();
		vrtAppID.strs.push_back(strAppID);
		addPrivateData(ApplicationID, vrtAppID);

		// enum parameters in url and add them to private data
		std::map<std::string, std::string> urlVars = urlStr.getEnumVars();
		for (std::map<std::string, std::string>::iterator urlVars_itor = urlVars.begin(); urlVars_itor != urlVars.end(); urlVars_itor ++)
		{
			TianShanIce::Variant vrtUrlVar;
			vrtUrlVar.bRange = false;
			vrtUrlVar.type = TianShanIce::vtStrings;
			vrtUrlVar.strs.clear();
			vrtUrlVar.strs.push_back(urlVars_itor->second);
			addPrivateData(urlVars_itor->first, vrtUrlVar);
		}

		// DO: add client session id to private data
		TianShanIce::Variant vrtCltSess;
		vrtCltSess.bRange = false;
		vrtCltSess.type = TianShanIce::vtStrings;
		vrtCltSess.strs.clear();
		vrtCltSess.strs.push_back(_session);
		addPrivateData(ClientSessionID, vrtCltSess);


		//add original url into weiwoo session's private data
		std::string originalURL = getRequestHeader( OriginalUrl );
		if( !originalURL.empty() )
		{
			TianShanIce::Variant vrtOriginalUrl;
			vrtOriginalUrl.bRange = false;
			vrtOriginalUrl.type = TianShanIce::vtStrings;
			vrtOriginalUrl.strs.clear();
			vrtOriginalUrl.strs.push_back(originalURL);

			addPrivateData( OriginalUrl, vrtOriginalUrl);

		}
		
		//add peer address into weiwoo's private data
		{
			char addr[64];
			addr[63] = '\0';
			IClientRequest::RemoteInfo info;
			info.size = sizeof(IClientRequest::RemoteInfo);
			info.addrlen = sizeof(addr);
			info.ipaddr = addr;
			if(_pRequest->getRemoteInfo(info))
			{
				char sPort[64];
				sPort[63] = '\0';
				snprintf(addr, 63, "%s:%u", info.ipaddr ,info.port);			
			}
			TianShanIce::Variant vrtClientAddress;
			vrtClientAddress.bRange = false;
			vrtClientAddress.type = TianShanIce::vtStrings;
			vrtClientAddress.strs.clear();
			vrtClientAddress.strs.push_back(addr);
			addPrivateData( clientAddress, vrtClientAddress);
		}


		// DO: add TianShan-AppData to private data
		STRINGVECTOR tsAppDatas;
		STRINGVECTOR_ITOR tsAppDatasItor;
		std::string tsAppDataStr = getRequestHeader(HeaderTianShanAppData);
		String::removeChar(tsAppDataStr, ' ');
		String::splitStr(tsAppDataStr, ";", tsAppDatas);
		for (tsAppDatasItor = tsAppDatas.begin(); tsAppDatasItor != tsAppDatas.end(); tsAppDatasItor ++)
		{
			std::string keyStr, valStr;
			keyStr = String::getLeftStr(*tsAppDatasItor, "=", true);
			valStr = String::getRightStr(*tsAppDatasItor, "=", true);
			if (false == keyStr.empty() && false == valStr.empty())
			{
				TianShanIce::Variant vrtAppData;
				vrtAppData.bRange = false;
				vrtAppData.type = TianShanIce::vtStrings;
				vrtAppData.strs.clear();
				vrtAppData.strs.push_back(valStr);
				addPrivateData(keyStr, vrtAppData);
			}
		}
		// if there is no mac-address, take the device-id as mac-address
		if (_pdMap.end() == _pdMap.find(ClientRequestPrefix MacAddress))
		{
			TianShanIce::ValueMap::iterator titor;
			titor = _pdMap.find(ClientRequestPrefix DeviceID);
			if (_pdMap.end() != titor && titor->second.strs.size() > 0)
			{
				TianShanIce::Variant vrtMacAddress;
				vrtMacAddress.bRange = false;
				vrtMacAddress.type = TianShanIce::vtStrings;
				vrtMacAddress.strs.clear();
				vrtMacAddress.strs.push_back(titor->second.strs[0]);
				addPrivateData(MacAddress, vrtMacAddress);
			}
		}

		// Parse "TianShan-Transport" head, if there is "nat_penetrating" and values "1", it's a request sent by a NAT server.
		bool bNatOpen = false;
		std::string tsTrspStr = getRequestHeader(HeaderTianShanTransport);
		String::removeChar(tsTrspStr, ' ');
		STRINGVECTOR tsTrspStrs;
		STRINGVECTOR_ITOR tsTrspStrs_itor;
		String::splitStr(tsTrspStr, ";", tsTrspStrs);
		for (tsTrspStrs_itor = tsTrspStrs.begin(); tsTrspStrs_itor != tsTrspStrs.end(); tsTrspStrs_itor ++)
		{
			std::string keyStr, valStr;
			keyStr = String::getLeftStr(*tsTrspStrs_itor, "=", true);
			valStr = String::getRightStr(*tsTrspStrs_itor, "=", true);
			if (false == keyStr.empty() && false == valStr.empty())
			{
				TianShanIce::Variant vrt;
				vrt.bRange = false;
				vrt.type = TianShanIce::vtStrings;
				vrt.strs.clear();
				vrt.strs.push_back(valStr);
				addPrivateData(keyStr, vrt);
				// enable nat mode, if "nat_penetrating" = "1";
				if (0 == stricmp(keyStr.c_str(), "nat_penetrating") && atoi(valStr.c_str()) == 1)
					bNatOpen = true;
			}
		}

		// Parse "Transport" head
		std::string destIP, destPort, destMac;
		::Ice::Long lBandWidth = -1; // bandwidth in transport header
		std::string peerIP, peerPort; // current connection's peerIP and peerPort.
		std::string trspStr = getRequestHeader(HeaderTransport);
		String::removeChar(trspStr, ' ');
		{
			STRINGVECTOR trspStrs;
			STRINGVECTOR_ITOR trspStrsItor;
			String::splitStr(trspStr, ";", trspStrs);
			for (trspStrsItor = trspStrs.begin(); trspStrsItor != trspStrs.end(); trspStrsItor ++)
			{
				std::string keyStr, valStr;
				keyStr = String::getLeftStr(*trspStrsItor, "=", true);
				valStr = String::getRightStr(*trspStrsItor, "=", true);
				if (false == keyStr.empty() && false == valStr.empty())
				{
					TianShanIce::Variant vrt;
					vrt.bRange = false;
					vrt.type = TianShanIce::vtStrings;
					vrt.strs.clear();
					vrt.strs.push_back(valStr);
					if ("ServiceGroup" == keyStr)
						addPrivateData(NodeGroupID, vrt);
					addPrivateData(keyStr, vrt);
				}
				if (0 == stricmp(keyStr.c_str(), Destination))
					destIP = valStr;
				if (0 == stricmp(keyStr.c_str(), ClientPort))
					destPort = valStr;
				if (0 == stricmp(keyStr.c_str(), ClientMac))
					destMac = valStr;
				if (0 == stricmp(keyStr.c_str(), BandWidth))
				{
					lBandWidth = atol(valStr.c_str());
				}
			}
		}

		bool bQam = false;
		if (NULL != strstr(trspStr.c_str(), "/DVBC/") || NULL != strstr(trspStr.c_str(), "/QAM"))
		{// QAM mode(MP2T/DVBC/QAM) 
			bQam = true;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is QAM mode"));
		}
		//add "|| NULL != strstr(trspStr.c_str(), HeaderVLCTransport)" by lxm at 2008.12.22 to support VLC
		else if (NULL != strstr(trspStr.c_str(), "/UDP") || NULL != strstr(trspStr.c_str(), HeaderVLCTransport))
		{// IP mode(MP2T/AVP/UDP)
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is IP mode"));

			{// get current connection's "peerPort", "peerIP"
				char addr[64];
				addr[63] = '\0';
				IClientRequest::RemoteInfo info;
				info.size = sizeof(IClientRequest::RemoteInfo);
				info.addrlen = sizeof(addr);
				info.ipaddr = addr;
				if(_pRequest->getRemoteInfo(info))
				{
					peerIP = info.ipaddr;
					char sPort[64];
					sPort[63] = '\0';
					snprintf(sPort, 63, "%d", info.port);
					peerPort = sPort;
				}
			}

			// if the "destPort" gained from "Transport" field is not exist or empty, use the "peerPort".
			if (true == destPort.empty())
				destPort = peerPort;

			// define resources and initialize them.
			TianShanIce::ValueMap rscStreamDest;
			rscStreamDest.clear();
			TianShanIce::Variant vrtDestIP, vrtDestPort, vrtDestMac;
			
			vrtDestIP.bRange = false;
			vrtDestIP.type = TianShanIce::vtStrings;
			vrtDestIP.strs.clear();
			
			vrtDestPort.bRange = false;
			vrtDestPort.type = TianShanIce::vtInts;
			vrtDestPort.ints.clear();
			vrtDestPort.ints.push_back(atoi(destPort.c_str()));
			
			vrtDestMac.bRange = false;
			vrtDestMac.type = TianShanIce::vtStrings;
			vrtDestMac.strs.clear();
			
			if ( !destPort.empty() )
			{
				rscStreamDest["destPort"] = vrtDestPort;
			}
			
			if ( !destMac.empty() )
			{
				vrtDestMac.strs.push_back(destMac);
				rscStreamDest["destMac"] = vrtDestMac;			
			}

			if (true == bNatOpen) // NAT mode
			{
				// according to the document NATPenetratingDesignSpec.doc
				// NAT mode is open, use the "peerIP" gained from current connection rather than "destIP" stored as "destination" in "Transport" field.
				destIP = peerIP; // copy the "peerIP" to "destIP"
			}
			else if (true == destIP.empty())
			{
				// not NAT mode and "destIP" stored as "destination" in "Transport" field is empty or not exist.
				// use the "peerIP" gained from current connection. the application will use the current peer ip
				// address as the streaming destination.
				destIP = peerIP;
			}
			vrtDestIP.strs.push_back(destIP);
			if (!destIP.empty())
				rscStreamDest["destIP"] = vrtDestIP;

			try
			{
			_srvrSessPrx->addResource(TianShanIce::SRM::rtEthernetInterface, TianShanIce::SRM::raMandatoryNonNegotiable, rscStreamDest);
			}
			catch (TianShanIce::BaseException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).addResource"
					, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				composeErrorResponse();
				return false;
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0302] caught by session(%s).addResource", 
					ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				composeErrorResponse();
				return false;
			}
		}
		else 
		{// 461 Unsupported Transport
			_error_statusline = ResponseUnsupportedTransport;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s not supported", trspStr.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		if (lBandWidth > 0)
		{
			TianShanIce::ValueMap vmpBandWidth;
			TianShanIce::Variant vrtBandWidth;
			vrtBandWidth.type = TianShanIce::vtLongs;
			vrtBandWidth.bRange = false;
			vrtBandWidth.lints.clear();
			vrtBandWidth.lints.push_back(lBandWidth);
			vmpBandWidth.clear();
			vmpBandWidth["bandwidth"] = vrtBandWidth;
			try
			{
			_srvrSessPrx->addResource(TianShanIce::SRM::rtTsDownstreamBandwidth, TianShanIce::SRM::raMandatoryNonNegotiable, vmpBandWidth);
			}
			catch (TianShanIce::BaseException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).addResource"
					, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				composeErrorResponse();
				return false;
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0308] caught by session(%s).addResource", 
					ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				composeErrorResponse();
				return false;
			}
		}

		// flush the private datas to weiwoo session
		flushPrivateData();

		try
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do session(%s).provision"), pSessionContext->srvrSessID.c_str());
//			provison() throw exception list: InvalidResource, InvalidParameter, InvalidStateOfArt, NotSupported, ServerError;
			Ice::Long timeUsed = ZQTianShan::now();
			_srvrSessPrx->provision();
			timeUsed = ZQTianShan::now() - timeUsed;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "session(%s).provision successfully, used [%lld]ms"), pSessionContext->srvrSessID.c_str(), timeUsed);
		}
		catch (TianShanIce::SRM::InvalidResource& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).provision"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_error_statusline = ResponseNotFound; // return "404 Not Found" error
			composeErrorResponse();
			return false;
		}
		catch (TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).provision"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_error_statusline = ResponseParameterNotUnderstood;
			composeErrorResponse();
			return false;
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).provision"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0303] caught by session(%s).provision",
				ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		try
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do session(%s).serve"), pSessionContext->srvrSessID.c_str());
//			serve() throw exception list: InvalidStateOfArt, InvalidResource, OutOfResource, InvalidParameter, NotSupported, ServerError;
			Ice::Long timeUsed = ZQTianShan::now();
			_srvrSessPrx->serve();
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "session(%s).serve successfully, used [%lld]ms"), pSessionContext->srvrSessID.c_str(), ZQTianShan::now()-timeUsed);
		}
		catch (TianShanIce::SRM::OutOfResource& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).serve"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_error_statusline = ResponseNotEnoughBandwidth; // "RTSP/1.0 453 Not Enough Bandwidth"
			composeErrorResponse();
			return false;
		}
		catch (TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).serve"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_error_statusline = ResponseParameterNotUnderstood;
			composeErrorResponse();
			return false;
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).serve"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0304] caught by session(%s).serve", 
				ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		try
		{
			pSessionContext->streamPrxID = _env._pCommunicator->proxyToString(_streamPrx = _srvrSessPrx->getStream());
			pSessionContext->streamID = _streamPrx->getIdent().name;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "stream(%s)"), pSessionContext->streamPrxID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getStream"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0305] caught by session(%s).getStream", 
				ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		// get stream property "primaryItemNPT"
		try
		{
			TianShanIce::Properties props = _streamPrx->getProperties();
// 			if (props.find("sys.primaryItemNPT") != props.end())
// 			{
// 				SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "get primaryItemNPT(%s)"), props["sys.primaryItemNPT"].c_str());
// 				// save to context
// 				pSessionContext->props.insert(std::make_pair("sys.primaryItemNPT", props["sys.primaryItemNPT"]));
// 				std::stringstream oss;
// 				oss << "primaryItemNPT=" << props["sys.primaryItemNPT"];
// 				_pResponse->setHeader(HeaderTianShanNoticeParam, oss.str().c_str());
// 			}
			if (props.find("SETUP.GlobalSession") != props.end())
			{
				SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "get GlobalSession(%s)"), props["SETUP.GlobalSession"].c_str());
				// save to context
				pSessionContext->props.insert(std::make_pair("GlobalSession", props["SETUP.GlobalSession"]));
				_pResponse->setHeader("GlobalSession", props["SETUP.GlobalSession"].c_str());
			}
			if (props.find("SETUP.ControlSession") != props.end())
			{
				SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "get ControlSession(%s)"), props["SETUP.ControlSession"].c_str());
				// save to context
				pSessionContext->props.insert(std::make_pair("ControlSession", props["SETUP.ControlSession"]));
				_pResponse->setHeader("ControlSession", props["SETUP.ControlSession"].c_str());
				TianShanIce::Variant vrtCtrlSession;
				vrtCtrlSession.bRange = false;
				vrtCtrlSession.type = TianShanIce::vtStrings;
				vrtCtrlSession.strs.clear();
				vrtCtrlSession.strs.push_back(props["SETUP.ControlSession"]);
				addPrivateData("sys.ControlSession", vrtCtrlSession);
				flushPrivateData();
			}
		}
		catch (Ice::Exception& ex)
		{
			// do not return 
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "session(%s) caught %s when get stream properties"), pSessionContext->srvrSessID.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			// do not return
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "session(%s) caught unknown exception when get stream properties"), pSessionContext->srvrSessID.c_str());
		}

		try
		{
			pSessionContext->purchasePrxID = _env._pCommunicator->proxyToString(_purchasePrx = _srvrSessPrx->getPurchase());
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "purchase(%s)"), pSessionContext->purchasePrxID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getPurchase"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0306] caught by session(%s).getPurchase", 
				ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		// Get server session's resource
		TianShanIce::SRM::ResourceMap rsMap;
		TianShanIce::SRM::ResourceMap::iterator rsMap_itor;
		try
		{
			rsMap = _srvrSessPrx->getReources();
		}
		catch(const TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getReources"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}
		catch(Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getReources", 
				ex.ice_name().c_str(), pSessionContext->srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			composeErrorResponse();
			return false;
		}

		// "TianShan-Transport" response field will be different depending different mode(IP or QAM)
		if (true == bQam)
		{
			int nProgramNumber = -1, nQamMode = -1;
			float nFrequency = -1, nSymbolRate = -1;
			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtMpegProgram)
				&& rsMap[TianShanIce::SRM::rtMpegProgram].resourceData.end() != rsMap[TianShanIce::SRM::rtMpegProgram].resourceData.find("Id")
				&& rsMap[TianShanIce::SRM::rtMpegProgram].resourceData["Id"].ints.size() > 0)
				nProgramNumber = rsMap[TianShanIce::SRM::rtMpegProgram].resourceData["Id"].ints[0];

			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtPhysicalChannel)
				&& rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData.end() != rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData.find("channelId")
				&& rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData["channelId"].ints.size() > 0)
				nFrequency = (float)rsMap[TianShanIce::SRM::rtPhysicalChannel].resourceData["channelId"].ints[0];

			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtAtscModulationMode)
				&& rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData.end() != rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData.find("modulationFormat")
				&& rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData["modulationFormat"].ints.size() > 0)
				nQamMode = rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData["modulationFormat"].ints[0];


			if (rsMap.end() != rsMap.find(TianShanIce::SRM::rtAtscModulationMode)
				&& rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData.end() != rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData.find("modulationFormat")
				&& rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData["symbolRate"].ints.size() > 0)
				nSymbolRate = (float)rsMap[TianShanIce::SRM::rtAtscModulationMode].resourceData["symbolRate"].ints[0];


			ChangeQamMode(nQamMode);

			// Accordding to the document TianShan Architecture (ZQ) RTSP Specification
			// RTSP head "TianShan-Transport" should be in format of "%s;program-number=%d;frequency=%d;qam-mode=%d"
			// confirmed with andy, '%s' should gained from RTSP "Transport" head which should be lies at before the first char ';'.
			if(3 == pSessionContext->requestType)	// NGOD spec
			{
				STRINGVECTOR strs;
				String::splitStr(trspStr, ";", strs);
				if(strs.size() >= 2)
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;%s;destination=%d.%d;qam-mode=%d", strs[0].c_str(), strs[1].c_str(), nFrequency*1000, nProgramNumber, nQamMode);
				else
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;destination=%d.%d;qam-mode=%d", String::getLeftStr(trspStr, ";", true).c_str(), nFrequency*1000, nProgramNumber, nQamMode);
				trspStr = _szBuf;
			}
			else
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;programid=%d;frequency=%f;modulation=%d;symbolRate=%f", String::getLeftStr(trspStr, ";", true).c_str(), nProgramNumber, (float)(nFrequency/1000), nQamMode, (float)(nSymbolRate/1000));
				trspStr = _szBuf;
			}
		}
		else 
		{
			TianShanIce::ValueMap::iterator vMap_itor;
			TianShanIce::Variant vrtNatPenetrating, vrtSrcIP, vrtSrcPort, vrtPokeholeSession;
			rsMap_itor = rsMap.find(TianShanIce::SRM::rtEthernetInterface);
			if (rsMap.end() != rsMap_itor)
			{
				vMap_itor = ( ((((*rsMap_itor).second)).resourceData)).find("natPenetrating");
				if (vMap_itor != ( ((((*rsMap_itor).second)).resourceData)).end())
					vrtNatPenetrating = vMap_itor->second;
				vMap_itor = ( ((((*rsMap_itor).second)).resourceData)).find("srcIP");
				if (vMap_itor != ( (( ((*rsMap_itor).second)).resourceData)).end())
					vrtSrcIP = vMap_itor->second;
				vMap_itor = ( ((((*rsMap_itor).second)).resourceData)).find("srcPort");
				if (vMap_itor != ( (( ((*rsMap_itor).second)).resourceData)).end())
					vrtSrcPort = vMap_itor->second;
				vMap_itor = ( ((((*rsMap_itor).second)).resourceData)).find("pokeholeSession");
				if (vMap_itor != ( (( ((*rsMap_itor).second)).resourceData)).end())
					vrtPokeholeSession = vMap_itor->second;
			}
			
			std::string srcIP, pokeholeSession;
			Ice::Int srcPort = -1, natPenetrating = -1;
			if (vrtNatPenetrating.type == TianShanIce::vtInts && vrtNatPenetrating.ints.size() > 0)
				natPenetrating = vrtNatPenetrating.ints[0];
			if (vrtSrcIP.type == TianShanIce::vtStrings && vrtSrcIP.strs.size() > 0)
				srcIP = vrtSrcIP.strs[0];
			if (vrtSrcPort.type == TianShanIce::vtInts && vrtSrcPort.ints.size() > 0)
				srcPort = vrtSrcPort.ints[0];
			if (vrtPokeholeSession.type == TianShanIce::vtStrings && vrtPokeholeSession.strs.size() > 0)
				pokeholeSession = vrtPokeholeSession.strs[0];
			int nPos = -1;
			if (String::hasChar(trspStr, ';', nPos))
			{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;nat_penetrating=%d;source=%s;server_port=%d;pokehole_session=%s"
				, String::getLeftStr(trspStr, ";", true).c_str(), natPenetrating, srcIP.c_str(), srcPort, pokeholeSession.c_str());
			}
			else 
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;nat_penetrating=%d;source=%s;server_port=%d;pokehole_session=%s"
				, trspStr.c_str(), natPenetrating, srcIP.c_str(), srcPort, pokeholeSession.c_str());
			}
		}

		// Set "TianShan-Transport" field
//		_pResponse->setHeader(HeaderTianShanTransport, _szBuf);
		
		// 1. Mode is QAM. Just copy the request "Transport" as response "Transport" head.
		// 2. Mode is IP. There is other work to be done. as following
		//		if there is no "client_port" in the request head of "Transport", we should add "client_port=<value>" to response "Transport" head
		//	the <value> is "peerPort" gained from current connection.
		//		if there is no "destination" in the request head of "Transport", we should add "destination=<value>" to response "Transport" head
		//	the <value> is "peerIP" gained from current connection.
		//		if the "destination" is already specified in the request "Transport" head and "nat_penetrating=1" is also specified in "TianShan-Transport".
		//		according to document NATPenetratingDesignSpec.doc, we should use the "peerIP" gained from current connection rather than the specified value
		//		in the request. so at this situation we have to replace the value in the request.
		if (false == bQam)
		{
			// if there is no "destination" or "client_port" in request "Transport" field, add them.
			{
			::std::string::size_type tmpSize = trspStr.size();
			if (tmpSize > 0 && trspStr[tmpSize - 1] == ';')
				trspStr.resize(tmpSize - 1); // remove the last ';' char.
			}
			const char* pDestination = NULL;
			const char* pTransport = trspStr.c_str();
			pDestination = strstr(pTransport, Destination "=");
			if (NULL == pDestination) // if no "destination" specified in the request, use the "peerIP" gained from current connection.
				trspStr += ";" Destination "=" + destIP;
			else if (true == bNatOpen) // replace the destination ip with the current connection's "peerIP"
			{
				// "posEqualAfterDest" specifies the equal char position after the "destination"
				// Transport: MP2T/AVP/UDP;unicast;destination=10.15.10.34;client_port=1234
				::std::string::size_type posEqualAfterDest = (::std::string::size_type)(pDestination - pTransport) + strlen(Destination);
				std::string leftStr, rightStrTmp, rightStr;
				leftStr = String::nLeftStr(trspStr, posEqualAfterDest); // gain the first n chars of a string
				rightStrTmp = String::rightStr(trspStr, posEqualAfterDest); // gain the string after the specified position
				int nPos;
				if (String::hasChar(rightStrTmp, ';', nPos))
				{
					rightStr = String::rightStr(rightStrTmp, nPos - 1); // gain the string after the specified ip address including char ';'
				}
				trspStr = leftStr + "=" + destIP + rightStr;
			}
			if (NULL == strstr(trspStr.c_str(), ClientPort)) // if no "client_port" specified in the request, use the "peerPort" gained from current connection.
				trspStr += ";" ClientPort "=" + destPort;
		}

		_pResponse->setHeader(HeaderTransport, (char*) trspStr.c_str());

//		snprintf(_szBuf, sizeof(_szBuf) - 1, "%d", _tsConfig._rtspSession._timeout);
//		_pResponse->setHeader(HeaderTianShanClientTimeout, _szBuf);

		int statusCode;
		renewSession(statusCode);

		if (false == saveSessionCtx(pSessionContext))
		{
			composeErrorResponse();
			return false;
		}

		composeRightResponse();

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
	
	SetupResponse::SetupResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}
	
	SetupResponse::~SetupResponse()
	{
	}
	
	bool SetupResponse::process()
	{
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
			if(!getRequestHeader(HeaderOnDemandSessionId).empty())
			{
				_pResponse->setHeader(HeaderOnDemandSessionId, (char *)getRequestHeader(HeaderOnDemandSessionId).c_str());
			}

			if(!getRequestHeader(HeaderClientSessionId).empty())
			{
				_pResponse->setHeader(HeaderClientSessionId, (char *)getRequestHeader(HeaderClientSessionId).c_str());
			}

			if(!getRequestHeader(HeaderContentType).empty())
			{
				_pResponse->setHeader(HeaderContentType, (char *)getRequestHeader(HeaderContentType).c_str());
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
		
		return true;
	}

}

