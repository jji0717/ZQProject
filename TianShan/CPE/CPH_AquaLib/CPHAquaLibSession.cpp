#include "CPH_AquaLibCfg.h"
#include <ErrorCode.h>
#include <CPHInc.h>
#include <RTFProc.h>
#include <urlstr.h>
#include <HostToIP.h>
#include <McastCapSrc.h>
#include <NICSelector.h>
#include <FTPSource.h>
#include <CIFSSource.h>
#include <AquaSource.h>
#include <MD5CheckSumUtil.h>
#include <TargetFactoryI.h>
#include <AquaFileSetTarget.h>
#include "AquaLibHelper.h"
#include <TianShanDefines.h>
#include <ContentSysMD.h>

#include "CPH_AquaLib.h"
#include "CPHAquaLibSession.h"
#include "RTIRawSource.h"

#include "CSILibAPI.h"
#include "CSILibErrors.h"

#define MOLOG (*_pCPELogger)

namespace ZQTianShan {
	namespace ContentProvision{

		CPHAquaLibSess::CPHAquaLibSess(ZQ::common::Log* log, ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: BaseCPHSession(helper, pSess), _pCPELogger(log), _filesize(0), _bQuit(false), _enableNoTrickSpeed(false),_bIndexVVC(false), BaseGraph(_gCPHCfg.maxAllocSampleCount)
		{
			_bCleaned = false;
			_pMainTarget = NULL;
			_bStartEventSent = false;
			_pRTFProc = NULL;
#ifdef ZQ_OS_LINUX
			_bSuccessMount = false;
			_sharePath = "";

#endif
			_pA3AquaContentMetadata = new CRM::A3Message::A3AquaContentMetadata(*log, *_pCdmiFuseOps, _gCPHCfg.aquaServer.mainFileExtension, _gCPHCfg.aquaServer.ContentNameFormat);

			_nSchedulePlayTime = 0;
			_processedTime = 0;
			_sourceType = TianShanIce::Storage::ctMPEG2TS;

		}

		CPHAquaLibSess::~CPHAquaLibSess()
		{
			 _nNetSelector->freeInterface(_strFileName);
			cleanup();
		}

		void CPHAquaLibSess::cleanup()
		{
			if (_bCleaned)
				return;

			BaseGraph::Close();
			_bCleaned = true;

			_pA3AquaContentMetadata = NULL;
		}

		bool CPHAquaLibSess::Start()
		{
			bool bRet = BaseGraph::Start();
			if(!bRet)
			{
				_pA3AquaContentMetadata->deleteFile(_pA3AquaContentMetadata->getAquaContentMDName(_paid, _pid));
			}
			return bRet;
		}
		bool CPHAquaLibSess::preLoad()
		{

			if(NULL == _pA3AquaContentMetadata)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "Aqua server is not initalize"));
				setErrorInfo(ERRCODE_AQUA_NOTINIT, "Aqua server is not initalize");	
				OnPreloadFailed();
				return false;
			}

