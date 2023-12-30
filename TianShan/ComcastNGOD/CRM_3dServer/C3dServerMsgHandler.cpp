#include <boost/thread.hpp>
#include "C3dServerMsgHandler.h"
#include "C3dServerConfig.h"
#include "HttpEngineInterface.h"
#include "TianShanDefines.h"
#include "Guid.h"
#include "ZQResource.h"
#include <time.h>

extern ZQ::common::Config::Loader< C3dServerCfg > _3dServerConfig;

namespace CRM
{
	namespace C3dServer
	{
		C3dServerMsgHandler::C3dServerMsgHandler(CRM::C3dServer::C3dServerEnv& env): _env(env)
		{
			_isNameExist = true;
			_isProviderExist = true;
			_isStartTimeExist = true;
		}
		C3dServerMsgHandler::~C3dServerMsgHandler(void)
		{
		}
		std::string getContentId()
		{
			ZQ::common::Guid id;
			id.create();
			char bufUUID[65] = "";
			int nUUID = id.toString(bufUUID, 65);
			return std::string(bufUUID);
		}
		void C3dServerMsgHandler::onRequest(const CRG::IRequest* request, CRG::IResponse* response)
		{
			std::string strFullURL = request->uri();
			CRG::IRequest::Method method = request->method();

			ZQ::common::StringMap conditionMap;
			std::string  conditionKey = "";
			std::string	contentId = "";
			std::string  strPath = "";

			if(method == CRG::IRequest::GET)
			{
				//message type:
				//								GET /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/
				//								GET /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/Status
				//								GET /Contents?Status=Ingesting

				//case:   1./Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/Status
				//			   2./Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/
				conditionMap = request->queryArguments();
				if(conditionMap.size() == 0)
				{
					size_t pos  = strFullURL.find_last_of("/");
					conditionKey = strFullURL.substr(pos);
					if(!conditionKey.empty() && conditionKey[0] == '/')
						conditionKey = conditionKey.substr(1);
					//if contentId not end of "/" in case /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/
					if(conditionKey != "Status")
					{
						if(strFullURL[strFullURL.length()-1] != '/')
							strFullURL += "/";
						pos = strFullURL.find_last_of("/");
						conditionKey.clear();
					}
					std::string subURL = strFullURL.substr(0,pos);
					pos = subURL.find_last_of("/");
					if(pos == std::string::npos)
					{
						setReponseWithoutBody(response, STATUS_NOTFOUND, REASON_NOTFOUND);
						envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "onRequest() bad URL:%s, can not find \"/\" after path"), strFullURL.c_str());
						return;
					}
					contentId = subURL.substr(pos);
					if(!contentId.empty() && contentId[0] == '/')
						contentId = contentId.substr(1);
					strPath = subURL.substr(0,pos);
				}
				//case :   /Contents?Status=Ingesting
				else
				{
					strPath = strFullURL;
				}

			}// end of if(method == GET)

