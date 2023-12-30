#include "A3MessageEventSinkI.h"
#include "A3MsgCommon.h"
#include "TsRepository.h"
#include "A3Config.h"
#include "ContentUser.h"
#include "ContentSysMD.h"
#include "ContentStore.h"
extern ZQ::common::Config::Loader< A3MessageCfg > _A3Config;
namespace CRM
{
	namespace A3Message
	{
		A3MessageEventSinkI::A3MessageEventSinkI(CRM::A3Message::A3MsgEnv& env): _env(env)
		{
		}

		A3MessageEventSinkI::~A3MessageEventSinkI(void)
		{
		}

		void A3MessageEventSinkI::ping(Ice::Long timestamp, const Ice::Current &cur)
		{
		}
/*
		Created
			params = [3](("content","/test/cdntest1234567812030xor.com"),("name","cdntest1234567812030xor.com"),("volume","/test"))
			statuschange
			params = [5](("content","/test/cdntest1234567812030_xor.com"),("name","cdntest1234567812030_xor.com"),("newState","Provisioning(1)"),("oldState","NotProvisioned(0)"),("volume","/test"))
			Destoryed
			params = [3](("content","/test/cdntest1234567891001_xor.com"),("name","cdntest1234567891001_xor.com"),("volume","/test"))
*/
		void A3MessageEventSinkI::post(const ::std::string& category, ::Ice::Int eventId, const ::std::string& eventName, 
			const ::std::string& stampUTC, const ::std::string& sourceNetId, 
			const ::TianShanIce::Properties& params, const ::Ice::Current& cur)
		{
			if (category != A3EventCategory || eventName != A3StateChangeEvent || params.size() < 5)
			{
				return;
			}
/*			if(eventName == A3CreateEvent || eventName == A3DestroyEvent)
			{
				std::string strContent = params.find(EventField_Content) != params.end() ? params.find(EventField_Content)->second : "";
				if ("" == strContent)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() content is empty"));
					return;
				}
				std::string strName = params.find(EventField_Name) != params.end() ? params.find(EventField_Name)->second : "";
				if ("" == strName)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() name is empty"));
					return;
				}
				std::string strVolume = params.find(EventField_Volume) != params.end() ? params.find(EventField_Volume)->second : "";
				if ("" == strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() volume is empty"));
					return;
				}
			}
*/

			if(eventName == A3StateChangeEvent)
			{
				std::string strContent = params.find(EventField_Content) != params.end() ? params.find(EventField_Content)->second : "";
				if ("" == strContent)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() content is empty"));
					return;
				}
				std::string strName = params.find(EventField_Name) != params.end() ? params.find(EventField_Name)->second : "";
				if ("" == strName)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() name is empty"));
					return;
				}
				std::string strVolume = params.find(EventField_Volume) != params.end() ? params.find(EventField_Volume)->second : "";
				if ("" == strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() volume is empty"));
					return;
				}

				std::string strNewStatus = params.find(EventField_NewStatus) != params.end() ? params.find(EventField_NewStatus)->second : "";
				if ("" == strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() newStatus is empty"));
					return;
				}

				std::string strOldStatus = params.find(EventField_OldStatus) != params.end() ? params.find(EventField_OldStatus)->second : "";
				if ("" == strVolume)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() oldStatus is empty"));
					return;
				}

				std::string strNetId = params.find(EventField_NetId) != params.end() ? params.find(EventField_NetId)->second : "";
				//if ("" == strNetId)
				//{
				//	envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "OnEvent() : Net Id is empty"));
				//	return;
				//}

