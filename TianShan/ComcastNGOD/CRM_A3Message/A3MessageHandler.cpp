#include "A3MessageHandler.h"
#include "ContentUser.h"
#include "A3Config.h"
#include "Guid.h"
#include "ContentStore.h"
#include "ContentUser.h"
#include "ContentSysMD.h"
#include "CPHInc.h"
#include "urlstr.h"
extern ZQ::common::Config::Loader< A3MessageCfg > _A3Config;

#define A3Fmt(_C, _X) CLOGFMT(_C, "[%s] " _X), reqID.c_str()

namespace CRM
{
	namespace A3Message
	{
		std::string getReqID()
		{
			ZQ::common::Guid id;
			id.create();
			char bufUUID[65] = "";
			int nUUID = id.toString(bufUUID, 65);
			return std::string(bufUUID);
		}

		A3MessageHandler::A3MessageHandler(CRM::A3Message::A3MsgEnv& env): _env(env)
		{
		}
		A3MessageHandler::~A3MessageHandler(void)
		{
		}
		void A3MessageHandler::onRequest(const CRG::IRequest* request, CRG::IResponse* response)
		{
			
			std::string reqID = getReqID();
			std::string strFullURL = request->uri();
			std::string strReqMethod = strFullURL.substr(strFullURL.find_last_of("/"));
			
			std::string strMsgContent;
			char buf[256]= "";

			request->getContent(strMsgContent);
			sprintf(buf, A3Fmt(A3MessageHandler, "onRequest() : receive request uri=[%s], messageContent: "),strFullURL.c_str());
			envlog.hexDump(ZQ::common::Log::L_INFO, strMsgContent.c_str(), strMsgContent.size(), buf, true);

			if ("/TransferContent" == strReqMethod)
			{
				return TransferContent(reqID, request, response);
			}
			else if ("/GetVolumeInfo" == strReqMethod)
			{
				return GetVolumeInfo(reqID,request, response);
			}
			else if ("/GetContentInfo" == strReqMethod)
			{
				return GetContentInfo(reqID,request, response);
			}
			else if ("/CancelTransfer" == strReqMethod)
			{
				return CancelTransfer(reqID,request, response);
			}
			else if ("/ExposeContent" == strReqMethod)
			{
				return ExposeContent(reqID,request, response);
			}
			else if ("/GetContentChecksum" == strReqMethod)
			{
				return GetContentChecksum(reqID,request, response);
			}
			else if ("/GetTransferStatus" == strReqMethod)
			{
				return GetTransferStatus(reqID,request, response);
			}
			else if ("/DeleteContent" == strReqMethod)
			{
				return DeleteContent(reqID,request, response);
			}
			else
			{
				envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "onRequest() : ingore this request uri=%s"), strFullURL.c_str());
			}
		}
/*
		Request Example:
		<?xml version="1.0" encoding="utf-8"?>
		<GetVolumeInfo
		volumeName=”Philly.Warminster.volume4”
		/>
*/
		void A3MessageHandler::GetVolumeInfo(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do GetVolumeInfo()")); 
            Ice::Long lStart = ZQTianShan::now();
			// parse get volume info request
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			StringMap xmlElement;
			request->getContent(strMsgContent);
			if (!parseMsgContent(reqID,strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID,request, response, 400, "invaild request for get volume info");
				return;
			}

			///check parameter key : volumeName
			StringMap::iterator  itorElement;
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID,request, response, 400, "invaild request for get volume info");
				return;
			}

			strVolume = itorElement->second;

			///get Volume info 
			int state = 200;
			Ice::Long freeMB = 0;
			Ice::Long totalMB = 0;
			std::string volumeId = "";

			///query volume info with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID,request, response, 400, GVITATUS_404);
					return;
				}
				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID,request, response, 500, "failed to connected to contentlib service");
					return;
				}
				try
				{
					volumeId = strNetId +"$" + strVolume;
					TianShanIce::Repository::MetaVolumePrx metavolumeprx = clPrx->toVolume(volumeId);
					::TianShanIce::Storage::VolumePrx volumeprx = metavolumeprx->theVolume();
					volumeprx->getCapacity(freeMB, totalMB);
				}
				catch(Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get volume[%s] info caught exception [%s]"), volumeId.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
					return;
				}          
			}
			else  if(_env._backStoreType == backCacheStore)///query volume info with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID,request, response, 500, "failed to connected to cachestore service");
					return;
				}

				try
				{	
					volumeId = strVolume;
					TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
//                  TianShanIce::Storage::VolumePrx vlprx = csprx->openVolume(xmlElement[Key_volumeName]);
					csprx->getCapacity(freeMB, totalMB);
				}
				catch(TianShanIce::ServerError&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get volume[%s] info caught exception [%s] "),volumeId.c_str(), ex.message.c_str()); 
					setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
					return;
				}
				catch(Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get volume[%s] info caught exception [%s] "), volumeId.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
				if(_A3Config.volumeName.empty())
				{
					setReponseWithoutBody(reqID,request, response, 500, "unsupported method");
					return;  
				}
				else if( _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetVolumeInfo() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

				int64 freebytes, totalbytes;
				if(_env._pCdmiFuse == NULL)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetVolumeInfo() CdmiFuseOps NULL Handle")); 
					setReponseWithoutBody(reqID,request, response, 500, GTSREASON_500);
					return;
				}
				CdmiFuseOps::CdmiRetCode retCode = _env._pCdmiFuse->getDiskSpace(freebytes,totalbytes);
				if(!CdmiRet_SUCC(retCode))
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetVolumeInfo() failed to get diskspace with error[%d==>%s]"), retCode, CdmiFuseOps::cdmiRetStr(retCode)); 
					setReponseWithoutBody(reqID,request, response, 500, GTSREASON_500);
					return;
				}

				if(totalbytes != 0)
					totalMB = totalbytes/1024/1024;
				if(freebytes != 0)
					freeMB = freebytes/1024/1024;
			}

			// output volume info
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<VolumeInfo " ;
			buf <<     "volumeName=\"" << xmlElement[Key_volumeName] << "\" ";
			buf <<     "state=\""      << state                  << "\" ";
			buf <<     "volumeSize=\"" << totalMB                << "\" ";
			buf <<     "freeSize=\""   << freeMB                 << "\" ";
			buf << "/>";  
			strMsgContent = buf.str();
			setResponseWithBody(reqID,request, response, 200, "OK", strMsgContent);
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "get volume [%s] info completed took %d ms"),xmlElement[Key_volumeName].c_str(), (int)(ZQTianShan::now() - lStart));
		}

