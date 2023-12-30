#include "StdAfx.h"
#include "A3MessageReq.h"
#include "Guid.h"
#include <sstream>
#include "TimeUtil.h"
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
namespace CRM
{
	namespace A3MessageClient
	{

		std::string getReqID()
		{
			ZQ::common::Guid id;
			id.create();
			char bufUUID[65] = "";
			int nUUID = id.toString(bufUUID, 65);
			return std::string(bufUUID);
		}
		A3MessageReq::A3MessageReq(ZQ::common::Log& log, std::string reqURL)
			:_log(log), _reqURL(reqURL)
		{
		}

		A3MessageReq::~A3MessageReq(void)
		{
		}
		bool A3MessageReq::init(int iomode, std::string bindIp, int nport)
		{
			// init request httpclient
			ZQ::common::HttpClient* phttpclient = new ZQ::common::HttpClient();
			if(!phttpclient)
				return false; 
			_pHttpClient.reset(phttpclient); 
			_pHttpClient->init(iomode);
			_pHttpClient->setLog(&_log);
			_pHttpClient->setLocalHost(bindIp, nport);
			return true;
		}
		
		/*Request Example:
		<?xml version="1.0" encoding="utf-8"?>
		<GetVolumeInfo
		volumeName=¡±Philly.Warminster.volume4¡±
		/>*/
	
		bool A3MessageReq::GetVolumeInfoReq(std::string volumeName)
		{
			std::string reqID = getReqID();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]get volume info [volumeName=%s]"),reqID.c_str(), volumeName.c_str());
			std::string strReq;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<GetVolumeInfo\n";
			buf << "volumeName=\""<< volumeName<<"\"\n/>";

			strReq = buf.str();

			std::string reqURL = _reqURL +"/GetVolumeInfo";
		    return sendRequest(reqID, reqURL, strReq);
		}

		/*	Request Example (All Content on Volume):
				<?xml version="1.0" encoding="utf-8"?>
				<GetContentInfo
				volumeName=¡±Philly.Warminster.volume1A¡±
				/>
				Request Example (Single Asset on Volume):
				<?xml version="1.0" encoding="utf-8"?>
				<GetContentInfo
				providerID=¡±comcast.com¡±
				assetID=¡±BAAA0000000000018377¡±
				volumeName=¡±Philly.Warminster.volume1A¡±
				/>*/

		bool A3MessageReq::GetContentInfoReq(std::string volumeName, std::string pid, std::string paid)
		{
			std::string reqID = getReqID();
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]get content info [volumeName=%s][pid=%s][paid=%s]"), reqID.c_str(), volumeName.c_str(), pid.c_str(), paid.c_str());
			std::string strReq;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<GetContentInfo\n";
			if(!pid.empty() && !paid.empty())
			{
				buf << "providerID=\""<< pid<<"\"\n";
				buf << "assetID=\""<< paid <<"\"\n";
			}
			buf << "volumeName=\""<< volumeName<<"\"\n/>";