			if (!_sess)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "provision session is 0"));
				setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");	
				OnPreloadFailed();
				return false;
			}

			_pRTFProc = NULL;
			_strMethod = _sess->methodType;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibSess, "[%s] provision method[%s]"),_sess->ident.name.c_str(),_strMethod.c_str());

			if (stricmp(_strMethod.c_str(), METHODTYPE_AQUA_FTPRTF)      &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_FTPRTFH264)  &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_FTPRTFH265)  &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_NTFSRTF)     &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_NTFSRTFH264) &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_NTFSRTFH265) &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_RTI)         &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_RTIH264)	 &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_RTIH265)	 &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_INDEX)       &&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_INDEXH264)	&&
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_INDEXH265)	&&
				stricmp(_strMethod.c_str(), METHODTYPE_RTIRAW))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
					_strMethod.c_str());
				setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");	
				OnPreloadFailed();
				return false;
			}

			_sourceType = TianShanIce::Storage::ctMPEG2TS;
			bool bH264Type = false;
			if (stricmp(_strMethod.c_str(),METHODTYPE_AQUA_FTPRTFH264)  == 0 || 
				stricmp(_strMethod.c_str(),METHODTYPE_AQUA_NTFSRTFH264) == 0 || 
				stricmp(_strMethod.c_str(),METHODTYPE_AQUA_RTIH264)     == 0 ||
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_INDEXH264) ==0 )
			{
				bH264Type = true;
				_sourceType = TianShanIce::Storage::ctH264;
			}
			else if (stricmp(_strMethod.c_str(),METHODTYPE_AQUA_FTPRTFH265)  == 0 || 
				stricmp(_strMethod.c_str(),METHODTYPE_AQUA_NTFSRTFH265) == 0 || 
				stricmp(_strMethod.c_str(),METHODTYPE_AQUA_RTIH265)     == 0 ||
				stricmp(_strMethod.c_str(), METHODTYPE_AQUA_INDEXH265) ==0 )
			{
				_sourceType = TianShanIce::Storage::ctH265;
			}
			std::string strSrcUrl;
			if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] could not find resource URI"), _sess->ident.name.c_str());
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");		
				OnPreloadFailed();
				return false;
			}

			TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
			if (resURI.end() == resURI.find(CPHPM_FILENAME))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
					CPHPM_FILENAME);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);	
				OnPreloadFailed();
				return false;
			}

			TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
			if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
					CPHPM_FILENAME);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);
				OnPreloadFailed();
				return false;
			}

			_strFileName = var.strs[0];

			if (resURI.end() != resURI.find(CPHPM_SOURCEURL))
			{
				TianShanIce::Variant& var2 = resURI[CPHPM_SOURCEURL];
				if (var2.type == TianShanIce::vtStrings && var2.strs.size()>0)
				{
					strSrcUrl = var2.strs[0];
				}
			}

			TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
			if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
					CPHPM_BANDWIDTH);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);
				OnPreloadFailed();
				return false;
			}

			TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
			if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
					CPHPM_BANDWIDTH);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);
				OnPreloadFailed();
				return false;
			}
			int nBandwidth = (int)var1.lints[0];
			_nBandwidth = nBandwidth;

			TianShanIce::Properties prop = _sess->props;
			TianShanIce::Properties::const_iterator it = prop.find(CPHPM_INDEXTYPE);
			if (it!=prop.end())
			{
				if (stricmp(it->second.c_str(),"VVC") == 0)
				{
					_bIndexVVC = true;				
				}
			}
			//	if (_bIndexVVC)
			//	{
			it = prop.find(CPHPM_PROVIDERID);
			if (it!=prop.end())
				_pid = it->second;

			it = prop.find(CPHPM_PROVIDERASSETID);
			if (it!=prop.end())
				_paid = it->second;
			//	}
			it = prop.find(CPHPM_NOTRICKSPEEDS);
			if (it != prop.end() && it->second == "1")
			{
				int major = 0, minor = 0;
				RTFProcess::getCTFLibVersion(major, minor);
				if(major < 3)
				{
					MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CPHAquaLibSess, "[%s] CTFLib Verion support No TrickSpeed File must >= 3.0 "), _sess->ident.name.c_str());
				}
				_enableNoTrickSpeed = true;
			}
			if (_paid.empty() || _pid.empty())
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "paid pid is empty"));
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find paid pid");
				OnPreloadFailed();
				return false;
			}

			int augmentationPidCount = 0;
			uint16 augmentationPids[ MAX_AUGMENTATION_PIDS ];
			memset((void*) augmentationPids, 0 , sizeof(augmentationPids));

			std::string strPIDs = "";
			it = prop.find(CPHPM_AUGMENTATIONPIDS);
			if (it!=prop.end())
			{
				strPIDs = it->second;
			}
			else
			{
				strPIDs = _gCPHCfg.strAugmentationPids;
			}
			if(!strPIDs.empty())
			{
				::TianShanIce::StrValues  strAugmentationPids;
				::ZQ::common::stringHelper::SplitString(strPIDs, strAugmentationPids, ";,");
				int i;
				for(i = 0; i < strAugmentationPids.size() && i < MAX_AUGMENTATION_PIDS; ++i)
				{
					augmentationPids[i] = atoi(strAugmentationPids[i].c_str());
				}
				augmentationPidCount = strAugmentationPids.size();
			}
			SetLog(_helper.getLog());
			SetMemAlloc(_helper.getMemoryAllocator());
			SetLogHint(_strFileName.c_str());

			int nMaxBandwidth;
			if (_gCPHCfg.bandwidthLimitRate)
			{
				nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
			}
			else
				nMaxBandwidth = nBandwidth;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibSess, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d] paid[%s] pid[%s]"), _strLogHint.c_str(), 
				nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth, _paid.c_str(), _pid.c_str());

			std::string url = strSrcUrl;
			ZQ::common::URLStr Url(strSrcUrl.c_str());
			BaseSource *pSource= NULL;

			if(0 ==stricmp("raw", Url.getProtocol()))
			{
				std::string multicastIp = strSrcUrl.substr(strSrcUrl.find_first_of(':')+3,strSrcUrl.find_last_of(':')-6);
				std::string strmulticastPort = strSrcUrl.substr(strSrcUrl.find_last_of(':')+1,strSrcUrl.size()-strSrcUrl.find_last_of(':')-1);
				int multicastPort = atoi(strmulticastPort.c_str());

				DWORD timeoutInterval = _gCPHCfg.capture.sessionTimeout;
				std::string localIp;
				if (!_nNetSelector->allocInterface(nMaxBandwidth,localIp,_strFileName))
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				if (!HostToIP::translateHostIP(localIp.c_str(), _strLocalIp))//translate host name to ip
					_strLocalIp = localIp;

				RTIRawTarget* pTarget = (RTIRawTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_RTIRAW);
                
				if(pTarget == NULL || !AddFilter(pTarget))
				{
					OnPreloadFailed();
					return false;
				}
				std::string strMainFileName  =  _pA3AquaContentMetadata->getMainFileName(_paid, _pid);
				pTarget->setFilename(_strFileName.c_str());
				pTarget->setMainFileName(strMainFileName);
				pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
				_pMainTarget = pTarget;

				RTIRawSource* rawSource = (RTIRawSource*)SourceFactory::Create(SOURCE_TYPE_RTIRAW);
				if(rawSource == NULL || !AddFilter(rawSource))
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create RAW source failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				//	pSource->setPool(RTIRawHelper::_theHelper->_pool);
				_nSchedulePlayTime = (_sess->scheduledEnd - _sess->scheduledStart);
				rawSource->setScheduleTime(_sess->scheduledStart + (_nSchedulePlayTime * 0.8));
				rawSource->setFilter(pTarget);
				rawSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,localIp);
			}
			else
			{
				std::list<float> trickspeed;
				std::list<float> trickspeedHD;

				::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;

				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibSess, "trickSpeed from session [%s]"), formatSpeed(trickcol).c_str());

				if (trickcol.size() == 0 || (trickcol.size() == 1 && fabs(trickcol[0]) <=1e-6 )) //add compare folat 
					trickcol.push_back(7.5);

				bool bFoundHD = false;
				for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
				{
					if(fabs(*iterTrick) <=1e-6)
					{
						bFoundHD = true;
						continue;
					}
					if(bFoundHD)
					{
						trickspeedHD.push_back((*iterTrick));
					}
					else
						trickspeed.push_back((*iterTrick));		
				}

				if(trickspeed.empty() && trickspeedHD.size() > 0)
				{
					trickspeed.assign(trickspeedHD.begin(), trickspeedHD.end());
				}

				if(!trickspeed.empty() && !trickspeedHD.empty() && trickspeed.size() != trickspeedHD.size())
				{
					trickspeed.sort();
					trickspeedHD.sort();

					if(trickspeed.size() < trickspeedHD.size())
					{
						int npos = trickspeedHD.size() - trickspeed.size();
						std::list<float>::iterator itorSpeed =  trickspeedHD.begin();
						for(uint i = 0; i < trickspeedHD.size() - npos ; i++)
						{
							itorSpeed++;
						}
						for(uint i = 0; i < npos ; i++)
						{
							trickspeed.push_back(*itorSpeed);
							itorSpeed++;
						}
					}
					else
					{
						int npos = trickspeed.size() - trickspeedHD.size();
						std::list<float>::iterator itorSpeed =  trickspeed.begin();
						for(uint i = 0; i < trickspeed.size() - npos ; i++)
						{
							itorSpeed++;
						}
						for(uint i = 0; i < npos ; i++)
						{
							trickspeedHD.push_back(*itorSpeed);
							itorSpeed++;
						}
					}
					trickspeed.sort();
					trickspeedHD.sort();
				}

				_bPushTrigger = false;

				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] sourceUrl[%s], SD trickspeed [%s], HD trickSpeed[%s]"),
					_strLogHint.c_str(),strSrcUrl.c_str(), formatSpeed(trickspeed).c_str(), formatSpeed(trickspeedHD).c_str());

				if (0 ==stricmp("ftp", Url.getProtocol()))
				{
					FTPIOSource* ftpSource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP,  &_helper._pool);
					if( ftpSource == NULL)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create FTP source failed"), _strLogHint.c_str());
						OnPreloadFailed();
						return false;
					}
					AddFilter(ftpSource);    //only after this, the log handle will be parsed in
					ftpSource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
					ftpSource->setURL(url.c_str());
					ftpSource->setMaxBandwidth(nMaxBandwidth);
					ftpSource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
					if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
					{
						ftpSource->setSourceUrlUTF8(true);
					}
					pSource = ftpSource;
				}
				else if(0 ==stricmp("cifs", Url.getProtocol()) || 0 ==stricmp("file", Url.getProtocol()))
				{
					std::string host = Url.getHost();
					std::string sourceFilename = Url.getPath();
					std::string strOpt,strsystype,strsharePath;
					bool bLocalSrcFlag;
					strsystype = "cifs";
					if (host.empty() || 0 == host.compare(".") || 0 == host.compare("localhost"))
					{
						bLocalSrcFlag = true;
						fixpath(sourceFilename, true);
						strOpt = "username=,password=";
					}
					else
					{
						bLocalSrcFlag = false;
						fixpath(sourceFilename, false);
						strsharePath =std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + Url.getHost() + LOGIC_FNSEPS + sourceFilename.substr(0,sourceFilename.find_first_of(FNSEPC));
						sourceFilename = sourceFilename.substr(sourceFilename.find_first_of(FNSEPC)+1);
						fixpath(sourceFilename, false);
						strOpt = "username=" + std::string(Url.getUserName()) + ",password=" + std::string(Url.getPwd());
					}

					CIFSIOSource* fsSource = (CIFSIOSource*)SourceFactory::Create(SOURCE_TYPE_CIFS, &_helper._pool);
					if (!fsSource)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create CIFS source failed"), _strLogHint.c_str());
						OnPreloadFailed();
						return false;
					}
					AddFilter(fsSource);

					if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
					{
						fsSource->setSourceUrlUTF8(true);
					}
					fsSource->setIOFactory(_pCifsFileIoFac.get());
					fsSource->setFileName(sourceFilename.c_str());
					fsSource->setMaxBandwidth(nMaxBandwidth);
					fsSource->setMountOpt(strOpt);
					fsSource->setSystemType(strsystype);
					fsSource->setLocalSourceFlag(bLocalSrcFlag);
					if (!bLocalSrcFlag)
					{
						fsSource->setSharePath(strsharePath);
#ifdef ZQ_OS_LINUX
						std::string strMountPoint;
						bool bMount = mountURL(strsharePath, strsystype, strOpt, strMountPoint, _strLogHint);
						if(!bMount)
						{
							OnPreloadFailed();
							return false;
						}
						_bSuccessMount = true;
						_sharePath = strsharePath;
						fsSource->setMountPath(strMountPoint);

#endif
					}
					pSource = fsSource;
				}
				else if(0 ==stricmp("udp", Url.getProtocol()))
				{
					std::string multicastIp = strSrcUrl.substr(strSrcUrl.find_first_of(':')+3,strSrcUrl.find_last_of(':')-6);
					std::string strmulticastPort = strSrcUrl.substr(strSrcUrl.find_last_of(':')+1,strSrcUrl.size()-strSrcUrl.find_last_of(':')-1);
					int multicastPort = atoi(strmulticastPort.c_str());

					DWORD timeoutInterval = _gCPHCfg.capture.sessionTimeout;
					std::string localIp;
					if (!_nNetSelector->allocInterface(nMaxBandwidth,localIp,_strFileName))
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
						OnPreloadFailed();
						return false;
					}
					if (!HostToIP::translateHostIP(localIp.c_str(), _strLocalIp))//translate host name to ip
						_strLocalIp = localIp;

						McastCapSource* mcastSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);
						if(mcastSource == NULL)
						{
							MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create mcast source failed"), _strLogHint.c_str());
							OnPreloadFailed();
							return false;
						}
						AddFilter(mcastSource);
						mcastSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,_strLocalIp);

						//dumper parameters
						mcastSource->enableDump(_gCPHCfg.captureDumper.enable==1);
						mcastSource->setDumpPath(_gCPHCfg.captureDumper.dumpPath.c_str());
						mcastSource->deleteDumpOnSuccess(_gCPHCfg.captureDumper.deleteOnSuccess);
						pSource = mcastSource;
				}
				else if (0 ==stricmp("aqua", Url.getProtocol()))
				{
#pragma message(__MSGLOC__"TODO: 加入Aqua Source处理")
					std::string host = Url.getHost();
					std::string sourceFilename = Url.getPath();
					AquaIOSource* aquaSource = (AquaIOSource*)SourceFactory::Create(SOURCE_TYPE_AQUA, &_helper._pool);
					if (!aquaSource)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create %s source failed"),_strLogHint.c_str(),Url.getProtocol());
						OnPreloadFailed();
						return false;
					}
					AddFilter(aquaSource);
					aquaSource->setIOFactory(_pFileIoFac.get());
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] set sourceFilename[%s]"),_strLogHint.c_str(),sourceFilename.c_str());
					aquaSource->setFileName(sourceFilename.c_str());
					aquaSource->setMaxBandwidth(nMaxBandwidth);
					pSource = aquaSource;
				}
				else
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] url protocol[%s] not support"), _strLogHint.c_str(),Url.getProtocol());
					OnPreloadFailed();
					return false;
				}

				ZQ::common::MD5ChecksumUtil md5;
				md5.checksum(_strFileName.c_str(), _strFileName.length());
				std::string strMd5 = md5.lastChecksum();

				unsigned short uFileCiscoExt = atoi(strMd5.substr(14, 2).c_str());
				uFileCiscoExt &= 0xfff0;

				transform (strMd5.begin(), strMd5.end(), strMd5.begin(), (int(*)(int))toupper);
				std::string strFileCiscoExt = strMd5.substr(0, 14);

				char buf[64] = "";
				memset(buf, 0, 64);
				snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
				std::string mainFileExt="";
				std::string replacePAID = replaceString(_paid);
				std::string replacePID = replaceString(_pid);
				if( 1 == _gCPHCfg.ciscofileext.mode)
				{
					mainFileExt = std::string(buf);
				}
				else if( 2 == _gCPHCfg.ciscofileext.mode)
				{
					memset(buf, 0, 64);
					snprintf(buf, sizeof(buf) - 2, ".00%s_%s\0", replacePAID.c_str(), replacePID.c_str());
					mainFileExt =  std::string(buf);
				}
				FileExtensions exMap;
				FileExtensions::iterator iter;
				std::string key="";
				std::map<std::string , int> exMapOutPutfile;
				//		trickspeed.sort();
				int index = 0;

				for (int i = 0; i < (int)trickspeed.size(); i++)
				{
					char ex[10]={0};
					char exr[10] ={0};

					FileExtension fileExt;
					if (bH264Type && _gCPHCfg.unifiedtrickfile.enable) //.h264 wgk
					{
						memset(buf, 0, 64);
						getUnifiedTrickExt(index,ex);
						key = std::string(ex);
						if( 1 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
						{
							++uFileCiscoExt;
							snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
							exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
							key = std::string(buf);
						}
						else if( 2 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
						{
							memset(buf, 0, 64);
							snprintf(buf, sizeof(buf)-2, ".B%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
							exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
							key = std::string(buf);
						}
						else
						{
							exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
						}
						fileExt.ext = ex;
						fileExt.extForCisco = buf;
						fileExt.position = i;
						exMap.insert(std::make_pair(key, fileExt));
						//				exMap.push_back(fileExt);
					}
					else
					{
						getTrickExt(index,ex,exr);
						key = std::string(ex);
						memset(buf, 0, 64);
						if( 1 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
						{
							++uFileCiscoExt;	
							snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
							exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
							key = std::string(buf);
						}
						else if( 2 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)// NVOD1234567890abcdef_cctv.com.F0NVOD1234567890abcdef_cctv$com
						{
							memset(buf, 0, 64);
							snprintf(buf, sizeof(buf)-2, ".F%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
							exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
							key = std::string(buf);
						}
						else
							exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
						fileExt.ext = ex;
						fileExt.extForCisco = buf;
						fileExt.position = i;
						exMap.insert(std::make_pair(key, fileExt));
						//                exMap.push_back(fileExt);


						key = std::string(exr);
						memset(buf, 0, 64);
						if( 1 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
						{
							++uFileCiscoExt;
							snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
							exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
							key = std::string(buf);
						}
						else if( 2 == _gCPHCfg.ciscofileext.mode && _bIndexVVC) // NVOD1234567890abcdef_cctv.com.R0NVOD1234567890abcdef_cctv$com
						{
							memset(buf, 0, 64);
							snprintf(buf, sizeof(buf)-2, ".R%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
							exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
							key = std::string(buf);
						}
						else
							exMapOutPutfile.insert(std::make_pair(std::string(exr), i));
						fileExt.ext = exr;
						fileExt.extForCisco = buf;
						fileExt.position = i;
						exMap.insert(std::make_pair(key, fileExt));
						//		exMap.push_back(fileExt);
					}
					index++;
				}

				int outPutNum = 2 + exMap.size();


				if(_enableNoTrickSpeed)
				{
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] no trickspeed file"), _strLogHint.c_str());
					outPutNum = 2;
				}

				if ( !_enableNoTrickSpeed && outPutNum < 2)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] Not specify trickspeed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}

				RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
				if(pProcess == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create process  failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				AddFilter(pProcess);
				if(!_enableNoTrickSpeed)
				{
					pProcess->setTrickFileEx(exMap);
					pProcess->settrickSpeedNumerator(trickspeed);
				}
				pProcess->setRetryCount(_gCPHCfg.retryCapture.retrycount);
				pProcess->settrickSpeedNumeratorHD(trickspeedHD);
				pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);
				if( 0 != _gCPHCfg.ciscofileext.mode)
					pProcess->setCsicoFileExt(1);
				//		pProcess->setCsicoFileExt(_gCPHCfg.ciscofileext.enable);
				pProcess->setCsicoMainFileExt(mainFileExt);
#ifndef ZQ_OS_MSWIN
				if(_bIndexVVC)
				{
					if(_sourceType == TianShanIce::Storage::ctH265)
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_H265);
					else if(_sourceType == TianShanIce::Storage::ctH264)
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_H264);
					else
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_MPEG2);

					pProcess->setAssetInfo(_pid,_paid);
				}
				else
#endif
				if (_sourceType == TianShanIce::Storage::ctH264)
				{
					pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
				}
				else
				{
					if(_sourceType == TianShanIce::Storage::ctH265)
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_H265);
					else
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVX, CTF_VIDEO_CODEC_TYPE_MPEG2);

				}			
				_pRTFProc = pProcess;
				{
					AquaFilesetTarget* pTarget = (AquaFilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_AQUAFILESET);
					if(pTarget == NULL)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create AquaFileset Target failed"), _strLogHint.c_str());
						OnPreloadFailed();
						return false;
					}
					pTarget->setPacingFactory(_pPacedIndexFac);
					pTarget->enableCacheForIndex(_gCPHCfg.enableCacheForIndex);

					if(!AddFilter(pTarget))
					{
						OnPreloadFailed();
						return false;
					}

					pTarget->setFilename(_strFileName.c_str());
					pTarget->setBandwidth(nMaxBandwidth);
					pTarget->enableProgressEvent(true);
					pTarget->enableMD5(_gCPHCfg.enableMD5);
					if(!_enableNoTrickSpeed)
					{
						pTarget->setTrickFile(exMapOutPutfile);
						pTarget->setTrickSpeed(trickspeed);
					}
					pTarget->setCsicoMainFileExt(mainFileExt);
					pTarget->enablePacing(_gCPHCfg.enablePacing);

					if (!_bIndexVVC  && _sourceType == TianShanIce::Storage::ctH264)
					{
						MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] H264 type"), _strLogHint.c_str());
						pTarget->setTypeH264();
						pTarget->enableStreamableEvent(false);
					}
					else
					{
						MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] VVC type"), _strLogHint.c_str());
						//pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
						pTarget->setIndexType(_bIndexVVC);
						pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
					}
					if (0 ==stricmp("aqua", Url.getProtocol()))
					{
						pTarget->setIgnoreMainfile(true);
						pTarget->setIndexTmpfile(true);
					}
					_pMainTarget = pTarget;

					InitPins();

					if (!ConnectTo(pSource, 0, pProcess, 0))
					{
						OnPreloadFailed();
						return false;
					}

					for (int i = 0; i < outPutNum; i++)
						if (!ConnectTo(pProcess, i, pTarget, i))
						{
							OnPreloadFailed();
							return false;
						}
				}
			}

			SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

			if (!Init())
			{
#ifdef ZQ_OS_LINUX
				if(_bSuccessMount)
				{
					umountURL(_sharePath, _strLogHint);
				}
#endif
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
				setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
				OnPreloadFailed();
				return false;
			}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaLibHelper, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void CPHAquaLibSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
    if (getStatus() == ZQ::common::NativeThread::stDeferred)
	{
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaLibHelper, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
		cleanup();
		try
		{
			delete this;
		}
		catch (...)
		{			
		}
	}
	else
	{
		//
		// thread already started, let final to delete this
		//
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
		_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool CPHAquaLibSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
} 

