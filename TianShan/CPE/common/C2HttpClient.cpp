#include "C2HttpClient.h"
#include "C2common.h"
#include "XMLPreferenceEx.h"
#include "ZQ_common_conf.h"
#include "SystemUtils.h"
#include "TimeUtil.h"
//#include "TianShanDefines.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
// #define C2HTTPClient "C2HTTPClient"
#include "C2HttpClientErr.h"

#define BUFFER_SIZE   (65536)  //64K

#define MLOG if(_log)(*_log)

#define C2FMT(_C, _X) CLOGFMT(_C, "C2Sess[Pid:%s Paid:%s subFile:%s] " _X), _providerId.c_str(), _providerAssetId.c_str(),_subFile.c_str() 

namespace ZQTianShan
{
	namespace ContentProvision
	{
		int C2HttpClient::_nCurrentPort;

		C2HttpClient::C2HttpClient(int iomode, ZQ::common::Log* pLog, int timeout, std::string locateRequestIP, std::string transferIP, int nport):
		_strErrMsg(""),_errorCode(0),_bErrorOccured(false), _timeout(timeout),
	    _exclustionlist(""),_IngressCapacity(0),_providerId(""),_providerAssetId(""),_subFile("")
		{
			//test for csico
			_maxPort = 65535;
			_nPortBegin = 49153;
			_nCurrentPort = _nPortBegin;

           //init http client
			if(pLog)
				_log = pLog;

			// init locate file request httpclient
			ZQ::common::HttpClient* phttpclient = new ZQ::common::HttpClient();
			if(!phttpclient)
				return; 
			_pHttpClient.reset(phttpclient); 
			_pHttpClient->init(iomode);
			_pHttpClient->setLog(_log);
			_pHttpClient->setLocalHost(locateRequestIP, nport);
			_locateRequestIP = locateRequestIP;
			_transferIP = transferIP;

			// init transfer file httpclient
			ZQ::common::HttpClient* pTranshttpclient = new ZQ::common::HttpClient();
			if(!pTranshttpclient)
				return; 
			_pTransferHttpClient.reset(pTranshttpclient); 
			_pTransferHttpClient->init(iomode);
			_pTransferHttpClient->setLog(_log);
			_pTransferHttpClient->setLocalHost(_transferIP, nport);
		}

		C2HttpClient::~C2HttpClient()
		{
		}
		void C2HttpClient::setTimeout(int timeout)
		{
			_timeout = timeout;
		}

		void C2HttpClient::reset()
		{
			_errorCode = 0;
			_strErrMsg = "";
			_bErrorOccured = false;

			try
			{
				_pHttpClient->setHeader(NULL, NULL);
				_pTransferHttpClient->setHeader(NULL, NULL);
			}
			catch (...){
			}
		}
		void C2HttpClient::setExclustionList(const std::string exclustionlist)
		{
			_exclustionlist = exclustionlist;
		}

		void C2HttpClient::setIngressCapacity(int64 IngressCapacity)
		{
			_IngressCapacity = IngressCapacity;
		}

		void C2HttpClient::setPIdAndPaid(std::string providerId, std::string providerAssetId)
		{
           _providerId = providerId;
		   _providerAssetId = providerAssetId;
		}

		void C2HttpClient::setSubfile(std::string subFile)
		{
          _subFile = subFile;
		}