/*			Request Example (All Content on Volume):
				<?xml version="1.0" encoding="utf-8"?>
				<GetContentInfo
				volumeName=”Philly.Warminster.volume1A”
				/>
				Request Example (Single Asset on Volume):
				<?xml version="1.0" encoding="utf-8"?>
				<GetContentInfo
				providerID=”comcast.com”
				assetID=”BAAA0000000000018377”
				volumeName=”Philly.Warminster.volume1A”
				/>
*/
		void A3MessageHandler::GetContentInfo(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do GetContentInfo()")); 
			Ice::Long lStart = ZQTianShan::now();
			/// parse query content info request
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			StringMap xmlElement;
			bool bIsSpecifiedContent = false;
			request->getContent(strMsgContent);
			if (!parseMsgContent(reqID,strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID,request, response, 400, "bad request");
				return;
			}

			///check parameter key : volumeName
			StringMap::iterator  itorElement;
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID,request, response, 400, "invaild request for get volume info");
				return;
			}
			strVolume = itorElement->second;
			///check parameter key : providerID
			itorElement = xmlElement.find(Key_providerID);
			if(itorElement != xmlElement.end())
			{
               strProviderId = itorElement->second;
			}

			///check parameter key : assetID
			itorElement = xmlElement.find(Key_assetID);
			if(itorElement != xmlElement.end())
			{
               strProviderAsserId = itorElement->second;
			}

			///get content info 
			///query content info with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID,request, response, 400, "bad request");
					return;
				}
				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID,request, response, 500, "failed to connect to contentlib service");
					return;
				}
				try
				{
					TianShanIce::Repository::MetaObjectInfos infos;
					TianShanIce::Properties prop;
					TianShanIce::StrValues listMetadata;
					Ice::Int totalcount = 0;

					listMetadata.push_back("user.ProviderId");
					listMetadata.push_back("user.ProviderAssetId");
					listMetadata.push_back("volumeName");
					listMetadata.push_back("sys.FileSize");
					listMetadata.push_back("sys.SupportFileSize");
					listMetadata.push_back("contentState");
					listMetadata.push_back("sys.StampCreated");
					listMetadata.push_back("sys.MD5CheckSum");

					//list specified content of volume , if the  volume = "/*", list all content for the netid
					if("/*" == strVolume)
					{
						strVolume = "";
					} 
                    
					//list all content of contentstore
					if(strProviderId.empty() && strProviderAsserId.empty())
					{
						infos = clPrx->locateContentByNetIDAndVolume(strNetId, strVolume, listMetadata, 0, -1, totalcount);
					}
					else if (!strProviderId.empty() && !strProviderAsserId.empty() && strProviderId != "*" && strProviderAsserId != "*")
					{
						bIsSpecifiedContent = true;
						infos = clPrx->locateContentByPIDAndPAID(strNetId, strVolume, strProviderId, strProviderAsserId, listMetadata);
					}
					else
					{
						setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
						return;
					}

					if(infos.empty() && bIsSpecifiedContent)
					{
						setReponseWithoutBody(reqID,request, response, 404, GCISTATUS_404);
						return;
					}

					std::ostringstream buf;
					buf << XML_HEADER ;
					buf << "<ContentList>\n";

					for(TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin(); it != infos.end(); it++)
					{
						strProviderId = ""; 
						strProviderAsserId  = "";

						std::string strMD5DataTime = GenerateUTCTime();
						std::string strVolumeName, strFilesize = "", strSupportFilesize = "", strContentState = "", strStampCreated = "", strMD5CheckSum = "";
						for (::TianShanIce::Repository::MetaDataMap::const_iterator iter = it->metaDatas.begin(); iter != it->metaDatas.end(); iter++)
						{
							//					std::cout << iter->first << "--" << iter->second.value << "\n";
							if(iter->first == "user.ProviderId")
							{                  
								strProviderId = iter->second.value;
							}
							else if(iter->first == "user.ProviderAssetId")
							{
								strProviderAsserId = iter->second.value;
							}
							else if(iter->first == "sys.FileSize")
							{
								strFilesize = iter->second.value;
							}
							else if(iter->first == "sys.SupportFileSize")
							{
								strSupportFilesize = iter->second.value;
							}
							else if(iter->first == "contentState")
							{
								strContentState = iter->second.value;
							}
							else if(iter->first == "sys.StampCreated")
							{
								strStampCreated = iter->second.value;
							}
							else if(iter->first == "sys.MD5CheckSum")
							{
								strMD5CheckSum = iter->second.value;
							}
							else if(iter->first == "volumeName")
							{
								strVolumeName = iter->second.value;
							}
						}

						buf << "<ContentInfo " ;
						buf <<    "providerID=\""   << strProviderId     << "\" ";
						buf <<    "assetID=\""      << strProviderAsserId         << "\" ";
						buf <<    "volumeName=\""   << xmlElement[Key_volumeName]      << "\" ";
						if (!stricmp(strContentState.c_str(),CONTENTSTATUS_COMPLETE))
						{
							{
								Ice::Long filesize,supportFileSize;
								sscanf((char*)strFilesize.c_str(), FMT64, &filesize);
								sscanf((char*)strSupportFilesize.c_str(),FMT64, &supportFileSize);

								filesize = filesize / 1024;
								supportFileSize = supportFileSize /1024;

								char tmp[40];
								memset(tmp, 0 , 40);
								sprintf(tmp, FMT64, filesize);
								strFilesize = tmp;

								memset(tmp, 0 , 40);
								sprintf(tmp, FMT64, supportFileSize);
								strSupportFilesize = tmp;
							}
							buf <<    "contentSize=\""       << strFilesize        << "\" ";
							buf <<    "supportFileSize=\""   << strSupportFilesize << "\" ";
							buf <<    "md5Checksum=\""       << strMD5CheckSum     << "\" ";
							buf <<    "md5DateTime=\""       << strMD5DataTime                  << "\" ";
						}
						buf <<    "contentState=\"" << strContentState                      << "\" ";
						buf << "/>\n";
					}
					buf << "</ContentList>\n";
					strMsgContent = buf.str();
				}
				catch (Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content info caught exception [%s]"), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID,request, response, 404, GCISTATUS_404);
					return;
				}
			}
			else if(_env._backStoreType == backCacheStore)///query content info with CacheStroe service
			{
				///list all content of contentstore
				if(strProviderId.empty() && strProviderAsserId.empty())
				{
					setReponseWithoutBody(reqID,request, response, 400, "unsupported for list all content");
					return;
				}
				
				if (!strProviderId.empty() && !strProviderAsserId.empty()&& strProviderId != "*" && strProviderAsserId != "*")
				{
					TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
					if(!cacheStroePrx)
					{
						setReponseWithoutBody(reqID,request, response, 500, "failed to connect to cachestore service");
						return;
					}	

					std::string strMD5DataTime = GenerateUTCTime();
					//std::string strVolumeName = "";
					std::string strFilesize = "", strSupportFilesize = "", strContentState = "", strStampCreated = "", strMD5CheckSum = "", strBitRate="0";

					try
					{
						std::string strContentName = strProviderAsserId + strProviderId;

						if(_A3Config.csOptimize)
						{
							TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
							if(cacheCandidates.size()  > 0)
							{
								cacheStroePrx = cacheCandidates[0].csd.theStore;
							}
						}

						std::string folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);
						folderContentName =  xmlElement[Key_volumeName] + folderContentName + strContentName;

						if( folderContentName.length() > 0 && folderContentName[0] != LOGIC_FNSEPC)
							folderContentName = LOGIC_FNSEPC + folderContentName;
						TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
						TianShanIce::Storage::ContentPrx contentprx = csprx->openContentByFullname(folderContentName);
						TianShanIce::Properties metadatas =  contentprx->getMetaData();

						char tmp[40];
						memset(tmp, 0 , 40);
						::Ice::Long  supportfilesize = contentprx->getSupportFileSize();
						sprintf(tmp, FMT64, supportfilesize);
						strSupportFilesize = tmp;

						memset(tmp, 0 , 40);
						::Ice::Long  filesize = contentprx->getFilesize();	
						sprintf(tmp, FMT64, filesize);
						strFilesize = tmp;

						::TianShanIce::Storage::ContentState contentstate = contentprx->getState();
						strContentState = convertState(contentstate);

						strMD5CheckSum = contentprx->getMD5Checksum();	

						TianShanIce::Properties::iterator itorMD;
						itorMD = metadatas.find("sys.StampCreated");
						if(itorMD != metadatas.end())
						{
							strStampCreated = itorMD->second;
						}

						memset(tmp, 0 , 40);
						::Ice::Int  bitrate = contentprx->getBitRate();	
						sprintf(tmp, "%d", bitrate);
						strBitRate = tmp;
					}
					catch (TianShanIce::ServerError& ex)
					{
						envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content info caught exception [%s]"), ex.message.c_str()); 
						setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
						return;
					}
					catch (Ice::Exception& ex)
					{
						envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content info caught ice exception [%s]"), ex.ice_name().c_str()); 
						setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
						return;
					}

					std::ostringstream buf;
					buf << XML_HEADER ;
					buf << "<ContentList>\n";
					buf << "<ContentInfo " ;
					buf <<    "providerID=\""   << strProviderId     << "\" ";
					buf <<    "assetID=\""      << strProviderAsserId         << "\" ";
					buf <<    "volumeName=\""   << xmlElement[Key_volumeName]      << "\" ";
					if (!stricmp(strContentState.c_str(),CONTENTSTATUS_COMPLETE))
					{
						{
							Ice::Long filesize,supportFileSize;
							sscanf((char*)strFilesize.c_str(), FMT64, &filesize);
							sscanf((char*)strSupportFilesize.c_str(),FMT64, &supportFileSize);

							filesize = filesize / 1024;
							supportFileSize = supportFileSize /1024;

							char tmp[40];
							memset(tmp, 0 , 40);
							sprintf(tmp, FMT64, filesize);
							strFilesize = tmp;

							memset(tmp, 0 , 40);
							sprintf(tmp, FMT64, supportFileSize);
							strSupportFilesize = tmp;
						}
						buf <<    "contentSize=\""       << strFilesize        << "\" ";
						buf <<    "bitrate=\""       << strBitRate        << "\" ";
						buf <<    "supportFileSize=\""   << strSupportFilesize << "\" ";
						buf <<    "md5Checksum=\""       << strMD5CheckSum     << "\" ";
						buf <<    "md5DateTime=\""       << strMD5DataTime                  << "\" ";
					}
					buf <<    "contentState=\"" << strContentState                      << "\" ";
					buf << "/>\n";
					buf << "</ContentList>\n";
					strMsgContent = buf.str();
				}
				else
				{
					setReponseWithoutBody(reqID,request, response, 400, "bad request");
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetContentInfo() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}
				///list all content
				if(strProviderId.empty() && strProviderAsserId.empty())
				{
					setReponseWithoutBody(reqID,request, response, 400, "unsupported for list all content");
					return;
				}

				if (!strProviderId.empty() && !strProviderAsserId.empty()&& strProviderId != "*" && strProviderAsserId != "*")
				{
					TianShanIce::Properties metadatas;
					if(!_env._aquaContentMdata->getMetadataInfo(strProviderAsserId, strProviderId, metadatas))
					{
						envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetContentInfo()[%s]failed to get metadata info]"),
							_env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId).c_str()); 

						if(metadatas.find("sys.State") != metadatas.end() && metadatas["sys.State"] == "NotFound")
						{
							setReponseWithoutBody(reqID,request, response, 404, GCISTATUS_404);
						}
						else
							setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
						return;
					}

					std::string strMD5DataTime = GenerateUTCTime();
					std::string strFilesize = "", strSupportFilesize = "", strContentState = "", strStampCreated = "", strMD5CheckSum = "", strBitRate="0";
					for (TianShanIce::Properties::const_iterator iter = metadatas.begin(); iter != metadatas.end(); iter++)
					{
                        if(iter->first == "sys.FileSize")
						{
							strFilesize = iter->second;
						}
						else if(iter->first == "sys.SupportFileSize")
						{
							strSupportFilesize = iter->second;
						}
						else if(iter->first == "sys.State")
						{
							strContentState = convertState(A3AquaBase::stateId((iter->second).c_str()));
						}
						else if(iter->first == "sys.StampCreated")
						{
							strStampCreated = iter->second;
						}
						else if(iter->first == "sys.MD5CheckSum")
						{
							strMD5CheckSum = iter->second;
						}
						else if(iter->first == "sys.BitRate")
						{
							strBitRate = iter->second;
						}
					}

					std::ostringstream buf;
					buf << XML_HEADER ;
					buf << "<ContentList>\n";
					buf << "<ContentInfo " ;
					buf <<    "providerID=\""   << strProviderId     << "\" ";
					buf <<    "assetID=\""      << strProviderAsserId         << "\" ";
					buf <<    "volumeName=\""   << xmlElement[Key_volumeName]      << "\" ";
					if (!stricmp(strContentState.c_str(),CONTENTSTATUS_COMPLETE))
					{
						buf <<    "contentSize=\""       << strFilesize        << "\" ";
						buf <<    "bitrate=\""       << strBitRate        << "\" ";
						buf <<    "supportFileSize=\""   << strSupportFilesize << "\" ";
						buf <<    "md5Checksum=\""       << strMD5CheckSum     << "\" ";
						buf <<    "md5DateTime=\""       << strMD5DataTime                  << "\" ";
					}
					buf <<    "contentState=\"" << strContentState                      << "\" ";
					buf << "/>\n";
					buf << "</ContentList>\n";
					strMsgContent = buf.str();
				}
				else
				{
					setReponseWithoutBody(reqID,request, response, 400, "bad request");
					return;
				}
			}

			setResponseWithBody(reqID,request, response, 200, "OK", strMsgContent);
  
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "get content info completed took %d ms"), (int)(ZQTianShan::now() - lStart));
		}

		void A3MessageHandler::TransferContent(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do TransferContent()"));
            Ice::Long lStart = ZQTianShan::now();
			/// parse transfer content request
			std::string strMsgContent("");
			StringMap xmlElement;
			StringMap metaDatas;
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			std::string strSourceURL,strResponseURL;
			std::string strProvisionStart, strProvisionEnd;

			request->getContent(strMsgContent);
			if (!parseMsgContentEx(reqID,strMsgContent.c_str(), strMsgContent.length(), xmlElement, metaDatas))
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request");
				return;
			}
			///check parameter key : volumeName
			StringMap::iterator  itorElement;
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_volumeName);
				return;
			}
			strVolume = itorElement->second;

            ///check parameter key : providerID
			itorElement = xmlElement.find(Key_providerID);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_providerID);
				return;
			}
			strProviderId = itorElement->second;

            ///check parameter key : assetID
			itorElement = xmlElement.find(Key_assetID);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_assetID);
				return;
			}
			strProviderAsserId = itorElement->second;

			///check parameter key : sourceURL
			itorElement = xmlElement.find(Key_sourceURL);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_sourceURL);
				return;
			}
			strSourceURL = itorElement->second;

			///check parameter key : responseURL
			itorElement = xmlElement.find(Key_responseURL);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_responseURL);
				return;
			}

			strResponseURL = itorElement->second;
			if(!strResponseURL.empty() && strResponseURL[strResponseURL.length() -1] == '/');
			strResponseURL = strResponseURL.substr(0, strResponseURL.length() -1);
			strResponseURL += "/TransferStatus";

			///check parameter key : transferBitRate
			itorElement = xmlElement.find(Key_transferBitRate);
			if(itorElement == xmlElement.end() || itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_transferBitRate);
				return;
			}
			int nTransferBitRate = atoi(itorElement->second.c_str());


			ZQ::common::URLStr strSrcUrl(strSourceURL.c_str());
			std::string strProtocol = strSrcUrl.getProtocol();

			int64 c = ZQ::common::now();
			char buf[512];
			memset(buf, 0, sizeof(buf));
			std::string startTimeUTC = ZQ::common::TimeUtil::TimeToUTC(c, buf, sizeof(buf) -2);
			//				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

			memset(buf, 0, sizeof(buf));
			std::string endTimeUTC = ZQ::common::TimeUtil::TimeToUTC(c + 3600 * 24 * 1000 * 7, buf, sizeof(buf) -2);

			strProvisionStart = startTimeUTC;
			strProvisionEnd = endTimeUTC;

			///check parameter key :captureStart
			itorElement = xmlElement.find(Key_captureStart);
			if(itorElement == xmlElement.end())
			{
				if(stricmp(strProtocol.c_str(), "udp") == 0)
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_captureStart);
					return;
				}
			}
			else if(!itorElement->second.empty())
			{
				strProvisionStart = itorElement->second;
			}

			///check parameter key :captureEnd
			itorElement = xmlElement.find(Key_captureEnd);
			if(itorElement == xmlElement.end())
			{
				if(stricmp(strProtocol.c_str(), "udp") == 0)
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_captureEnd);
					return;
				}
			}
			else if(!itorElement->second.empty())
			{
				strProvisionEnd = itorElement->second;
			}

			///vailde the source url
			if (stricmp(strProtocol.c_str(), "ftp") &&
				stricmp(strProtocol.c_str(), "nfs") &&
				stricmp(strProtocol.c_str(), "pgm") &&
				stricmp(strProtocol.c_str(), "cifs") &&
				stricmp(strProtocol.c_str(),"udp") &&
				stricmp(strProtocol.c_str(),"c2pull") &&
				stricmp(strProtocol.c_str(), "c2http") &&
				stricmp(strProtocol.c_str(), "aqua") &&
				stricmp(strProtocol.c_str(), "raw"))
			{
				setReponseWithoutBody(reqID, request, response, 451, "Unsupported transfer protocol");
				return;
			}

			if(stricmp(strProtocol.c_str(), "ftp") == 0 ||
				stricmp(strProtocol.c_str(), "nfs") == 0 ||
				stricmp(strProtocol.c_str(), "cifs") == 0)
			{
				// URL format : ftp://user:password@ip:port/filename 
				// URL format : cifs://user:password@ip:port/filename 
				// URL format : nfs://user:password@ip:port/filename 
				size_t pos = strSourceURL.find("//");
				if (pos == std::string::npos)
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}

				std::string urlUserName = strSrcUrl.getUserName();
				std::string urlUserPwd = strSrcUrl.getPwd();

				std::string strUserName = urlUserName;
				std::string strPassWord = urlUserPwd;

				///check parameter key :userName
				if(urlUserName.empty())
				{
					itorElement = xmlElement.find(Key_userName);
					if(urlUserName.empty() && (itorElement == xmlElement.end() || itorElement->second.empty()))
					{
						setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key " Key_userName);
						return;
					}
					else
						strUserName = itorElement->second;
				}

				if(strPassWord.empty())
				{
					///check parameter key :password
					itorElement = xmlElement.find(Key_password);
					if(itorElement == xmlElement.end() || itorElement->second.empty())
					{
						setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content, missed key" Key_password);
						return;
					}
					else
						strPassWord = itorElement->second;
				}

				if(urlUserPwd.empty())
					strSrcUrl.setPwd(strPassWord.c_str());
				if(urlUserName.empty())
					strSrcUrl.setUserName(strUserName.c_str());

				strSourceURL = strSrcUrl.generate();
			}
			if(stricmp(strProtocol.c_str(), "aqua") == 0)
			{
				// URL format : aqua://ip:port/filename 
				size_t pos = strSourceURL.find("//");
				if (pos == std::string::npos)
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}
				std::string filename = strSrcUrl.getPath();
				///fixup filename: delete "/" in the path if need;
				// trim right
				filename.erase(filename.find_last_not_of("/")+1);
				// trim left
				filename.erase(0,filename.find_first_not_of("/"));
				std::string aquaMainFilename = _env._aquaContentMdata->getMainFileName(strProviderAsserId,strProviderId);
                if(filename != aquaMainFilename)
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request invalid source url");
					return;
				}
			}