int CPHAquaLibSess::run(void)
{	
	bool bRet;
	TianShanIce::Properties params;

	Ice::Long llStart = ZQTianShan::now();
    _helper.increaseLoad(_strMethod,_nBandwidth);

#ifdef PROCESSED_BY_TIME
	_nsessStartTime = llStart;
#endif

	if (!_bStartEventSent)
	{
		updateMainMetadataOnStart(params);
		notifyStarted(params);
		_bStartEventSent = true;
	}

	bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);

    _nNetSelector->freeInterface(_strFileName);

#ifdef ZQ_OS_LINUX
	if(_bSuccessMount)
	{
		umountURL(_sharePath, _strLogHint);
	}
#endif
	// get the range info from 
	std::map<std::string, std::string> fileinfomap;
	std::map<std::string, std::string>::iterator iter;
	std::string extCol;

	if(stricmp(_strMethod.c_str(), METHODTYPE_RTIRAW) == 0)
	{
		extCol = _gCPHCfg.aquaServer.mainFileExtension + std::string(";");

		if(extCol.size() > 0 && extCol[0] == '.')
			extCol = extCol.substr(1);
	}
	else
	{
		if (_pRTFProc)
		{
			RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
			pProcess->getOutputFileInfo(fileinfomap);
			pProcess->getOutputFileExtCol(extCol);
		}
	}

	params[EVTPM_MEMBERFILEEXTS] = extCol;
	params[EVTPM_SOURCETYPE] = _sourceType;

	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			

/*		if (!_bStartEventSent)
		{
           // updateMainMetadataOnStart(params);
			notifyStarted(params);
		}
*/
        char tmp[40];
		sprintf(tmp, "False");
		params[EVTPM_OPENFORWRITE] = tmp;
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s]OpenForWrite [%s]"), _strLogHint.c_str(),tmp);

        params[EVTPM_ERRORMESSAGE] = _strErrMsg;		
        char errcode[16];
        sprintf(errcode, "%d", _nErrCode);
        params[EVTPM_ERRORCODE] = errcode;

       // updateMainMetadataOnStop(true, params);
        notifyStopped(true, params);

		// upload failed, delele metedata file 
		_pA3AquaContentMetadata->deleteFile(_pA3AquaContentMetadata->getAquaContentMDName(_paid, _pid));

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] provision done, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());

		Close();

		return 0;
	}

	if (_pMainTarget)
	{
		AquaFilesetTarget* pTarget = (AquaFilesetTarget*)_pMainTarget;
		pTarget->renameBackIndexTmpName();
	}
	int nActualBps = 0;
	int nTimeSpentMs = 0;

	if(stricmp(_strMethod.c_str(), METHODTYPE_RTIRAW) == 0)
	{
        _processed = _llProcBytes;
        _filesize = _llProcBytes;
		OnProgress(_llProcBytes);

		Ice::Long nRealPlayTime = ZQ::common::now() - llStart;
		char tmp[64];

		snprintf(tmp, sizeof(tmp), "%lld", nRealPlayTime/1000);
		params["realplaytime"] = tmp;
		params[EVTPM_PLAYTIME] = tmp;

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);

		snprintf(tmp, sizeof(tmp), "%lld",_nSchedulePlayTime/1000);
		params["allplaytime"] = tmp;

		int bitratebps  = 0;

		if(nRealPlayTime)
			bitratebps = int(_llProcBytes*8000/nRealPlayTime);
		
        memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%d", bitratebps);
		params[EVTPM_MPEGBITRATE] = tmp;
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	}
	// set to params
	for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
	{
		params[iter->first.c_str()] = iter->second;
	}

	Close();

	int64 supportFileSize=0;
	std::string md5;
	std::vector <std::string> outputfilelist;

	if (_pMainTarget)
	{
		AquaFilesetTarget* pTarget = (AquaFilesetTarget*)_pMainTarget;

		supportFileSize = pTarget->getSupportFileSize();
		pTarget->getMD5(md5);
	}

	char tmp[64];
	if (!_llTotalBytes)
	{
		_llTotalBytes = _llProcBytes;
	}
	sprintf(tmp, FMT64, _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, FMT64, supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		sprintf(tmp, FMT64, _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else if(stricmp(_strMethod.c_str(), METHODTYPE_RTIRAW))
	{
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}
	sprintf(tmp, "False");
	params[EVTPM_OPENFORWRITE] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] OpenForWrite[%s]"), _strLogHint.c_str(),tmp);

	if (_pRTFProc && (!stricmp(_strMethod.c_str(), METHODTYPE_AQUA_FTPRTF) || !stricmp(_strMethod.c_str(), METHODTYPE_AQUA_NTFSRTF)))
	{
		RTFProcess* pRtfprocess = (RTFProcess*)_pRTFProc;
		bool bEncrypt = pRtfprocess->getPreEncrypt();
		int augmentedBitRate;
		int originalBitRate;
		std::string augmentionpids;
		if(bEncrypt)
		{
			pRtfprocess->getAugmentationPids(augmentionpids);	
			params[EVTPM_AUGMENTATIONPIDS] = augmentionpids;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] AugmentationPids[%s]"), _strLogHint.c_str(),augmentionpids.c_str());

			pRtfprocess->getPreEncryptBitRate(augmentedBitRate, originalBitRate);

			sprintf(tmp, "%d", originalBitRate);
			params[EVTPM_ORIGINALBITRATE] = tmp;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] OrignalBiterate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "%d", augmentedBitRate);
			params[EVTPM_AUGMENTATEDBITRATE] = tmp;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] AugmentatedBitarate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "1");
			params[EVTPM_PREENCRYPTION] = tmp;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] PreEncryption[%s]"), _strLogHint.c_str(),tmp);
		}
	}

	nTimeSpentMs = int(ZQTianShan::now() - llStart);
	if (nTimeSpentMs)
	{
		nActualBps = int(_llProcBytes*8000/nTimeSpentMs);
	}
	else
	{
		nActualBps = 0;
	}

	updateMainMetadataOnStop(false, params);
	updateMainFileMetadata();
	notifyStopped(false, params);

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), _strLogHint.c_str(), nTimeSpentMs, nActualBps);
	return 0;
}