			strReq = buf.str();
			std::string reqURL = _reqURL +"/GetContentInfo";
			return sendRequest(reqID, reqURL, strReq);
		}
		/////implement here
	/*	Request Example (IP Multicast):
		<?xml version="1.0" encoding="utf-8"?>
			<TransferContent
			providerID="comcast.com"
			assetID="BAAA0000000000018377"
			captureStart="2004-10-03T12:59:00Z"
			captureEnd="2004-10-03T4:16:00Z"
			transferBitRate="3750000"
			sourceURL="UDP://228.1.1.1:6000/"
			sourceIP="10.1.2.3:5000"
			sourceURL1="UDP://228.1.1.2:7000/"
			sourceIP1="10.3.4.5:8000"
			volumeName="Philly.Warminster.volume1A"
			responseURL="http://192.168.1.1:8001/">
			<ContentAsset>
			<!-- metadata -->
			</ContentAsset>
			</TransferContent>*/

	/*	Request Example (HTTP):
		<?xml version="1.0" encoding="utf-8"?>
			<TransferContent
			providerID=¡±comcast.com¡±
			assetID=¡±BAAA0000000000018377¡±
			captureStart=¡±2004-10-03T12:59:00Z¡±
			captureEnd=¡±2004-10-03T4:16:00Z¡±
			transferBitRate=¡±3750000¡±
			sourceURL=¡±http://192.168.1.12/Content/Content.mpg¡±
		volumeName=¡±Philly.Warminster.volume1A¡±
			responseURL=¡±http://192.168.1.1:8001/¡±>
		<ContentAsset>
			<<!¡ªmetadata -->
			</ContentAsset>
			</TransferContent>*/


		bool A3MessageReq::TransferContentReq(ContentInfo& transInfo)
		{
			std::string reqID = getReqID();
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]transfer content info [volumeName=%s][pid=%s][paid=%s]"), reqID.c_str(), transInfo.volumeName.c_str(), transInfo.pid.c_str(), transInfo.paid.c_str());
			std::string strReq;

			std::string strProtocal;
			int npos = transInfo.sourceURL.find(':');
			if(npos < 0)
				return false;
			strProtocal =  transInfo.sourceURL.substr(0, npos);

			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<TransferContent\n";
			buf << "	providerID=\""<< transInfo.pid<<"\"\n";
			buf << "	assetID=\""<< transInfo.paid <<"\"\n";
			buf << "	volumeName=\""<<transInfo.volumeName<<"\"\n";
			buf << "	sourceURL=\""<< transInfo.sourceURL <<"\"\n";
			buf << "	responseURL=\""<< transInfo.responseURL <<"\"\n";	
			buf << "	transferBitRate=\""<< transInfo.transferBitRate <<"\"\n";
			if(!transInfo.captureStart.empty())
				buf << "	captureStart=\""<< transInfo.captureStart <<"\"\n";
			if(!transInfo.captureEnd.empty())
				buf << "	captureEnd=\""<< transInfo.captureEnd <<"\"\n";

			if(!transInfo.sourceURL1.empty())
				buf << "	sourceURL1=\""<< transInfo.sourceURL1 <<"\"\n";

            if(!stricmp(strProtocal.c_str(),"pgm") || !stricmp(strProtocal.c_str(),"udp"))
			{
				if(!transInfo.sourceIP.empty())
					buf << "	sourceIP=\""<< transInfo.sourceIP <<"\"\n";
				if(!transInfo.sourceIP1.empty())
					buf << "	sourceIP1=\""<< transInfo.sourceIP1 <<"\"\n";
			}
			else
			{
				buf << "	userName=\""<< transInfo.userName <<"\"\n";
				buf << "	password=\""<< transInfo.password <<"\"\n";
			}
			buf << "	>\n";
			buf << "  <ContentAsset>\n";

			if(!transInfo.props.empty())
			{
				
				std::map<std::string, std::string>::iterator itorprops;
				for(itorprops = transInfo.props.begin(); itorprops != transInfo.props.end(); itorprops++)
				{
					buf << "  <metadata ";
					buf << "name=\"" <<  itorprops->first << "\" value=\"" << itorprops->second << "\"";
					buf << "  />\n";
				}
				
			}

			buf << "  </ContentAsset>\n";	
			buf << "</TransferContent>";		

			strReq = buf.str();
			std::string reqURL = _reqURL +"/TransferContent";
			return sendRequest(reqID, reqURL, strReq);
		}
		
		/*Request Example:
		<?xml version="1.0" encoding="utf-8"?>
			<CancelTransfer
			providerID=¡±comcast.com¡±
			assetID=¡±BAAA0000000000018377¡±
			volumeName=¡±Philly.Warminster.volume1A¡±
			reasonCode=¡±404¡±
			/>*/
		

		bool A3MessageReq::CancelTransferReq(std::string volumeName, std::string pid, std::string paid, int reasonCode)
		{
			std::string reqID = getReqID();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]cancel transfer content[volumeName=%s][pid=%s][paid=%s][reasonCode=%d]"), reqID.c_str(), volumeName.c_str(), pid.c_str(),paid.c_str(), reasonCode);
			std::string strReq;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<CancelTransfer\n";
			buf << "providerID=\""<< pid<<"\"\n";
			buf << "assetID=\""<< paid <<"\"\n";
			buf << "volumeName=\""<< volumeName<<"\"\n";
			buf << "reasonCode=\""<< reasonCode <<"\"\n/>";

			strReq = buf.str();

			std::string reqURL = _reqURL +"/CancelTransfer";
			return sendRequest(reqID, reqURL, strReq);
		}
		/*Request Example:
		<?xml version="1.0" encoding="utf-8"?>
			<DeleteContent
			providerID=¡±comcast.com¡±
			assetID=¡±BAAA0000000000018377¡±
			volumeName=¡±Philly.Warminster.volume1A¡±
			reasonCode=¡±201¡±
			/>*/

		bool A3MessageReq::DeleteContentReq(std::string volumeName, std::string pid, std::string paid, int reasonCode)
		{
			std::string reqID = getReqID();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]delete content[volumeName=%s][pid=%s][paid=%s][reasonCode=%d]"), reqID.c_str(), volumeName.c_str(), pid.c_str(),paid.c_str(), reasonCode);
			std::string strReq;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<DeleteContent\n";
			buf << "providerID=\""<< pid<<"\"\n";
			buf << "assetID=\""<< paid <<"\"\n";
			buf << "volumeName=\""<< volumeName<<"\"\n";
			buf << "reasonCode=\""<< reasonCode <<"\"\n/>";

			strReq = buf.str();

			std::string reqURL = _reqURL +"/DeleteContent";
			return sendRequest(reqID, reqURL, strReq);
		}


	/*	Request Example:
		<?xml version="1.0" encoding="utf-8"?>
			<GetContentChecksum
			providerID=¡±comcast.com¡±
			assetID=¡±BAAA0000000000018377¡±
			volumeName=¡±Philly.Warminster.volume1A¡±
			responseURL=¡±http://192.168.1.1:8001/¡±
		/>*/
  
		bool A3MessageReq::GetContentChecksumReq(std::string volumeName, std::string pid, std::string paid, std::string responseURL)
		{
			std::string reqID = getReqID();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]get content checksum[volumeName=%s][pid=%s][paid=%s][responseURL=%s]"), reqID.c_str(), volumeName.c_str(), pid.c_str(),paid.c_str(), responseURL.c_str());
			std::string strReq;
			std::ostringstream buf;
			buf << "<GetContentChecksum\n";
			buf << "providerID=\""<< pid<<"\"\n";
			buf << "assetID=\""<< paid <<"\"\n";
			buf << "volumeName=\""<< volumeName<<"\"\n";
			buf << "responseURL=\""<< responseURL <<"\"\n/>";

			strReq = buf.str();
			std::string reqURL = _reqURL +"/GetContentChecksum";
			return sendRequest(reqID, reqURL, strReq);
		}
		/*Request Example:
		  <?xml version="1.0" encoding="utf-8"?>
			<ExposeContent
			providerID=¡±comcast.com¡±
			assetID=¡±BAAA0000000000018377¡±
			volumeName=¡±Philly.Warminster.volume1A¡±
			protocol=¡±ftp¡±
			transferBitRate=¡±5000000¡±
			/>*/

		bool A3MessageReq::ExposeContentReq(std::string volumeName, std::string pid, std::string paid, std::string protocol, int transferBitRate)
		{
			std::string reqID = getReqID();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]expose content[volumeName=%s][pid=%s][paid=%s][protocol=%s][transferBitRate=%d]"),reqID.c_str(), volumeName.c_str(), pid.c_str(),paid.c_str(),protocol.c_str(), transferBitRate);
			std::string strReq;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<ExposeContent\n";
			buf << "providerID=\""<< pid<<"\"\n";
			buf << "assetID=\""<< paid <<"\"\n";
			buf << "volumeName=\""<< volumeName<<"\"\n";
			buf << "protocol=\""<< protocol<<"\"\n";
			buf << "transferBitRate=\""<< transferBitRate<<"\"\n/>";

			strReq = buf.str();

			std::string reqURL = _reqURL +"/ExposeContent";
			return sendRequest(reqID, reqURL, strReq);
		}

		/*			Request Example (All Content on Volume):
		<?xml version="1.0" encoding="utf-8"?>
			<GetTransferStatus
			providerID=¡±comcast.com¡±
			assetID=¡±BAAA0000000000018377¡±
			volumeName=¡±Philly.Warminster.volume1A¡±
			/>*/
			

		bool A3MessageReq::GetTransferStatusReq(std::string volumeName, std::string pid, std::string paid)
		{
			std::string reqID = getReqID();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]get transfer status[volumeName=%s][pid=%s][paid=%s]"), reqID.c_str(), volumeName.c_str(), pid.c_str(),paid.c_str());

			std::string strReq;
			std::ostringstream buf;

			buf << XML_HEADER ;
			buf << "<GetTransferStatus\n";
			buf << "providerID=\""<< pid<<"\"\n";
			buf << "assetID=\""<< paid <<"\"\n";
			buf << "volumeName=\""<< volumeName<<"\"\n/>";

			strReq = buf.str();

			std::string reqURL = _reqURL +"/GetTransferStatus";
			return sendRequest(reqID, reqURL, strReq);
		}
	

		bool A3MessageReq::sendRequest(std::string reqID, std::string reqURL, std::string& reqContent)
		{
			int64 lStart = ZQ::common::now();
			_pHttpClient->setHeader(NULL,NULL);
			if (_pHttpClient->httpConnect(reqURL.c_str(), ZQ::common::HttpClient::HTTP_POST))
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageReq, "[%s]failed to send request [%s] with error [%s]"), reqID.c_str(), reqURL.c_str(), _pHttpClient->getErrorstr());
				return false;
			}

			_pHttpClient->setHeader("User-Agent", "A3MessageClient");
	
			char buf[256]= "";
			sprintf(buf, CLOGFMT(A3MessageReq, "[%s] "), reqID.c_str());
			_log.hexDump(ZQ::common::Log::L_INFO, reqContent.c_str(), reqContent.size(), buf, true);

			if(_pHttpClient->httpSendContent(reqContent.c_str(), reqContent.size()))
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageReq, "[%s]failed to send request [%s] with error [%s]"), reqID.c_str(), reqURL.c_str(), _pHttpClient->getErrorstr());
				_pHttpClient->uninit();
				return false;
			}
			if (_pHttpClient->httpEndSend() )
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageReq, "[%s]failed to send request [%s] with error [%s]"), reqID.c_str(), reqURL.c_str(), _pHttpClient->getErrorstr());
				_pHttpClient->uninit();
				return false;
			}

			std::string ReponseConent;
			if (_pHttpClient->httpBeginRecv())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageReq, "[%s]failed to send request [%s] with error [%s]"), reqID.c_str(), reqURL.c_str(), _pHttpClient->getErrorstr());

				_pHttpClient->uninit();
				return false;
			}

			int status = _pHttpClient->getStatusCode();

			std::string strRC = "";
			while(!_pHttpClient->isEOF())
			{
				strRC.clear();
				if(_pHttpClient->httpContinueRecv())
				{
					_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageReq, "[%s]failed to receiver request response[%s] with error [%s]"), reqID.c_str(), reqURL.c_str(), _pHttpClient->getErrorstr());
					_pHttpClient->uninit();
					return false;
				}
				_pHttpClient->getContent(strRC);
				ReponseConent += strRC;
			}

			if ( _pHttpClient->httpEndRecv() )
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageReq, "[%s]failed to receiver request response [%s] with error [%s]"), reqID.c_str(), reqURL.c_str(), _pHttpClient->getErrorstr());
				_pHttpClient->uninit();
				return false;
			}
			_pHttpClient->getContent(strRC);
			ReponseConent += strRC;

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s] response [%s]"), reqID.c_str(), ReponseConent.c_str());

			memset(buf, 0, 256);
			sprintf(buf, CLOGFMT(A3MessageReq, "[%s]response  "), reqID.c_str());
			_log.hexDump(ZQ::common::Log::L_INFO, ReponseConent.c_str(), ReponseConent.size(), buf, true);

			_pHttpClient->uninit();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MessageReq, "[%s]send request [%s]took %dms"), reqID.c_str(), reqURL.c_str(), (int)(ZQ::common::now() - lStart));
			return true;
		}

	}///end namespace A3MessageClient
} ///end namespace CRM