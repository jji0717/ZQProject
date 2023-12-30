
#include "SetupRequest.h"
#include <TimeUtil.h>
#include "TsConfig.h"
#include "urlstr.h"
#include <string>
#include "auth5i.h"

#define STATUS_CODE_WITH_NGOD_EXT

#ifndef ZQ_OS_MSWIN
#include <arpa/inet.h>
#endif
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
	
	std::string FixupSetup::findServiceGroupByIP(std::string ip)
	{
		std::string group = "";

		//convert ip to binary string
		uint64 destIP = ntohl(inet_addr(ip.c_str()));
		uint64 subnet, mask;

		std::vector<IpSubnet::IpSubnetHolder> ipSubnets = _tsConfig._ngods1._sessionGroups.ipSubnets;
		for (std::vector<IpSubnet::IpSubnetHolder>::iterator it = ipSubnets.begin(); it != ipSubnets.end(); it++)
		{
			mask   = ntohl(inet_addr(it->mask.c_str()));
			subnet = ntohl(inet_addr(it->ip.c_str())) & mask;

			if (subnet == (destIP & mask))
			{			
				group = calculateGroup(it->group, subnet);
			}
		}

		return group;
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
		for(std::vector<SessionGroupHolder>::iterator iter = _tsConfig._ngods1._sessionGroups.sessionGroups.begin();
			iter != _tsConfig._ngods1._sessionGroups.sessionGroups.end(); iter++ )
		{
			boost::regex qamRegex(iter->expression);
			boost::cmatch result;
			if(!boost::regex_match(qam.c_str(),result, qamRegex))
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
			std::string uriPath = "";
			STRINGVECTOR strs;
			String::splitStr(strUrl, ";&?", strs);
			if(strs.size() > 0)
			{
				size_t pos = strs[0].rfind('/');
				if(pos != strs[0].npos)
					uriPath = strs[0].substr(pos+1);
			}
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
					if(uriPath.empty())
					{
						if(temp[temp.size()-1] != '/')
							temp += "/";
						strUrl = temp + strAppID + "?";
					}
					else
					{
						strUrl = temp + "?";
					}
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
			else if (transport.find("MP2T/AVP/C2") != std::string::npos && serviceGroup.empty())
			{
				//step 1. find ip from transport where key="destination"
				std::string ip = "";
				String::splitStr(transport, ";", strs);
				for (STRINGVECTOR_ITOR strsItor = strs.begin(); strsItor != strs.end(); strsItor ++)
				{
					std::string keystr, valstr;
					keystr = String::getLeftStr(*strsItor, "=", true);
					valstr = String::getRightStr(*strsItor, "=", true);
					if ("destination" == keystr)
					{
						ip = valstr;
						break;
					}
				}

				//step 2. if ip is empty, find it from request
				if (ip.empty())
				{
					char addr[64];
					addr[63] = '\0';
					IClientRequest::RemoteInfo info;
					info.size = sizeof(IClientRequest::RemoteInfo);
					info.addrlen = sizeof(addr);
					info.ipaddr = addr;

					_pRequest->getRemoteInfo(info);
					ip = info.ipaddr;
				}
				
				//use IPsubnet generate service group ID
				serviceGroup = findServiceGroupByIP(ip);
				if (!serviceGroup.empty())
				{
					bFound = true;
				}
			}

			if(!serviceGroup.empty())// NGOD spec
			{
				_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char *)serviceGroup.c_str());
			}
			else if (_tsConfig._ngods1._sessionGroups.defaultServiceGroup >= 0)
			{
				serviceGroup = _tsConfig._ngods1._sessionGroups.defaultServiceGroup;
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
		if( p && p[0] )
		{
			sscanf( p , "%lld" , &mTimestampStart );
		}
	}
	
	HandleSetup::~HandleSetup()
	{
		if( !mbOpSuccess && _srvrSessPrx != NULL )
		{
			try
			{
				//destroy srm session
				if( mErrMsg.length() <= 0 )
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

		if( mTimestampStart == 0 )
		{
			return true;
		}
		
		int64 cur = ZQ::common::now();
		
		//TODO: should be a configuration 
		unsigned int delta =  _tsConfig._rtspSession._setupMsgTimeoutInterval ;
		delta = MAX( delta, 500 );

		if( cur > ( mTimestampStart + delta ) )
		{			
			SSMLOG(ErrorLevel,HandlerFmt(HandleSetup,"timed out in processing setup command, %u]") ,delta);
			setErrMsg("212012 timeout due to timed out in processing setup command, %u]",delta);			

			//response with error code 503 
			composeResponse( 503 ,"timed out in message processing" );
			return false;
		}
		return true;
	}
	
	void HandleSetup::setErrMsg( const char* fmt , ... )
	{
		//notice error message should never exceed 2048 byte
		char msg[2048];
		va_list args;

		va_start(args, fmt);
		int nCount = vsnprintf(msg, 2047, fmt, args);
		va_end(args);
		if( nCount == -1 )
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

		std::string userAgent = getRequestHeader(HeaderUserAgent);
		MAPSET(TianShanIce::Properties, _eventParams, "ua", userAgent);

		_cltSessCtx.rangePrefix = "npt";
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), NGODFormat))
		{
			_cltSessCtx.requestType = SPEC_NGOD_S1; // NGOD spec

			std::string clientSessID = getRequestHeader(HeaderClientSessionId);
			if (!clientSessID.empty())
			{
				MAPSET(::TianShanIce::Properties, _cltSessCtx.props, HeaderClientSessionId, clientSessID);
			}
		}
		else if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
			_cltSessCtx.requestType = SPEC_NGOD_SeaChange; // SeaChange
		else 
		{
			_bNeedModifyResponse = false;
			_cltSessCtx.requestType = SPEC_NGOD_TianShan; // TianShan
		}

		// because for SETUP, no key such as sessionId to connect log between RTSPService and this Plugin
		// take the client connection instead
		std::string strConn;
		{
			char buff[100];
			IClientRequest::RemoteInfo info;
			info.size = sizeof(IClientRequest::RemoteInfo);
			info.addrlen = sizeof(buff);
			info.ipaddr = buff;
			if (_pRequest->getRemoteInfo(info))
			{
				strConn = info.ipaddr;
				buff[0] = '/'; buff[sizeof(buff)-1] = '\0';
				snprintf(buff+1, sizeof(buff)-3, "%d", info.port);
				strConn += buff;
			}
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
			snprintf(_szBuf, sizeof(_szBuf) - 1, "session already setuped");
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 455;
			composeResponse(_statusCode);

			//_ok_statusline = ResponseMethodNotValidInThisState;
			//composeRightResponse();
			return true; // notice here, must return true;
		}
		
		_statusCode = 503; // start _statusCode with 503 about initialization

		// create rtsp client session
		SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "createRtspSession() creating session for conn[%s]"), strConn.c_str());
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
		SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "createRtspSession() RTSP session created for conn[%s]"), strConn.c_str());

		if( !checkTimeStampAndConnection() )
			return false;
			
		std::string svcGroup = getRequestHeader(HeaderTianShanServiceGroup);
		String::trimAll(svcGroup);

		// create weiwoo session
		::TianShanIce::SRM::Resource sessRC;
		sessRC.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
		sessRC.status = TianShanIce::SRM::rsRequested;

		::TianShanIce::Variant vStrs;		
		vStrs.bRange = false;
		vStrs.type = TianShanIce::vtStrings;
		vStrs.strs.clear();
		vStrs.strs.push_back(_cltSessCtx.url);
		MAPSET(::TianShanIce::ValueMap, sessRC.resourceData, "uri", vStrs);

		vStrs.strs.clear();
		vStrs.strs.push_back(svcGroup);
		MAPSET(::TianShanIce::ValueMap, sessRC.resourceData, "sessiongroup", vStrs);

		// MAPSET(TianShanIce::Properties, _eventParams, "asset", assetID);
		MAPSET(TianShanIce::Properties, _eventParams, "svcg",  svcGroup);

		std::string strSM;
		try 
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do weiwooService.createSession"));

			//bug#19830, for multiple Weiwoo instances
			bool bFind = false;
			int nSvcGroup = atoi(svcGroup.c_str());
			std::vector<NodeGroup::NodeGroupHolder>::iterator it = _tsConfig._sessionManager._ngodGroups.begin();
			for (; it != _tsConfig._sessionManager._ngodGroups.end(); it++)
			{
				if ((nSvcGroup >= it->_rangeStart) && (nSvcGroup <= it->_rangeEnd))
				{
					bFind = true;
					strSM = it->_sm;
					_sessMgrPrx = TianShanIce::SRM::SessionManagerPrx::checkedCast(_env._pCommunicator->stringToProxy(it->_sm));				
					break;
				}
			}

			if (!bFind)
			{
				strSM = _tsConfig._sessionManager._endpoint;
				_sessMgrPrx = TianShanIce::SRM::SessionManagerPrx::checkedCast(_env._pCommunicator->stringToProxy(_tsConfig._sessionManager._endpoint));
			}

			_srvrSessPrx = _sessMgrPrx->createSession(sessRC);
			if( _env._iceOverrideTimeout > 0 )
				_srvrSessPrx = TianShanIce::SRM::SessionPrx::uncheckedCast(_srvrSessPrx->ice_timeout( _env._iceOverrideTimeout ));

			_cltSessCtx.srvrSessPrxID = _env._pCommunicator->proxyToString(_srvrSessPrx);
			_cltSessCtx.srvrSessID = _srvrSessPrx->getId();
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "weiwoo session(%s) created , and proxy is [%s]"),
				_cltSessCtx.srvrSessID.c_str() , _cltSessCtx.srvrSessPrxID.c_str() );
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by weiwooService(%s).createSession"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), strSM.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse( _statusCode );
			return false;
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught by weiwooService(%s).createSession", 
				ex.ice_name().c_str(), strSM.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);			
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch(  const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught by weiwooService(%s).createSession", 
				ex.ice_name().c_str(), strSM.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0301] caught by weiwooService(%s).createSession", 
				ex.ice_name().c_str(), strSM.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}
		
		if( !checkTimeStampAndConnection() )
			return false;

		// DO: add application-id to private data which gained from url's path
		ZQ::common::URLStr urlStr(_cltSessCtx.url.c_str(),true);
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

		// DO: add node-group-id to private data
		TianShanIce::Variant vrtNodeGroup;
		vrtNodeGroup.bRange = false;
		vrtNodeGroup.type = TianShanIce::vtStrings;
		vrtNodeGroup.strs.clear();
		vrtNodeGroup.strs.push_back(svcGroup);
		addPrivateData(NodeGroupID, vrtNodeGroup);

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
		bool _bNatOpen = false;
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
					_bNatOpen = true;
			}
		}

		// Parse "Transport" head
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
					addPrivateData(keyStr, vrt);
				}
				if (0 == stricmp(keyStr.c_str(), Destination))
					_destIP = valStr;
				if (0 == stricmp(keyStr.c_str(), ClientPort))
					_destPort = valStr;
				if (0 == stricmp(keyStr.c_str(), ClientMac))
					_destMac = valStr;
				if (0 == stricmp(keyStr.c_str(), BandWidth))
				{
					_lBandWidth = atol(valStr.c_str());
				}
			}
		}

		if(!importTransportToAsResource())
			return false;	

		// flush the private datas to weiwoo session
		flushPrivateData();

		if( !checkTimeStampAndConnection() )
			return false;

		_statusCode = 500; // during process, set the default _statusCode to 500
		try
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do session(%s).provision"), _cltSessCtx.srvrSessID.c_str());
			//			provison() throw exception list: InvalidResource, InvalidParameter, InvalidStateOfArt, NotSupported, ServerError;
			Ice::Long timeUsed = ZQTianShan::now();
			_srvrSessPrx->provision();
			timeUsed = ZQTianShan::now() - timeUsed;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "session(%s).provision successfully, used [%lld]ms"), _cltSessCtx.srvrSessID.c_str(), timeUsed);
		}
		catch (TianShanIce::SRM::InvalidResource& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).provision"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//_error_statusline = ResponseNotFound; // return "404 Not Found" error
			//composeErrorResponse();
			_statusCode = 404;
			composeResponse(_statusCode);
			return false;
		}
		catch (TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).provision"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			//composeErrorResponse();
			_statusCode = 451;
			composeResponse(_statusCode);
			return false;
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).provision"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0303] caught by session(%s).provision",
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			setErrMsg("212010 provision failed due to [%s]",ex.ice_name().c_str() );
			return false;
		}
		catch( const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0303] caught by session(%s).provision",
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			setErrMsg("212010 provision failed due to [%s]",ex.ice_name().c_str() );
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0303] caught by session(%s).provision",
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);

			setErrMsg("212010 provision failed due to [%s]",ex.ice_name().c_str() );

			return false;
		}

		if ( !checkTimeStampAndConnection() )
			return false;

		try
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleSetup, "do session(%s).serve"), _cltSessCtx.srvrSessID.c_str());
//			serve() throw exception list: InvalidStateOfArt, InvalidResource, OutOfResource, InvalidParameter, NotSupported, ServerError;
			Ice::Long timeUsed = ZQTianShan::now();
			_srvrSessPrx->serve();
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "session(%s).serve successfully, used [%lld]ms"), _cltSessCtx.srvrSessID.c_str(), ZQTianShan::now()-timeUsed);
		}
		catch (TianShanIce::SRM::OutOfResource& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).serve"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//_error_statusline = ResponseNotEnoughBandwidth; // "RTSP/1.0 453 Not Enough Bandwidth"
			//composeErrorResponse();
			_statusCode = 453;
			composeResponse(_statusCode);
			return false;
		}
		catch (TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).serve"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			//composeErrorResponse();
			_statusCode = 451;
			composeResponse(_statusCode);
			return false;
		}
		catch (TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf)-1, "ServerError[%s:%04d] %s caught by session(%s).serve",
				ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;

			if ((SPEC_NGOD_S1 == _cltSessCtx.requestType) && (1172 == ex.errorCode || 1180 == ex.errorCode))
				_statusCode = 676;