void CPHAquaLibSess::final(int retcode, bool bCancelled)
{
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool AquaLibHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "provision session is 0"));
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "provision session is 0");
	}

	std::string strMethod = sess->methodType;
	if (stricmp(strMethod.c_str(), METHODTYPE_AQUA_FTPRTF)      &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_FTPRTFH264)  &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_FTPRTFH265)  &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_NTFSRTF)     &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_NTFSRTFH264) &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_NTFSRTFH265) &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_RTI)         &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_RTIH264)		&&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_RTIH265)		&&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_INDEX)       &&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_INDEXH264)	&&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_INDEXH265)	&&
		stricmp(strMethod.c_str(), METHODTYPE_RTIRAW)	&&
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_CSI))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	if((stricmp(strMethod.c_str(), METHODTYPE_AQUA_FTPRTF)     == 0) || 
       (stricmp(strMethod.c_str(), METHODTYPE_AQUA_FTPRTFH264) == 0) ||
	   (stricmp(strMethod.c_str(), METHODTYPE_AQUA_FTPRTFH265) == 0))
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		std::string url = var2.strs[0];
		std::string strpro = "";
		if(url.size() > 6)
			strpro = url.substr(0,6);
		if (stricmp(strpro.c_str(),"ftp://")!= 0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] Can't find the FTP protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url(%s)" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if(stricmp(strMethod.c_str(), METHODTYPE_AQUA_NTFSRTF)     == 0 || 
            stricmp(strMethod.c_str(), METHODTYPE_AQUA_NTFSRTFH264) == 0 ||
			stricmp(strMethod.c_str(), METHODTYPE_AQUA_NTFSRTFH265) == 0)
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find URI resource " CPHPM_SOURCEURL);
		}
		std::string url = var2.strs[0];
		std::string strpro = "";
		if(url.size() > 7)
			strpro = url.substr(0,7);
		if (stricmp(strpro.c_str(),"cifs://")!= 0 && stricmp(strpro.c_str(),"file://")!= 0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] Can't find the CIFS protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if (stricmp(strMethod.c_str(), METHODTYPE_AQUA_RTI)     == 0 || 
             stricmp(strMethod.c_str(), METHODTYPE_AQUA_RTIH264) == 0 ||
			  stricmp(strMethod.c_str(), METHODTYPE_AQUA_RTIH265) == 0)
	{
        TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
        if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
        {
            MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
                CPHPM_SOURCEURL);
            ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find multicast resource " CPHPM_SOURCEURL);
        }

        TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
        if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
        {
            MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
                CPHPM_SOURCEURL);
            ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find multicast resource" CPHPM_SOURCEURL);
        }
		std::string url = var2.strs[0];
		std::string strpro = "";
		if(url.size() > 6)
			strpro = url.substr(0,6);

		if (stricmp(strpro.c_str(),"udp://")!= 0)
        {
            MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] Can't find the udp protocol from url %s"), sess->ident.name.c_str(), url.c_str());
            ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the udp header from url[%s]" ,url.c_str());
        }

        sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
        //sess->preload = randomPreload(_gCPHCfg.preloadTime);
        //MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaLibHelper, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	}
	else if (stricmp(strMethod.c_str(), METHODTYPE_AQUA_INDEX)     == 0 || 
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_INDEXH264) == 0 ||
		stricmp(strMethod.c_str(), METHODTYPE_AQUA_INDEXH265) == 0)
	{
		TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find multicast resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find multicast resource" CPHPM_SOURCEURL);
		}
		std::string url = var2.strs[0];
		std::string strpro = "";
		if(url.size() > 7)
			strpro = url.substr(0,7);
		if (stricmp(strpro.c_str(),"aqua://")!= 0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] Can't find the aqua protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the aqua header from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
		//sess->preload = randomPreload(_gCPHCfg.preloadTime);
		//MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaLibHelper, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	}
	else if (stricmp(strMethod.c_str(), METHODTYPE_RTIRAW) == 0)
	{
		TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find multicast resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", 0, "could not find multicast resource" CPHPM_SOURCEURL);
		}
		std::string url = var2.strs[0];
		std::string strpro = "";
		if(url.size() > 6)
			strpro = url.substr(0,6);

		if (stricmp(strpro.c_str(),"raw://")!= 0)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaLibHelper, "[%s] Can't find the raw protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>("AquaLibHelper", TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the raw header from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else
	{
		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(AquaLibHelper, "[%s] Unknown method[%s], setSessionType() with default type"), sess->ident.name.c_str(), strMethod.c_str());
	}

	return true;
}

bool CPHAquaLibSess::prime()
{
	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	if (!BaseCPHSession::prime())
		return false;

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void CPHAquaLibSess::OnProgress(int64& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);

	::TianShanIce::Properties params;
	char tmp[40];
	// query the available range info, and fill in params
	if (_pRTFProc)
	{
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		// get the range info from 
		std::map<std::string, std::string> fileinfomap;
		std::map<std::string, std::string>::iterator iter;
		std::string extCol;

		pProcess->getOutputFileInfo(fileinfomap);
		pProcess->getOutputFileExtCol(extCol);
		// set to params
		for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
		{
			params[iter->first.c_str()] = iter->second;
			//MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
		}

		//MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
		params[EVTPM_MEMBERFILEEXTS] = extCol;
	}
#ifdef PROCESSED_BY_TIME
	if(stricmp(_strMethod.c_str(), METHODTYPE_RTIRAW) == 0)
	{
		Ice::Long nRealPlayTime = ZQ::common::now() - _nsessStartTime;
		char tmp[64];

		snprintf(tmp, sizeof(tmp), "%lld", nRealPlayTime);
		params["realplaytime"] = tmp;


		snprintf(tmp, sizeof(tmp), "%lld",_nSchedulePlayTime);
		params["allplaytime"] = tmp;
	}

#endif

	sprintf(tmp, "True");
	params[EVTPM_OPENFORWRITE] = tmp;


	if(_processedTime == 0 || _gCPHCfg.minIntervalMs <= 0 || ZQ::common::now() -  _processedTime > _gCPHCfg.minIntervalMs)
	{
		updateMainMetadataOnProcess(params);
		_processedTime = ZQ::common::now();
	}

	updateProgress(_llProcBytes, _llTotalBytes, params);
}

void CPHAquaLibSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
    
    ::TianShanIce::Properties params;
    updateMainMetadataOnStreamable(params);

	notifyStreamable(bStreamable);
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void CPHAquaLibSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	if (_pRTFProc)
	{
		std::string indextype;
		
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		pProcess->getIndexType(indextype);

		params[EVTPM_INDEXEXT] = indextype;
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] IndexTypeExt[%s]"), _strLogHint.c_str(),indextype.c_str());
		
	}
	
    updateMainMetadataOnStart(params);

	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] notifyStarted() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

void CPHAquaLibSess::updateMainMetadataOnStart(::TianShanIce::Properties& params)
{
    // update metadata
    std::string strParams;
    ::TianShanIce::Properties csProEvent;
    TianShanIce::Properties::const_iterator iter;

    iter = params.find(EVTPM_MPEGBITRATE);
    if (iter != params.end()) {
        csProEvent[METADATA_BitRate] = iter->second;
        strParams += std::string(" Bitrate[") + iter->second + "]";
        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] MPEG Bitrate: %s"), contentKey.content.c_str(), iter->second.c_str());
    }

    iter = params.find(EVTPM_FRAMERATE);
    if(iter != params.end()) {
        csProEvent[METADATA_FrameRate] = iter->second;
        strParams += std::string(" Framerate[") + iter->second + "]";

        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] Framerate: %s"), contentKey.content.c_str(), iter->second.c_str());
    }

    iter = params.find(EVTPM_VIDEOHEIGHT);
    if (iter != params.end()) {
        csProEvent[METADATA_PixelVertical] = iter->second;
        strParams += std::string(" VideoHeight[") + iter->second + "]";

        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] VideoHeight: %s"), contentKey.content.c_str(), iter->second.c_str());
    }		

    iter = params.find(EVTPM_VIDEOWIDTH);
    if (iter != params.end()) {
        csProEvent[METADATA_PixelHorizontal] = iter->second;
        strParams += std::string(" VideoWidth[") + iter->second + "]";

        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] VideoWidth: %s"), contentKey.content.c_str(), iter->second.c_str());
    }

    iter = params.find(EVTPM_INDEXEXT);
    if (iter != params.end()) {
        csProEvent["sys.IndexFileExt"] = iter->second;
        strParams += std::string(" IndexFileExt[") + iter->second + "]";

        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] IndexFileExt: %s"), contentKey.content.c_str(), iter->second.c_str());
    }

    iter = params.find(EVTPM_SOURCETYPE);
    if (iter != params.end()) {
        csProEvent["sys.SourceType"] = iter->second;
        strParams += std::string(" SourceType[") + iter->second + "]";
    }

    char buf[64];
    memset(buf, 0, sizeof(buf));
    csProEvent[SYS_PROP(State)] = CRM::A3Message::A3AquaBase::stateStr(TianShanIce::Storage::csProvisioning);
    strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

    csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
    strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";


	int64 lStart = ZQ::common::now();
	_pA3AquaContentMetadata->updateMetadata(_paid, _pid, csProEvent);  
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibSess, "update metadata, paid[%s] pid[%s] params[%s] took %dms"), _paid.c_str(), _pid.c_str(), strParams.c_str(), (int)(ZQ::common::now() - lStart));

}