			else
			{
				if(strFullURL[strFullURL.length()-1] != '/')
					strFullURL += "/";
				size_t pos = strFullURL.find_last_of("/");
				std::string subURL = strFullURL.substr(0,pos);
				pos = subURL.find_last_of("/");
				if(pos == std::string::npos)
				{
					setReponseWithoutBody(response, STATUS_NOTFOUND, REASON_NOTFOUND);
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "onRequest() bad URL:%s, can not find \"/\" after path"), strFullURL.c_str());
					return;
				}
				contentId = subURL.substr(pos);
				if(!contentId.empty() && contentId[0] == '/')
					contentId = contentId.substr(1);
				strPath = subURL.substr(0,pos);
			}

			std::string strMsgContent;	
			request->getContent(strMsgContent);
			char buf[256]= "";
			if ("/Content" == strPath || "/Contents" == strPath)
			{	
				switch(method)
				{
				case CRG::IRequest::GET:
					if(!conditionKey.empty())
					{
						envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "method[GET/Status] contentId[%s]"), contentId.c_str());
						return getContentInfo(contentId,conditionKey,response);
					}
					else if(conditionMap.size() > 0)
					{
						envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "method[GET/?Status=Argu] contentId[%s]"), contentId.c_str());
						return getContentInfo(conditionMap,response);
					}
					else
					{
						envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "method[GET] contentId[%s]"), contentId.c_str());
						return getContentInfo(contentId,response);
					}
				case CRG::IRequest::PUT:	
					memset(buf, sizeof(buf), 0);
					sprintf(buf,  CLOGFMT(C3dServerMsgHandler, "method[PUT] content[%s]: message body is: "), contentId.c_str());
					envlog.hexDump(ZQ::common::Log::L_INFO, strMsgContent.c_str(), strMsgContent.length(), buf, true);
					return createContent(contentId, strMsgContent, response);
				case CRG::IRequest::M_DELETE:
					envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "method[DELETE] content[%s]"), contentId.c_str());
					return deleteContent(contentId, response);
				default:
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "onRequest():uri=[%s] contentId=[%s] unsupported or unknown method"),strFullURL.c_str(),contentId.c_str());
					return;
				}
			}

			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "bad request:uri=[%s] contentid=[%s] , wrong path[%s]"), strFullURL.c_str(), contentId.c_str(), strPath.c_str());
			setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
			return;
		}

		/*
		>>>> PUT /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/ HTTP/1.1     
				  <?xml version="1.0" encoding="utf-8" ?>     
				  <Content id="4d854d85-4e6c-d06f-b15b-d06fa6d85edf" xmlns="urn:eventis:cpi:1.0">       
				  <Name>The nine o'clock news</Name>       
				  <Provider>BBC</Provider>       
				  <SourceUri>ChannelName</SourceUri>       
				  <IngestStartTime>2013-07-01T21:00:00Z</IngestStartTime>       
				  <IngestEndTime>2013-07-01T22:00:00Z</IngestEndTime>       
				  <BitrateInBps>4500000</BitrateInBps>     
				  </Content>  

		<<<< HTTP/1.1 200 OK
		*/
		void C3dServerMsgHandler::createContent(const std::string& contentId, const std::string& strMsgBody, CRG::IResponse* response)
		{
			int64 stampBeg = ZQ::common::TimeUtil::now();
			if(strMsgBody.empty())
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "createContent[%s] message body is empty"), contentId.c_str());
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}

			envlog(ZQ::common::Log::L_DEBUG,  CLOGFMT(C3dServerMsgHandler,  "create Content[%s]"), contentId.c_str());
			//step 1:parse and valid request
			std::string content_id("");
			std::string content_xmlns("");
			std::string name("");
			std::string provider("");
			std::string sourceUri("");
			std::string ingestStartTime("");
			std::string ingestEndTime("");
			std::string BitrateInBps("");

			ZQ::common::XMLPreferenceDocumentEx xmlDoc;
			if(!readXMLDoc(xmlDoc, strMsgBody.c_str(), strMsgBody.length()))
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "createContent() content[%s] phase XMLMessage body error"),contentId.c_str());
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}
			ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
			if(NULL == pXMLRoot)
			{
				xmlDoc.clear();
				pXMLRoot->free();
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "createContent() content[%s] get root preference failed"),contentId.c_str());
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}
			char node[256] = "";
			memset(node, 0, sizeof(node));

			//get root node content
			pXMLRoot->getPreferenceName(node);
			if(!strcmp(node, KEY_CONTENT))
			{
				ZQ::common::StringMap xmlElment = pXMLRoot->getProperties();
				if(xmlElment.find(KEY_CONTENT_ID) != xmlElment.end())
					content_id = xmlElment[KEY_CONTENT_ID];
				else
				{
					pXMLRoot->free();
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "createContent() content[%s]: get xmlElment[%s] failed"),contentId.c_str(),KEY_CONTENT_ID);
					setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
					return;
				}
				if(xmlElment.find(KEY_CONTENT_XMLNS) != xmlElment.end())
					content_xmlns = xmlElment[KEY_CONTENT_XMLNS];
				else
				{
					pXMLRoot->free();
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "createContent() content[%s]:get xmlElement[%s] failed"),contentId.c_str(),KEY_CONTENT_XMLNS);
					setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
					return;
				}
			}
			if(contentId != content_id)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "message body content_id[%s] conflict with that[%s] on URI"), content_id.c_str(), contentId.c_str());
				setResponseWithBody(response, STATUS_BADREQUEST, REASON_BADREQUEST, "message body contentId conflict with that on URI");
				return;
			}
			//get child node
			_isNameExist = true;
			if(!getChildNode(contentId, pXMLRoot, std::string(KEY_NAME), name))
			{
				//bug 18506
				_isNameExist = false;
				name = contentId;
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "not find key[%s] in message, and set it to contentId[%s]"), KEY_NAME, contentId.c_str());
			}

			_isProviderExist = true;
			if(!getChildNode(contentId, pXMLRoot, std::string(KEY_PROVIDER), provider))
			{
				//bug 18506
				_isProviderExist = false;
				provider = _3dServerConfig.providerId;
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "not find key[%s] in message, and set it to configuration[%s]"), KEY_PROVIDER, _3dServerConfig.providerId.c_str());
			}
			if(provider.empty())
			{
				_isProviderExist = false;
				provider = _3dServerConfig.providerId;
			}
			if(name.empty())
			{
				_isNameExist = false;
				name = contentId;
			}

			if(!getChildNode(contentId, pXMLRoot, std::string(KEY_SOURCEURI), sourceUri))
			{
				pXMLRoot->free();
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "can not find key[%s] in message body"), KEY_SOURCEURI);
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}

			int64 iCt = ZQ::common::TimeUtil::now();
			char timeBuf[100] = "";
			memset(timeBuf, 0 ,sizeof(timeBuf));
			if(!ZQ::common::TimeUtil::TimeToUTC(iCt, timeBuf, sizeof(timeBuf)))
			{
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "not find key[%s] in message, and set StartTime to current time failed"), KEY_INGESTSTARTTIME);
				return;
			}
			std::string currentTime = timeBuf;
			_isStartTimeExist = true;
			if(!getChildNode(contentId, pXMLRoot, std::string(KEY_INGESTSTARTTIME), ingestStartTime))
			{
				//bug 18507
				_isStartTimeExist = false;
				ingestStartTime = currentTime;
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "not find key[%s] in message body, than set it to current time[%s]"), KEY_INGESTSTARTTIME, ingestStartTime.c_str());
			}
			if(ingestStartTime.empty())
			{
				_isStartTimeExist = false;
				ingestStartTime = currentTime;
			}

			if(!getChildNode(contentId, pXMLRoot, std::string(KEY_INGESTENDTIME), ingestEndTime))
			{
				pXMLRoot->free();
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "can not find key[%s] in message body"), KEY_INGESTENDTIME);
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}
			if(!getChildNode(contentId, pXMLRoot, std::string(KEY_BITRATEINBPS), BitrateInBps))
			{
				pXMLRoot->free();
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "can not find key[%s] in message body"), KEY_BITRATEINBPS);
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}
			pXMLRoot->free();

			if(sourceUri.empty() || ingestEndTime.empty() || BitrateInBps.empty())
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "content[%s] get XMLMessage nodes failed"), contentId.c_str());
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}
			
			//step 2: create container under aqua, jump this step if container has been exist
			//TestMode
			if(_3dServerConfig.testMode.enable)
			{
				int64 IingestStartTime = ZQ::common::TimeUtil::now() + _3dServerConfig.testMode.delay * 1000;
				int64 IingestEndTime = IingestStartTime + _3dServerConfig.testMode.duration * 1000;
				char buffer[50];
				ZQ::common::TimeUtil::TimeToUTC(IingestStartTime, buffer, sizeof(buffer)-1);
				ingestStartTime = buffer;
				memset(buffer,0,sizeof(buffer));
				ZQ::common::TimeUtil::TimeToUTC(IingestEndTime, buffer, sizeof(buffer)-1);
				ingestEndTime = buffer;
			}
			envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(C3dServerMsgHandler, "phase content[%s]:name[%s], provider[%s], sourceUri[%s], ingestStartTime[%s], ingestEndTime[%s], BitrateInBps[%s]"),
				contentId.c_str(), name.c_str(), provider.c_str(), sourceUri.c_str(), ingestStartTime.c_str(), ingestEndTime.c_str(),BitrateInBps.c_str());

			//step2.1 read container info. if not exist, creat content folder.
			// if exist, compare provider and source value,if different return 403
			Json::Value valResult;
			int retCode = 0;
			bool bContentFolderExisted = false;
			std::string containerFullUri = _env._pCdmiClient->pathToUri(_3dServerConfig.contentFolder + content_id);
			retCode = _env._pCdmiClient->cdmi_ReadContainer(valResult, containerFullUri);
			if(CdmiRet_FAIL(retCode))
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "content[%s] not exist, than create content and session"), contentId.c_str());
			}
			if(CdmiRet_SUCC(retCode))
			{
				//Any PUT to an existed contentId with different <Provider> or <SourceUri> will be rejected via 403
				//"metadata":{"npvr_bandwidth":"4500000","cdmi_owner":"cstest","cdmi_ctime":"2013-07-16T07:07:32.000938Z","npvr_recording_end":"2013-07-01T22:00:00Z","npvr_source":"npvr/contents/4d854d85-4e6c-d06f-b15b-d06fa6d85edf","cdmi_mtime":"2013-07->>omitted
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "content[%s] has exist, than update container and session"), contentId.c_str());
				// forbidden cases
				std::string status = "";
				status = getStatus(valResult);
				if(status == STATUS_NOTVALID)
				{
					setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "get status failed when update content[%s]"), contentId.c_str());
					return;
				}
				else if(status == STATUS_INGESTMISSED)
				{
					setReponseWithoutBody(response, STATUS_METHODNOTVALID, REASON_METHODNOTVALID);
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "can not get valid status in this method"));
					return;
				}
				else if(status == STATUS_INGESTED)
				{
					setReponseWithoutBody(response, STATUS_METHODNOTVALID, REASON_METHODNOTVALID);
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "update content[%s] failed with wrong status[Ingested]"), contentId.c_str());
					return;
				}
				else if(status == STATUS_INGESTING || status == STATUS_INGESTINGANDPLAYABLE)
				{
					if(valResult["metadata"][METADATA_START].asString() != ingestStartTime)
					{
						setReponseWithoutBody(response, STATUS_FORBIDDEN, REASON_FORBIDDEN);
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "update content[%s][startTime:%s] with wrong start time[%s] in status[%s]"), contentId.c_str(), valResult["metadata"][METADATA_START].asString().c_str(), ingestStartTime.c_str(), status.c_str());
						return;
					}
				}
				std::string jProvider = "";
				std::string jSource = "";
				if(valResult["metadata"].isMember(METADATA_SOURCE) && valResult["metadata"].isMember(METADATA_PROVIDER))
				{
					jProvider = valResult["metadata"][METADATA_PROVIDER].asString();
					jSource = valResult["metadata"][METADATA_SOURCE].asString();
				}
				if(jProvider != provider || jSource != sourceUri)
				{
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "update content[%s][provider:%s, sourceUri:%s] with wrong new provider[%s] or sourceUri[%s]"), containerFullUri.c_str(), jProvider.c_str(), jSource.c_str(), provider.c_str(), sourceUri.c_str());
					//reject via 403
					setReponseWithoutBody(response, STATUS_FORBIDDEN, REASON_FORBIDDEN);
					return;
				}
				bContentFolderExisted = true;
				std::string contentFolder = _3dServerConfig.contentFolder + content_id;
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "content folder[%s] has been existed"), contentFolder.c_str());
			}
			else if(retCode == CdmiClientBase::cdmirc_NotFound)	
			{
                bContentFolderExisted = false;
			}
			else
			{
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]", retCode);
				setResponseWithBody(response, STATUS_ERROR, REASON_ERROR, buf);
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "create container[%s] failed,aqua returned error code[%d"), containerFullUri.c_str(), retCode);
				return;
			}
			
			//adjust the startTime and endTime
			std::string oldStartTime;
			std::string newStartTime = ingestStartTime;
			int pos = currentTime.find(".");
			if(pos != std::string::npos)
			{
				currentTime = currentTime.substr(0, pos) + "Z";
			}
			if(bContentFolderExisted)
				oldStartTime = valResult["metadata"][METADATA_START].asString();
			else
				oldStartTime = currentTime;
			if(newStartTime < oldStartTime)
			{
				if(newStartTime < currentTime)
					newStartTime = currentTime;
			}
			ingestStartTime = newStartTime;

			if(ingestEndTime < ingestStartTime)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "contentId[%s] method[PUT] wrong startTime[%s] and EndTime[%s]"), contentId.c_str(), ingestStartTime.c_str(), ingestEndTime.c_str());
				setReponseWithoutBody(response, STATUS_BADREQUEST, REASON_BADREQUEST);
				return;
			}

            ///step2.2, content folder not exist, create content folder.
			//build metadata for create aqua container
			TianShanIce::Properties AquaMetadata;
			if(name == contentId)
			{
				if(_3dServerConfig.contentNameWithProviderId)
					name += _3dServerConfig.providerId;
			}
			
			MAPSET(TianShanIce::Properties, AquaMetadata, METADATA_NAME, name);
			MAPSET(TianShanIce::Properties, AquaMetadata, METADATA_PROVIDER, provider);
			MAPSET(TianShanIce::Properties, AquaMetadata, METADATA_SOURCE, sourceUri);
			MAPSET(TianShanIce::Properties, AquaMetadata, METADATA_START, ingestStartTime);
			MAPSET(TianShanIce::Properties, AquaMetadata, METADATA_END, ingestEndTime);
			MAPSET(TianShanIce::Properties, AquaMetadata, METADATA_BITRATE, BitrateInBps);
			if(!bContentFolderExisted)
			{
				valResult.clear();
				retCode = _env._pCdmiClient->cdmi_CreateContainer(valResult, containerFullUri, AquaMetadata);
				if(CdmiRet_FAIL(retCode))
				{
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "create container[%s] failed,aqua returned error code[%d]"), containerFullUri.c_str(), retCode);
					char buf[200] = "";
					snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",retCode);
					setResponseWithBody(response, retCode, getErrReason(retCode).c_str(), buf);
					return;
				}
				envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "create container[%s] with metadata:name[%s], provider[%s], sourceUri[%s], ingestStartTime[%s], ingestEndTime[%s], BitrateInBps[%s]"),
						containerFullUri.c_str(), name.c_str(), provider.c_str(), sourceUri.c_str(), ingestStartTime.c_str(), ingestEndTime.c_str(),BitrateInBps.c_str());
			}
			else
			{
				std::string location;
				int ret = _env._pCdmiClient->cdmi_UpdateContainer(valResult, location, containerFullUri, AquaMetadata);
				if(CdmiRet_FAIL(ret))
				{
					char buf[200];
					snprintf(buf, sizeof(buf)-2, "Aqua returned error code[%d]", ret);
					setResponseWithBody(response, ret, getErrReason(ret).c_str(), buf);
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "update container[%s] failed, aqua returned error code[%d]"), containerFullUri.c_str(), ret);

					return;
				}
			}
			///step2.3 get cpe session by content key
			//build metadata for create CPE session
			std::string  sessionFullPath = _3dServerConfig.contentFolder + contentId;
			std::string  sourceFullPath = _3dServerConfig.sourceFolder + sourceUri;

			TianShanIce::ContentProvision::ProvisionContentKey key;
			key.content = sessionFullPath;
			key.contentStoreNetId = _3dServerConfig.CSId;
			key.volume = _3dServerConfig.CSVolume;

			TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx;
			TianShanIce::ContentProvision::ProvisionSessionPrx sessPrx;

			cpePrx = _env.getCPEProxy();
			if(!cpePrx)
			{
               //todo: 返回500
				setResponseWithBody(response, STATUS_ERROR, REASON_ERROR, "invalid CPE Server");
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "invalid CPE Server"));
				return;
			}

			try
			{
				TianShanIce::ContentProvision::ProvisionSessionPrx sessPrx = cpePrx->openSession(key);
				//container and session are existed ,then update session
				if(sessPrx)
				{
					sessPrx->updateScheduledTime(ingestStartTime, ingestEndTime);
					setReponseWithoutBody(response, STATUS_OK, REASON_OK);
					envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "contentId[%s] update session success with startTime[%s] and endTime[%s]"), contentId.c_str(), ingestStartTime.c_str(), ingestEndTime.c_str());
					return;
				}
			}
			catch (const Ice::Exception& ex)
			{
				//todo:错误处理, 返回500
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "contentId[%s] open session caught Ice::exception[%s]"), contentId.c_str(), ex.ice_name().c_str());
				return;
			}
			
			//step 2.4:create CPE session
			try
			{
				//run here in 2 cases:
				// 1:container has existed, but session not found
				// 2:container not found

				TianShanIce::Properties CPEMetadata;
				MAPSET(TianShanIce::Properties, CPEMetadata, CPHPM_STARTTIME, ingestStartTime);
				MAPSET(TianShanIce::Properties, CPEMetadata, CPHPM_ENDTIME, ingestEndTime);
				MAPSET(TianShanIce::Properties, CPEMetadata, CPHPM_PROVIDERID, provider);
				MAPSET(TianShanIce::Properties, CPEMetadata, CPHPM_PROVIDERASSETID, sessionFullPath);

				//build resource of createCPE session
				TianShanIce::ValueMap	vMapURI, vMapBw;
				TianShanIce::Variant v1;
				v1.bRange = false;
				v1.type = TianShanIce::vtStrings;
				v1.strs.push_back(sourceFullPath);
				MAPSET(TianShanIce::ValueMap, vMapURI, CPHPM_SOURCEURL, v1);
				TianShanIce::Variant v2;
				v2.bRange = false;
				v2.type = TianShanIce::vtStrings;
				v2.strs.push_back(sessionFullPath);
				MAPSET(TianShanIce::ValueMap, vMapURI, CPHPM_FILENAME, v2);
				TianShanIce::Variant v3;
				v3.bRange = false;
				v3.type = TianShanIce::vtLongs;
				Ice::Long bw = atol(BitrateInBps.c_str());
				v3.lints.push_back(bw);
				MAPSET(TianShanIce::ValueMap, vMapBw, CPHPM_BANDWIDTH, v3);

				envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(C3dServerMsgHandler,"content[%s] create session with metadata:ingestStartTime[%s], ingestEndTime[%s], provider[%s], providerAssetId[%s]    resources:courceUrl[%s], fileName[%s], bandwidth[%s]"),
						contentId.c_str(), ingestStartTime.c_str(), ingestEndTime.c_str(), provider.c_str(), sessionFullPath.c_str(),sourceFullPath.c_str(), sessionFullPath.c_str(), BitrateInBps.c_str());

				//create session
				sessPrx = cpePrx->createSession(key, METHODTYPE_AQUAREC, TianShanIce::ContentProvision::potDirect, NULL, NULL);
				::TianShanIce::ContentProvision::ProvisionSubscribeMask mask = {1, false, false, true, true, true};
				sessPrx->setSubscribeMask(mask);
				for(TianShanIce::Properties::iterator iter = CPEMetadata.begin();iter != CPEMetadata.end();iter++)
				{
					sessPrx->setProperty(iter->first, iter->second);
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(C3dServerMsgHandler, "[%s][%s]setProperty[%s][%s]"),
						key.content.c_str(), key.contentStoreNetId.c_str(), iter->first.c_str(), iter->second.c_str());
				}
				sessPrx->addResource(TianShanIce::SRM::rtURI, vMapURI);
				sessPrx->addResource(TianShanIce::SRM::rtProvisionBandwidth,vMapBw);
				sessPrx->setup(ingestStartTime, ingestEndTime);
				sessPrx->commit();
				setReponseWithoutBody(response,STATUS_OK,REASON_OK);
				envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed PUT contentId[%s] name[%s] providerId[%s] sourceUri[%s] recordingTime[%s ~ %s] response[200 OK] took %dms"), 
										contentId.c_str(), name.c_str(), provider.c_str(), sourceUri.c_str(), ingestStartTime.c_str(), ingestEndTime.c_str(), ZQ::common::TimeUtil::now() - stampBeg);

				return;
			}
			catch(const TianShanIce::InvalidParameter& ex)
			{	
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "failed to createSession[%s] caught InvalidParameter exception: %s"), 
					sessionFullPath.c_str(), ex.message.c_str());	
			}
			catch(const TianShanIce::ContentProvision::OutOfResource& ex)
			{	
				envlog(Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "failed to createSession[%s] caught OutOfResource exception: %s"), 
					sessionFullPath.c_str(), ex.message.c_str());	
			}
			catch(const Ice::Exception& ex)
			{
				envlog(Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "failed to createSession[%s] caught ice exception caught: %s"), 
					sessionFullPath.c_str(), ex.ice_name().c_str());	
			}

			setReponseWithoutBody(response,STATUS_ERROR,REASON_ERROR);
			return;
		}

		/*
		>>>>DELETE /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/ HTTP/1.1   
		<<<HTTP/1.1 200 OK 
		*/
		void C3dServerMsgHandler::deleteContent(const std::string& contentId, CRG::IResponse* response)
		{
			int64 stampBeg = ZQ::common::TimeUtil::now();
			TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx = _env.getCPEProxy();
			if(!cpePrx)
			{
				setResponseWithBody(response, STATUS_ERROR, REASON_ERROR, "invalid CPE Server");
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "invalid CPE Server"));
				return;
			}
			std::string  sessionFullPath = _3dServerConfig.contentFolder + contentId;

			TianShanIce::ContentProvision::ProvisionContentKey key;
			key.content = sessionFullPath;
			key.contentStoreNetId = _3dServerConfig.CSId;
			key.volume = _3dServerConfig.CSVolume;
			TianShanIce::ContentProvision::ProvisionSessionPrx sessPrx = cpePrx->openSession(key);
			if(!sessPrx)
			{
				envlog(ZQ::common::Log::L_WARNING,CLOGFMT(C3dServerMsgHandler,"deleteContent(%s) session(%s) not exist"),contentId.c_str(),sessionFullPath.c_str());
			}
			else
			{
				TianShanIce::ContentProvision::ProvisionState sessStatus = sessPrx->getState();
				if(sessStatus == TianShanIce::ContentProvision::cpsProvisioning)
				{
					sessPrx->cancel(STATUS_STOP, REASON_STOP);
					envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"deleteContent(%s) stop the session(%s)"),contentId.c_str(),sessionFullPath.c_str());
				}
			}

			//contentId valid check
			std::string contentFullPath = _3dServerConfig.contentFolder + contentId;
			std::string newContentPath = _3dServerConfig.contentFolder + "trash_" + contentId;
			std::string contentFullUri = _env._pCdmiClient->pathToUri(contentFullPath);
			std::string newContentUri = _env._pCdmiClient->pathToUri(newContentPath);

			Json::Value jValues;
			int ret = _env._pCdmiClient->cdmi_ReadContainer(jValues,contentFullUri);
			if(CdmiRet_FAIL(ret))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "deleteContent[%s] read container failed, aqua returned error code[%d]"), contentFullUri.c_str(), ret);
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-1, "aqua returned error code[%d]",ret);
				setResponseWithBody(response, ret, getErrReason(ret).c_str(), buf);
				return;
			}
			Json::Value destResult;
			CdmiClientBase::Properties metaData;
			//if(jValues.isMember("metadata"))
				//metaData = jValues["metadata"];
			Json::Value exports;
			if(!contentFullUri.empty() && contentFullUri[contentFullUri.length()-1] != '/')
				contentFullUri += "/";
			envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(CdmiClientBase,"calling cdmi_MoveContainer(%s) to create a new container "),newContentUri.c_str());
			CdmiClientBase::CdmiRetCode cdmirc = _env._pCdmiClient->cdmi_CreateContainer(destResult,newContentUri,metaData,exports,"","","",contentFullUri);																														
			if(CdmiRet_FAIL(cdmirc))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CdmiClientBase,"cdmi_CreateContainer(%s) failed, aqua returned error code[%d]"),newContentUri.c_str(),cdmirc);
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",cdmirc);
				setResponseWithBody(response, cdmirc, getErrReason(cdmirc).c_str(), buf);
				return;
			}
			//delete containers
			jValues.clear();
			ret = _env._pCdmiClient->cdmi_DeleteContainer(jValues,newContentUri);
			if(CdmiRet_FAIL(ret))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"deleteContent[%s] delete container failed,aqua returned error code[%d]"),newContentUri.c_str(), ret);
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",ret);
				setResponseWithBody(response, ret, getErrReason(ret).c_str(), buf);

				return;
			}

			setReponseWithoutBody(response,STATUS_OK,REASON_OK);
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed DELETE contentId[%s] took %dms"), contentId.c_str(), ZQ::common::TimeUtil::now() - stampBeg);
			return;
		}
		/*
		>>>>GET /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/ HTTP/1.1   
		<<<<HTTP/1.1 200 OK     
				<?xml version="1.0" encoding="utf-8" ?>     
				<Content id="4d854d85-4e6c-d06f-b15b-d06fa6d85edf" xmlns="urn:eventis:cpi:1.0">       
				<Name>Spiderman 3</Name>       
				<Provider>Sony</Provider>       
				<SourceUri>ftp://10.0.0.10/Spiderman3.mpg</SourceUri>       
				<IngestStartTime>2009-09-13T13:11:00Z</IngestStartTime>       
				<IngestEndTime>2009-09-13T13:13:00Z</IngestEndTime>       
				<BitrateInBps>3750000</BitrateInBps>       
				<Status id=”Ingested”/>     
				</Content>  
		*/
		void C3dServerMsgHandler::getContentInfo(const std::string& contentId, CRG::IResponse* response)
		{
			int64 stampBeg = ZQ::common::TimeUtil::now();
			std::string contentPath = _3dServerConfig.contentFolder + contentId;
			std::string contentUri = _env._pCdmiClient->pathToUri(contentPath);
			Json::Value JValues;
			int ret  = _env._pCdmiClient->cdmi_ReadContainer(JValues,contentUri);
			if(CdmiRet_FAIL(ret))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo() readContainer[%s] failed, aqua returned error code[%d]"),contentUri.c_str(),ret);
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",ret);
				setResponseWithBody(response, ret, getErrReason(ret).c_str(), buf);

				return;
			}
			std::string name = "";
			std::string provider = "";
			std::string sourceUri = "";
			std::string startTime = "";
			std::string endTime = "";
			std::string biterate = "";
			std::string status = "";
			if(!JValues.isMember("metadata"))
			{
				setReponseWithoutBody(response,STATUS_ERROR,REASON_ERROR);
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo[%s] read container metadata failed"),contentUri.c_str());
				return;
			}
			if(!JValues["metadata"].isMember(METADATA_NAME) || !JValues["metadata"].isMember(METADATA_PROVIDER) ||
						!JValues["metadata"].isMember(METADATA_SOURCE) || !JValues["metadata"].isMember(METADATA_START)|| 
								!JValues["metadata"].isMember(METADATA_END) || !JValues["metadata"].isMember(METADATA_BITRATE))
			{
				setReponseWithoutBody(response,STATUS_ERROR,REASON_ERROR);
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo[%s] query metadatas failed"),contentUri.c_str());
				return;
			}
			name = JValues["metadata"][METADATA_NAME].asString();
			provider = JValues["metadata"][METADATA_PROVIDER].asString();
			sourceUri = JValues["metadata"][METADATA_SOURCE].asString();
			startTime = JValues["metadata"][METADATA_START].asString();
			endTime = JValues["metadata"][METADATA_END].asString();
			biterate = JValues["metadata"][METADATA_BITRATE].asString();
			status = getStatus(JValues);
			if(status == STATUS_INGESTMISSED || status == STATUS_NOTVALID)
			{
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo[%s] query container status failed"),contentUri.c_str());
				return;
			}

			//build response message
			std::ostringstream buf;
			buf << XML_HEADER << "\r\n";
			buf << "<Content ";
			buf <<    "id=\"" << contentId << "\"";
			buf <<    " xmlns=\"" << XML_XMLNS << "\">\r\n";
			if(_isNameExist)
				buf << "<Name>" << name << "</Name>\r\n";
			if(_isProviderExist)
				buf << "<Provider>" << provider << "</Provider>\r\n";
			buf << "<SourceUri>" << sourceUri << "</SourceUri>\r\n";
			if(_isStartTimeExist)
				buf << "<IngestStartTime>" << startTime << "</IngestStartTime>\r\n";
			buf << "<IngestEndTime>" << endTime << "</IngestEndTime>\r\n";
			buf << "<BitrateInBps>" << biterate << "</BitrateInBps>\r\n";
			buf << "<Status ";
			buf <<     "id=\"" << status <<"\"/>\r\n";
			buf << "</Content>\r\n";

			setResponseWithBody(response,STATUS_OK,REASON_OK,buf.str());

			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed GET contentId[%s] took %dms"), contentId.c_str(), ZQ::common::TimeUtil::now()-stampBeg);

			return;
		}

		/*
		>>>>GET /Content/4d854d85-4e6c-d06f-b15b-d06fa6d85edf/Status HTTP/1.1   
		<<<<HTTP/1.1 200 OK     
			     <?xml version="1.0" encoding="utf-8" ?>     
				 <Content id="4d854d85-4e6c-d06f-b15b-d06fa6d85edf" xmlns="urn:eventis:cpi:1.0">       
				 <Status id="Ingesting">         
				 <IngestPercentage>20</IngestPercentage>  
				 </Status>     
				 </Content>
		*/
		void C3dServerMsgHandler::getContentInfo(const std::string& contentId, const std::string& conditionKey, CRG::IResponse* response)
		{
			int64 stampBeg = ZQ::common::TimeUtil::now();
			int64 begin = ZQ::common::TimeUtil::now();
			//step 1: get startTime ,endTime and channelId
			std::string contentPath = _3dServerConfig.contentFolder + contentId;
			std::string contentUri = _env._pCdmiClient->pathToUri(contentPath);
			Json::Value JValues;
			int ret = _env._pCdmiClient->cdmi_ReadContainer(JValues,contentUri);
			if(CdmiRet_FAIL(ret))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) read container[%s] failed, aqua returned error code[%d]"), contentUri.c_str(), ret);
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",ret);
				setResponseWithBody(response, ret, getErrReason(ret).c_str(), buf);
				return;
			}
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed read container[%s] took %dms"),contentUri.c_str(),ZQ::common::TimeUtil::now() - begin);
			begin = ZQ::common::TimeUtil::now();

			std::string status = getStatus(JValues);
			if(status == STATUS_INGESTMISSED || status == STATUS_NOTVALID)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) read container[%s] status failed"), contentUri.c_str());
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				return;
			}

			int64 stampStart = 0;
			int64 stampEnd = 0;
			if(!JValues.isMember("metadata"))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) metadata is not exist in container[%s]"),contentUri.c_str());
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				return;
			}
			if(!JValues["metadata"].isMember(METADATA_START) || !JValues["metadata"].isMember(METADATA_END) || !JValues["metadata"][METADATA_SOURCE])
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) get startTime or endTime failed from container[%s]"),contentUri.c_str());
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				return;
			}
			stampStart = ZQ::common::TimeUtil::ISO8601ToTime(JValues["metadata"][METADATA_START].asString().c_str());
			stampEnd = ZQ::common::TimeUtil::ISO8601ToTime(JValues["metadata"][METADATA_END].asString().c_str());

			time_t  maxCreateTime = 0;
			std::string channelId = JValues["metadata"][METADATA_SOURCE].asString();
			std::string sourceFullPath = _3dServerConfig.sourceFolder + channelId;
			std::string sourceFullUri = _env._pCdmiClient->pathToUri(sourceFullPath);
			JValues.clear();

			//Folder Structure:-	npvr/sources/[channelid]/bitrate1/segments[1-n]
			//step 2: travelsal all subcontainer in source/channel/ to find the latest file,
			//			   get the maxCreateTime from the file name
			ret = _env._pCdmiClient->cdmi_ReadContainer(JValues,sourceFullUri);
			if(CdmiRet_FAIL(ret))
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) read source container[%s] failed, aqua returned error code[%d]"), contentUri.c_str(), ret);
				char buf[200] = "";
				snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",ret);
				setResponseWithBody(response, ret, getErrReason(ret).c_str(), buf);
				return;
			}
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed read container[%s] took %dms"), sourceFullUri.c_str(), ZQ::common::TimeUtil::now() - begin);
			begin = ZQ::common::TimeUtil::now();

			if(!JValues.isMember("children"))
			{
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) read metadata[\"children\"] from container[%s] failed"), contentUri.c_str());
				return;
			}
			Json::Value children = JValues["children"];
			if(children.size() == 0)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) not found any subcontainer in [%s]"), sourceFullUri.c_str());
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				return;
			}
			//step 2.1: read all subContainer in first directory ---> /sources/channel/
			std::string lastObject = "";
			int32 fileCount = 0;
			for(Json::Value::iterator iter = children.begin(); iter != children.end(); iter++)
			{
				//read subcontainer of child
				std::string name = (*iter).asString();
				if(name[name.length()-1] != '/')
				{
					lastObject = name;
				}
				if(sourceFullUri[sourceFullUri.length()-1] != '/')
					sourceFullUri += "/";
				std::string subPath = sourceFullUri + name;
				std::string subUri = _env._pCdmiClient->pathToUri(subPath);
				JValues.clear();
				int cdmiRet = _env._pCdmiClient->cdmi_ReadContainer(JValues,subUri);
				if(CdmiRet_FAIL(cdmiRet))
				{
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) read subContainer[%s] error, aqua returned error code[%d]"),subUri.c_str(), cdmiRet);
					char buf[200] = "";
					snprintf(buf, sizeof(buf)-2, "aqua returned error code[%d]",cdmiRet);
					setResponseWithBody(response, cdmiRet, getErrReason(cdmiRet).c_str(), buf);
					return;
				}
				envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed read container[%s] took %dms"), subUri.c_str(),ZQ::common::TimeUtil::now() - begin);
				begin = ZQ::common::TimeUtil::now();
				if(!JValues.isMember("children"))
				{
					setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) read metadata[\"children\"] from container[%s] failed"), subUri.c_str());
					return;
				}
				Json::Value subChildren = JValues["children"];
				if(subChildren.size() == 0)
				{
					envlog(ZQ::common::Log::L_WARNING,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) not found any subcontainer in [%s]"), subUri.c_str());
					continue;
				}
				//step 2.2: read all subContainer in second directory ---> /sources/chnnelId/bitrateN
				fileCount += subChildren.size();
				for(Json::Value::iterator iter = subChildren.begin(); iter != subChildren.end(); iter++)
				{
					std::string subName = (*iter).asString();
					if(!subName.empty() && subName[subName.length()-1] == '/')
						continue;
					if(subName > lastObject)
						lastObject = subName;
				}
			}
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"latest file name[%s]"),lastObject.c_str());
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"%s has %d files"), sourceFullUri.c_str(),fileCount);
			if(lastObject.empty())
			{
				//build response message
				std::ostringstream buf;
				buf << XML_HEADER << "\r\n";
				buf << "<Content ";
				buf <<    "id=\"" << contentId << "\"";
				buf <<    " xmlns=\"" << XML_XMLNS <<"\">" <<"\r\n";
				buf << "<Status ";
				buf <<    "id=\""  << status  << "\">\r\n";
				buf <<         "<IngestPercentage>0</IngestPercentage>\r\n";
				buf <<  "</Status>\r\n";
				buf <<  "</Content>\r\n";
			    
				setResponseWithBody(response, STATUS_OK, REASON_OK, buf.str());
				return;
			}

			size_t pos = 0;
			if((pos = lastObject.find('.')) == std::string::npos)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) file name formate error[%s\\%s]"), sourceFullUri.c_str(), lastObject.c_str());
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				return;
			}
			lastObject = lastObject.substr(0,pos);
			if((pos = lastObject.find('_')) == std::string::npos)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(status) file name formate error[%s\\%s]"), sourceFullUri.c_str(), lastObject.c_str());
				setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
				return;
			}
			lastObject = lastObject.substr(pos);
			if(lastObject[0] == '_')
				lastObject = lastObject.substr(1);

			//step 3: calculate the percentage
			// file name formate:
			//<PREFFIX>_<YYYYMMDDHHmmSS>[.<EXTName>]
			// the date formate may be local time
			std::string strUTCTime;
			strUTCTime = lastObject.substr(0,4) + "-" + lastObject.substr(4,2) + "-" + lastObject.substr(6,2) + "T" 
				+ lastObject.substr(8,2) + ":" + lastObject.substr(10,2) + ":" + lastObject.substr(12,2);
			if(_3dServerConfig.timeZoneOfFileName)
			{
				struct tm* pTmLocal;
				time_t lTime;
				time(&lTime);
				pTmLocal = localtime(&lTime);

				char temp[64] = "";
				if(!ZQ::common::TimeUtil::Local2Iso(strUTCTime.c_str(), temp, sizeof(temp)))
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "formate local time to standard UTC failed, local time[%s]"), strUTCTime.c_str());
					setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
					return;
				}
				if(pTmLocal->tm_isdst)
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "formate DST time[%s] to standard UTC formate[%s]"), strUTCTime.c_str(), temp);
				}
				else
					envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"formate local time[%s] to standard UTC time[%s]"), strUTCTime.c_str(), temp);

				strUTCTime = temp;
			}
			maxCreateTime = ZQ::common::TimeUtil::ISO8601ToTime(strUTCTime.c_str());
			std::string strPercentage = "";
			int IPercentage = 0;
			IPercentage = ((maxCreateTime - stampStart)*100) / (stampEnd - stampStart);
			if(IPercentage > 100)
				IPercentage = 100;
			if(IPercentage < 0)
				IPercentage = 0;

			if(status == STATUS_INITIAL)
				IPercentage = 0;
			else if(status == STATUS_INGESTED)
				IPercentage = 100;
		
			char temp[10];
			memset(temp,0,sizeof(temp));
			sprintf(temp,"%d",IPercentage);
			strPercentage = temp;

			//build response message
			std::ostringstream buf;
			buf << XML_HEADER << "\r\n";
			buf << "<Content ";
			buf <<    "id=\"" << contentId << "\"";
			buf <<    " xmlns=\"" << XML_XMLNS <<"\">" <<"\r\n";
			buf << "<Status ";
			buf <<    "id=\""  << status  << "\">\r\n";
			buf <<         "<IngestPercentage>" << strPercentage << "</IngestPercentage>\r\n";
			buf <<  "</Status>\r\n";
			buf <<  "</Content>\r\n";

			setResponseWithBody(response, STATUS_OK, REASON_OK, buf.str());

			envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler,"processed GET/Status contentId[%s] took %dms"), contentId.c_str(), ZQ::common::TimeUtil::now()-stampBeg);
			
			return;
		}

		/*
		>>>>GET /Contents?Status=Ingesting HTTP/1.1   
		<<<HTTP/1.1 200 OK     
			    <?xml version="1.0" encoding="utf-8" ?>     
			    <Contents xmlns="urn:eventis:cpi:1.0">       
				<Content id="4d854d85-4e6c-d06f-b15b-d06fa6d85edf" statusId="Ingesting"/>       
				<Content id="34854e55-1e7d-e06a-a15a-a06fa6a55fff" statusId="Ingesting"/>     
				</Contents>  
		*/
		void C3dServerMsgHandler::getContentInfo(const ZQ::common::StringMap& conditionMap, CRG::IResponse* response)
		{
			int64 stampBeg = ZQ::common::TimeUtil::now();
			if(conditionMap.size() == 0)
			{
				setReponseWithoutBody(response, STATUS_NOTFOUND, REASON_NOTFOUND);
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "getContentInfo(argus) query condition failed"));
				return;
			}
			ZQ::common::StringMap::const_iterator itBeg = conditionMap.begin();
			std::string status = itBeg->second;
			TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx = NULL;
			try
			{
				cpePrx = _env.getCPEProxy();
				if(!cpePrx)
				{
					setResponseWithBody(response, STATUS_ERROR, REASON_ERROR, "invalid CPE Server");
					envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler, "getContentInfo(argus) invalid CPESvc Proxy"));
					return;
				}
				TianShanIce::StrValues paraNames;
				paraNames.push_back(SYS_PROP(contentName));
				paraNames.push_back(SYS_PROP(scheduledStart));
				paraNames.push_back(SYS_PROP(scheduledEnd));
				TianShanIce::ContentProvision::ProvisionInfos proInfos;
				proInfos = cpePrx->listSessions(METHODTYPE_AQUAREC, paraNames, "", MAX_SESSIOONCOUNT);
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "list [%d] sessions frome CPE"), proInfos.size());

				std::ostringstream buf;
				buf << XML_HEADER << "\r\n";
				buf << "<Contents ";
				buf <<    "xmlns=\"" << XML_XMLNS << "\">\r\n";
				for(TianShanIce::ContentProvision::ProvisionInfos::iterator iter = proInfos.begin();iter != proInfos.end();iter++)
				{
					std::string contentid = iter->contentKey.content;
					size_t pos = contentid.find_last_of('/');
					if(pos != std::string::npos)
						contentid = contentid.substr(pos+1);

					int64 stampNow = ZQ::common::TimeUtil::now();
					int64 stampStart = 0;
					int64 stampEnd = 0;
					
					if(iter->params.find(SYS_PROP(scheduledStart)) == iter->params.end() || 
									iter->params.find(SYS_PROP(scheduledEnd)) == iter->params.end())
					{
						/*
						envlog(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerMsgHandler,"getContentInfo(argus) can't find scheduledStart or scheduledEnd from container[%s]"),contentid.c_str());
						setReponseWithoutBody(response,STATUS_ERROR,REASON_ERROR);
						return;
						*/
						continue;
					}
					
					stampStart = ZQ::common::TimeUtil::ISO8601ToTime(iter->params[SYS_PROP(scheduledStart)].c_str());
					stampEnd = ZQ::common::TimeUtil::ISO8601ToTime(iter->params[SYS_PROP(scheduledEnd)].c_str());
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "content[%s] startTime[%s]  endTime[%s]"), contentid.c_str(), iter->params[SYS_PROP(scheduledStart)].c_str(), iter->params[SYS_PROP(scheduledEnd)].c_str());
					/*												            Ingesting
								---------------------------------------------------------------------------------------------
								|                                         |                                                |												   |
								---------------------------------------------------------------------------------------------
							createTime            startTime			 +timePeriod          endTime
				status:						Initial			streamable                                    Ingested(and any bitrate subcontainers include one data object at least)																
					*/
					if(status == STATUS_INGESTING)
					{
						if(stampNow > stampStart && stampNow < stampEnd)
						{
							buf << "<Content ";
							buf <<    "id=\"" << contentid << "\"";
							buf <<    " statusId=\"" << STATUS_INGESTING << "\"/>\r\n";
						}
						else
							continue;
						// if(stampNow > startTime && stampNow < (startTime + timePeriod))  set status=streamable
					}
					if(status == STATUS_INITIAL)
					{
						//todo: logic in status initial
						if(stampNow < stampStart)
						{
							buf << "<Content ";
							buf <<    "id=\"" << contentid << "\"";
							buf <<    " statusId=\"" << STATUS_INITIAL << "\"/>\r\n";
						}
						else
							continue;
					}
					if(status == STATUS_INGESTED)
					{
						//todo: logic in status ingesting
					}
				//	if
				}
				buf << "</Contents>\r\n";
				setResponseWithBody(response, STATUS_OK, REASON_OK, buf.str().c_str());

				envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "processed GET/Status=%s %dms"), status.c_str(), ZQ::common::TimeUtil::now()-stampBeg);

				return;
			}///todo: CPE listsession 的 Exception是不是这些??? log 打的有问题.
			catch(const::TianShanIce::ServerError& ex)
			{	
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "Failed to list Session on CPE[%s], caught Server Error exception: %s"), 
					(_env._communicator->proxyToString(cpePrx)).c_str(),ex.message.c_str());	
				ex.ice_throw();
			}
			catch(const Ice::Exception& ex)
			{
				envlog(Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "Failed to list Session on CPE[%s], ice exception caught: %s"), 
					(_env._communicator->proxyToString(cpePrx)).c_str(), ex.ice_name().c_str());	
			}	

			setReponseWithoutBody(response, STATUS_ERROR, REASON_ERROR);
		}

		void C3dServerMsgHandler::setReponseWithoutBody(CRG::IResponse* response, int statusCode, const char* reasonPhrase)
		{
			response->setStatus(statusCode, reasonPhrase);
			char buf[64] = "";
			snprintf(buf, sizeof(buf)-2, "HttpCRG/CRM_3dServer %d.%d", ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR);
			response->setHeader("Server",buf);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "setReponseHeaderWithNoBody() : %d %s"), statusCode, reasonPhrase);
		}

		void C3dServerMsgHandler::setResponseWithBody(CRG::IResponse* response, int statusCode, const char* reasonPhrase, std::string strMsgContent)
		{
			char length[20];
			snprintf(length, sizeof(length), "%ld", strMsgContent.length());
			response->setStatus(statusCode, reasonPhrase);
			response->setHeader("Content-type", "text/xml");
			response->setHeader("Content-length", length);
			char buf[64] = "";
			snprintf(buf, sizeof(buf)-2, "HttpCRG/CRM_3dServer %d.%d", ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR);
			response->setHeader("Server",buf);
			response->setContent(strMsgContent.data(), strMsgContent.length());
			//envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "setResponseWithBody() : Response content is [%s]"), strMsgContent.c_str());
		}
		bool C3dServerMsgHandler::readXMLDoc(ZQ::common::XMLPreferenceDocumentEx& xmlDoc, const char* buffer, size_t bufLen)
		{
			try
			{
				if(!xmlDoc.read((void*)buffer, (int)bufLen, 1))
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "failed to parse request content"));
					return false;
				}
			}
			catch (ZQ::common::XMLException& xmlEx)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "readXMLDoc() : read xml catch a exception [%s]"), xmlEx.getString());
				return false;
			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "readXMLDoc() : read xml catch unknown exception[%d]"), SYS::getLastErr());
				return false;
			}
			return true;
		}

		bool  C3dServerMsgHandler::getChildNode(const std::string& contentId, ZQ::common::XMLPreferenceEx* pXMLRoot, const std::string childName, std::string& childText)
		{
			char nodeText[512] = "";
			ZQ::common::XMLPreferenceEx* pChildNode = pXMLRoot->findChild(childName.c_str());
			if(NULL == pChildNode)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "contentId[%s] get child node[%s] failed"), contentId.c_str(), childName.c_str());
				return false;
			}
			memset(nodeText, 0, sizeof(nodeText));
			if(!pChildNode->getPreferenceText(nodeText, sizeof(nodeText)))
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "contentId[%s] get node[%s] text failed"), contentId.c_str(), childName.c_str());
				return false;
			}
			childText = nodeText;

			return true;
		}

		std::string C3dServerMsgHandler::getStatus(const Json::Value& jVals)
		{
			if(!jVals.isMember("metadata"))
				return STATUS_NOTVALID;
			if(!jVals["metadata"].isMember(METADATA_START) || !jVals["metadata"].isMember(METADATA_END))
				return STATUS_NOTVALID;
			std::string status;
			std::string startTime = jVals["metadata"][METADATA_START].asString();
			std::string endTime = jVals["metadata"][METADATA_END].asString();
			char buf[100] = "";
			int64  stampStart;
			int64  stampEnd;

			int64  stampNow = ZQ::common::TimeUtil::now();
			ZQ::common::TimeUtil::TimeToUTC(stampNow, buf, sizeof(buf));
			stampStart = ZQ::common::TimeUtil::ISO8601ToTime(startTime.c_str());
			stampEnd = ZQ::common::TimeUtil::ISO8601ToTime(endTime.c_str());
			if(stampNow < stampStart)
			{
				return STATUS_INITIAL;
			}

			//if there are data objects
			if(!jVals.isMember("children"))
				return STATUS_NOTVALID;
			Json::Value children = jVals["children"];
			int  objectCount = 0;

			if(!jVals.isMember("parentURI") || !jVals.isMember("objectName"))
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "member [parentURI] or [objectName] is not exist"));
				return STATUS_NOTVALID;
			}
			std::string strUri = jVals["parentURI"].asString();
			if(strUri[strUri.length()-1] != '/')
				strUri += '/';
			std::string objectName = jVals["objectName"].asString();
			if(objectName[objectName.length()-1] != '/')
				objectName += "/";
			std::string srcChildUri = strUri + objectName;

			for(Json::Value::iterator it = children.begin(); it != children.end(); it++)
			{
				std::string strChild = (*it).asString();
				std::string childUri = srcChildUri + strChild;
				childUri = _env._pCdmiClient->pathToUri(childUri);
				Json::Value jvals;
				int ret = _env._pCdmiClient->cdmi_ReadContainer(jvals, childUri);
				if(CdmiRet_FAIL(ret))
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(C3dServerMsgHandler, "read [%s] failed, error code[%d]"), childUri.c_str(), ret);
					return STATUS_NOTVALID;
				}
				if(!jvals.isMember("children"))
					return STATUS_NOTVALID;
				Json::Value childVal = jvals["children"];
				for(Json::Value::iterator it = childVal.begin(); it != childVal.end(); it++)
				{
					std::string fileName = (*it).asString();
					if(fileName[fileName.length()-1] != '/')
					{
						objectCount++;
						break;
					}
				}
				envlog(ZQ::common::Log::L_INFO,CLOGFMT(C3dServerMsgHandler, "[%d] data oeject under [%s]"), objectCount, childUri.c_str());
				if(objectCount > 0)
					break;
			}
			
			if(stampNow > stampStart && stampNow < stampEnd)
			{
				if(objectCount < 1)
					status = STATUS_INGESTING;
				else
					status = STATUS_INGESTINGANDPLAYABLE;
			}
			else if(stampNow > stampEnd)
			{
				if (objectCount > 0)
					status = STATUS_INGESTED;
				else
					status =  STATUS_INGESTMISSED;
			}
			else
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerMsgHandler, "ingesting complete, but there are no object in content[%s]"), srcChildUri.c_str());
				status = STATUS_NOTVALID;
			}

			return status;
		}

		std::string C3dServerMsgHandler::getErrReason(const int errCode)
		{
			switch(errCode)
			{
				case CdmiClientBase::cdmirc_OK:
					return "OK";
				case CdmiClientBase::cdmirc_Created:
					return "Created";
				case CdmiClientBase::cdmirc_Accepted:
					return "Accepted";
				case CdmiClientBase::cdmirc_NoContent:
					return "No Content";
				case CdmiClientBase::cdmirc_PartialContent:
					return "Partial Content";
				case CdmiClientBase::cdmirc_Found:
					return "Found";
				case CdmiClientBase::cdmirc_BadRequest:
					return "Bad Request";
				case CdmiClientBase::cdmirc_Unauthorized:
					return "Unauthorized";
				case CdmiClientBase::cdmirc_Forbidden:
					return "Forbidden";
				case CdmiClientBase::cdmirc_NotFound:
					return "Not Found";
				case CdmiClientBase::cdmirc_NotAcceptable:
					return "Not Acceptable";
				case CdmiClientBase::cdmirc_Conflict:
					return "Conflict";
				case CdmiClientBase::cdmirc_InvalidRange:
					return "Invalid Range";
				case CdmiClientBase::cdmirc_ServerError:
					return "Server Error";
				case CdmiClientBase::cdmirc_ExtErr:
					return "ExtError";
				case CdmiClientBase::cdmirc_RequestFailed:
					return "Request Failed";
				case CdmiClientBase::cdmirc_RequestTimeout:
					return "Request Timeout";
				case CdmiClientBase::cdmirc_AquaLocation:
					return "Aqua Location error";
				case CdmiClientBase::cdmirc_RetryFailed:
					return "Retry Failed";
				case CdmiClientBase::cdmirc_MAX:
					return "MAX";
				default:
					return NULL;
			}
		}
	}///end namespace C3dServer
}///end namespace CRM