#ifdef STATUS_CODE_WITH_NGOD_EXT
			int d = -1;
			const char *pNGODErr = strstr(ex.message.c_str(), "doAction");
			if (pNGODErr)
			{
				if (sscanf(pNGODErr, "doAction(%[^)]) failed; error[%d", _szBuf, &d)>1 && d>770 && d<799)
					_statusCode = d;
			}
#endif // STATUS_CODE_WITH_NGOD_EXT

			composeResponse(_statusCode);
			return false;
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).serve"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0304] caught by session(%s).serve", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			setErrMsg("212020 serve failed due to [%s]",ex.ice_name().c_str() );
			return false;
		}
		catch( const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0304] caught by session(%s).serve", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 503;
			composeResponse(_statusCode);
			setErrMsg("212020 serve failed due to [%s]",ex.ice_name().c_str() );
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0304] caught by session(%s).serve", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			_statusCode = 500;
			composeResponse(_statusCode);
			setErrMsg("212020 serve failed due to [%s]",ex.ice_name().c_str() );
			return false;
		}
	
		if (!prepareResponse())
		{
			return false;
		}

		renewSession();

		SessionContextImplPtr pSessionContext = convertSessionData(_cltSessCtx);

		if (false == saveSessionCtx(pSessionContext))
		{
			_statusCode = 500; // if renewSession() encounter timeout exception
			composeResponse(_statusCode,"failoed to add record into database");
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
	bool HandleSetup::importTransportToAsResource()
	{
		// Parse "Transport" head
		std::string peerIP, peerPort; // current connection's peerIP and peerPort.
		std::string trspStr = getRequestHeader(HeaderTransport);
		String::removeChar(trspStr, ' ');

		if (NULL != strstr(trspStr.c_str(), "/DVBC/") || NULL != strstr(trspStr.c_str(), "/QAM"))
		{// QAM mode(MP2T/DVBC/QAM) 
			_bQam = true;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is QAM mode"));
		}
		//add "|| NULL != strstr(trspStr.c_str(), HeaderVLCTransport)" by lxm at 2008.12.22 to support VLC
		else if (NULL != strstr(trspStr.c_str(), "/UDP") || NULL != strstr(trspStr.c_str(), HeaderVLCTransport))
		{// IP mode(MP2T/AVP/UDP)
			_bIP = true;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is IP mode"));
		}
		else if (NULL != strstr(trspStr.c_str(), "/C2") && _tsConfig._c2stream.enable != 0)
		{
			// C2 mode(MP2T/AVP/C2)
			_bC2 = true;
			SSMLOG(InfoLevel, HandlerFmt(HandleSetup, "The request is C2 mode"));
		}
		else 
		{// 461 Unsupported Transport
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s not supported", trspStr.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
			// _error_statusline = ResponseUnsupportedTransport;
			//composeErrorResponse();
			_statusCode = 461;
			composeResponse(_statusCode);
			return false;
		}

		if (_bIP || _bC2)
		{
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
			if (true == _destPort.empty())
				_destPort = peerPort;

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
			vrtDestPort.ints.push_back(atoi(_destPort.c_str()));

			vrtDestMac.bRange = false;
			vrtDestMac.type = TianShanIce::vtStrings;
			vrtDestMac.strs.clear();

			if ( !_destPort.empty() )
			{
				rscStreamDest["destPort"] = vrtDestPort;
			}

			if ( !_destMac.empty() )
			{
				vrtDestMac.strs.push_back(_destMac);
				rscStreamDest["destMac"] = vrtDestMac;			
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
				rscStreamDest["destIP"] = vrtDestIP;

			try
			{
				_srvrSessPrx->addResource(TianShanIce::SRM::rtEthernetInterface, TianShanIce::SRM::raMandatoryNonNegotiable, rscStreamDest);
			}
			catch (TianShanIce::BaseException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).addResource"
					, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}
			catch( const Ice::TimeoutException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0302] caught by session(%s).addResource", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				_statusCode = 503;
				composeResponse(_statusCode);
				return false;
			}
			catch( const Ice::SocketException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0302] caught by session(%s).addResource", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				_statusCode = 503;
				composeResponse(_statusCode);
				return false;
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0302] caught by session(%s).addResource", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}
		}

		if( !checkTimeStampAndConnection() )
			return false;

		if (_lBandWidth > 0)
		{
			TianShanIce::ValueMap vmpBandWidth;
			TianShanIce::Variant vrtBandWidth;
			vrtBandWidth.type = TianShanIce::vtLongs;
			vrtBandWidth.bRange = false;
			vrtBandWidth.lints.clear();
			vrtBandWidth.lints.push_back(_lBandWidth);
			vmpBandWidth.clear();
			vmpBandWidth["bandwidth"] = vrtBandWidth;
			try
			{
				_srvrSessPrx->addResource(TianShanIce::SRM::rtTsDownstreamBandwidth, TianShanIce::SRM::raMandatoryNonNegotiable, vmpBandWidth);
			}
			catch (TianShanIce::BaseException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).addResource"
					, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}
			catch( const Ice::TimeoutException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0308] caught by session(%s).addResource", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				_statusCode = 503;
				composeResponse(_statusCode);
				return false;
			}
			catch( const Ice::SocketException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0308] caught by session(%s).addResource", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				_statusCode = 503;
				composeResponse(_statusCode);
				return false;
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0308] caught by session(%s).addResource", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}
		}

		return true;
	}

	bool HandleSetup::prepareResponse()
	{
		std::string trspStr = getRequestHeader(HeaderTransport);
        std::string trspIP;
        std::string trspServerPort;
		String::removeChar(trspStr, ' ');

		if( !checkTimeStampAndConnection() )
			return false;

		try
		{
			_streamPrx = _srvrSessPrx->getStream();
			if( _env._iceOverrideTimeout > 0 )
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
				String::splitStr(trspStr, ";", strs);
				if(strs.size() >= 2)
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;%s;destination=%lld.%d;qam-mode=%d;modulation=qam%d", strs[0].c_str(), strs[1].c_str(), ((int64)nFrequency)*1000, nProgramNumber, nQamMode, nQamMode);
				else
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;destination=%lld.%d;qam-mode=%d;modulation=qam%d", String::getLeftStr(trspStr, ";", true).c_str(), ((int64)nFrequency)*1000, nProgramNumber, nQamMode, nQamMode);
				trspStr = _szBuf;
			}
			else
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;program-number=%d;frequency=%d;qam-mode=%d", String::getLeftStr(trspStr, ";", true).c_str(), nProgramNumber, nFrequency, nQamMode);
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
			if (String::hasChar(trspStr, ';', nPos))
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;nat_penetrating=%d;source=%s;server_port=%d;pokehole_session=%s", 
					String::getLeftStr(trspStr, ";", true).c_str(), natPenetrating, srcIP.c_str(), srcPort, pokeholeSession.c_str());
			}
			else 
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s;nat_penetrating=%d;source=%s;server_port=%d;pokehole_session=%s",
					trspStr.c_str(), natPenetrating, srcIP.c_str(), srcPort, pokeholeSession.c_str());
			}

			if (_bC2)
			{
				_pResponse->setHeader(HeaderContentType, "text/xml");
				std::string tsVersion = getRequestHeader(HeaderTianShanVersion);
				_pResponse->setHeader(HeaderTianShanVersion, (char *)tsVersion.c_str());

				std::string rtspSessionId, weiwooSessionId, clientSessionId;
				try
				{
					rtspSessionId = _session;
					weiwooSessionId = _srvrSessPrx->getId();
					clientSessionId = getRequestHeader(HeaderClientSessionId);
				}
				catch(const TianShanIce::BaseException& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getId()",
						ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
					SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
					_statusCode = 500;
					composeResponse(_statusCode);
					return false;
				}
				catch( const Ice::SocketException& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getId", 
						ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
					SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
					_statusCode = 503;
					composeResponse(_statusCode);
					return false;
				}
				catch( const Ice::TimeoutException& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getId", 
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

				std::string txnId = rtspSessionId + ":" + weiwooSessionId;

				int64 expire = (ZQ::common::TimeUtil::now() + _tsConfig._c2stream.sessionTTL +999) /1000;
				expire *= 1000;
				char buf[128];
				ZQ::common::TimeUtil::TimeToUTC(expire, buf, sizeof(buf)-2, true);
				std::string expiration = buf;

				vector<std::string> xmlStrs; //present each item as an XML line, and save them in xmlStrs

				Authen5i auth5i(*s1log);
				std::string keyfilePath = _tsConfig._c2stream.keyfile;

				if (!auth5i.loadKeyFile(keyfilePath.c_str()))
				{
					SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "failed to load keyfile[%s]"), keyfilePath.c_str());
					_statusCode = 610;
					composeResponse(_statusCode);
					return false;
				}

				std::string plXmlStr;
				if (_tsConfig._response._exportPlaylist != 0)
                {
					::TianShanIce::Streamer::PlaylistPrx playlistPrx = ::TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_streamPrx);
					playlistPrx = ::TianShanIce::Streamer::PlaylistPrx::uncheckedCast(playlistPrx->ice_collocationOptimized(false));

					//get "ADType"
					::TianShanIce::Properties adTypeMap;
					::TianShanIce::Application::PlaylistInfo plInfo;
					try
					{
						plInfo = _purchasePrx->getPlaylistInfo();
					}
					catch(const TianShanIce::BaseException& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getPlaylistInfo()",
							ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 500;
						composeResponse(_statusCode);
						return false;
					}
					catch( const Ice::SocketException& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getPlaylistInfo", 
							ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 503;
						composeResponse(_statusCode);
						return false;
					}
					catch( const Ice::TimeoutException& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getPlaylistInfo", 
							ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 503;
						composeResponse(_statusCode);
						return false;
					}
					catch(Ice::Exception& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getPlaylistInfo", 
							ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 500;
						composeResponse(_statusCode);
						return false;
					}
					
					for (::TianShanIce::Application::PlaylistInfo::const_iterator it = plInfo.begin(); it != plInfo.end(); it++)
					{
						std::string strAd;
						ZQTianShan::Util::getValueMapDataWithDefault(it->privateData,"isAD","",strAd);
						adTypeMap.insert(::TianShanIce::Properties::value_type(it->contentName, strAd));
					}

					//get other elements
					::TianShanIce::Streamer::PlaylistItemSetupInfoS items;
					try
					{
						items = playlistPrx->getPlaylistItems();
					}
					catch(const TianShanIce::BaseException& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by session(%s).getPlaylistItems()",
							ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 500;
						composeResponse(_statusCode);
						return false;
					}
					catch( const Ice::SocketException& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getPlaylistItems", 
							ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 503;
						composeResponse(_statusCode);
						return false;
					}
					catch( const Ice::TimeoutException& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getPlaylistItems", 
							ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 503;
						composeResponse(_statusCode);
						return false;
					}
					catch(Ice::Exception& ex)
					{
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[SetupHandle:0307] caught by session(%s).getPlaylistItems", 
							ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
						SSMLOG(ErrorLevel, HandlerFmt(HandleSetup, "%s"), _szBuf);
						_statusCode = 500;
						composeResponse(_statusCode);
						return false;
					}
					
					for (::TianShanIce::Streamer::PlaylistItemSetupInfoS::const_iterator itor = items.begin(); itor != items.end(); itor++)
					{
						::TianShanIce::Streamer::PlaylistItemSetupInfo item = *itor;
						std::string contentName = item.contentName;

						// 1. read the PAID, PID, cueIn, cueOut of each item
						::Ice::Long cueIn  = item.inTimeOffset;
						char cueInBuf[20];
						sprintf(cueInBuf, "%f", (float)cueIn);
						std::string cueInStr = cueInBuf;

						::Ice::Long cueOut = item.outTimeOffset;
						char cueOutBuf[20];
						sprintf(cueOutBuf, "%f", (float)cueOut);
						std::string cueOutStr = cueOutBuf;

						::Ice::Long nDuration = item.outTimeOffset - item.inTimeOffset;
						char durationBuf[20];
						sprintf(durationBuf, "%f", (float)nDuration);
						std::string duration = durationBuf;

						std::string adType = adTypeMap[contentName];

						std::string paid;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData,"AssetID","", paid);

						std::string pid;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "ProviderID", "", pid);

						//std::string subType       = item.privateData["subType"].strs[0];
						std::string playTime;
						int nPlayTime;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "playTime", 0 ,nPlayTime);
						std::ostringstream playTimeOss;
						playTimeOss<<nPlayTime;
						playTime = playTimeOss.str();

						std::string muxBitrate;
						int nMuxBitrate;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "muxBitrate", 0, nMuxBitrate);
						std::ostringstream muxBitrateOss;
						muxBitrateOss<<nMuxBitrate;
						muxBitrate = muxBitrateOss.str();

						std::string extName;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "extName", "", extName);

						std::string startOffset;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "startOffset", "", startOffset);

						std::string endOffset;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "endOffset", "", endOffset);

						std::string recording;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "recording", "", recording);


						//2. use PAID, PID, cueIn, cueOut generate signature
						std::string signature = auth5i.signC2PlaylistItem(paid, pid, txnId, clientSessionId, expiration);

						//3. present each item as an XML line, such as:
						//<Item assetId="14050418184200036541" providerId="jetsen_ID"
						//  cueIn="300.00" cueOut="0.00" duration="1500.00"
						//  signature="2D7DDBBBA05B8F3498077BFC216258AB" ADType="1" playTime="1799.105"
						//  muxBitrate="3131352" extName="0X0000" startOffset="0" endOffset="704203879"
						//  recording="0"/>

						std::ostringstream itemXmlLine;
						itemXmlLine<<"<Item assetId=\""<<paid<<"\" providerId=\""<<pid<<"\" \r\n"                       \
							<<"cueIn=\""<<cueInStr<<"\" cueOut=\""<<cueOutStr<<"\" duration=\""<<duration<<"\" \r\n"    \
							<<"signature=\""<<signature<<"\" ADType=\""<<adType<<"\" playTime=\""<<playTime<<"\" \r\n"  \
							<<"muxBitrate=\""<<muxBitrate<<"\" extName=\""<<extName<<"\" startOffset=\""<<startOffset   \
							<<"\" endOffset=\""<<endOffset<<"\" \r\n"                                                   \
							<<"recording=\""<<recording<<"\" />";

						xmlStrs.push_back(itemXmlLine.str());
					}

					/*4. Format the Playlist in XML as the response body of SETUP, such as:
                    <Playlist expiration="20140512T232459Z" txnId="928113670:
                        5hwCSpT8IFIFCiv8mNSrFW" clientSessionId="02606e700db612345" >
                        <Item assetId="14050418184200036541" providerId="jetsen_ID"
                        cueIn="300.00" cueOut="0.00" duration="1500.00"
                        signature="2D7DDBBBA05B8F3498077BFC216258AB" ADType="1" playTime="1799.105"
                        muxBitrate="3131352" extName="0X0000" startOffset="0" endOffset="704203879"
                        recording="0"/>
                    </Playlist>*/

					plXmlStr = "<Playlist expiration=\"" + expiration + "\" txnId=\"" + txnId + "\" \r\n" \
						+ "clientSessionId=\"" + clientSessionId + "\" >";
					for (vector<std::string>::const_iterator itor = xmlStrs.begin(); itor != xmlStrs.end(); itor++)
					{
						plXmlStr += "\r\n" + *itor;
					}
					plXmlStr += "\r\n</Playlist>";
                }
                else
                {
                    ::TianShanIce::Application::PlaylistInfo plInfo = _purchasePrx->getPlaylistInfo();

                    for (::TianShanIce::Application::PlaylistInfo::const_iterator itor = plInfo.begin(); itor != plInfo.end(); itor++)
                    {
                        ::TianShanIce::Streamer::PlaylistItemSetupInfo item = *itor;

                        // 1. read the PAID, PID, cueIn, cueOut of each item
                        ::Ice::Long cueIn  = item.inTimeOffset;
                        char cueInBuf[20];
                        sprintf(cueInBuf, "%f", (float)cueIn);
                        std::string cueInStr = cueInBuf;

                        ::Ice::Long cueOut = item.outTimeOffset;
                        char cueOutBuf[20];
                        sprintf(cueOutBuf, "%f", (float)cueOut);
                        std::string cueOutStr = cueOutBuf;

                        std::string paid;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "PAID", "", paid);

                        std::string pid;
						ZQTianShan::Util::getValueMapDataWithDefault(item.privateData, "PID", "", pid);

                        //2. use PAID, PID, cueIn, cueOut generate signature
                        std::string signature = auth5i.signC2PlaylistItem(paid, pid, txnId, clientSessionId, expiration);

                        //3. present each item as an XML line, such as:
                        //  <Item assetId="EGEZ0595967209325440" providerId="so.xor-media.com"
                        //  cueIn="0.00" cueOut="0.00" signature="2D7DDBBBA05B8F3498077BFC216258AB" />
                        
                        std::string itemXmlLineStr = "<Item assetId=\"" + paid + "\" providerId=\"" + pid + "\" \r\n" \
                            + "cueIn=\"" + cueInStr + "\" cueOut=\"" + cueOutStr + "\" signature=\"" + signature + "\" />";
                        xmlStrs.push_back(itemXmlLineStr);
                    }

                    //4. Format the Playlist in XML as the response body of SETUP, such as:
                    //  <Playlist expiration="20140512T232459Z" txnId="928113670:
                    //      5hwCSpT8IFIFCiv8mNSrFW" clientSessionId="02606e700db612345" >
                    //      <Item assetId="EGEZ0595967209325440" providerId="so.xor-media.com"
                    //      cueIn="0.00" cueOut="0.00" signature="2D7DDBBBA05B8F3498077BFC216258AB" />
                    //      <Item assetId="EGEZ0595967209325488" providerId="so.xor-media.com"
                    //      cueIn="0.00" cueOut="0.00" signature="6D7DDBBBF05B8F559AA744FC216338AB" /> 
                    //  </Playlist>

                    plXmlStr = "<Playlist expiration=\"" + expiration + "\" txnId=\"" + txnId + "\" \r\n" \
                        + "clientSessionId=\"" + clientSessionId + "\" >";
                    for (vector<std::string>::const_iterator itor = xmlStrs.begin(); itor != xmlStrs.end(); itor++)
                    {
                        plXmlStr += "\r\n" + *itor;
                    }
                    plXmlStr += "\r\n</Playlist>";
                }

				string content = "\r\n" + plXmlStr;
				_pResponse->printf_postheader(content.c_str());

				//_pResponse->setHeader(HeaderContentType, "text/xml");
				std::string source;
				::TianShanIce::Variant sessionInterfaceVar = rsMap[TianShanIce::SRM::rtStreamer].resourceData["sessionInterface"];
				if (!sessionInterfaceVar.strs.empty())
				{
					source = sessionInterfaceVar.strs[0];

					size_t posS = source.find_first_of(':');				// http://
					size_t posH = source.find_first_of(':', posS + 1);		// ~192.168.81.2:12000
					if (std::string::npos != posH)
					{
						trspIP = source.substr(posS + 3, posH - posS - 3);

						size_t posT = source.find_first_of('/', posH);		// ~192.168.81.2:12000/
						if (std::string::npos != posT)
						{
							trspServerPort = source.substr(posH + 1, posT - posH - 1);
						}
						else
						{
							trspServerPort = source.substr(posH + 1);
						}
					}else{
						size_t posT = source.find_first_of('/', posS + 3);		// ~192.168.81.2/
						if (std::string::npos != posT)
						{
							trspIP = source.substr(posS + 3, posT - posS - 3);
						}
						else
						{
							trspIP = source.substr(posS + 3);
						}
					}
					snprintf(_szBuf, sizeof(_szBuf) - 1, "MP2T/AVP/C2;source=%s;server_port=%s", source.c_str(), trspServerPort.c_str());
				}
				else
				{
					source = _tsConfig._c2stream.defaultC2Server;
                    trspIP = source;

                    int32 serverPort = _tsConfig._c2stream.defaultC2Port;
                    std::ostringstream oss;
                    oss<<serverPort;
					trspServerPort   = oss.str();
					snprintf(_szBuf, sizeof(_szBuf) - 1, "MP2T/AVP/C2;source=%s;server_port=%s", source.c_str(), trspServerPort.c_str());
				}
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
				::std::string::size_type tmpSize = trspStr.size();
				if (tmpSize > 0 && trspStr[tmpSize - 1] == ';')
					trspStr.resize(tmpSize - 1); // remove the last ';' char.
			}

			const char* pDestination = NULL;
			const char* pTransport = trspStr.c_str();
			pDestination = strstr(pTransport, Destination "=");
			if (NULL == pDestination) // if no "destination" specified in the request, use the "peerIP" gained from current connection.
				trspStr += ";" Destination "=" + _destIP;
			else if (true == _bNatOpen) // replace the destination ip with the current connection's "peerIP"
			{
				// "posEqualAfterDest" specifies the equal char position after the "destination"
				// Transport: MP2T/AVP/UDP;unicast;destination=10.15.10.34;client_port=1234
				::std::string::size_type posEqualAfterDest = (::std::string::size_type)(pDestination - pTransport) + strlen(Destination);
				std::string leftStr, rightStrTmp, rightStr;
				leftStr = String::nLeftStr(trspStr, posEqualAfterDest); // gain the first n chars of a string
				rightStrTmp = String::rightStr(trspStr, posEqualAfterDest); // gain the string after the specified position
				int nPos;
				if (String::hasChar(rightStrTmp, ';', nPos))
					rightStr = String::rightStr(rightStrTmp, nPos - 1); // gain the string after the specified ip address including char ';'

				trspStr = leftStr + "=" + _destIP + rightStr;
			}

			if (NULL == strstr(trspStr.c_str(), ClientPort)) // if no "client_port" specified in the request, use the "peerPort" gained from current connection.
				trspStr += ";" ClientPort "=" + _destPort;

            //ticket#16399, add "source" and "server_port" to Response Header "Transport"
            trspStr += ";" Source "=" + trspIP;
            trspStr += ";" ServerPort "=" + trspServerPort;
		}

		_pResponse->setHeader(HeaderTransport, (char*) trspStr.c_str());
		_pResponse->setHeader(HeaderRequire, getRequestHeader(HeaderRequire).c_str());

		snprintf(_szBuf, sizeof(_szBuf) - 1, "%d", _tsConfig._rtspSession._timeout);
		_pResponse->setHeader(HeaderTianShanClientTimeout, _szBuf);

		// prepare the TianShanNoticeParam
		{
			static const NamePair TsNoticeParams[] = {
				{"primaryItemNPT", SYS_PROP(primaryItemNPT)},
				{"primaryEndNPT",  SYS_PROP(primaryEndNPT)},
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
			else if (strstr(getRequestHeader(HeaderTransport).c_str(), "/C2") != NULL)
			{
				bC2 = true;
				_pResponse->setHeader(HeaderContentType, "text/xml");
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
                std::string address;
				std::string retContent;
				retContent = "v=0\r\n";
				retContent += std::string("o=- ") + scSession + " " + stampUTP + " IN IP4 " + rtspServerIP + "\r\n";
				retContent += "s=\r\n";
				retContent += "c=IN IP4 " + rtspServerIP +"\r\n";
				retContent += "t=0 0\r\n";	

                // bug 20955, for Henan, use domain instead of ip address when send setup response
                if (!_tsConfig._ngods1._streamCtrl.exportEndpoint.empty())
                {
                    address = _tsConfig._ngods1._streamCtrl.exportEndpoint;
                }else{
                    address = rtspServerIP + ":" + rtspServerPort;
                }
				retContent += std::string("a=control:rtsp://") + address + "/" + scSession + "\r\n";
				retContent += "m=video 0 " + sTransType + "\r\n";
				_pResponse->printf_postheader(retContent.c_str());
			}
		}
		
		return true;
	}

}