void CPHAquaLibSess::updateMainMetadataOnStop(bool errorOccurred, ::TianShanIce::Properties& params)
{
    ::TianShanIce::Properties csProEvent;
    std::string strTotalSize, strPlaytime, strSupportFilesize, strMd5sum;
    std::string strMsg;
    std::string strMsgCode;

    std::string strParams;
    ::TianShanIce::Properties::const_iterator it = params.find(EVTPM_TOTOALSIZE);
    if (it!=params.end())
    {
        strTotalSize = it->second;
        csProEvent[METADATA_FileSize] = strTotalSize;
        strParams += std::string(" FileSize[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [FileSize]: %s", contentKey.content.c_str(), it->second.c_str());
    }

	it = params.find(EVTPM_SOURCETYPE);
	if (it != params.end()) {
		csProEvent["sys.SourceType"] = it->second;
		strParams += std::string(" SourceType[") + it->second + "]";
	}

    it = params.find(EVTPM_PLAYTIME);
    if(it != params.end()) 
    {
        strPlaytime = it->second;
        csProEvent[METADATA_PlayTime] = strPlaytime;
        strParams += std::string(" Playtime[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [Playtime]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_SUPPORTFILESIZE);
    if(it != params.end()) 
    {
        strSupportFilesize = it->second;
        csProEvent[METADATA_SupportFileSize] = strSupportFilesize;
        strParams += std::string(" SupportFileSize[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [SupportFileSize]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_MD5CHECKSUM);
    if(it != params.end()) 
    {
        strMd5sum = it->second;
        csProEvent[METADATA_MD5CheckSum] = strMd5sum;
        strParams += std::string(" MD5[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [MD5 Checksum]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_OPENFORWRITE);
    if (it != params.end())
    {
        csProEvent["sys.OpenForWrite"] = it->second;
        strParams += std::string(" OpenForWrite[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [OpenForWrite]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    std::string strAugmentationPid, strEncrypt, strAugmentBitrate, strOrigateBitate;
    it = params.find(EVTPM_AUGMENTATIONPIDS);
    if(it != params.end()) 
    {
        strAugmentationPid = it->second;
        csProEvent[METADATA_AugmentationPids] = strAugmentationPid;
        strParams += std::string(" AugmentationpPid[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [AugmentationpPid]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_AUGMENTATEDBITRATE);
    if(it != params.end()) 
    {
        strAugmentBitrate = it->second;
        csProEvent[METADATA_AugmentedBitRate] = strAugmentBitrate;
        strParams += std::string(" AugmentedBitRate[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [AugmentedBitRate]: %s", contentKey.content.c_str(), it->second.c_str());
    }
    it = params.find(EVTPM_ORIGINALBITRATE);
    if(it != params.end()) 
    {
        strOrigateBitate = it->second;
        csProEvent[METADATA_OriginalBitRate] = strOrigateBitate;
        strParams += std::string(" OriginalBitRate[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [OriginalBitRate]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_PREENCRYPTION);
    if(it != params.end()) 
    {
        strEncrypt = it->second;
        csProEvent[METADATA_PreEncryption] = strEncrypt;
        strParams += std::string(" PreEncryption[") + it->second + "]";
        //		glog(Log::L_INFO, "(%s) [Encryption]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_ERRORCODE);
    if (it!=params.end())
    {
        strMsgCode = it->second;

        if (!strMsgCode.empty())
            strParams += std::string(" ErrCode[") + it->second + "]";
        //			glog(Log::L_INFO, "(%s) [error code]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    it = params.find(EVTPM_ERRORMESSAGE);
    if (it!=params.end())
    {
        strMsg = it->second;
        if (!strMsg.empty())
            strParams += std::string(" ErrMsg[") + it->second + "]";
        //			glog(Log::L_INFO, "(%s) [error message]: %s", contentKey.content.c_str(), it->second.c_str());
    }

    int errCode = atoi(strMsgCode.c_str());

    int nCSErrorCode;
    char buf[256];
    if (!errorOccurred)
    {
        nCSErrorCode = TianShanIce::Storage::cs200OK;	//200 means ok
    }
    else
    {
        if(errCode == ERRCODE_VSTRM_DISK_FULL) {
            nCSErrorCode = TianShanIce::Storage::csexpVstrmDiskFull;
        }
        else if(errCode == ERRCODE_VSTRM_BANDWIDTH_EXCEEDED) {
            nCSErrorCode = TianShanIce::Storage::csexpVstrmBwExceeded;
        }
        else if(errCode == ERRCODE_VSTRM_NOT_READY) {
            nCSErrorCode = TianShanIce::Storage::csexpVstrmNotReady;
        }
        else if(errCode == ERRCODE_INVALID_SRCURL) {
            nCSErrorCode = TianShanIce::Storage::csexpInvalidSourceURL;
        }
        else if(errCode == ERRCODE_USER_CANCELED) {
            nCSErrorCode = TianShanIce::Storage::csexpUserCanceled;
        }
        else {
            nCSErrorCode = TianShanIce::Storage::csexpInternalError;
        }	
    }
    sprintf(buf, "%d", nCSErrorCode);	

    csProEvent["sys.LastError"] = buf;
    uint32 lastError = (uint32) atoi(buf);

    csProEvent["sys.LastErrMsg"] = strMsg;

    memset(buf, 0, 256);
    std::string ext;
    std::vector<std::string> vecExt;
    std::vector<std::string>::iterator vecIter;

    TianShanIce::Properties::const_iterator iter;
    iter = params.find(EVTPM_MEMBERFILEEXTS);
    if (iter != params.end()) {
        csProEvent["sys.memberFileExts"] = iter->second;
        ext = iter->second;
        strParams += std::string(" memberFileExts[") + iter->second + "]";

        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] memberFileExts:[%s]"), contentKey.content.c_str(), iter->second.c_str());
    }

    vecExt = ZQ::common::stringHelper::split(ext, ';');
    for (vecIter = vecExt.begin();vecIter != vecExt.end();vecIter++)
    {
        iter = params.find(*vecIter);
        strParams += std::string("; memberFile[") + (*vecIter) + "]" ;
        if (iter != params.end()) {
            std::vector<std::string> vecSize = ZQ::common::stringHelper::split(iter->second,'-');
            std::string begOffset,endOffset;
            if (vecSize.size() == 2)
            {
                begOffset = vecSize[0];
                endOffset = vecSize[1];
                std::string strKeyFileSize;
                std::string strKeyOffset; 
                if (vecIter == vecExt.begin())
                {
                    strKeyFileSize = std::string("sys.FileSize");
                    strKeyOffset = std::string("sys.FirstOffset");
                    //csProEvent[strKeyFileSize] = csProEvent["sys.ProgressTotal"];
                }
                else
                {
                    strKeyFileSize = std::string("sys.FileSize.")+(*vecIter);
                    strKeyOffset = std::string("sys.FirstOffset.")+(*vecIter);
                    csProEvent[strKeyFileSize] = endOffset;
                }

                csProEvent[strKeyOffset] = begOffset;
                strParams += std::string(" range[") + begOffset + "~" + csProEvent[strKeyFileSize] + "]";

                // MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] FileExt[%s],FirstOffset[%s],FileSize[%s]"), contentKey.content.c_str(),(*vecIter).c_str(),begOffset.c_str(),csProEvent[strKeyFileSize].c_str());
            }

        }
    }

    if (lastError < 300)
    {
        csProEvent[SYS_PROP(State)] =  CRM::A3Message::A3AquaBase::stateStr(TianShanIce::Storage::csInService);

        strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

        csProEvent[SYS_PROP(StampProvisioned)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
        strParams += std::string(" sys.StampProvisioned[") + csProEvent[SYS_PROP(StampProvisioned)]  + "]";

        csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
        strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";
    }
    else //上传失败，清掉临时文件
    {
        csProEvent[SYS_PROP(State)] =  CRM::A3Message::A3AquaBase::stateStr(TianShanIce::Storage::csOutService);
        strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

        csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
        strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

        //std::string strMdataFileName = _pA3AquaContentMetadata->getAquaContentMDName(_paid, _pid);

        //MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "provision failed, delete temp file[%s]"), strMdataFileName.c_str());
        //_pA3AquaContentMetadata->deleteFile(strMdataFileName); 
    }

	iter = params.find("realplaytime");
	if (iter != params.end())
		csProEvent["sys.realplaytime"] = iter->second;

	iter = params.find("allplaytime");
	if (iter != params.end())
		csProEvent["sys.allplaytime"] = iter->second;

	iter = params.find(EVTPM_MPEGBITRATE);
	if (iter != params.end()) {
		csProEvent[METADATA_BitRate] = iter->second;
		strParams += std::string(" Bitrate[") + iter->second + "]";
	}

	int64 lStart = ZQ::common::now(); 
	_pA3AquaContentMetadata->updateMetadata(_paid, _pid, csProEvent);   
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibSess, "updateMainMetadataOnStop, paid[%s] pid[%s] params[%s] took %dms"), _paid.c_str(), _pid.c_str(), strParams.c_str(), (int)(ZQ::common::now() - lStart));
}

void CPHAquaLibSess::updateMainMetadataOnProcess(::TianShanIce::Properties& params)
{
    // update mainfile metadata
    char buf[256];
    std::string ext, strParams;
    std::vector<std::string> vecExt;
    std::vector<std::string>::iterator vecIter;

    ::TianShanIce::Properties csProEvent;
    sprintf(buf, FMT64, _filesize);
    csProEvent["sys.ProgressTotal"] = buf;
    sprintf(buf, FMT64, _llProcBytes);
    csProEvent["sys.ProgressProcessed"] = buf;

    TianShanIce::Properties::const_iterator iter;
    iter = params.find(EVTPM_MEMBERFILEEXTS);
    if (iter != params.end()) {
        csProEvent["sys.memberFileExts"] = iter->second;
        strParams += std::string(" memberFileExts[") + iter->second + "]";
        ext = iter->second;
        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] memberFileExts:[%s]"), contentKey.content.c_str(), iter->second.c_str());
    }

    iter = params.find(EVTPM_OPENFORWRITE);
    if (iter != params.end()) {
        csProEvent["sys.OpenForWrite"] = iter->second;
        strParams += std::string(" OpenForWrite[") + iter->second + "]";
        //		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] OpenForWrite:[%s]"), contentKey.content.c_str(), iter->second.c_str());
    }

//#ifdef PROCESSED_BY_TIME
    iter = params.find("realplaytime");
    if (iter != params.end())
        csProEvent["sys.realplaytime"] = iter->second;

    iter = params.find("allplaytime");
    if (iter != params.end())
        csProEvent["sys.allplaytime"] = iter->second;
//#endif//PROCESSED_BY_TIME

    vecExt = ZQ::common::stringHelper::split(ext, ';');
    for (vecIter = vecExt.begin();vecIter != vecExt.end();vecIter++)
    {
        strParams += std::string("; memberFile[") + (*vecIter) + "]" ;
        iter = params.find(*vecIter);
        if (iter != params.end()) {
            std::vector<std::string> vecSize = ZQ::common::stringHelper::split(iter->second,'-');
            std::string begOffset,endOffset;
            if (vecSize.size() == 2)
            {
                begOffset = vecSize[0];
                endOffset = vecSize[1];
                std::string strKeyFileSize;
                std::string strKeyOffset; 
                if (vecIter == vecExt.begin())
                {
                    strKeyFileSize = std::string("sys.FileSize");
                    strKeyOffset = std::string("sys.FirstOffset");
                    csProEvent[strKeyFileSize] = csProEvent["sys.ProgressTotal"];

                }
                else
                {
                    strKeyFileSize = std::string("sys.FileSize.")+(*vecIter);
                    strKeyOffset = std::string("sys.FirstOffset.")+(*vecIter);
                    csProEvent[strKeyFileSize] = endOffset;
                }
                csProEvent[strKeyOffset] = begOffset;
                strParams += std::string(" range[") + begOffset + "~" + csProEvent[strKeyFileSize] + "]";

                //				MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] FileExt[%s],FirstOffset[%s],FileSize[%s]"), contentKey.content.c_str(),(*vecIter).c_str(),begOffset.c_str(),csProEvent[strKeyFileSize].c_str());
            }

        }
    }

    {
        char buf[64];
        memset(buf, 0, sizeof(buf));
        csProEvent[SYS_PROP(State)] = CRM::A3Message::A3AquaBase::stateStr(TianShanIce::Storage::csProvisioning);
        strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

        csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
        strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";
    }
    
	int64 lStart = ZQ::common::now(); 
	_pA3AquaContentMetadata->updateMetadata(_paid, _pid, csProEvent);
    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibSess, "updateMainMetadataOnProcess(), paid[%s] pid[%s] params[%s] took %dms"), _paid.c_str(), _pid.c_str(), strParams.c_str(), (int)(ZQ::common::now() - lStart));
}

void CPHAquaLibSess::updateMainMetadataOnStreamable(::TianShanIce::Properties& params)
{
    ::TianShanIce::Properties csProEvent;
    std::string strParams;

    char buf[64];
    memset(buf, 0, sizeof(buf));
    csProEvent[SYS_PROP(State)] = CRM::A3Message::A3AquaBase::stateStr(TianShanIce::Storage::csProvisioningStreamable);
    strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

    csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
    strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

	int64 lStart = ZQ::common::now();
    _pA3AquaContentMetadata->updateMetadata(_paid, _pid, csProEvent);
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "updateMainMetadataOnStreamable(), paid[%s] pid[%s] params[%s] took %dms"), _paid.c_str(), _pid.c_str(), strParams.c_str(), (int)(ZQ::common::now() - lStart));
}

bool CPHAquaLibSess::updateMainFileMetadata()
{
    std::string strMdataFileName;
    std::string strMainFileName;
    ::TianShanIce::ContentProvision::ProvisionContentKey contentKey = _sess->getContentKey();
    if(_paid.empty() || _pid.empty())
    {
        strMdataFileName = contentKey.content + _pA3AquaContentMetadata->getAquaMdataFileExtension();
        strMainFileName  = contentKey.content + _pA3AquaContentMetadata->getMainFileExtension();
    }
    else
    {
        strMdataFileName = _pA3AquaContentMetadata->getAquaContentMDName(_paid, _pid);
        strMainFileName  = _pA3AquaContentMetadata->getMainFileName(_paid, _pid);
    }

    ///1. read metadata from content.mdata File
	int cdmiCode;
    TianShanIce::Properties metadatas;
    if(!_pA3AquaContentMetadata->getMDFileMetadata(strMdataFileName,metadatas,cdmiCode))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "updateMainFileMetadata() failed to get metadata from %s with error[%d]"), strMdataFileName.c_str(), cdmiCode);
        return false;
	}

    ///2.update metadata to mainFile metadata
    return _pA3AquaContentMetadata->updateMainFileMetadata(strMainFileName,metadatas);
}

void CPHAquaLibSess::OnPreloadFailed()
{
    TianShanIce::Properties params;
    params[EVTPM_ERRORMESSAGE] = _strErrMsg;		
    char errcode[16];
    sprintf(errcode, "%d", _nErrCode);
    params[EVTPM_ERRORCODE] = errcode;

    updateMainMetadataOnStop(true, params);
	_pA3AquaContentMetadata->deleteFile(_pA3AquaContentMetadata->getAquaContentMDName(_paid, _pid));
}

		/////////////////////////////////////////////////////////////////////////
		//////////// class CPHAquaLibAutoCheckSess //////////////////////////////
		/////////////////////////////////////////////////////////////////////////

		CPHAquaLibAutoCheckSess::CPHAquaLibAutoCheckSess(ZQ::common::Log* log, ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: CPHAquaLibSess(log, helper,pSess)
		{
			_pSource = NULL;
		}

		CPHAquaLibAutoCheckSess::~CPHAquaLibAutoCheckSess()
		{

		}
		bool CPHAquaLibAutoCheckSess::checkFileFormat()
		{
			std::string strFileNameTemp = _strFileName + ".checkFF";
			bool bChecked = false;
			ZQ::common::URLStr sourceURL(_sourceURL.c_str());
			BaseSource* pSource = NULL;
			if (!stricmp((char*)_protocol.c_str(), "file") || !stricmp((char*)_protocol.c_str(), "cifs"))
			{
				std::string host =  sourceURL.getHost();
				std::string sourceFilename =  sourceURL.getPath();
				std::string strOpt,strsystype,strsharePath;
				bool bLocalSrcFlag;
				strsystype = "cifs";
				if (host.empty() || 0 == host.compare(".") || 0 == host.compare("localhost"))
				{
					bLocalSrcFlag = true;
					fixpath(sourceFilename, true);
					strOpt = "username=,password=";
				}
				else
				{
					bLocalSrcFlag = false;
					fixpath(sourceFilename, false);
					strsharePath =std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + sourceURL.getHost() + LOGIC_FNSEPS + sourceFilename.substr(0,sourceFilename.find_first_of(FNSEPC));
					sourceFilename = sourceFilename.substr(sourceFilename.find_first_of(FNSEPC)+1);
					fixpath(sourceFilename, false);
					strOpt = "username=" + std::string( sourceURL.getUserName()) + ",password=" + std::string( sourceURL.getPwd());
				}

				CIFSIOSource* fsSource = (CIFSIOSource*)SourceFactory::Create(SOURCE_TYPE_CIFS, &_helper._pool);
				if (!fsSource)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] create CIFS source failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}

				fsSource->SetLog(&glog);
				fsSource->SetLogHint(_strFileName.c_str());
				if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
				{
					fsSource->setSourceUrlUTF8(true);
				}
				fsSource->setIOFactory(_pCifsFileIoFac.get());
				fsSource->setFileName(sourceFilename.c_str());
				fsSource->setMountOpt(strOpt);
				fsSource->setSystemType(strsystype);
				fsSource->setLocalSourceFlag(bLocalSrcFlag);
				if (!bLocalSrcFlag)
				{
					fsSource->setSharePath(strsharePath);
#ifdef ZQ_OS_LINUX
					///preload中已经Mount好了，只需要去读文件即可
					fsSource->setMountPath(_strMountPoint);
#endif
				}
				pSource = fsSource;
			}
			else if (!stricmp((char*)_protocol.c_str(), "ftp"))
			{
				FTPIOSource* ftpSource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP, &_helper._pool);
				if( ftpSource == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] create FTP source failed"), _strLogHint.c_str());
					return false;
				}
				ftpSource->SetLog(&glog);
				ftpSource->SetLogHint(_strFileName.c_str());
				ftpSource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
				ftpSource->setURL( _sourceURL.c_str());
				ftpSource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
				if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
				{
					ftpSource->setSourceUrlUTF8(true);
				}
				pSource = ftpSource;

			}	
			else if(!stricmp((char*)_protocol.c_str(), "udp"))
			{
				std::string multicastUrl =  _sourceURL;
				std::string multicastIp =  _sourceURL.substr( _sourceURL.find_first_of(':')+3, _sourceURL.find_last_of(':')-6);
				std::string strmulticastPort =  _sourceURL.substr( _sourceURL.find_last_of(':')+1, _sourceURL.size()- _sourceURL.find_last_of(':')-1);
				int multicastPort = atoi(strmulticastPort.c_str());

				DWORD timeoutInterval = _gCPHCfg.capture.sessionTimeout;
				std::string localIp;
				if (!_nNetSelector->allocInterface(_nMaxBandwidth,localIp,_strFileName))
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
					return false;
				}
				if (!HostToIP::translateHostIP(localIp.c_str(), _strLocalIp))//translate host name to ip
					_strLocalIp = localIp;

				McastCapSource* mcastSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);
				if(mcastSource == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] create mcast source failed"), _strLogHint.c_str());
					return false;
				}

				mcastSource->SetLog(&glog);
				mcastSource->SetLogHint(_strFileName.c_str());
				mcastSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,_strLocalIp);
				pSource = mcastSource;
			}
		  /*else if(!stricmp((char*)_protocol.c_str(), "aqua"))
			{
				std::string host =  sourceURL.getHost();
				std::string sourceFilename =  sourceURL.getPath();
				AquaIOSource* aquaSource = (AquaIOSource*)SourceFactory::Create(SOURCE_TYPE_AQUA, &_helper._pool);
				if (!aquaSource)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create aqua source failed"),_strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				aquaSource->setIOFactory(_pFileIoFac.get());
				aquaSource->setFileName(sourceFilename.c_str());
				aquaSource->setMaxBandwidth(_nMaxBandwidth);
				pSource = aquaSource;
			}*/
			else
			{
				setErrorInfo(ERRCODE_AQUA_CSI_UNSUPPORT_PROTOCOL, "CSI unsupport protocol");
				return false;
			}
			pSource->SetGraph(this);
			pSource->InitPins();
			if(!pSource->Init())
			{
				delete pSource;
				return false;
			}
			pSource->Start();

			MediaSample* pSample = pSource->GetData();
			if (NULL == pSample)
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] checkFileFormat() GetData return 0"), _strLogHint.c_str());

				if(!stricmp((char*)_protocol.c_str(), "udp"))
				{
					_nNetSelector->freeInterface(strFileNameTemp);
				}

				pSource->Stop();
				pSource->Close();
				delete pSource;

				setErrorInfo(ERRCODE_AQUA_READFILE, "faided to read file");
				return false;
			}

			CSILib::CSI_SES_HANDLE phSession = NULL;
			CSILib::CSI_ERROR error =  CSILib::csiAddSession(&phSession );
			if(error != CSI_MSG_SUCCESS)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s]failed to create csi sSession"), _strLogHint.c_str());

				if(!stricmp((char*)_protocol.c_str(), "udp"))
				{
					_nNetSelector->freeInterface(strFileNameTemp);
				}
				pSource->Stop();
				pSource->Close();
				delete pSource;

				setErrorInfo(ERRCODE_AQUA_CSI_ADDSESSION, "faided to create csi session");
				return false;
			}

			do 
			{
				pSample->addRef();

				CSILib::CSI_STREAM_INFORMATION streamInfo;
				memset((void*)&streamInfo, 0, sizeof(CSILib::CSI_STREAM_INFORMATION));

				error = CSILib::csiProcessBuffer(phSession, (BYTE *)pSample->getPointer(),pSample->getBufSize(), &streamInfo);

				if(! (streamInfo.flags &  CSI_INFO_VCODEC))
				{
					freeMediaSample(pSample);
					pSample = pSource->GetData();
					continue;
				}
				else
				{
					MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibAutoCheckSess,"[%s]checkFileFormat() flags(%d), length(%d), videoCodec(%s), TBD(%s)"), _strLogHint.c_str(), streamInfo.flags, streamInfo.length, streamInfo.videoCodec, streamInfo.TBD);
					if(!stricmp(streamInfo.videoCodec,"H.264"))
					{
						_sourceType = TianShanIce::Storage::ctH264;	
					}
					else if(!stricmp(streamInfo.videoCodec,"Mpeg2"))
					{
						_sourceType = TianShanIce::Storage::ctMPEG2TS;	
					}
					else if(!stricmp(streamInfo.videoCodec,"H.265"))
					{
						_sourceType = TianShanIce::Storage::ctH265;	
						_enableNoTrickSpeed = true;
					}
					else
					{
						setErrorInfo(ERRCODE_AQUA_CSI_UNKNOWN_CONTENTTYPE, "unknown contentType");
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess,"[%s]checkFileFormat() unkonwn source file format"), _strLogHint.c_str());
						break;
					} 
					bChecked = true;
					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibAutoCheckSess,"[%s]checkFileFormat() source contentType =[%s]"), _strLogHint.c_str(), streamInfo.videoCodec);
					break;

				}

			} while(pSample != NULL);


			if(pSample)
			{
				freeMediaSample(pSample);
				pSample = NULL;
			}

			if(phSession)
				CSILib::csiCloseSession(phSession);

			if(!stricmp((char*)_protocol.c_str(), "udp"))
			{
				_nNetSelector->freeInterface(strFileNameTemp);
			}

			pSource->Stop();
			pSource->Close();
			delete pSource;
			return bChecked;
		}
		bool  CPHAquaLibAutoCheckSess::setSpeedInfo()
		{
			ZQ::common::MD5ChecksumUtil md5;
			md5.checksum(_strFileName.c_str(), _strFileName.length());
			std::string strMd5 = md5.lastChecksum();

			unsigned short uFileCiscoExt = atoi(strMd5.substr(14, 2).c_str());
			uFileCiscoExt &= 0xfff0;

			transform (strMd5.begin(), strMd5.end(), strMd5.begin(), (int(*)(int))toupper);
			std::string strFileCiscoExt = strMd5.substr(0, 14);

			char buf[64] = "";
			memset(buf, 0, 64);
			snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
			std::string mainFileExt="";
			std::string replacePAID = replaceString(_paid);
			std::string replacePID = replaceString(_pid);
			if( 1 == _gCPHCfg.ciscofileext.mode)
			{
				mainFileExt = std::string(buf);
			}
			else if( 2 == _gCPHCfg.ciscofileext.mode)
			{
				memset(buf, 0, 64);
				snprintf(buf, sizeof(buf) - 2, ".00%s_%s\0", replacePAID.c_str(), replacePID.c_str());
				mainFileExt =  std::string(buf);
			}
			FileExtensions exMap;
			FileExtensions::iterator iter;
			std::string key="";
			std::map<std::string , int> exMapOutPutfile;
			//		trickspeed.sort();
			int index = 0;

			for (int i = 0; i < (int)_trickspeed.size(); i++)
			{
				char ex[10]={0};
				char exr[10] ={0};

				FileExtension fileExt;
				if (_sourceType == TianShanIce::Storage::ctH264 && _gCPHCfg.unifiedtrickfile.enable) //.h264 wgk
				{
					memset(buf, 0, 64);
					getUnifiedTrickExt(index,ex);
					key = std::string(ex);
					if( 1 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
					{
						++uFileCiscoExt;
						snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
						exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
						key = std::string(buf);
					}
					else if( 2 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
					{
						memset(buf, 0, 64);
						snprintf(buf, sizeof(buf)-2, ".B%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
						exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
						key = std::string(buf);
					}
					else
					{
						exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
					}
					fileExt.ext = ex;
					fileExt.extForCisco = buf;
					fileExt.position = i;
					exMap.insert(std::make_pair(key, fileExt));
					//				exMap.push_back(fileExt);
				}
				else
				{
					getTrickExt(index,ex,exr);
					key = std::string(ex);
					memset(buf, 0, 64);
					if( 1 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
					{
						++uFileCiscoExt;	
						snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
						exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
						key = std::string(buf);
					}
					else if( 2 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)// NVOD1234567890abcdef_cctv.com.F0NVOD1234567890abcdef_cctv$com
					{
						memset(buf, 0, 64);
						snprintf(buf, sizeof(buf)-2, ".F%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
						exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
						key = std::string(buf);
					}
					else
						exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
					fileExt.ext = ex;
					fileExt.extForCisco = buf;
					fileExt.position = i;
					exMap.insert(std::make_pair(key, fileExt));
					//                exMap.push_back(fileExt);
					key = std::string(exr);
					memset(buf, 0, 64);
					if( 1 == _gCPHCfg.ciscofileext.mode && _bIndexVVC)
					{
						++uFileCiscoExt;
						snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
						exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
						key = std::string(buf);
					}
					else if( 2 == _gCPHCfg.ciscofileext.mode && _bIndexVVC) // NVOD1234567890abcdef_cctv.com.R0NVOD1234567890abcdef_cctv$com
					{
						memset(buf, 0, 64);
						snprintf(buf, sizeof(buf)-2, ".R%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
						exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
						key = std::string(buf);
					}
					else
						exMapOutPutfile.insert(std::make_pair(std::string(exr), i));
					fileExt.ext = exr;
					fileExt.extForCisco = buf;
					fileExt.position = i;
					exMap.insert(std::make_pair(key, fileExt));
					//		exMap.push_back(fileExt);
				}
				index++;
			}

			int outPutNum = 2 + exMap.size();

			if(_enableNoTrickSpeed)
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] no trickspeed file"), _strLogHint.c_str());
				outPutNum = 2;
				exMap.clear();
			}

			if ( !_enableNoTrickSpeed && outPutNum < 2)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] Not specify trickspeed"), _strLogHint.c_str());
				return false;
			}

			if(NULL == _pRTFProc)
			{
				return false;
			}

			RTFProcess* pProcess = (RTFProcess*)_pRTFProc;

			if(!_enableNoTrickSpeed)
			{
				pProcess->setTrickFileEx(exMap);
				pProcess->settrickSpeedNumerator(_trickspeed);
			}

			pProcess->setRetryCount(_gCPHCfg.retryCapture.retrycount);
			pProcess->settrickSpeedNumeratorHD(_trickspeedHD);
			pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);
			if( 0 != _gCPHCfg.ciscofileext.mode)
				pProcess->setCsicoFileExt(1);
			pProcess->setCsicoMainFileExt(mainFileExt);

#ifndef ZQ_OS_MSWIN
			if(_bIndexVVC)
			{
				if(_sourceType == TianShanIce::Storage::ctH265)
					pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_H265);
				else if(_sourceType == TianShanIce::Storage::ctH264)
					pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_H264);
				else
					pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_MPEG2);

				pProcess->setAssetInfo(_pid,_paid);
			}
			else