/*			// create content if no exist
			std::string strLocalType = "";
			std::string strSubType = "";
			StringMap::iterator itorMetadatas;

			itorMetadatas = metaDatas.find("LocalType");
			if(itorMetadatas != metaDatas.end())
				strLocalType =  itorMetadatas->second;

			itorMetadatas = metaDatas.find("SubType");
			if(itorMetadatas != metaDatas.end())
				strSubType =  itorMetadatas->second;

			std::string strContentType = TianShanIce::Storage::ctMPEG2TS;

			if(!strLocalType.empty())
			{
               if(strSubType.empty())
				   strContentType = strLocalType;
			   else
				   strContentType = strLocalType + ":" +strSubType;
			}	
			//	std::string strContentType = (strLocalType != "") ? (strLocalType + ":" +strSubType) : TianShanIce::Storage::ctMPEG2TS;
*/

			std::string strContentType = TianShanIce::Storage::ctMPEG2TS;

			StringMap::iterator itorMetadatas;

			itorMetadatas = metaDatas.find("cscontenttype");
			if(itorMetadatas != metaDatas.end())
				strContentType =  itorMetadatas->second;

			TianShanIce::Storage::ContentPrx contentProxy = NULL;
			TianShanIce::Storage::VolumePrx volumeProxy = NULL;
			std::string strContentName;
			std::string strCPESessionProxy;

			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "TransferConent() source[%s] type[%s] overwrite[%d] bitrate[%d] timeWindow[%s ~ %s]"), 
				strSourceURL.c_str(), strContentType.c_str(), 1, nTransferBitRate, strProvisionStart.c_str(), strProvisionEnd.c_str());

			///transfer content with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}

				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}

				strContentName = xmlElement[Key_assetID] + "_" + xmlElement[Key_providerID];
				try
				{
					TianShanIce::Repository::ContentStoreReplicaPrx  csReplicaprx = clPrx->toStoreReplica(strNetId);
					std::string strCSreplica = _env._communicator->proxyToString(csReplicaprx);
					TianShanIce::Storage::ContentStorePrx contentstoreprx = csReplicaprx->theStore();
					std::string strContentsreplica = _env._communicator->proxyToString(contentstoreprx);
					volumeProxy = contentstoreprx->openVolume(strVolume);
					if (!volumeProxy)
					{
						envlog(ZQ::common::Log::L_WARNING, A3Fmt(A3MessageHandler, "failed to open volume [%s] when transfer content"), xmlElement[Key_volumeName].c_str());
						setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
						return ;
					}
					if ("" == strContentType)
					{
						strContentType = TianShanIce::Storage::ctMPEG2TS;
					}	
					// true : create if not exist;
					contentProxy = volumeProxy->openContent(strContentName, strContentType, true);
				}
				catch(Ice::Exception&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to open volume[%s] caught ice exception[%s]"), xmlElement[Key_volumeName].c_str(), ex.ice_name().c_str()); 

					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}

				// transfer content 
				try
				{
					TianShanIce::Properties metedatas;
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_ResponseURL, strResponseURL));
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_ProviderAssetId, strProviderAsserId));
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_ProviderId, strProviderId));
					contentProxy->setUserMetaData2(metedatas);
					contentProxy->provision(strSourceURL, strContentType, true, strProvisionStart, strProvisionEnd, nTransferBitRate);
				}
				catch(TianShanIce::InvalidStateOfArt& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception [%d,%s]"),strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch(TianShanIce::InvalidParameter&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception [%d,%s]"),strSourceURL.c_str(), ex.errorCode, ex.message.c_str()); 

					setReponseWithoutBody(reqID, request, response, 500, "Internal server error");
					return;
				}
				catch(TianShanIce::ServerError&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%d,%s]"), strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch(TianShanIce::Storage::NoResourceException& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%d,%s]"), strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch (const Ice::Exception& ex)
				{	
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%s] "),strSourceURL.c_str(), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
			}
			else if(_env._backStoreType == backCacheStore)///transfer content with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, "failed to connect to cachestore service");
					return;
				}

				strContentName = strProviderAsserId + strProviderId;
				std::string folderContentName;
                TianShanIce::Storage::FolderPrx folderPxy;
				try
				{
					if(_A3Config.csOptimize)
					{
						TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
						if(cacheCandidates.size()  > 0)
						{
							cacheStroePrx = cacheCandidates[0].csd.theStore;
						}
					}

					folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);

					TianShanIce::Storage::ContentStoreExPrx csprx  =  TianShanIce::Storage::ContentStoreExPrx::checkedCast(cacheStroePrx->theContentStore());
					/// provider specify interface for cache server open volume.
					folderContentName = strVolume + folderContentName;
					if( folderContentName.length() > 0 && folderContentName[0] != LOGIC_FNSEPC)
						folderContentName = LOGIC_FNSEPC + folderContentName;
					folderPxy = csprx->openFolderEx(folderContentName, true, 0);
					if (!folderPxy)
					{
						envlog(ZQ::common::Log::L_WARNING, A3Fmt(A3MessageHandler, "failed to open volume [%s] when transfer content"), strVolume.c_str());
						setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
						return ;
					}
					if ("" == strContentType)
					{
						strContentType = TianShanIce::Storage::ctMPEG2TS;
					}	
					// true : create if not exist;
					contentProxy = folderPxy->openContent(strContentName, strContentType, true);
				}
				catch(TianShanIce::InvalidParameter&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to open folder[%s] or Content [%s] or folder when transfer content caught exception [%d,%s]"),folderContentName.c_str(),strContentName.c_str(), ex.errorCode, ex.message.c_str()); 

					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch(TianShanIce::NotImplemented&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to open folder[%s] or Content [%s] or folder when transfer content caught exception [%d,%s]"),folderContentName.c_str(),strContentName.c_str(), ex.errorCode, ex.message.c_str()); 

					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch(Ice::Exception&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to open open folder[%s] or Content [%s] when transfer content caught exception [%s]"),folderContentName.c_str(),strContentName.c_str(), ex.ice_name().c_str()); 

					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}

				// transfer content 
				try
				{
//					METADATA_PersistentTill(ISO);
					TianShanIce::Properties metedatas;
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_ResponseURL, strResponseURL));
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_ProviderAssetId, strProviderAsserId));
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_ProviderId, strProviderId));

					Ice::Long persistentTill = ZQ::common::now() + 60 * 24 * 3600 * 100;

					std::string strPersistentTill;
					char buf[64] = "";

					strPersistentTill = ZQTianShan::TimeToUTC(persistentTill, buf, sizeof(buf));
					metedatas.insert(TianShanIce::Properties::value_type(METADATA_PersistentTill, strPersistentTill));

					contentProxy->setUserMetaData2(metedatas);
					contentProxy->provision(strSourceURL, strContentType, true, strProvisionStart, strProvisionEnd, nTransferBitRate);
				}
				catch(TianShanIce::InvalidStateOfArt& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception [%d,%s]"),strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch(TianShanIce::InvalidParameter&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%d,%s]"), strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch(TianShanIce::Storage::NoResourceException& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%d,%s]"), strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch(TianShanIce::ServerError&ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%d,%s]"), strSourceURL.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
				catch (const Ice::Exception& ex)
				{	
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to provision content with sourceURL[%s] caught exception[%s]"), strSourceURL.c_str(), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "TransferContent() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

			    /////////////add code here///////////////////////////////////////////////////////////////////////////

				////setp1. find content by ContentName from AuqaServer
				/// strProtocol != "auqa" : if content Exist, return 400
				/// strProtocol == "auqa" : getMedatafile();
				if(stricmp(strProtocol.c_str(), "aqua") == 0)//protocol != aqua, 说明需要将文件上传到AquaServer，首先判断文件是否已经上传，看看主文件和Index文件的状态
				{
					///1)getMetadataInfo获得主文件metadata 信息， 判断上传状态，如果是 inServer状态，找index文件名字，
					///  并且判断Index文件是否存在，如果存在返回 400， 已上
					TianShanIce::Properties metadatas;
					if(_env._aquaContentMdata->getMetadataInfo(strProviderAsserId, strProviderId, metadatas))
					{
						int statId = -1;
						if(metadatas.find("sys.State") != metadatas.end())
						{
							statId = (int)A3AquaBase::stateId(metadatas["sys.State"].c_str());
							envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "metadatas.find state:[%s]"),metadatas["sys.State"].c_str());
						}
						if( statId >= TianShanIce::Storage::csProvisioning && statId <= TianShanIce::Storage::csProvisioningStreamable )
						{
							Json::Value value;
							std::string indexFileNameTmp;
						    std::string strAuqaContentName = _env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId);
							indexFileNameTmp = strAuqaContentName + ".index.tmp";
							CdmiFuseOps::CdmiRetCode retCodeTmp = _env._aquaContentMdata->checkFileStatus(indexFileNameTmp, value);
							envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "file: %s checkFileStatus is %d"),indexFileNameTmp.c_str(),retCodeTmp);
							if(CdmiRet_SUCC(retCodeTmp)) //.index.tmp file already exist.
							{
								envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "When Provisioning OR ProvisioningStreamable, find file: [%s]"),indexFileNameTmp.c_str());
								setReponseWithoutBody(reqID, request, response,400, "bad request for transfer content,content already exist");
								return;
							}
						}
						if(statId == TianShanIce::Storage::csInService)
						{
							Json::Value value;
							std::string indexFileName;
							std::string strAuqaContentName = _env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId);
							indexFileName = strAuqaContentName + ".index";
							CdmiFuseOps::CdmiRetCode retCode = _env._aquaContentMdata->checkFileStatus(indexFileName, value);
							envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "file: %s checkFileStatus is %d"),indexFileName.c_str(),retCode);
							if(CdmiRet_SUCC(retCode)) //.index file already exist.
							{
								envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "When InService, find file: [%s]"),indexFileName.c_str());
								setReponseWithoutBody(reqID, request, response,400, "bad request for transfer content,content already exist");
								return;
							}
						}
					}						
				}
				else//protocol == aqua, 判断主文件是否存在， 不存在，返回，因为需要根据Aqua上的主文件生成index文件
				{
					std::string strAuqaContentName = _env._aquaContentMdata->getMainFileName(strProviderAsserId, strProviderId);
					Json::Value value;
					CdmiFuseOps::CdmiRetCode retCode = _env._aquaContentMdata->checkFileStatus(strAuqaContentName, value);
					envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "checking file[%s]"),strAuqaContentName.c_str());
					if(retCode != CdmiFuseOps::cdmirc_NotFound)
					{
						envlog(ZQ::common::Log::L_WARNING, A3Fmt(A3MessageHandler, "transfer content, checkfile status return[%s]"), CdmiFuseOps::cdmiRetStr(retCode));
						if(CdmiRet_SUCC(retCode))
						{
							setReponseWithoutBody(reqID, request, response,400, "bad request for transfer content,content already exist");
						}
						else
						{
							setReponseWithoutBody(reqID, request, response,500, "Internal server error");
						}
						return;
					}
				}
            
			   ////setp2. 组织CPESession所需要的Metadata以及需要存放在AquaServer上ContentName.metadata的Metadata
				Json::Value vMetadata;
                
				::Ice::Long stampCreated = ZQ::common::TimeUtil::now();
				::Ice::Long stampProvisioned = 0;
				::Ice::Long stampLastUpdated = 0;

				char buf[128];
				memset(buf, 0, sizeof(buf));
				std::string strCreated = ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf));
				memset(buf, 0, sizeof(buf));
				std::string strProvisioned = ZQTianShan::TimeToUTC(stampProvisioned, buf, sizeof(buf));
				memset(buf, 0, sizeof(buf));
				std::string strLastUpdated = ZQTianShan::TimeToUTC(stampLastUpdated, buf, sizeof(buf));

				vMetadata[SYS_PROP(StampCreated)] = strCreated;
				vMetadata[SYS_PROP(StampProvisioned)] = strProvisioned;
				vMetadata[SYS_PROP(StampLastUpdated)] = strLastUpdated;

				::TianShanIce::Storage::ContentState state  = ::TianShanIce::Storage::csNotProvisioned;
				vMetadata[SYS_PROP(State)] = A3AquaBase::stateStr(state);

				strContentName = _env._aquaContentMdata->getContentName(strProviderAsserId , strProviderId);
				vMetadata[SYS_PROP(Name)] = strContentName;

				vMetadata[METADATA_ResponseURL] = strResponseURL;
				vMetadata[METADATA_ProviderAssetId] = strProviderAsserId;
				vMetadata[METADATA_ProviderId] = strProviderId;
				vMetadata[METADATA_SourceUrl]= strSourceURL;
				vMetadata[METADATA_SourceType]= strContentType;
				vMetadata[USER_PROP(Volume)]= strVolume;

				::Ice::Long stopTime = ZQTianShan::ISO8601ToTime(strProvisionEnd.c_str()),
					startTime = ZQTianShan::ISO8601ToTime(strProvisionStart.c_str()),
					stampNow = ZQTianShan::now();

				if (stopTime <=0)
					stopTime = ZQTianShan::ISO8601ToTime("2999-12-31T23:59:59Z");
				if (startTime < stampNow)
					startTime = ((::Ice::Long) ((stampNow +999)/1000)) *1000; // ticket#12077, round up to sec

				if (stopTime <= startTime)
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request for transfer content,captureEnd <= captureStart");
					return;
				}
				std::string strStartUTC,strStopUTC;
				memset(buf,0,sizeof(buf));
				strStartUTC =  ZQTianShan::TimeToUTC(startTime,buf,sizeof(buf));
				memset(buf,0,sizeof(buf));
				strStopUTC = ZQTianShan::TimeToUTC(stopTime,buf,sizeof(buf));


				vMetadata[METADATA_ScheduledProvisonStart]= strStartUTC;
				vMetadata[METADATA_ScheduledProvisonEnd]= strStopUTC;

				memset(buf,0,sizeof(buf));
				std::stringstream os;
				os<< nTransferBitRate;
				os>>buf;
				vMetadata[METADATA_MaxProvisonBitRate] = buf;

				int transferBitrate = nTransferBitRate;
				if (!transferBitrate)
				{
					transferBitrate = _A3Config.defaultProvisionBW;
				}

				bool bNPVRSession = false;

				::TianShanIce::Properties sessMdata;

				if (stricmp(_A3Config.defaultIndexType.c_str(),"VVC") == 0)
				{
					sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVC"));
					vMetadata[METADATA_IndexType] = "VVC";
				}
				else
				{
					sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVX"));
					vMetadata[METADATA_IndexType] = "VVX";
				}

				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERID, strProviderId));
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERASSETID, strProviderAsserId));

				/*if(stricmp(strProtocol.c_str(), "aqua") == 0)
				{
					sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_NOTRICKSPEEDS, "1"));
				}*/

				std::string strFilePathName = strContentName;

				/* std::string strFilePathName = _A3Config.mainFilePath  + strContentName;

				//remove the first possible '\' or "\\"
				if (strFilePathName[0]=='\\' || strFilePathName[0]=='/')
					strFilePathName = strFilePathName.substr(1);
					*/

				TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
				/*if(_A3Config.enableRaw)
				{
					contentKey.content = _A3Config.mainFilePath  + strContentName;;
				}
				else*/
				contentKey.content = strContentName;
				contentKey.contentStoreNetId = "";
				contentKey.volume = strVolume;

				if(NULL == _env._cpWrapper)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to transfer content: ContentProvisionWrapper handle is NULL"));
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}


				////setp3.在AquaServer上创建ContentName.mdata文件，并将vMetadata内容写入
				std::string strContentMdataFile = _env._aquaContentMdata->getAquaContentMDName(strProviderAsserId, strProviderId);
				if(!_env._aquaContentMdata->updateMetadata(strContentMdataFile, vMetadata))
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create MetadateFile[%s]"), strContentMdataFile.c_str());
					setReponseWithoutBody(reqID, request, response,500, "Internal server error");
					return;
				}

				////setp4. 创建CPESession， 并将 sys.CPE.NetId 和sys.ProvSess存放在 ContentName.metadata的Metadata中
				TianShanIce::Properties outProps;
				TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = NULL;
				try
				{
					 pPrx = _env._cpWrapper->activeProvision(
						NULL,
						contentKey,
						strFilePathName,	
						strSourceURL,
						strContentType, 
						strStartUTC,
						strStopUTC, 
						transferBitrate,
						sessMdata,
						outProps,
						bNPVRSession);
				}
				catch (const TianShanIce::InvalidParameter& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session with error [InvalidParameter: %s]"), ex.message.c_str());
					pPrx =  NULL;
				}
				catch (const TianShanIce::ServerError& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session with error [ServerError: %s]"), ex.message.c_str());
					pPrx =  NULL;
				}
				catch (const TianShanIce::Storage::NoResourceException& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session with error [NoResourceException: %s]"), ex.message.c_str());
					pPrx =  NULL;
				}
				catch (const TianShanIce::InvalidStateOfArt& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session with error [InvalidStateOfArt: %s]"), ex.message.c_str());
					pPrx =  NULL;
				}
				catch (const Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session with error [%s]"), ex.ice_name().c_str());
					pPrx =  NULL;
				}
				catch (...)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session with unkonw error"));
					pPrx =  NULL;
				}

				if(pPrx == NULL)
				{
					_env._aquaContentMdata->deleteFile(strContentMdataFile);
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to create provision session"));
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}

				vMetadata.clear();
				for(TianShanIce::Properties::iterator itor = outProps.begin(); itor != outProps.end(); itor++)
				{
					vMetadata[itor->first] = itor->second;
				}
				std::string strCPENetId = sessMdata["sys.CPE.NetId"];
				if (!strCPENetId.empty())
				{
					vMetadata["sys.CPE.NetId"] = strCPENetId;
				}

				strCPESessionProxy = _env._communicator->proxyToString(pPrx);
				vMetadata["sys.ProvSess"] = strCPESessionProxy;

				////setp5.更新ContentName.mdata文件，并将vMetadata内容写入
				_env._aquaContentMdata->updateMetadata(strContentMdataFile, vMetadata);

				////setp6.更新AquaServer上A3SessionList.bin文件， 将新的Session写进去
				envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "update cpe session proxy[%s] in sessionlist file"), strCPESessionProxy.c_str());
				_env._cpeSessionMgr->addSession(strContentName, strCPESessionProxy);
			}
		
			ContentInfo contentInfo;
			contentInfo.responseUrl = strResponseURL;
			contentInfo.pid = strProviderId;
			contentInfo.paid = strProviderAsserId;
			contentInfo.sessionCPEProxy = strCPESessionProxy;
			_env.addContentInfo(strContentName, contentInfo);

			setReponseWithoutBody(reqID, request, response, 200, "Server accepted the distribution request");
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "Transfering Content volume[%s]contentname[%s]completed took %d ms"), strVolume.c_str(), strContentName.c_str(), (int)(ZQTianShan::now() - lStart));
		}
		void A3MessageHandler::CancelTransfer(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do CancelTransfer()"));
            Ice::Long lStart = ZQTianShan::now();
			// parse and valide request
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			StringMap xmlElement;
			request->getContent(strMsgContent);
			if (!parseMsgContent(reqID, strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request");
				return;
			}
			StringMap::iterator  itorElement;
			///check parameter key : volumeName
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end() && itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for cancel transfer");
				return;
			}
			strVolume = itorElement->second;
			///check parameter key : providerID
			itorElement = xmlElement.find(Key_providerID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for cancel transfer");
				return;

			}
			strProviderId = itorElement->second;

			///check parameter key : assetID
			itorElement = xmlElement.find(Key_assetID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for cancel transfer");
				return;

			}
			strProviderAsserId = itorElement->second;

		    std::string strContentName = "";
			///query cancel transfer with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}

				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				// cancel provision
				try
				{
					
					if(!getContentName(strNetId, strVolume, strProviderId, strProviderAsserId, strContentName))
					{
						setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
						return;
					}
					std::string contentReplicaId = strNetId +"$" + strVolume + "/" + strContentName;
					TianShanIce::Repository::ContentReplicaPrx contentreplicaprx = clPrx->toContentReplica(contentReplicaId);
					TianShanIce::Storage::ContentPrx contentProxy =  contentreplicaprx->theContent();
					contentProxy->cancelProvision();
				}
				catch(const TianShanIce::InvalidStateOfArt& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel transfer content[%s] caught excepiton [%d,%s]"),strContentName.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch (const Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel transfer content[%s] caught excepiton[%s]"),strContentName.c_str(), ex.ice_name().c_str());

					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
			}
			else if(_env._backStoreType == backCacheStore) ///query cancel transfer info with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, "failed to connect to cachestore service");
					return;
				}
				strContentName = strProviderAsserId + strProviderId;
				try
				{
					if(_A3Config.csOptimize)
					{
						TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
						if(cacheCandidates.size()  > 0)
						{
							cacheStroePrx = cacheCandidates[0].csd.theStore;
						}
					}

					std::string folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);
					folderContentName = xmlElement[Key_volumeName] + folderContentName +strContentName;
					if( folderContentName.length() > 0 && folderContentName[0] != LOGIC_FNSEPC)
						folderContentName = LOGIC_FNSEPC + folderContentName;
					TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
					TianShanIce::Storage::ContentPrx contentprx = csprx->openContentByFullname(folderContentName);
                    contentprx->cancelProvision();
				}
				catch(const TianShanIce::InvalidStateOfArt& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel transfer content[%s] caught excepiton [%d,%s]"),strContentName.c_str(), ex.errorCode, ex.message.c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch(const TianShanIce::ServerError& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel transfer content[%s] info caught exception [%s]"),strContentName.c_str(), ex.message.c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch(const Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel transfer content[%s] info caught exception [%s]"),strContentName.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "CancelTransfer() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

				strContentName = _env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId);

				std::string strCPESessionProxy;
				ContentInfo contInfo = _env.getContentInfo(strContentName);

				if(contInfo.sessionCPEProxy.empty())
				{
					TianShanIce::Properties sessionList;
					if(!_env._cpeSessionMgr->getSessionList(sessionList) || sessionList.find(strContentName) == sessionList.end())
					{
						envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel transfer content[%s],provisioning session not exist"),strContentName.c_str()); 
						setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
						return;
					}
					strCPESessionProxy = sessionList[strContentName];
				}
				else
					strCPESessionProxy = contInfo.sessionCPEProxy;

				std::string strMDFile = _env._aquaContentMdata->getAquaContentMDName(strProviderAsserId, strProviderId);
				try 
				{
					ProvisionSessionPrx session = ProvisionSessionPrx::uncheckedCast(_env._communicator->stringToProxy(strCPESessionProxy));
					session->cancel(0, "");
					envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "[%s]provisioning session[%s] canceled"), strContentName.c_str(), strCPESessionProxy.c_str()); 
				}
				catch(const IceUtil::NullHandleException&) 
				{
					/*envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "provision session[%s] not found for content(%s) while cancelProvision()"),
						strCPESessionProxy.c_str(),strContentName.c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
                    return;*/
					
				}
				catch(const Ice::Exception& ex) 
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to cancel provision session[%s] for content(%s): (%s)"),
						strCPESessionProxy.c_str(), strContentName.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				_env._aquaContentMdata->deleteFile(strMDFile);
				_env._cpeSessionMgr->removeSession(strContentName);
			}

			setReponseWithoutBody(reqID, request, response, 200, "OK (Transfer canceled)");
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "Cancel Transfer complete took %d ms"), (int)(ZQTianShan::now() - lStart));
		}
		void A3MessageHandler::DeleteContent(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do DeleteContent()"));
            Ice::Long lStart = ZQTianShan::now();
			// parse and valide request
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			StringMap xmlElement;
			std::string strReason;
			int reasonCode;

			request->getContent(strMsgContent);

			if (!parseMsgContent(reqID, strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request");
				return; 
			}
             
			StringMap::iterator  itorElement;
			///check parameter key : volumeName
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end() && itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for delete content");
				return;
			}
			strVolume = itorElement->second;

			///check parameter key : providerID
			itorElement = xmlElement.find(Key_providerID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for delete content");
				return;

			}
			strProviderId = itorElement->second;

			///check parameter key : assetID
			itorElement = xmlElement.find(Key_assetID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for  delete content");
				return;

			}
			strProviderAsserId = itorElement->second;

			///check parameter key : reasonCode
			itorElement = xmlElement.find(Key_reasonCode);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for  delete content");
				return;

			}

			reasonCode = atoi(itorElement->second.c_str());
			strReason =  GetDeleteContentReason(reasonCode);


			///query destroy content with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}
				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				std::string strContentName = "";
				// destroy provision
				try
				{	
					if(!getContentName(strNetId, strVolume, xmlElement[Key_providerID], xmlElement[Key_assetID], strContentName))
					{
						setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
						return;
					}
					std::string contentReplicaId = strNetId +"$" + strVolume + "/" + strContentName;
					TianShanIce::Repository::ContentReplicaPrx contentreplicaprx = clPrx->toContentReplica(contentReplicaId);
					TianShanIce::Storage::ContentPrx contentProxy =  contentreplicaprx->theContent();
					contentProxy->destroy();
					contentreplicaprx->destroy(strReason);
				}
				catch (const TianShanIce::InvalidStateOfArt& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to destory content [%s]caught exception [%s]"), strContentName.c_str(), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
				catch (const Ice::Exception& ex)
				{	
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to destory content [%s]caught exception [%s]"), strContentName.c_str(), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
			}
			else  if(_env._backStoreType == backCacheStore)///query destroy content with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				std::string strContentName =strProviderAsserId + strProviderId;
				try
				{
					if(_A3Config.csOptimize)
					{
						TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
						if(cacheCandidates.size()  > 0)
						{
							cacheStroePrx = cacheCandidates[0].csd.theStore;
						}
					}

					std::string folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);	
					folderContentName = strVolume + folderContentName + strContentName;
					TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
					TianShanIce::Storage::ContentPrx contentprx = csprx->openContentByFullname(folderContentName);	
					contentprx->destroy();
				}
				catch (TianShanIce::ServerError& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to destory content [%s]caught exception [%s]"),strContentName.c_str(), ex.message.c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch (Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to destory content [%s]caught exception [%s]"),strContentName.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "DeleteContent() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

				TianShanIce::Properties metadatas;
				if(!_env._aquaContentMdata->getMetadataInfo(strProviderAsserId, strProviderId, metadatas))
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "DeleteContent()[%s]failed to get metadata info]"),_env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId).c_str()); 
					
					if(metadatas.find("sys.State") != metadatas.end() && metadatas["sys.State"] == "NotFound")
					{
						setReponseWithoutBody(reqID,request, response, 404, GCISTATUS_404);
					}
					else
						setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
					return;
				}
				TianShanIce::StrValues filelist;
				std::string fileExtentionnames, contentName;

				TianShanIce::Properties::iterator itor = metadatas.find("sys.Name");
				if(itor == metadatas.end())
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "DeleteContent[%s]failed to find sys.Name key]"),_env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId).c_str()); 
					setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
					return;
				}
				contentName = metadatas["sys.Name"];

				itor = metadatas.find("sys.memberFileExts");
				if(itor == metadatas.end())
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "DeleteContent[%s]failed to find sys.memberFileExts key]"),contentName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
					return;
				}
				fileExtentionnames = metadatas["sys.memberFileExts"];

	/*			itor = metadatas.find("sys.IndexFileName");
				if(itor == metadatas.end())
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "DeleteContent[%s]failed to find sys.IndexFileName key]"),contentName.c_str()); 
				}
				else
				{
					///add index file to deletefile list
					std::string indexFile =  metadatas["sys.IndexFileName"];
					int npos = indexFile.rfind("/");
					if(npos > 0 )
						filelist.push_back( indexFile.substr(npos +1));
					else
						filelist.push_back(indexFile);
				}
  */              
				std::string tempStr = fileExtentionnames;
				///add main file FF FR file to deletefile list
				{
					int npos = tempStr.find(";");
					while(npos >= 0)
					{
						if(npos == 0)
						{
							if(_A3Config.deleteMainFile) //判断是否要将主文件加入到deletelist中
								filelist.push_back(contentName); //main file, no Extentionnames
						}
						else
						{
							std::string strExtention = tempStr.substr(0, npos);

							if(_A3Config.mainFileExtension != std::string("." + strExtention) || _A3Config.deleteMainFile )//（非主文件） 或者 （是主文件但是deleteMainFile=1）
								filelist.push_back(contentName  + "." + strExtention);
						}
						tempStr = tempStr.substr(npos+1);
						npos = tempStr.find(";");	
					}
					if(!tempStr.empty())
						filelist.push_back(contentName + "."  + tempStr);
				}

				///add .mdata file to deletefile list
				filelist.push_back(_env._aquaContentMdata->getAquaContentMDName(strProviderAsserId, strProviderId));
				for(TianShanIce::StrValues::iterator itorfile = filelist.begin(); itorfile != filelist.end(); itorfile++)
				{
					envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "DeleteContent[%s] delete file[%s]"),contentName.c_str(), (*itorfile).c_str()); 
					_env._aquaContentMdata->deleteFile(*itorfile);
				}
			}

			if( 0 == _A3Config.deleteMainFile)
			{
				///如果是主文件不删除的情况下，对于已经上传后又删除的主文件的Metadata要更新成 metadata_state["sys.State"] = "NotProvisioned";
				TianShanIce::Properties metadata_state;
				int cdmiCode = 0;
				bool bMainFileMd = _env._aquaContentMdata->getMainFileMetadata(strProviderAsserId,strProviderId,metadata_state, cdmiCode);
				if(bMainFileMd)
				{
					if(metadata_state.find("sys.State") != metadata_state.end())
					{
						metadata_state["sys.State"] = "NotProvisioned";
						_env._aquaContentMdata->updateMainFileMetadata(strProviderAsserId,strProviderId, metadata_state);
					}
				}
			}
			setReponseWithoutBody(reqID, request, response, 200, "OK");
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "Delete Content() complete took %d ms"), (int)(ZQTianShan::now() - lStart));
		}
		void A3MessageHandler::GetContentChecksum(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do GetContentChecksum"));
            Ice::Long lStart = ZQTianShan::now();
			// parse and valide request
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			std::string strResponseURL;
			StringMap xmlElement;
			request->getContent(strMsgContent);
			if (!parseMsgContent(reqID, strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request");
				return; 
			}

			StringMap::iterator  itorElement;
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end() && itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get content checksum");
				return;
			}
			strVolume = itorElement->second;

			itorElement = xmlElement.find(Key_providerID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get content checksum");
				return;

			}
			strProviderId = itorElement->second;

			itorElement = xmlElement.find(Key_assetID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get content checksum");
				return;

			}
			strProviderAsserId = itorElement->second;

			itorElement = xmlElement.find(Key_responseURL);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get content checksum");
				return;

			}
			strResponseURL = xmlElement[Key_responseURL];
			if(!strResponseURL.empty() && strResponseURL[strResponseURL.length() -1] == '/');
			strResponseURL = strResponseURL.substr(0, strResponseURL.length() -1);

			strResponseURL += "/ContentChecksum";

			std::string strMD5CheckSum = "";
			std::string strContentState = "";
			///query get content checksum info with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}

				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
				// Get Content Checksum
				try
				{
					TianShanIce::Repository::MetaObjectInfos infos;
					TianShanIce::Properties prop;
					TianShanIce::StrValues listMetadata;
					// 				MAPSET(TianShanIce::Properties, prop, Key_volumeName, strVolume);
					// 				MAPSET(TianShanIce::Properties, prop, Key_netId, strNetId);
					// 				MAPSET(TianShanIce::Properties, prop, "objectType", "ContentReplica");
					// 				MAPSET(TianShanIce::Properties, prop, "user.ProviderId", xmlElement[Key_providerID]);
					// 				MAPSET(TianShanIce::Properties, prop, "user.ProviderAssetId", xmlElement[Key_assetID]);

					listMetadata.push_back("sys.MD5CheckSum");
					listMetadata.push_back("contentState");

					infos = clPrx->locateContentByPIDAndPAID(strNetId, strVolume, xmlElement[Key_providerID], xmlElement[Key_assetID], listMetadata);

					if(infos.size() != 1)
					{
						setReponseWithoutBody(reqID, request, response, 404, "Content Not Found");
						return;
					}
					TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin();
					for (::TianShanIce::Repository::MetaDataMap::const_iterator iter = it->metaDatas.begin(); iter != it->metaDatas.end(); iter++)
					{
						if(iter->first == "sys.MD5CheckSum")
						{
							strMD5CheckSum = iter->second.value;
						}
						else if(iter->first == "contentState")
						{
							strContentState = iter->second.value;
						}
					}

				}
				catch (const Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content checksum caught exception [%s]"), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}

			}
			else  if(_env._backStoreType == backCacheStore)///query get content checksum info with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, "failed to connect to cachestore service");
					return;
				}
				std::string strContentName = strProviderAsserId + strProviderId;
				try
				{
					if(_A3Config.csOptimize)
					{
						TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
						if(cacheCandidates.size()  > 0)
						{
							cacheStroePrx = cacheCandidates[0].csd.theStore;
						}
					}

					std::string folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);
					folderContentName = strVolume + folderContentName +strContentName;
					if( folderContentName.length() > 0 && folderContentName[0] != LOGIC_FNSEPC)
						folderContentName = LOGIC_FNSEPC + folderContentName;
					TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
					TianShanIce::Storage::ContentPrx contentprx = csprx->openContentByFullname(folderContentName);	
					::TianShanIce::Storage::ContentState contentstate = contentprx->getState();
					strContentState = convertState(contentstate);

					strMD5CheckSum = contentprx->getMD5Checksum();	
				}
				catch (TianShanIce::ServerError& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content[%s] checksum parameter caught exception [%s]"),strContentName.c_str(), ex.message.c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch (Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content[%s] checksum parameter caught exception [%s]"),strContentName.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
			}
			else  if(_env._backStoreType == backAuqaServer)
			{
				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetContentChecksum() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

				TianShanIce::Properties metadatas;
				if(!_env._aquaContentMdata->getMetadataInfo(strProviderAsserId, strProviderId, metadatas))
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetContentChecksum()[%s]failed to get metadata info]"),
						_env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId).c_str()); 

					if(metadatas.find("sys.State") != metadatas.end() && metadatas["sys.State"] == "NotFound")
					{
						setReponseWithoutBody(reqID,request, response, 404, GCISTATUS_404);
					}
					else
						setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);

					return;
				}
				TianShanIce::Properties::iterator itorMd;
				
				if(metadatas.find(METADATA_MD5CheckSum) != metadatas.end())
				{
					strMD5CheckSum = metadatas[METADATA_MD5CheckSum];
				}
				if(metadatas.find("sys.State") != metadatas.end())
				{
					strContentState = convertState(A3AquaBase::stateId(metadatas["sys.State"].c_str()));
				}
			}
			
			int resultCode = 404;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<ContentChecksum ";
			buf <<    "providerID=\""   << strProviderId   << "\" ";
			buf <<    "assetID=\""      << strProviderAsserId      << "\" ";
			buf <<    "volumeName=\""   << strVolume   << "\" ";
			if (!stricmp(strContentState.c_str(),CONTENTSTATUS_COMPLETE))
			{
				resultCode = 200;
				buf <<    "md5Checksum=\""  << strMD5CheckSum    << "\" ";
				buf <<    "md5DateTime=\""  << GenerateUTCTime() << "\" ";
			}
			buf <<    "resultCode=\""   <<  resultCode           << "\" ";
			buf << "/>";

			std::string strContent = buf.str();

			// set response 
			setReponseWithoutBody(reqID, request, response, 200, "OK Async request accepted");

			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "send content Checksum reponse info[%s] to responseURL[%s]"), strContent.c_str(), strResponseURL.c_str());
			_env._A3Client->SendRequest(strProviderAsserId+ strProviderId, strResponseURL, strContent);
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "get content checksum complete took %d ms"), (int)(ZQTianShan::now() - lStart));

		}
		void A3MessageHandler::ExposeContent(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do ExposeContent()"));
            Ice::Long lStart = ZQTianShan::now();
			// parse and valide request
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			std::string strProtocol, strSourceURL;
			int         transferBitrate;
			StringMap xmlElement;
			request->getContent(strMsgContent);
			if (!parseMsgContent(reqID, strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request");
				return;
			}

			///check parameter key : volumeName
			StringMap::iterator  itorElement;
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for expose content");
				return;
			}
			strVolume = itorElement->second;

			///check parameter key : providerID
			itorElement = xmlElement.find(Key_providerID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for expose content");
				return;
			}
			strProviderId = itorElement->second;

			///check parameter key : assetID
			itorElement = xmlElement.find(Key_assetID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for expose content");
				return;
			}
			strProviderAsserId = itorElement->second;

			///check parameter key : protocol
			itorElement = xmlElement.find(Key_protocol);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for expose content");
				return;
			}
			strProtocol = itorElement->second;

			///check parameter key : transferBitRate
			itorElement = xmlElement.find(Key_transferBitRate);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for expose content");
				return;
			}
			transferBitrate = atoi(itorElement->second.c_str());

			///vailde the source url
			if ((strProtocol.find(TianShanIce::Storage::potoFTP)  == std::string::npos) &&
				(strProtocol.find(TianShanIce::Storage::potoNFS)  == std::string::npos) &&
				(strProtocol.find(TianShanIce::Storage::potoPGM)  == std::string::npos) &&
				(strProtocol.find(TianShanIce::Storage::potoCIFS) == std::string::npos) &&
				(strProtocol.find("aqua")  == std::string::npos) &&
				(strProtocol.find("udp") == std::string::npos) &&
				(strProtocol.find("c2pull") == std::string::npos)&&
				(strProtocol.find("c2http") == std::string::npos))
			{
				setReponseWithoutBody(reqID, request, response, 451, ECSTATUS_451);
				return;
			}

			// get expose url
			int ttl = 0;
			std::string strURL;
			TianShanIce::Properties addtionalInfos;
            TianShanIce::Storage::ContentPrx contentprx;
			std::string strContentName = "";
			///query get expose url with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}

				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}

				try
				{
					if(!getContentName(strNetId, strVolume, strProviderId, strProviderAsserId, strContentName))
					{
						setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
						return;
					}
					std::string contentReplicaId = strNetId +"$" + strVolume + "/" + strContentName;
					TianShanIce::Repository::ContentReplicaPrx contentreplicaprx = clPrx->toContentReplica(contentReplicaId);
					contentprx =  contentreplicaprx->theContent();

					strURL = contentprx->getExportURL(strProtocol, transferBitrate, ttl, addtionalInfos);
				}
				catch (const Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to expose content[%s] caught exception [%s]"), strContentName.c_str(), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
			}
			else  if(_env._backStoreType == backCacheStore)///query get transferstatus info with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, "failed to connect to cachestore service");
					return;
				}
				try
				{
					strContentName = strProviderAsserId + strProviderId;

					if(_A3Config.csOptimize)
					{
						TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
						if(cacheCandidates.size()  > 0)
						{
							cacheStroePrx = cacheCandidates[0].csd.theStore;
						}
					}

					std::string folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);
					folderContentName = xmlElement[Key_volumeName] + folderContentName +strContentName;
					if( folderContentName.length() > 0 && folderContentName[0] != LOGIC_FNSEPC)
						folderContentName = LOGIC_FNSEPC + folderContentName;
					TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
					contentprx = csprx->openContentByFullname(folderContentName);	
					//TianShanIce::Storage::ContentState contentstate = contentprx->getState();
					strURL = contentprx->getExportURL(strProtocol, transferBitrate, ttl, addtionalInfos);
				}
				catch (TianShanIce::ServerError& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to expose content[%s] caught exception [%s]"),strContentName.c_str(), ex.message.c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch (Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to expose content[%s] caught exception [%s]"),strContentName.c_str(), ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
			//	setReponseWithoutBody(reqID,request, response, 500, "unsupported method");
			//	return;
				std::string mainFileName = _env._aquaContentMdata->getMainFileName(strProviderAsserId, strProviderId);
				strURL = _A3Config.exposeURL +  mainFileName;

			}

			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "success to expose content [%s] with url[%s]"), strContentName.c_str(), strURL.c_str());

			std::string strOriURL = "";
			std::string strUserName ="";
			std::string strPassword ="";
			if(strProtocol == TianShanIce::Storage::potoFTP || 
				strProtocol == TianShanIce::Storage::potoCIFS)
			{
				// ftp://aa:aa@192.168.81.98:21/filname
				// find user name
				size_t nStart = strURL.find("//");
				if (std::string::npos == nStart)
				{
					setReponseWithoutBody(reqID, request, response, 451, ECSTATUS_451);
					return;
				}
				size_t nEnd = strURL.find(":", nStart);
				if (std::string::npos == nEnd)
				{
					setReponseWithoutBody(reqID, request, response, 451, ECSTATUS_451);
					return;
				}
				nStart += strlen("//");
				strUserName = strURL.substr(nStart, nEnd - nStart);

				// find password
				nStart = nEnd + strlen(":");
				nEnd = strURL.find("@", nStart);
				if (std::string::npos == nEnd)
				{
					setReponseWithoutBody(reqID, request, response, 451, ECSTATUS_451);
					return;
				}
				strPassword = strURL.substr(nStart, nEnd - nStart);

				// find url
				nStart = nEnd + strlen("@");
				strOriURL = strProtocol + "://" + strURL.substr(nStart);
			}
			else
			{
				strOriURL = strURL;
			}
			std::string expTransBitrate = "";
			TianShanIce::Properties::iterator itorProp = addtionalInfos.find(TianShanIce::Storage::expTransferBitrate);
			if(itorProp != addtionalInfos.end())
				expTransBitrate = itorProp->second;
			
			std::ostringstream buf;
			buf << XML_HEADER;
			buf << "<ExposeResponse " ;
			buf <<    "providerID=\""   << strProviderId      << "\" ";
			buf <<    "assetID=\""      << strProviderAsserId << "\" ";
			buf <<    "URL=\""          << strOriURL          << "\" ";
			buf <<    "userName=\""     << strUserName        << "\" ";
			buf <<    "password=\""     << strPassword        << "\" ";
			buf <<    "ttl=\""          << ttl                << "\" ";
			buf <<    "transferBitRate=\"" << expTransBitrate << "\" ";
			buf << "/>";
			strMsgContent = buf.str();
			setResponseWithBody(reqID, request, response, 200, "OK", strMsgContent);
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "Expose Content complete took %d ms"), (int)(ZQTianShan::now() - lStart));

		}
		void A3MessageHandler::GetTransferStatus(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response)
		{
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "do GetTransferStatus()"));

            Ice::Long lStart = ZQTianShan::now();

			/// parse and valide request 
			std::string strMsgContent("");
			std::string strNetId("");
			std::string strVolume("");
			std::string strProviderId = "", strProviderAsserId = "";
			StringMap xmlElement;
			request->getContent(strMsgContent);
			if (!parseMsgContent(reqID, strMsgContent.c_str(), strMsgContent.length(), xmlElement))
			{
				setReponseWithoutBody(reqID, request, response, 400, "bad request");
				return;
			}
           
			///check parameter key : volumeName
			StringMap::iterator  itorElement;
			itorElement = xmlElement.find(Key_volumeName);
			if(itorElement == xmlElement.end() && itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get transfer status");
				return;
			}

			strVolume = itorElement->second;
			///check parameter key : providerID
			itorElement = xmlElement.find(Key_providerID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get transfer status");
				return;
				
			}
            strProviderId = itorElement->second;

			///check parameter key : assetID
			itorElement = xmlElement.find(Key_assetID);
			if(itorElement == xmlElement.end()|| itorElement->second.empty())
			{
				setReponseWithoutBody(reqID, request, response, 400, "invaild request for get transfer status");
				return;
			
			}
			strProviderAsserId = itorElement->second;

	        /// get transfer status info 
			int reasonCode = 200;
			std::string strMD5DataTime = GenerateUTCTime();
			std::string strFilesize = "", strSupportFilesize = "", strContentState = "", strMD5CheckSum = "", strBitRate="0";

			Ice::Long  bitRate = 0, duration = 0,filesize = 0;

			///query get transfer status info with contentlib service 
			if(_env._backStoreType == backContentLib)
			{
				if(!getVolumeNameAndId(xmlElement[Key_volumeName], strNetId, strVolume))
				{
					setReponseWithoutBody(reqID, request, response, 400, "bad request");
					return;
				}
				const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
				if(!clPrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
				try
				{
					TianShanIce::Repository::MetaObjectInfos infos;
					TianShanIce::Properties prop;
					TianShanIce::StrValues listMetadata;
					// MAPSET(TianShanIce::Properties, prop, Key_volumeName, strVolume);
					// MAPSET(TianShanIce::Properties, prop, Key_netId, strNetId);
					// MAPSET(TianShanIce::Properties, prop, "objectType", "ContentReplica");
					// MAPSET(TianShanIce::Properties, prop, "user.ProviderId",  xmlElement[Key_providerID]);
					// MAPSET(TianShanIce::Properties, prop, "user.ProviderAssetId", xmlElement[Key_assetID]);

					listMetadata.push_back("sys.FileSize");
					listMetadata.push_back("sys.SupportFileSize");
					listMetadata.push_back("contentState");
					listMetadata.push_back("sys.MD5CheckSum");
					listMetadata.push_back("sys.BitRate");

					infos = clPrx->locateContentByPIDAndPAID(strNetId, strVolume, strProviderId, strProviderAsserId, listMetadata);

					if(infos.size() != 1)
					{
						setReponseWithoutBody(reqID, request, response, 404, GTSSTATUS_404);
						return ;
					}
					TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin();
					for (::TianShanIce::Repository::MetaDataMap::const_iterator iter = it->metaDatas.begin(); iter != it->metaDatas.end(); iter++)
					{
						if(iter->first == METADATA_FileSize)
						{
							strFilesize = iter->second.value;
						}
						else if(iter->first == METADATA_SupportFileSize)
						{
							strSupportFilesize = iter->second.value;
						}
						else if(iter->first == "contentState")
						{
							strContentState = iter->second.value;
						}
						else if(iter->first == METADATA_MD5CheckSum)
						{
							strMD5CheckSum = iter->second.value;
						}
						else if(iter->first == METADATA_BitRate)
						{
							strBitRate = iter->second.value;
						}
					}

				}
				catch (const Ice::Exception& ex)
				{
					reasonCode = 500;
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get transfer status caught exception [%s]"), ex.ice_name().c_str());
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return ;
				}
			}
			else  if(_env._backStoreType == backCacheStore)///query get transfer status info with CacheStroe service
			{
				TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
				if(!cacheStroePrx)
				{
					setReponseWithoutBody(reqID, request, response, 500, "failed to connect to cachestore service");
					return;
				}
				std::string strContentName = strProviderAsserId + strProviderId ;
				try
				{
					if(_A3Config.csOptimize)
					{
						TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strContentName, false);
						if(cacheCandidates.size()  > 0)
						{
							cacheStroePrx = cacheCandidates[0].csd.theStore;
						}
					}

					std::string folderContentName = cacheStroePrx->getFolderNameOfContent(strContentName);
					folderContentName = xmlElement[Key_volumeName] + folderContentName +strContentName;
					if( folderContentName.length() > 0 && folderContentName[0] != LOGIC_FNSEPC)
						folderContentName = LOGIC_FNSEPC + folderContentName;
					TianShanIce::Storage::ContentStorePrx csprx =  cacheStroePrx->theContentStore();
					TianShanIce::Storage::ContentPrx contentprx = csprx->openContentByFullname(folderContentName);
					//TianShanIce::Properties metadatas =  contentprx->getMetaData();

					char tmp[40];
					memset(tmp, 0 , 40);
					::Ice::Long  supportfilesize = contentprx->getSupportFileSize();
					sprintf(tmp, FMT64, supportfilesize);
					strSupportFilesize = tmp;

					memset(tmp, 0 , 40);
					::Ice::Long  filesize = contentprx->getFilesize();	
					sprintf(tmp, FMT64, filesize);
					strFilesize = tmp;

					::TianShanIce::Storage::ContentState contentstate = contentprx->getState();
					strContentState = convertState(contentstate);

					strMD5CheckSum = contentprx->getMD5Checksum();	

					memset(tmp, 0 , 40);
					::Ice::Long  bitRate= contentprx->getBitRate();	
					sprintf(tmp, FMT64, bitRate);
					strBitRate = tmp;
				}
				catch (TianShanIce::ServerError& ex)
				{
					reasonCode = 500;
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content[%s] transfer status caught exception [%s]"),strContentName.c_str(), ex.message.c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
				catch (Ice::Exception& ex)
				{
					reasonCode = 500;
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to get content[%s] transfer status exception [%s]"),strContentName.c_str(),  ex.ice_name().c_str()); 
					setReponseWithoutBody(reqID, request, response, 500, GCISTATUS_500);
					return;
				}
			}
			else if(_env._backStoreType == backAuqaServer)
			{
				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetTransferStatus() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

				if(!_A3Config.volumeName.empty() && _A3Config.volumeName != strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetTransferStatus() invalid volumeName[%s], configration VolumeName is [%s]"),strVolume.c_str(), _A3Config.volumeName.c_str()); 
					setReponseWithoutBody(reqID,request, response, 400, "bad request, invalid volumeName");
					return;
				}

				TianShanIce::Properties metadatas;
				if(!_env._aquaContentMdata->getMetadataInfo(strProviderAsserId, strProviderId, metadatas))
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "GetTransferStatus()[%s]failed to get metadata info]"),
						_env._aquaContentMdata->getContentName(strProviderAsserId, strProviderId).c_str()); 

					if(metadatas.find("sys.State") != metadatas.end() && metadatas["sys.State"] == "NotFound")
					{
						setReponseWithoutBody(reqID,request, response, 404, GCISTATUS_404);
					}
					else
						setReponseWithoutBody(reqID,request, response, 500, GCISTATUS_500);
					return;
				}

				for (TianShanIce::Properties::const_iterator iter = metadatas.begin(); iter != metadatas.end(); iter++)
				{
					if(iter->first == METADATA_FileSize)
					{
						strFilesize = iter->second;
					}
					else if(iter->first == METADATA_SupportFileSize)
					{
						strSupportFilesize = iter->second;
					}
					else if(iter->first == "sys.State")
					{
						strContentState = convertState(A3AquaBase::stateId((iter->second).c_str()));
					}
					else if(iter->first == METADATA_MD5CheckSum)
					{
						strMD5CheckSum = iter->second;
					}
					else if(iter->first == METADATA_BitRate)
					{
						strBitRate = iter->second;
					}
				}
			}
			// output response
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<TransferStatus ";
			buf <<    "providerID=\""   << strProviderId    << "\" ";
			buf <<    "assetID=\""      << strProviderAsserId       << "\" ";
			buf <<    "volumeName=\""   << xmlElement[Key_volumeName]    << "\" ";
			buf <<    "state=\""        << strContentState << "\" ";
			buf <<    "reasonCode=\""   << reasonCode                << "\" ";
			if (!stricmp(strContentState.c_str(),CONTENTSTATUS_COMPLETE))
			{
				{
					Ice::Long filesize,supportFileSize;
					sscanf((char*)strFilesize.c_str(), FMT64, &filesize);
					sscanf((char*)strSupportFilesize.c_str(),FMT64, &supportFileSize);

					filesize = filesize / 1024;
					supportFileSize = supportFileSize /1024;

					char tmp[40];
					memset(tmp, 0 , 40);
					sprintf(tmp, FMT64, filesize);
					strFilesize = tmp;

					memset(tmp, 0 , 40);
					sprintf(tmp, FMT64, supportFileSize);
					strSupportFilesize = tmp;
				}
				buf <<    "contentSize=\""       << strFilesize        << "\" ";
				buf <<    "bitrate=\""       << strBitRate        << "\" ";
				buf <<    "supportFileSize=\""   << strSupportFilesize << "\" ";
				buf <<    "md5Checksum=\""       << strMD5CheckSum     << "\" ";
				buf <<    "md5DateTime=\""       << GenerateUTCTime()  << "\" ";
			}
			sscanf((char*)strBitRate.c_str(), FMT64, &bitRate);
			sscanf((char*)strFilesize.c_str(), FMT64, &filesize);
			if(bitRate != 0)
			{
				duration = filesize * 8 / bitRate;
			}

			buf <<    "duration=\""       << duration << "\" ";
			envlog(ZQ::common::Log::L_DEBUG, A3Fmt(A3MessageHandler, "********filesize[%lld]bitRate[%lld]Duration[%lld]"), filesize, bitRate, duration);
			buf << "/>";
			strMsgContent = buf.str();

			setResponseWithBody(reqID, request, response, 200, "OK", strMsgContent);
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "Get Transfer Status complete took %d ms"), (int)(ZQTianShan::now() - lStart));

		}
		bool A3MessageHandler::readXMLDoc(const std::string reqID,ZQ::common::XMLPreferenceDocumentEx& xmlDoc, const char* buffer, size_t bufLen)
		{
			try
			{
				if(!xmlDoc.read((void*)buffer, (int)bufLen, 1))
				{
					envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "failed to parse request content"));
					return false;
				}
			}
			catch (ZQ::common::XMLException& xmlEx)
			{
				envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "readXMLDoc() : read xml catch a exception [%s]"), xmlEx.getString());
				return false;
			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "readXMLDoc() : read xml catch unknown exception[%d]"), SYS::getLastErr());
				return false;
			}
			return true;
		}

		bool A3MessageHandler::parseMsgContent(const std::string reqID,const char* buffer, size_t bufLen, StringMap& xmlElement)
		{
			if (!buffer || 0 == bufLen)
			{
				return false;
			}
			char buf[256]= "";
			sprintf(buf, A3Fmt(A3MessageHandler,"request content is: "));
			envlog.hexDump(ZQ::common::Log::L_INFO, buffer, bufLen, buf, true);


//			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "parseMsgContent() : request content is [%s]"),
//				buffer);
			ZQ::common::XMLPreferenceDocumentEx xmlDoc;
			if (!readXMLDoc(reqID, xmlDoc, buffer, bufLen))
			{
				return false;
			}
			ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
			if(NULL == pXMLRoot)
			{
				xmlDoc.clear();
				envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "parseMsgContent() : getRootPreference error")); 
				return false;
			}
			xmlElement.clear();
			xmlElement = pXMLRoot->getProperties();
			pXMLRoot->free();
			xmlDoc.clear();
			return true;
		}

		bool A3MessageHandler::parseMsgContentEx(const std::string reqID,const char* buffer, size_t bufLen, 
			StringMap& xmlElement, StringMap& metaDatas)
		{
			if (!buffer || 0 == bufLen)
			{
				return false;
			}

			char buf[256]= "";
			sprintf(buf, A3Fmt(A3MessageHandler,"request content is: "));
			envlog.hexDump(ZQ::common::Log::L_INFO, buffer, bufLen, buf, true);

//			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "parseMsgContent() : request content is [%s]"),
//				buffer);
			ZQ::common::XMLPreferenceDocumentEx xmlDoc;
			if (!readXMLDoc(reqID, xmlDoc, buffer, bufLen))
			{
				return false;
			}
			ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
			if(NULL == pXMLRoot)
			{
				xmlDoc.clear();
				envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "parseMsgContent() : getRootPreference error")); 
				return false;
			}
			ZQ::common::XMLPreferenceEx* contentAsset = pXMLRoot->findChild("ContentAsset");
			if(NULL == contentAsset)
			{
				pXMLRoot->free();
				xmlDoc.clear();
				envlog(ZQ::common::Log::L_ERROR, A3Fmt(A3MessageHandler, "parseMsgContent() : get ContentAsset Preference error")); 
				return false;
			}

			// get root properties
			xmlElement.clear();
			xmlElement = pXMLRoot->getProperties();

			metaDatas.clear();
			ZQ::common::XMLPreferenceEx* metaDatasProps = contentAsset->firstChild("metadata");

			while(NULL != metaDatasProps)
			{
				StringMap mds;
				mds.clear();
				mds = metaDatasProps->getProperties();

				if(mds.find("name") != mds.end() && mds.find("value") != mds.end())
					metaDatas[mds["name"]] = mds["value"];

				metaDatasProps = contentAsset->nextChild();
			}
            
			pXMLRoot->free();
			xmlDoc.clear();
			return true;
		}

		void A3MessageHandler::setReponseWithoutBody(const std::string reqID,const CRG::IRequest* request, CRG::IResponse* response, 
			int statusCode, const char* reasonPhrase)
		{
			response->setStatus(statusCode, reasonPhrase);
			response->setHeader("CSeq", request->header("CSeq"));
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "setReponseHeaderWithNoBody() : %d %s"), statusCode, reasonPhrase);
		}

		void A3MessageHandler::setResponseWithBody(const std::string reqID,const CRG::IRequest* request, CRG::IResponse* response, 
			int statusCode, const char* reasonPhrase, std::string strMsgContent)
		{
			char length[20];
			snprintf(length, sizeof(length), "%ld", strMsgContent.length());
			response->setStatus(statusCode, reasonPhrase);
			response->setHeader("Content-type", "text/xml");
			response->setHeader("Content-length", length);
			response->setHeader("CSeq", request->header("CSeq"));
			response->setContent(strMsgContent.data(), strMsgContent.length());
			envlog(ZQ::common::Log::L_INFO, A3Fmt(A3MessageHandler, "setResponseWithBody() : Response content is [%s]"), strMsgContent.c_str());
		}

		bool A3MessageHandler::getVolumeNameAndId(const std::string &strFullVol, std::string &strNetId, std::string& strVolume)
		{
			if (strFullVol.empty())
			{
				return false;
			}
	/*		size_t nPosition = strFullVol.find("/");
			if (nPosition != std::string::npos)
			{
				strNetId = strFullVol.substr(0, nPosition);
				strVolume = strFullVol.substr(nPosition);
				if (strNetId.empty() || strVolume.empty())
				{
					return false;
				}
				return true;
			}
			return false;*/

		     ContentStoreMountInfos::iterator itorcsinfo = _A3Config.csmounts.find(strFullVol);
			if(itorcsinfo != _A3Config.csmounts.end())
			{
				strNetId = itorcsinfo->second.netId;
				strVolume = itorcsinfo->second.volumename;
				return true;
			}
			return false;
		}
		bool A3MessageHandler::getContentName(std::string& strNetId, std::string& strVolumeName, std::string& strProviderID, std::string& strAssetId, std::string& strContentName)
		{
			const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
			if(!clPrx)
			{
				return false;
			}

			try
			{
				TianShanIce::Repository::MetaObjectInfos infos;
				TianShanIce::Properties prop;
				TianShanIce::StrValues listMetadata;
// 				MAPSET(TianShanIce::Properties, prop, Key_volumeName, strVolumeName);
// 				MAPSET(TianShanIce::Properties, prop, Key_netId, strNetId);
// 				MAPSET(TianShanIce::Properties, prop, "objectType", "ContentReplica");
// 				MAPSET(TianShanIce::Properties, prop, "user.ProviderId", strProviderID);
// 				MAPSET(TianShanIce::Properties, prop, "user.ProviderAssetId", strAssetId);

				listMetadata.push_back("contentName");

				infos = clPrx->locateContentByPIDAndPAID(strNetId, strVolumeName, strProviderID, strAssetId, listMetadata);

				if(infos.size() != 1)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageHandler, "[%s$/%s, %s, %s]content not found"), 
						strNetId.c_str(), strVolumeName.c_str(), strProviderID.c_str(), strAssetId.c_str()); 
					return false;
				}
				TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin();
				::TianShanIce::Repository::MetaDataMap::const_iterator iter = it->metaDatas.find("contentName");
				if(iter == it->metaDatas.end())
				{
					return false;
				}
				strContentName = iter->second.value;	
			}
			catch (const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageHandler, "getContentName() : catch an ice exception [%s]"), ex.ice_name().c_str());
				return false;
			}
			return true;
		}
	}///end namespace A3Server
}///end namespace CRM