		bool C2HttpClient::getFileNameFromResponse(std::string& strFileName)
		{
			int npos = strFileName.find("filename=");
			if(npos < 0)
			{
				return false;
			}
			strFileName = strFileName.substr(npos + 9);
			npos = strFileName.find("/");
			if(npos >= 0)
			{
				strFileName = strFileName.substr(npos+1);
			}
			return true;
		}
		int64  C2HttpClient::downloadFile(const std::string& transferURL, std::string& filePath, const int64 startOffset, int64 maxLen, bool bUsedLoaclFileName)
		{
			int64 totalFilesize = 0;
			MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "downloading [filepath: %s][URL: %s][startOffset: %lld][maxLen: %lld]"), filePath.c_str(),transferURL.c_str(), startOffset, maxLen); 
			int64 lStart = ZQ::common::now();
			try
			{
				_pTransferHttpClient->setHeader(NULL,NULL);
				_pTransferHttpClient->setHeader("User-Agent", "Example Comcast User-Agent");

				char strIngressCapacity[65] = "";
				if(_IngressCapacity >0 )
				{   
					sprintf(strIngressCapacity, "%lld", _IngressCapacity);
					_pTransferHttpClient->setHeader("Ingress-Capacity", strIngressCapacity);	
				}
                char strRang[128] = "";
				if(maxLen > 0)
				{
				  sprintf(strRang, "bytes=%lld-%lld", startOffset, startOffset + maxLen -1);
				}
				else
				{
                  sprintf(strRang, "bytes=%lld-", startOffset);
				}
				std::string tempRange = strRang;

				//	std::string tempRange = "bytes=0-";
				//	replace(tempRange.begin(), tempRange.end(), ',', '-');

				_pTransferHttpClient->setHeader("Range",(char*) tempRange.c_str());

				MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "downloading User-Agent(Example Comcast User-Agent Ingress-Capacity(%s) Range(%s)"), strIngressCapacity ,tempRange.c_str());

				//	_pTransferHttpClient->setLocalHost(_locateRequestIP, getPorts());

				if (_pTransferHttpClient->httpConnect(transferURL.c_str(), ZQ::common::HttpClient::HTTP_GET) || _pTransferHttpClient->httpEndSend())
				{
					_strErrMsg = "connect to url " + transferURL + " with error: " + _pTransferHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_ConnectionFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}
				_pTransferHttpClient->setRecvTimeout(_timeout);

				if (_pTransferHttpClient->httpBeginRecv())
				{
					_strErrMsg = std::string("begin receive file with error:") + _pTransferHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_BeginReceiveFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}
				int status = _pTransferHttpClient->getStatusCode();

				if(status != 206 && status != 200)
				{
					MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "status(%d) transferring terminited with error: %s"), status, _pTransferHttpClient->getMsg()); 
					_strErrMsg = std::string("transferring terminited with error:") + _pTransferHttpClient->getMsg();		
					_errorCode = status;
					_pTransferHttpClient->uninit();
					return -1;
				}
                
				if(!bUsedLoaclFileName)
				{
					///get filename from response
					std::string strFileName = "";
					std::map<std::string,std::string>  head = _pTransferHttpClient->getHeader(); 
					std::map<std::string,std::string>::iterator itor = head.find("Content-Disposition");
					if(itor!= head.end())
					{
						strFileName = itor->second;
						if(getFileNameFromResponse(strFileName))
						{
							int npos = -1;
#ifdef  ZQ_OS_MSWIN

							npos = filePath.rfind('\\');
#else
							npos = filePath.rfind('/');
#endif
							if(npos > 0)
							{
								filePath = filePath.substr(0, npos + 1) + strFileName;
							}
						}
					}		
				}

				std::ofstream out;
				out.open(filePath.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
				if (!out)
				{
					std::string errmsg = "(" + SYS::getErrorMessage() + ")";
					_strErrMsg = std::string("failed to open local file: ") + filePath + errmsg;
					_errorCode = ERRCODE_C2HTTPClient_CreateFileFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}
				int64 filesize = 0;
				std::string strRC = "", strBuf ="";
				while(!_pTransferHttpClient->isEOF())
				{
					if(_bErrorOccured)
					{
						_errorCode = ERRCODE_C2HTTPClient_UnkownError;
						_strErrMsg = "User stopped or unkonwn error";
						_pTransferHttpClient->uninit();
						return -1;
					}

					strRC.clear();
					if(_pTransferHttpClient->httpContinueRecv())
					{
						out.close();
						_strErrMsg = std::string("continue receiving file with error:") + _pTransferHttpClient->getErrorstr();
						_errorCode = ERRCODE_C2HTTPClient_ContinueReceiveFailed;
						_pTransferHttpClient->uninit();
						return -1;
					}
					_pTransferHttpClient->getContent(strRC);

					strBuf+=strRC;
					filesize += strRC.size();
					if(strBuf.size() >= BUFFER_SIZE)
					{
						out.write(strBuf.c_str(), strBuf.size());
						totalFilesize += strBuf.size();
						strBuf.clear();
					}
					if(maxLen > 0 && filesize >= (maxLen + startOffset))
						break;
				}

				if (_pTransferHttpClient->httpEndRecv())
				{
					out.close();
					_strErrMsg = std::string("finished receiving file subFile[") + filePath + std::string("] with error:") + _pTransferHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_EndReceiveFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}
				_pTransferHttpClient->getContent(strRC);
				strBuf+=strRC;
				out.write(strBuf.c_str(), strBuf.size());
				totalFilesize += strBuf.size();
				out.close();
				_pTransferHttpClient->uninit();
				MLOG(ZQ::common::Log::L_INFO, C2FMT(C2HTTPClient, "downloaded file took %dms"),(int)(ZQ::common::now() - lStart)); 
			}
			catch(...)
			{
				_strErrMsg = std::string("downloading caught unknown exception");

				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "%s (%d)"), _strErrMsg.c_str(), SYS::getLastErr());
				_errorCode = ERRCODE_C2HTTPClient_UnkownError;
				_pTransferHttpClient->uninit();
				return -1;
			}
            
			return totalFilesize;
		}

		int64 C2HttpClient::downloadPartialFile(ReadBuf transferbuf,  void *pCtx, const std::string& transferURL, const int64 startOffset, int64 maxLen)
		{
			int64 totalFilesize = 0;
			MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "downloading chunk file[URL: %s][startOffset: %lld][maxLen: %lld]"), transferURL.c_str(), startOffset, maxLen); 
			try
			{
				_pTransferHttpClient->setHeader(NULL,NULL);
				_pTransferHttpClient->setHeader("User-Agent", "Example Comcast User-Agent");
				char strIngressCapacity[65] = "";
				if(_IngressCapacity > 0)
				{
					sprintf(strIngressCapacity, "%lld", _IngressCapacity);
					_pTransferHttpClient->setHeader("Ingress-Capacity", strIngressCapacity);
				}

				std::string tempRange = "";

				char strRang[128] = "";
				if(maxLen > 0)
				{
					sprintf(strRang, "bytes=%lld-%lld", startOffset, startOffset + maxLen -1);
				}
				else
				{
					sprintf(strRang, "bytes=%lld-", startOffset);
				}

				tempRange  = strRang;

				//replace(tempRange.begin(), tempRange.end(), ',', '-');
				_pTransferHttpClient->setHeader("Range",(char*) tempRange.c_str());

				MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "downloading User-Agent(Example Comcast User-Agent Ingress-Capacity(%s) Range(%s)"), strIngressCapacity ,tempRange.c_str());

				//_pTransferHttpClient->setLocalHost(_locateRequestIP, getPorts());
				if (_pTransferHttpClient->httpConnect(transferURL.c_str(), ZQ::common::HttpClient::HTTP_GET) || _pTransferHttpClient->httpEndSend())
				{
					_strErrMsg = "connect to url " + transferURL + " with error: " + _pTransferHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_ConnectionFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}

				_pTransferHttpClient->setRecvTimeout(_timeout);
				if (_pTransferHttpClient->httpBeginRecv())
				{
					_strErrMsg = std::string("receive file subFile[") + _subFile + std::string("] with error:") + _pTransferHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_BeginReceiveFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}
				int status = _pTransferHttpClient->getStatusCode();

				if(status != 206 && status != 200)
				{
					MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "status(%d) transferring terminited with error: %s"), status, _pTransferHttpClient->getMsg()); 
					_strErrMsg = std::string("transferring terminated with error:") + _pTransferHttpClient->getMsg();		
					_errorCode = status;
					_pTransferHttpClient->uninit();
					return -1;
				}
				std::string strRC, strBuf="";
				int64 ltotalbyte = 0;
				int64 lStart = ZQ::common::now();
				while(!_pTransferHttpClient->isEOF())
				{	
					if(_bErrorOccured)
					{
						_errorCode = ERRCODE_C2HTTPClient_CallBackError;
						_pTransferHttpClient->uninit();
						return -1;
					}
					if(_pTransferHttpClient->httpContinueRecv())
					{
						_strErrMsg = std::string("continue receiving with error:") + _pTransferHttpClient->getErrorstr();
						_errorCode = ERRCODE_C2HTTPClient_ContinueReceiveFailed;
						_pTransferHttpClient->uninit();
						return -1;
					}
					_pTransferHttpClient->getContent(strRC);
					strBuf+=strRC;

					//				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2HTTPClient, "recevie %d byte took %dms"), strRC.size(), (int)(ZQ::common::now() - lStart));
					if(!_bErrorOccured && strBuf.size() >= BUFFER_SIZE * 32)
					{
						ltotalbyte += strBuf.size();
						int64 ltemp = ZQ::common::now() - lStart;
						if(ltemp != 0  && ltotalbyte * 8 / ltemp < 500)
							MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "received %lld byte took %lldms, %d Kbps"), ltotalbyte, ltemp,(int)(ltotalbyte * 8 / ltemp));
						if(!transferbuf(pCtx, (char*)strBuf.c_str(), strBuf.size()))
						{
							_pTransferHttpClient->uninit();
							return -1;
						}
						totalFilesize += strBuf.size();
						strBuf.clear();
					}
				}

				if(_bErrorOccured)
				{
					_errorCode = ERRCODE_C2HTTPClient_CallBackError;
					_pTransferHttpClient->uninit();
					return -1;
				}
				if (_pTransferHttpClient->httpEndRecv())
				{
					_strErrMsg = std::string("close receiving with error:") + _pTransferHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_EndReceiveFailed;
					_pTransferHttpClient->uninit();
					return -1;
				}
				_pTransferHttpClient->getContent(strRC);
				strBuf+=strRC;
				transferbuf(pCtx, (char*)strBuf.c_str(), strBuf.size());
				totalFilesize += strBuf.size();

				ltotalbyte += strBuf.size();
				int64 ltemp = ZQ::common::now() - lStart;
				int64 lTotalKbps = 0;
				if(ltemp != 0)
				{
					lTotalKbps = ltotalbyte * 8 / ltemp;
				}		
				_pTransferHttpClient->uninit();

				MLOG(ZQ::common::Log::L_INFO, C2FMT(C2HTTPClient, "download chunk file successully took %dms, totalbyte %lld  took %lldms, bitrate[%lld]Kbps"),(int)(ZQ::common::now() - lStart), ltotalbyte, ltemp, lTotalKbps); 
			}
			catch(...)
			{
				_strErrMsg = std::string("receiving chunk file caught unknown exception");

				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "%s (%d)"), _strErrMsg.c_str(), SYS::getLastErr());
				_errorCode = ERRCODE_C2HTTPClient_UnkownError;
				_pTransferHttpClient->uninit();
				return -1;
			}
			return totalFilesize;
		}

		bool C2HttpClient::parserLocateReponse(const char* buffer, size_t bufLen, LocateResponse& locateResponse)
		{
			int64 lStart = ZQ::common::now();
			char temp[513] = "";
			if (!buffer || 0 == bufLen)
			{
				return false;
			}
			char buf[256]= "";
			sprintf(buf, C2FMT(C2HttpClient, "parser LocateRequest"));
//			if(_log)
//				_log->hexDump(ZQ::common::Log::L_INFO, buffer, bufLen, buf, true);
			ZQ::common::XMLPreferenceDocumentEx xmlDoc;
			if(!xmlDoc.read((void*)buffer, (int)bufLen, 1))//successful
			{
				return false;
			}
			ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
			if(NULL == pXMLRoot)
			{
				xmlDoc.clear();
				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "parserLocateReponse() getRootPreference error")); 
				return false;
			}
			ZQ::common::XMLPreferenceEx* transferPortpre = pXMLRoot->findChild("TransferPort");
			if(NULL == transferPortpre)
			{
				pXMLRoot->free();
				xmlDoc.clear();
				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "parserLocateReponse() get TransferPort Preference error")); 
				return false;
			}
			transferPortpre->getPreferenceText(temp, 512);
			locateResponse.transferHost = temp;

			memset(temp, 0 , 513);
			ZQ::common::XMLPreferenceEx* transferIdpre = pXMLRoot->findChild("TransferID");
			if(NULL == transferIdpre)
			{
				pXMLRoot->free();
				xmlDoc.clear();
				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "parserLocateReponse() get TransferID Preference error")); 
				return false;
			}
			transferIdpre->getPreferenceText(temp, 512);
			locateResponse.transferId = temp;

			memset(temp, 0 , 513);
			ZQ::common::XMLPreferenceEx* transferTOpre = pXMLRoot->findChild("TransferTimeout");
			if(NULL == transferTOpre)
			{
				/*pXMLRoot->free();
				xmlDoc.clear();
				glog(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "parserLocateReponse() get TransferTimeout Preference error")); 
				return false;*/
				locateResponse.transferTimeout  = 500;
			}
			else
			{
				transferTOpre->getPreferenceText(temp, 512);
				locateResponse.transferTimeout = atoi(temp);
			}


			memset(temp, 0 , 513);
			ZQ::common::XMLPreferenceEx* AvailableRangepre = pXMLRoot->findChild("AvailableRange");
			if(NULL == AvailableRangepre)
			{
				pXMLRoot->free();
				xmlDoc.clear();
				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "parserLocateReponse() get AvailableRange Preference error")); 
				return false;
			}
			AvailableRangepre->getPreferenceText(temp, 512);
			locateResponse.availableRange = temp;

			memset(temp, 0 , 513);
			ZQ::common::XMLPreferenceEx* OpenForWritepre = pXMLRoot->findChild("OpenForWrite");
			if(NULL == OpenForWritepre)
			{
				pXMLRoot->free();
				xmlDoc.clear();
				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "parserLocateReponse() get OpenForWrite Preference error")); 
				return false;
			}
			OpenForWritepre->getPreferenceText(temp, 512);
			locateResponse.OpenForWrite = stricmp(temp,"no")== 0 ? false: true;


			memset(temp, 0 , 513);
			ZQ::common::XMLPreferenceEx* TransferPortNumpre = pXMLRoot->findChild("PortNum");
			if(NULL == TransferPortNumpre)
			{
				locateResponse.transferportnum = "";
			}
			else
			{
				TransferPortNumpre->getPreferenceText(temp, 512);
				locateResponse.transferportnum = temp;	
			}

			pXMLRoot->free();
			xmlDoc.clear();
			MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "parsed LocateResponse took %dms"),(int)(ZQ::common::now() - lStart)); 

			return true;
		}
		bool C2HttpClient::locateRequest(const std::string& url, const LocateRequest& locateReqest, std::string& reponseConent)
		{
			int64 lStart = ZQ::common::now();
			_pHttpClient->setHeader(NULL,NULL);
			//		_pHttpClient->setLocalHost(_locateRequestIP, getPorts());
			if (_pHttpClient->httpConnect(url.c_str(), ZQ::common::HttpClient::HTTP_POST))
			{
				_strErrMsg = "connect to url " + url + " with error: " + _pHttpClient->getErrorstr();
				_errorCode = ERRCODE_C2HTTPClient_ConnectionFailed;
				return false;
			}

			std::string strRequestContent;
			try
			{
				std::ostringstream buf;
//				buf << XML_HEADER ;
				buf << "<LocateRequest>\n";
				buf << "  <Object>\n" ;
				buf << "    <Name>\n";
				buf << "     <AssetID>" << locateReqest.paid << "</AssetID>\n";
				buf << "     <ProviderID>" << locateReqest.pid << "</ProviderID>\n";
				buf << "    </Name>\n";
				buf << "    <SubType>"<< locateReqest.subFile << "</SubType>\n";
				buf << "  </Object>\n";
				buf << "  <TransferRate>"<< locateReqest.bitRate<< "</TransferRate>\n";
				buf << "  <IngressCapacity>" << _IngressCapacity << "</IngressCapacity>\n";
				if(!_exclustionlist.empty())
				{
					buf << "  <ExclusionList>";
					buf << _exclustionlist;
					buf << "  </ExclusionList>\n";
				}
				else
				{
					buf << "<ExclusionList/>\n";
				}
	
				buf << "  <TransferDelay>";
				buf << locateReqest.transferDelay;
				buf << "  </TransferDelay>\n";
				if(locateReqest.beginPos >= 0)
				{
					buf << "  <Range> ";
					buf << locateReqest.beginPos << " - ";
					if(locateReqest.endPos > 0)
						buf<< locateReqest.endPos ;
					buf << "\n  </Range>\n";
				}
				else
				{
					buf << "  <Range>0-</Range>\n";
				}
				buf << "</LocateRequest>\n";

				strRequestContent = buf.str();

				_pHttpClient->setHeader("User-Agent", "CPE User-Agent");
				_pHttpClient->setHeader("Content-Type", "text/xml");
			}
			catch(...)
			{
				_strErrMsg = std::string("composing LocateRequest subfile[") + locateReqest.subFile + std::string("] caught exception:") + _pHttpClient->getErrorstr();
				_errorCode = ERRCODE_C2HTTPClient_UnkownError;
				_pHttpClient->uninit();
				return false;
			}
			char buf[256]= "";
			sprintf(buf, C2FMT(C2HttpClient, "sending LocateRequest for subFile[%s] "), locateReqest.subFile .c_str());
			if(_log)
				_log->hexDump(ZQ::common::Log::L_INFO, strRequestContent.c_str(), strRequestContent.size(), buf, true);

			if(_pHttpClient->httpSendContent(strRequestContent.c_str(), strRequestContent.size()))
			{
				_strErrMsg = std::string("send locate subFile[") + _subFile + std::string("] file request with error:") + _pHttpClient->getErrorstr();
				_errorCode = ERRCODE_C2HTTPClient_ConnectionFailed;
				_pHttpClient->uninit();
				return false;
			}
			if (_pHttpClient->httpEndSend() )
			{
				_strErrMsg = std::string("send locate subFile[") + _subFile + std::string("] file request: httpEndSend error");
				_errorCode = ERRCODE_C2HTTPClient_EndReceiveFailed;
				_pHttpClient->uninit();
				return false;
			}

			if (_pHttpClient->httpBeginRecv())
			{
				_strErrMsg = std::string("begin receive locate subFile[") + _subFile + std::string("] file reponse with error:") + _pHttpClient->getErrorstr();
				_errorCode = ERRCODE_C2HTTPClient_BeginReceiveFailed;
				_pHttpClient->uninit();
				return false;
			}

			int status = _pHttpClient->getStatusCode();

			if(status != 201)
			{
				MLOG(ZQ::common::Log::L_ERROR, C2FMT(C2HTTPClient, "status(%d) locate subFile[%s]with error: %s"), status, _subFile.c_str(), _pHttpClient->getMsg()); 
				_strErrMsg = std::string("locate subFile[") + _subFile + std::string("]with error:") + _pHttpClient->getMsg();
				_errorCode = status;
				_pHttpClient->uninit();
				return false;
			}

			std::string strRC = "";
			while(!_pHttpClient->isEOF())
			{
				strRC.clear();
				if(_pHttpClient->httpContinueRecv())
				{
					_strErrMsg = std::string("continue receiver locate subFile[") + _subFile + std::string("] reponse with error:") + _pHttpClient->getErrorstr();
					_errorCode = ERRCODE_C2HTTPClient_ContinueReceiveFailed;
					_pHttpClient->uninit();
					return false;
				}
				_pHttpClient->getContent(strRC);
				reponseConent += strRC;
			}

			if ( _pHttpClient->httpEndRecv() )
			{
				_strErrMsg = std::string("finished receiver locate subFile[") + _subFile + std::string("] file reponse: httpEndRecv error");
				_errorCode = ERRCODE_C2HTTPClient_EndReceiveFailed;
				_pHttpClient->uninit();
				return false;
			}
			_pHttpClient->getContent(strRC);
			reponseConent += strRC;

			_pHttpClient->uninit();

//			MLOG(ZQ::common::Log::L_INFO, C2FMT(C2HTTPClient, "step 1. LocateRequest subFile[%s] sent, took %dms"),  locateReqest.subFile.c_str(), (int)(ZQ::common::now() - lStart)); 
			return true;
		}

		bool C2HttpClient::prepareLocateFile(const std::string& url, const LocateRequest& locateReqest, LocateResponse& locateResponse)
		{
			int64 lStart = ZQ::common::now();
			reset();

			if(url.empty() || locateReqest.pid.empty() || locateReqest.paid.empty())
			{
				char temp[512] = "";
				snprintf(temp, 512, "invalid locate request paramter URL(%s) PID(%s) PAID(%s)", url.c_str(), locateReqest.pid.c_str(), locateReqest.paid.c_str());
				_strErrMsg = temp;
				_errorCode =  ERRCODE_C2HTTPClient_InvalidParameter;
				return false;
			}

			std::string reponseConent;
			if(!locateRequest(url, locateReqest, reponseConent))
			{
				return false;
			}
			if(!parserLocateReponse(reponseConent.c_str(), reponseConent.size(), locateResponse))
			{
				char temp[512] = "";
				snprintf(temp, 512, "parser locate request URL(%s) PID(%s) PAID(%s) with unknown reponseConent(%s) format",
					url.c_str(), locateReqest.pid.c_str(), locateReqest.paid.c_str(), reponseConent.c_str());
				_strErrMsg = temp;
				_errorCode =  ERRCODE_C2HTTPClient_InvalidResponse;
				return false;
			}

			MLOG(ZQ::common::Log::L_INFO, C2FMT(C2HTTPClient, "TransferSession created, took %dms: TransferId[%s] TransferPort[%s] TransferTimeout[%d] AvailableRange[%s] OpenForWrite[%s]"),
				(int)(ZQ::common::now() - lStart), locateResponse.transferId.c_str(), locateResponse.transferHost.c_str(), locateResponse.transferTimeout, locateResponse.availableRange.c_str(), locateResponse.OpenForWrite == false ? "no": "true"); 
			return true;
		}

		bool  C2HttpClient::getLastErrorMsg(int& nRetCode, std::string& errMsg)
		{
			nRetCode = 1;
			errMsg  = _strErrMsg;
			return true;
		}

		bool C2HttpClient::parserTotalFilesize(const std::string& contentRange, int64& nBeginPos, int64& nEndPos, int64& totalSize)
		{
			int nPos = contentRange.find("/");
			if(nPos < 0 )
			{
				char temp[512] = "";
				snprintf(temp, 512, "parser total file size (%s)with unknown contentRange format", contentRange.c_str());
				_strErrMsg = temp;
				_errorCode =  ERRCODE_C2HTTPClient_InvalidAviRange;
				return false;
			}
			//		totalSize = _atoi64((contentRange.substr(nPos +1)).c_str());
			sscanf((contentRange.substr(nPos +1)).c_str(), FMT64, &totalSize);
			std::string strSub = contentRange.substr(0, nPos);
			if(!parserAavailableRange(strSub, nBeginPos, nEndPos))
				return false;
			return true;	
		}

		bool C2HttpClient::parserAavailableRange(std::string& availableRang, int64& nBeginPos, int64& nEndPos)
		{
			nBeginPos = -1;
			nEndPos = -1;
			//		int nPos = availableRang.find(",");
			int nPos = availableRang.find("-");
			if(nPos < 0)
			{
				char temp[512] = "";
				snprintf(temp, 512, "parser availableRange(%s)with unknown availabledRang format", availableRang.c_str());
				_strErrMsg = temp;
				_errorCode =  ERRCODE_C2HTTPClient_InvalidAviRange;
				return false;
			}
			//		nBeginPos = _atoi64((availableRang.substr(0, nPos)).c_str());
			//		nEndPos = _atoi64((availableRang.substr(nPos + 1)).c_str());
			sscanf((availableRang.substr(0, nPos)).c_str(), FMT64, &nBeginPos);
			sscanf((availableRang.substr(nPos + 1)).c_str(), FMT64, &nEndPos);

			return true;
		}

		int64  C2HttpClient::getFileSize(const std::string& url, const LocateRequest& locateReqest, LocateResponse& locateResponse)
		{
			int64 lStart = ZQ::common::now();
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2HTTPClient, "C2Sess[Pid:%s Paid:%s subFile:%s] get file size"),locateReqest.pid.c_str(), locateReqest.paid.c_str(), locateReqest.subFile.c_str()); 

			int64 filesize = -1;

			if(!prepareLocateFile(url, locateReqest, locateResponse ))
			{
				return filesize;
			}

			deleteTransferId(url, locateResponse.transferId, locateResponse.transferHost);
			reset();

			int64 nBeginPos, nEndPos;
			if(!parserAavailableRange(locateResponse.availableRange, nBeginPos, nEndPos))
			{
				return filesize;
			}

			filesize = nEndPos + 1;

			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2HTTPClient, "C2Sess[Pid:%s Paid:%s subFile:%s] get filesize[%lld] took %dms"),locateReqest.pid.c_str(), locateReqest.paid.c_str(), locateReqest.subFile.c_str(), filesize, (int)(ZQ::common::now() - lStart)); 
			return filesize;
		}

		bool C2HttpClient::deleteTransferId(const std::string& url, const std::string& transferId, const std::string& transferIpPort)
		{
			if(transferIpPort.empty() || transferId.empty())
				return true;

			MLOG(ZQ::common::Log::L_DEBUG, C2FMT(C2HTTPClient, "deleteTransferId() TransferId(%s)TransferHost(%s)"),transferId.c_str(), transferIpPort.c_str()); 

			_pHttpClient->setHeader(NULL,NULL);

			//		_pHttpClient->setLocalHost(_locateRequestIP, getPorts());

			ZQ::common::URLStr urlstr(url.c_str());
			urlstr.setPath("*");
			std::string strUrl = urlstr.generate();
			if (_pHttpClient->httpConnect(strUrl.c_str(), ZQ::common::HttpClient::HTTP_POST))
			{
				_pHttpClient->uninit();
//				_errorCode = ERRCODE_C2HTTPClient_ConnectionFailed;
				return false;
			}

			std::string strRequestContent;
			try
			{
				std::ostringstream buf;
//				buf << XML_HEADER ;
				buf << "<LocateRequest>\n";
				buf << " <ClientTransfer>" << transferIpPort << "</ClientTransfer>\n";
				buf << " <TransferIDDelete>\n ";
				buf << transferId <<"\n";
				buf << " </TransferIDDelete>\n";
				buf << "</LocateRequest>\n";
				strRequestContent = buf.str();
				_pHttpClient->setHeader("User-Agent", "Example Comcast User-Agent");
				_pHttpClient->setHeader("Content-Type", "text/xml");
			}
			catch(...)
			{
//				_errorCode = ERRCODE_C2HTTPClient_UnkownError;
				_pHttpClient->uninit();
				return false;
			}

			char buf[256] = "";
			sprintf(buf, C2FMT(C2HTTPClient, "deleteTransferId()"));
			if(_log)
				_log->hexDump(ZQ::common::Log::L_INFO, strRequestContent.c_str(), strRequestContent.size(), buf, true);

			if(_pHttpClient->httpSendContent(strRequestContent.c_str(), strRequestContent.size()))
			{
				_pHttpClient->uninit();
				return false;
			}
			if (_pHttpClient->httpEndSend())
			{
				_pHttpClient->uninit();
				return false;
			}

			if (_pHttpClient->httpBeginRecv())
			{
				_errorCode = ERRCODE_C2HTTPClient_BeginReceiveFailed;
				_pHttpClient->uninit();
				return false;
			}
			int status = _pHttpClient->getStatusCode();

			MLOG(ZQ::common::Log::L_INFO, C2FMT(C2HTTPClient, "deleteTransferId()TransferId(%s)TransferHost(%s)ReponseMessage(status %d, %s)"), transferId.c_str(), transferIpPort.c_str(), status, _pHttpClient->getMsg()); 

			std::string strRC = "";
			while(!_pHttpClient->isEOF())
			{
				strRC.clear();
				if(_pHttpClient->httpContinueRecv())
				{
//					_errorCode = ERRCODE_C2HTTPClient_ContinueReceiveFailed;
					_pHttpClient->uninit();
					return false;
				}
				_pHttpClient->getContent(strRC);
			}

			if ( _pHttpClient->httpEndRecv() )
			{
//				_errorCode = ERRCODE_C2HTTPClient_EndReceiveFailed;
				_pHttpClient->uninit();
				return false;
			}
			_pHttpClient->getContent(strRC);
			_pHttpClient->uninit();
			return true;
		}
		int C2HttpClient::getPorts()
		{
			if(_nCurrentPort >= _maxPort )
				_nCurrentPort = _nPortBegin;
			else 
				++_nCurrentPort;
			return _nCurrentPort;
		}
	}

}