#endif
				if (_sourceType == TianShanIce::Storage::ctH264)
				{
					pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
				}
				else
				{
					if(_sourceType == TianShanIce::Storage::ctH265)
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_H265);
					else
						pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVX, CTF_VIDEO_CODEC_TYPE_MPEG2);

				}			
			if(NULL == _pMainTarget)
			{
				return false;
			}

			AquaFilesetTarget* pTarget = (AquaFilesetTarget*)_pMainTarget;

			if(!_enableNoTrickSpeed)
			{
				pTarget->setTrickFile(exMapOutPutfile);
				pTarget->setTrickSpeed(_trickspeed);
			}
			pTarget->setCsicoMainFileExt(mainFileExt);

			if (!_bIndexVVC  && _sourceType == TianShanIce::Storage::ctH264)
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] H264 type"), _strLogHint.c_str());
				pTarget->setTypeH264();
				pTarget->enableStreamableEvent(false);
			}
			else
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] VVC type"), _strLogHint.c_str());
				//pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
				pTarget->setIndexType(_bIndexVVC);
				pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
			}

			InitPins();

			if (!ConnectTo(_pSource, 0, pProcess, 0))
			{
				return false;
			}

			for (int i = 0; i < outPutNum; i++)
				if (!ConnectTo(pProcess, i, pTarget, i))
				{
					return false;
				}

				if (!Init())
				{
#ifdef ZQ_OS_LINUX
					if(_bSuccessMount)
					{
						umountURL(_sharePath, _strLogHint);
					}
#endif
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
					setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
					return false;
				}

			return true;

		}

		bool CPHAquaLibAutoCheckSess::Start()
		{
			if(!checkFileFormat())
			{
				OnPreloadFailed();
				return false;
			}

			if(!setSpeedInfo())
			{
				OnPreloadFailed();
				return false;
			}
	/*		std::vector<BaseFilter*>::iterator it;
			for(it=_filters.begin();it!=_filters.end();it++)
			{
				if (*it!=NULL)
				{
					if (!(*it)->Start())
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] (%s) failed to start"), _strLogHint.c_str(), (*it)->GetName());
						_pA3AquaContentMetadata->deleteFile(_pA3AquaContentMetadata->getAquaContentMDName(_paid, _pid));
						return false;
					}			

					MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] (%s) started successfully"), _strLogHint.c_str(), (*it)->GetName());
				}
			}

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] Graph started"), _strLogHint.c_str());

			return true;*/
			return CPHAquaLibSess::Start();
		}

		bool CPHAquaLibAutoCheckSess::preLoad()
		{

			if(NULL == _pA3AquaContentMetadata)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "Aqua server is not initalize"));
				setErrorInfo(ERRCODE_AQUA_NOTINIT, "Aqua server is not initalize");
				OnPreloadFailed();
				return false;
			}

			if (!_sess)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "provision session is 0"));
				setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");
				OnPreloadFailed();
				return false;
			}

			_pRTFProc = NULL;
			_strMethod = _sess->methodType;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] provision method[%s]"),_sess->ident.name.c_str(),_strMethod.c_str());
			if(stricmp(_strMethod.c_str(), METHODTYPE_AQUA_CSI))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
					_strMethod.c_str());
				setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");
				OnPreloadFailed();
				return false;
			}

			std::string strFilename;
			std::string strSrcUrl;
			if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] could not find resource URI"), _sess->ident.name.c_str());
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");
				OnPreloadFailed();
				return false;
			}

			TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
			if (resURI.end() == resURI.find(CPHPM_FILENAME))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
					CPHPM_FILENAME);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);
				OnPreloadFailed();
				return false;
			}

			TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
			if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
					CPHPM_FILENAME);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);
				OnPreloadFailed();
				return false;
			}

			strFilename = var.strs[0];
			_strFileName = strFilename;

			if (resURI.end() != resURI.find(CPHPM_SOURCEURL))
			{
				TianShanIce::Variant& var2 = resURI[CPHPM_SOURCEURL];
				if (var2.type == TianShanIce::vtStrings && var2.strs.size()>0)
				{
					strSrcUrl = var2.strs[0];
				}
			}

			TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
			if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
					CPHPM_BANDWIDTH);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);
				OnPreloadFailed();
				return false;
			}

			TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
			if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
					CPHPM_BANDWIDTH);
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);
				OnPreloadFailed();
				return false;
			}
			int nBandwidth = (int)var1.lints[0];
			_nBandwidth = nBandwidth;

			TianShanIce::Properties prop = _sess->props;
			TianShanIce::Properties::const_iterator it = prop.find(CPHPM_INDEXTYPE);
			if (it!=prop.end())
			{
				if (stricmp(it->second.c_str(),"VVC") == 0)
				{
					_bIndexVVC = true;				
				}
			}

			it = prop.find(CPHPM_PROVIDERID);
			if (it!=prop.end())
				_pid = it->second;

			it = prop.find(CPHPM_PROVIDERASSETID);
			if (it!=prop.end())
				_paid = it->second;

			it = prop.find(CPHPM_NOTRICKSPEEDS);
			if (it != prop.end())
			{
				int major = 0, minor = 0;
				RTFProcess::getCTFLibVersion(major, minor);
				if(major < 3)
				{
					MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] CTFLib Verion support No TrickSpeed File must >= 3.0 "), _sess->ident.name.c_str());
				}
				_enableNoTrickSpeed = true;
			}
			if (_paid.empty() || _pid.empty())
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "paid pid is empty"));
				setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find paid pid");
				OnPreloadFailed();
				return false;
			}

			memset((void*) _augmentationPids, 0 , sizeof(_augmentationPids));
			_augmentationPidCount = 0;

			std::string strPIDs = "";
			it = prop.find(CPHPM_AUGMENTATIONPIDS);
			if (it!=prop.end())
			{
				strPIDs = it->second;
			}
			else
			{
				strPIDs = _gCPHCfg.strAugmentationPids;
			}

			if(!strPIDs.empty())
			{
				TianShanIce::StrValues  strAugmentationPids;
				::ZQ::common::stringHelper::SplitString(strPIDs, strAugmentationPids, ";,");
				unsigned int i;
				for(i = 0; i < strAugmentationPids.size() && i < MAX_AUGMENTATION_PIDS; ++i)
				{
					_augmentationPids[i] = atoi(strAugmentationPids[i].c_str());
				}
				_augmentationPidCount = strAugmentationPids.size();
			}

			::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibAutoCheckSess, "trickSpeed from session [%s]"), formatSpeed(trickcol).c_str());

			if (trickcol.size() == 0 || (trickcol.size() == 1 && fabs(trickcol[0]) <=1e-6 )) //add compare folat 
				trickcol.push_back(7.5);

			bool bFoundHD = false;
			for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
			{
				if(fabs(*iterTrick) <= 1e-6)
				{
					bFoundHD = true;
					continue;
				}
				if(bFoundHD)
				{
					_trickspeedHD.push_back((*iterTrick));
				}
				else
					_trickspeed.push_back((*iterTrick));		
			}

			if(_trickspeed.empty() && _trickspeedHD.size() > 0)
			{
				_trickspeed.assign(_trickspeedHD.begin(), _trickspeedHD.end());
			}

			if(!_trickspeed.empty() && !_trickspeedHD.empty() && _trickspeed.size() != _trickspeedHD.size())
			{
				_trickspeed.sort();
				_trickspeedHD.sort();

				if(_trickspeed.size() < _trickspeedHD.size())
				{
					int npos = _trickspeedHD.size() - _trickspeed.size();
					std::list<float>::iterator itorSpeed =  _trickspeedHD.begin();
					for(uint i = 0; i < _trickspeedHD.size() - npos ; i++)
					{
						itorSpeed++;
					}
					for(uint i = 0; i < npos ; i++)
					{
						_trickspeed.push_back(*itorSpeed);
						itorSpeed++;
					}
				}
				else
				{
					int npos = _trickspeed.size() - _trickspeedHD.size();
					std::list<float>::iterator itorSpeed =  _trickspeed.begin();
					for(uint i = 0; i < _trickspeed.size() - npos ; i++)
					{
						itorSpeed++;
					}
					for(uint i = 0; i < npos ; i++)
					{
						_trickspeedHD.push_back(*itorSpeed);
						itorSpeed++;
					}
				}
				_trickspeed.sort();
				_trickspeedHD.sort();
			}


			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] sourceUrl[%s], SD trickspeed [%s], HD trickSpeed[%s]"),
				_strLogHint.c_str(),strSrcUrl.c_str(), formatSpeed(_trickspeed).c_str(), formatSpeed(_trickspeedHD).c_str());

			SetLog(_helper.getLog());
			SetMemAlloc(_helper.getMemoryAllocator());
			SetLogHint(strFilename.c_str());

			if (_gCPHCfg.bandwidthLimitRate)
			{
				_nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
			}
			else
				_nMaxBandwidth = nBandwidth;

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
				nBandwidth, _gCPHCfg.bandwidthLimitRate, _nMaxBandwidth);

			ZQ::common::URLStr src(strSrcUrl.c_str());
			const char* proto = src.getProtocol();

			/*if(proto == NULL)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] could not find Protocol in sourceUrl %s"),_strLogHint.c_str(),  strSrcUrl.c_str());
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>("CPHAquaLibAutoCheckSess", 0, " could not find Protocol in sourceUrl: %s",  strSrcUrl.c_str());
			}*/

			_sourceURL = strSrcUrl;
			_protocol = proto;

			if (!stricmp(proto, "file") || !stricmp(proto, "cifs"))
			{
				std::string host = src.getHost();
				std::string sourceFilename = src.getPath();
				std::string strOpt,strsystype,strsharePath;
				bool bLocalSrcFlag;
				strsystype = "cifs";
				if (host.empty() || 0 == host.compare(".") || 0 == host.compare("localhost"))
				{
					bLocalSrcFlag = true;
					fixpath(sourceFilename, true);
					strOpt = "username=,password=";
				}
				else
				{
					bLocalSrcFlag = false;
					fixpath(sourceFilename, false);
					strsharePath =std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + src.getHost() + LOGIC_FNSEPS + sourceFilename.substr(0,sourceFilename.find_first_of(FNSEPC));
					sourceFilename = sourceFilename.substr(sourceFilename.find_first_of(FNSEPC)+1);
					fixpath(sourceFilename, false);
					strOpt = "username=" + std::string(src.getUserName()) + ",password=" + std::string(src.getPwd());
				}

				CIFSIOSource* fsSource = (CIFSIOSource*)SourceFactory::Create(SOURCE_TYPE_CIFS, &_helper._pool);
				if (!fsSource)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create CIFS source failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				AddFilter(fsSource);

				if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
				{
					fsSource->setSourceUrlUTF8(true);
				}
				fsSource->setIOFactory(_pCifsFileIoFac.get());
				fsSource->setFileName(sourceFilename.c_str());
				fsSource->setMaxBandwidth(_nMaxBandwidth);
				fsSource->setMountOpt(strOpt);
				fsSource->setSystemType(strsystype);
				fsSource->setLocalSourceFlag(bLocalSrcFlag);
				if (!bLocalSrcFlag)
				{
					fsSource->setSharePath(strsharePath);
#ifdef ZQ_OS_LINUX
					bool bMount = mountURL(strsharePath, strsystype, strOpt, _strMountPoint, _strLogHint);
					if(!bMount)
					{
						OnPreloadFailed();
						return false;
					}
					_bSuccessMount = true;
					_sharePath = strsharePath;
					fsSource->setMountPath(_strMountPoint);
#endif
				}
				_pSource = fsSource;
			}
			else if (!stricmp(proto, "ftp"))
			{
				FTPIOSource* ftpSource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP, &_helper._pool);
				if( ftpSource == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] create FTP source failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				AddFilter(ftpSource);    //only after this, the log handle will be parsed in
				ftpSource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
				ftpSource->setURL(_sourceURL.c_str());
				ftpSource->setMaxBandwidth(_nMaxBandwidth);
				ftpSource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
				if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
				{
					ftpSource->setSourceUrlUTF8(true);
				}
				//ftpsource->setConnectionMode(_gCPHCfg.enableFtpPassiveMode);
				//ftpsource->setConnectionInterval(_gCPHCfg.ftpConnectionInterval);
				_pSource = ftpSource;
			}	
			else if (!stricmp(proto, "udp"))
			{
				std::string multicastIp = strSrcUrl.substr(strSrcUrl.find_first_of(':')+3,strSrcUrl.find_last_of(':')-6);
				std::string strmulticastPort = strSrcUrl.substr(strSrcUrl.find_last_of(':')+1,strSrcUrl.size()-strSrcUrl.find_last_of(':')-1);
				int multicastPort = atoi(strmulticastPort.c_str());

				DWORD timeoutInterval = _gCPHCfg.capture.sessionTimeout;
				std::string localIp;
				if (!_nNetSelector->allocInterface(_nMaxBandwidth,localIp,_strFileName))
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				if (!HostToIP::translateHostIP(localIp.c_str(), _strLocalIp))//translate host name to ip
					_strLocalIp = localIp;

				McastCapSource* mcastSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);
				if(mcastSource == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibAutoCheckSess, "[%s] create mcast source failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				AddFilter(mcastSource);
				mcastSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,_strLocalIp);
				//dumper parameters
				mcastSource->enableDump(_gCPHCfg.captureDumper.enable==1);
				mcastSource->setDumpPath(_gCPHCfg.captureDumper.dumpPath.c_str());
				mcastSource->deleteDumpOnSuccess(_gCPHCfg.captureDumper.deleteOnSuccess);

				_pSource = mcastSource;
			}