				envlog(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageEventSinkI, "OnEvent() receive an event[%s] CotentName[%s]VolumeName[%s]OldStatus[%s]NewStatus[%s] from SourceNetId[%s]"), 
					eventName.c_str(), strContent.c_str(), strVolume.c_str(), strOldStatus.c_str(), strNewStatus.c_str(), sourceNetId.c_str());

				sendTransferStatus(strNetId, strContent, strName, strVolume, strNewStatus);
			}
		}

		void A3MessageEventSinkI::sendTransferStatus(const std::string& strNetId,
			                                         const std::string& strContent,
		                                             const std::string& strName, 
													 const std::string& strVolume, 
													 const std::string& contentState)
		{
			std::string strFilesize = "", strSupportFilesize = "", strMD5CheckSum = "", strBitRate="0";
			std::string strResponseURL = "";
			std::string strPID, strPAID;

			if (contentState == "InService(3)" || contentState == "InService" || contentState =="Provisioning" || contentState =="Provisioning(1)")
			{
				if(_env._backStoreType == backContentLib)
				{
					int nPosition = strName.find("_");	
					std::string strPAID="", strPID="";
					if(nPosition > 0)
					{
						strPAID = strName.substr(0, nPosition);
						strPID = strName.substr(nPosition + 1);
					}

					const TianShanIce::Repository::ContentLibPrx&  clPrx = _env.connectToContentLib();
					if(!clPrx)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus() : Content Not exist"));
						return ;
					}

					try
					{
						TianShanIce::Repository::MetaObjectInfos infos;
						TianShanIce::Properties prop;
						TianShanIce::StrValues listMetadata;
						MAPSET(TianShanIce::Properties, prop, Key_volumeName, strVolume);
						MAPSET(TianShanIce::Properties, prop, Key_netId, strNetId);
						//	MAPSET(TianShanIce::Properties, prop, "objectType", "ContentReplica");
						MAPSET(TianShanIce::Properties, prop, "user.ProviderId", strPID);
						MAPSET(TianShanIce::Properties, prop, "user.ProviderAssetId", strPAID);

						listMetadata.push_back("sys.FileSize");
						listMetadata.push_back("sys.SupportFileSize");
						listMetadata.push_back("sys.MD5CheckSum");
						listMetadata.push_back("sys.BitRate");
						listMetadata.push_back("user.ResponseURL");

						infos = clPrx->locateContent(prop, listMetadata, 0, false);

						if(infos.size() != 1)
						{
							envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()failed to connect to contentlib service"));
							return ;
						}

						TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin();
						for (::TianShanIce::Repository::MetaDataMap::const_iterator iter = it->metaDatas.begin(); iter != it->metaDatas.end(); iter++)
						{
							if(iter->first == "sys.MD5CheckSum")
							{
								strMD5CheckSum = iter->second.value;
							}
							else if(iter->first == "user.ResponseURL")
							{
								strResponseURL = iter->second.value;
							}
							else if(iter->first == "sys.FileSize")
							{
								strFilesize = iter->second.value;
							}
							else if(iter->first == "sys.SupportFileSize")
							{
								strSupportFilesize = iter->second.value;
							}
							else if(iter->first == "sys.BitRate")
							{
								strBitRate = iter->second.value;
							}
						}
					}
					catch (Ice::Exception& ex)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s]failed to get metedata info with exception[%s]"),strContent.c_str(), ex.ice_name().c_str());
						return;
					}
				}
				else  if(_env._backStoreType == backCacheStore)///query get transfer status info with CacheStroe service
				{
					TianShanIce::Storage::CacheStorePrx cacheStroePrx = _env.connectToCacheStore();
					if(!cacheStroePrx)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()failed to connect to cachestore service"));
						return;
					}
					try
					{
						if(_A3Config.csOptimize)
						{
							TianShanIce::Storage::CacheCandidates cacheCandidates = cacheStroePrx->getCandidatesOfContent(strName, false);
							if(cacheCandidates.size()  > 0)
							{
								cacheStroePrx = cacheCandidates[0].csd.theStore;
							}
						}

						TianShanIce::Storage::ContentStoreExPrx csprx  =  TianShanIce::Storage::ContentStoreExPrx::checkedCast(cacheStroePrx->theContentStore());

						TianShanIce::Storage::ContentPrx contentprx = csprx->openContentByFullname(strContent);

						TianShanIce::Properties metadatas =  contentprx->getMetaData();

						TianShanIce::Properties::iterator itorMd;
						itorMd = metadatas.find(METADATA_ResponseURL);
						if(itorMd != metadatas.end())
						{
							strResponseURL = metadatas[METADATA_ResponseURL];
						}

						itorMd = metadatas.find(METADATA_SupportFileSize);
						if(itorMd != metadatas.end())
						{
							strSupportFilesize = metadatas[METADATA_SupportFileSize];
						}

						itorMd = metadatas.find(METADATA_FileSize);
						if(itorMd != metadatas.end())
						{
							strFilesize = metadatas[METADATA_FileSize];
						}

						itorMd = metadatas.find(METADATA_MD5CheckSum);
						if(itorMd != metadatas.end())
						{
							strMD5CheckSum = metadatas[METADATA_MD5CheckSum];
						}

						itorMd = metadatas.find(METADATA_ProviderAssetId);
						if(itorMd != metadatas.end())
						{
							strPAID = metadatas[METADATA_ProviderAssetId];
						}

						itorMd = metadatas.find(METADATA_ProviderId);
						if(itorMd != metadatas.end())
						{
							strPID = metadatas[METADATA_ProviderId];
						}
						itorMd = metadatas.find("sys.BitRate");
						if(itorMd != metadatas.end())
						{
							strBitRate = metadatas["sys.BitRate"];
						}
					}
					catch (TianShanIce::ServerError& ex)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s]failed to get metedata info with exception[%d,%s]"),strContent.c_str(), ex.errorCode, ex.message.c_str());
						return;
					}
					catch (Ice::Exception& ex)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s]failed to get metedata info with exception[%s]"),strContent.c_str(), ex.ice_name().c_str());
						return;
					}
				}
			}
			else
			{
                ContentInfo contentInfo = _env.getContentInfo(strName);
				strResponseURL = contentInfo.responseUrl;
				strPID = contentInfo.pid;
				strPAID = contentInfo.paid;
            }

 			if ("" == strResponseURL)
 			{
 				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s] No Response URL to send status"),strContent.c_str());
 				return;
 			}
 			int reasonCode = 200;
 			if (contentState == "OutService(4)" || contentState == "Cleaning(5)" || contentState == "OutService" || contentState == "Cleaning")
 			{
//				_env.removeContentInfo(strName);
 				reasonCode = 500;
 			}
			std::string strVol = strVolume;
			if(!strVolume.empty() && strVolume[0] == '/')
			{
				strVol = strVolume.substr(1);
			}
			int npos  = strVol.find('/');
			if(npos > 0)
				strVol = strVol.substr(0, npos);

 			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<TransferStatus " ;
			buf <<    "providerID=\""   << strPID                            << "\" ";
			buf <<    "assetID=\""      << strPAID                           << "\" ";
			buf <<    "volumeName=\""   << strVol                        << "\" ";
			buf <<    "state=\""        << eventStateToA3State(contentState) << "\" ";
			buf <<    "reasonCode=\""   << reasonCode                        << "\" ";
			if (contentState == "InService(3)" || contentState == "InService")
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
				buf <<    "md5DateTime=\""       << GenerateUTCTime()               << "\" ";
			}
			buf << "/>";
			std::string strRequestBoby = buf.str();
			_env._A3Client->SendRequest(strContent, strResponseURL, strRequestBoby);
		}

	}///end namespace A3Server
}///end namespace CRM