/*			else if ( 0 ==!stricmp(proto, "aqua"))
			{
#pragma message(__MSGLOC__"TODO: 加入Aqua Source处理")
				std::string host = src.getHost();
				std::string sourceFilename = src.getPath();
				AquaIOSource* aquaSource = (AquaIOSource*)SourceFactory::Create(SOURCE_TYPE_AQUA, &_helper._pool);
				if (!aquaSource)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create aqua source failed"),_strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				AddFilter(aquaSource);
				aquaSource->setIOFactory(_pFileIoFac.get());
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(CPHAquaLibSess, "[%s] set sourceFilename[%s]"),_strLogHint.c_str(),sourceFilename.c_str());
				aquaSource->setFileName(sourceFilename.c_str());
				aquaSource->setMaxBandwidth(nMaxBandwidth);
				_pSource = aquaSource;
			}*/
			else
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] url protocol[%s] not support"), _strLogHint.c_str(),proto);
				OnPreloadFailed();
				return false;
			}

			RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
			if(pProcess == NULL)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create process  failed"), _strLogHint.c_str());
				OnPreloadFailed();
				return false;
			}
			AddFilter(pProcess);

			_pRTFProc = pProcess;

			{
				AquaFilesetTarget* pTarget = (AquaFilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_AQUAFILESET);
				if(pTarget == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CPHAquaLibSess, "[%s] create AquaFileset Target failed"), _strLogHint.c_str());
					OnPreloadFailed();
					return false;
				}
				pTarget->setPacingFactory(_pPacedIndexFac);
				pTarget->enableCacheForIndex(_gCPHCfg.enableCacheForIndex);

				if(!AddFilter(pTarget))
				{
					OnPreloadFailed();
					return false;
				}

				pTarget->setFilename(_strFileName.c_str());
				pTarget->setBandwidth(_nMaxBandwidth);
				pTarget->enableProgressEvent(true);
				pTarget->enableMD5(_gCPHCfg.enableMD5);
				/*if (0 ==stricmp("aqua", proto))
				{
					pTarget->setIgnoreMainfile(true);
					pTarget->setIndexTmpfile(true);
				}*/
				_pMainTarget = pTarget;
			}

			SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

			BaseCPHSession::preLoad();

			_bCleaned = false;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaLibHelper, "[%s] preload successful"), _strLogHint.c_str());
			return true;
		}
}} // namespace ZQTianShan::ContentProvision
