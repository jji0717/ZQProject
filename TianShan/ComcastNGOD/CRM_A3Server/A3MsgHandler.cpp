// FileName : A3MsgHandler.cpp
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : handle A3 request message from APM client

#include "A3MsgHandler.h"
#include "A3Common.h"
#include "A3Config.h"
#include "A3Client.h"

// A3 Message attribute
const std::string providerID = "providerID";
const std::string assetID = "assetID";
const std::string volumeName = "volumeName";
const std::string captureStart = "captureStart"; // need for PGM
const std::string captureEnd = "captureEnd";   // need for PGM
const std::string transferBitRate = "transferBitRate";
const std::string sourceURL = "sourceURL";
const std::string userName = "userName"; // for others but PGM
const std::string password = "password"; // for others but PGM
const std::string responseURL = "responseURL";
const std::string reasonCode = "reasonCode";
const std::string protocol = "protocol";

using namespace CRG::Plugin::A3Server;

A3MsgHandler::A3MsgHandler(A3FacedeIPtr a3FacedeIPtr, A3Client* a3Client)
: _a3FacedeIPtr(a3FacedeIPtr), _a3Client(a3Client)
{
}


A3MsgHandler::~A3MsgHandler(void)
{
}

void A3MsgHandler::onRequest(const CRG::IRequest *request, CRG::IResponse *response)
{
	std::string strFullURL = request->uri();
	std::string strReqMethod = strFullURL.substr(strFullURL.find_last_of("/"));
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "onRequest() : receive request uri=%s"), strFullURL.c_str());
	if ("/TransferContent" == strReqMethod)
	{
		fixupTransferContent(request, response);
		return;
	}
	if ("/GetVolumeInfo" == strReqMethod)
	{
		fixupGetVolumeInfo(request, response);
		return ;
	}
	if ("/GetContentInfo" == strReqMethod)
	{
		fixupGetContentInfo(request, response);
		return;
	}
	if ("/CancelTransfer" == strReqMethod)
	{
		fixupCancelTransfer(request, response);
		return;
	}
	if ("/ExposeContent" == strReqMethod)
	{
		fixupExposeContent(request, response);
		return;
	}
	if ("/GetContentChecksum" == strReqMethod)
	{
		fixupGetContentChecksum(request, response);
		return;
	}
	if ("/GetTransferStatus" == strReqMethod)
	{
		fixupGetTransferStatus(request, response);
		return;
	}
	if ("/DeleteContent" == strReqMethod)
	{
		fixupDeleteContent(request, response);
		return;
	}
}

void A3MsgHandler::fixupGetVolumeInfo(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetVolumeInfo() : do fixupGetVolumeInfo()")); 
	
	// parse and valide request
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) || 
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume))
	{
		setReponseWithoutBody(request, response, 404, "Unknown Volume");
		return;
	}

	// get volume info
	int state = 0;
	Ice::Long freeMB = 0;
	Ice::Long totalMB = 0;
	if (!_a3FacedeIPtr->getVolumeInfo(strNetId, strVolume, totalMB, freeMB, state))
	{
		setReponseWithoutBody(request, response, 404, "Unknown Volume");
		return;
	}

	// output volume info
	std::ostringstream buf;
	buf << XML_HEADER ;
	buf << "<VolumeInfo " ;
	buf <<     "volumeName=\"" << xmlElement[volumeName] << "\" ";
	buf <<     "state=\""      << state                  << "\" ";
	buf <<     "volumeSize=\"" << totalMB                << "\" ";
	buf <<     "freeSize=\""   << freeMB                 << "\" ";
	buf << "/>";
	strMsgContent = buf.str();
	setResponseWithBody(request, response, 200, "OK", strMsgContent);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetVolumeInfo() : leave fixupGetVolumeInfo()")); 
}

void A3MsgHandler::fixupGetContentInfo(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetContentInfo() : do fixupGetContentInfo()")); 
	
	// parse and valide request
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) || 
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume))
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return;
	}

	StringVector volumeNames;
	_a3FacedeIPtr->getVolumes(strNetId, volumeNames); // get all volume names in special content store
	if ("/*" != strVolume) // request special volume
	{
		if (std::find(volumeNames.begin(), volumeNames.end(), strVolume) == volumeNames.end())
		{
			setReponseWithoutBody(request, response, 404, "Content Not Found");
			return;
		}
		volumeNames.clear();
		volumeNames.push_back(strVolume);
	}

	bool bOneContent = false;
	if (!xmlElement[providerID].empty() && !xmlElement[assetID].empty()
		&& xmlElement[providerID] != "*" && xmlElement[assetID] != "*")
	{
		// request special content
		if ((_a3FacedeIPtr->findContentsByAsset(xmlElement[providerID], xmlElement[assetID])).empty())
		{
			setReponseWithoutBody(request, response, 404, "Content Not Found");
			return;
		}
		bOneContent = true;
	}

    // get all request content info
	::A3Module::A3Contents a3ContentsOfVolume;
	std::string strMD5DataTime = GenerateUTCTime();
	TianShanIce::Properties metaData;
	std::string strPAID;
	std::string strPID;
	TianShanIce::Storage::ContentState state;
	std::ostringstream buf;
	buf << XML_HEADER ;
	buf << "<ContentList>\n";
	for (StringVector::iterator volumeIt = volumeNames.begin(); volumeIt != volumeNames.end(); volumeIt++)
	{
		std::string strFullVol = strNetId + (*volumeIt);
		if (!bOneContent)
		{
			a3ContentsOfVolume = _a3FacedeIPtr->listContentsByVolume(strNetId, *volumeIt);
			for (A3Module::A3Contents::const_iterator contentIt = a3ContentsOfVolume.begin();
				contentIt != a3ContentsOfVolume.end(); contentIt++)
			{
				 try
				 {
					 metaData = (*contentIt)->getMetaData();
					 (*contentIt)->getAssetId(strPID, strPAID);
					 state = (*contentIt)->getState();
				 }
				 catch (const Ice::Exception& ex)
				 {
					 glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupGetContentInfo() : catch an exception [%s]"), ex.ice_name().c_str());
					 continue;
				 }
				 buf << "<ContentInfo " ;
				 buf <<    "providerID=\""   << strPID      << "\" ";
				 buf <<    "assetID=\""      << strPAID     << "\" ";
				 buf <<    "volumeName=\""   << strFullVol  << "\" ";
				 if (state == ::TianShanIce::Storage::csInService)
				 {
					 buf <<    "contentSize=\""       << metaData["sys.FileSize"]        << "\" ";
					 buf <<    "supportFileSize=\""   << metaData["sys.SupportFileSize"] << "\" ";
					 buf <<    "md5Checksum=\""       << metaData["sys.MD5CheckSum"]     << "\" ";
					 buf <<    "md5DateTime=\""       << strMD5DataTime                  << "\" ";
				 }
				 buf <<    "contentState=\"" << convertState(state)                      << "\" ";
				 buf << "/>\n";
			} 
		}// if all content
		else
		{
			A3Module::A3ContentPrx a3ContentProxy = _a3FacedeIPtr->openA3Content(xmlElement[providerID], 
				xmlElement[assetID], strNetId, strVolume);
			if (!a3ContentProxy)
			{
				continue;
			}
			try
			{
				metaData = a3ContentProxy->getMetaData();
				state = a3ContentProxy->getState();
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupGetContentInfo() : catch an exception [%s]"), ex.ice_name().c_str());
				continue;
			}
			buf << "<ContentInfo " ;
			buf <<    "providerID=\""   << xmlElement[providerID]      << "\" ";
			buf <<    "assetID=\""      << xmlElement[assetID]         << "\" ";
			buf <<    "volumeName=\""   << xmlElement[volumeName]      << "\" ";
			if (state == ::TianShanIce::Storage::csInService)
			{
				buf <<    "contentSize=\""       << metaData["sys.FileSize"]        << "\" ";
				buf <<    "supportFileSize=\""   << metaData["sys.SupportFileSize"] << "\" ";
				buf <<    "md5Checksum=\""       << metaData["sys.MD5CheckSum"]     << "\" ";
				buf <<    "md5DateTime=\""       << strMD5DataTime                  << "\" ";
			}
			buf <<    "contentState=\"" << convertState(state)                      << "\" ";
			buf << "/>\n";
		} // if one content
	} // for all volume
	buf << "</ContentList>\n";
	strMsgContent = buf.str();
	setResponseWithBody(request, response, 200, "OK", strMsgContent);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetContentInfo() : leave fixupGetContentInfo()"));
}

void A3MsgHandler::fixupTransferContent(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupTransferContent() : do fixupTransferContent()"));
	
    // parse and valide request
	std::string strMsgContent("");
	StringMap xmlElement;
	StringMap metaDatas;
	std::string strNetId("");
	std::string strVolume("");
	request->getContent(strMsgContent);
	if (!parseMsgContentEx(strMsgContent.c_str(), strMsgContent.length(), xmlElement, metaDatas) ||
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume) ||
		xmlElement[assetID].empty() || xmlElement[providerID].empty() ||
		xmlElement[sourceURL].empty() || xmlElement[responseURL].empty() ||
		0 == atoi(xmlElement[transferBitRate].c_str()))
	{
		setReponseWithoutBody(request, response, 400, "Invalid request format");
		return;
	}
	std::string strSourceURL = xmlElement[sourceURL];
	if ((strSourceURL.find(TianShanIce::Storage::potoFTP)  == std::string::npos) &&
		(strSourceURL.find(TianShanIce::Storage::potoNFS)  == std::string::npos) &&
		(strSourceURL.find(TianShanIce::Storage::potoPGM)  == std::string::npos) &&
		(strSourceURL.find(TianShanIce::Storage::potoCIFS) == std::string::npos))
	{
		setReponseWithoutBody(request, response, 451, "Unsupported transfer protocol");
		return;
	}
	if (strSourceURL.find(TianShanIce::Storage::potoPGM) != std::string::npos)
	{
		if (xmlElement[captureStart].empty() || xmlElement[captureEnd].empty())
		{
			setReponseWithoutBody(request, response, 400, "Invalid request format");
			return ;
		}
	}
	else
	{
		// URL format : ftp://user:password@ip:port/filename 
		size_t pos = strSourceURL.find("//");
		if (pos == std::string::npos)
		{
			setReponseWithoutBody(request, response, 400, "Invalid request format");
			return;
		}
		pos += strlen("//");
		strSourceURL.insert(pos, xmlElement[userName] + ":" + xmlElement[password] + "@" );
	}

	// create content if no exist
	std::string strResponseURL = xmlElement[responseURL] + "/TransferStatus";
	std::string strContentType = (metaDatas["sys.LocalType"] != "") ? (metaDatas["sys.LocalType"] + ":" +
		metaDatas["sys.SubType"]) : TianShanIce::Storage::ctMPEG2TS;
	A3Module::A3ContentPrx a3ContentProxy = _a3FacedeIPtr->createContent(xmlElement[providerID],
		xmlElement[assetID], strNetId, strVolume, strResponseURL, strContentType);
	if (!a3ContentProxy)
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return;
	}

	// transfer content 
	int nTransferBitRate = atoi(xmlElement[transferBitRate].c_str());
	try
	{
		TianShanIce::Storage::ContentPrx contentProxy = a3ContentProxy->theContent();
		contentProxy->provision(strSourceURL, strContentType, true, 
			xmlElement[captureStart], xmlElement[captureEnd], nTransferBitRate);
	}
	catch (const Ice::Exception& ex)
	{
		setReponseWithoutBody(request, response,500, "Internal server error");
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupTransferContent() : catch an ice exception [%s] when provision content with [%s]"), ex.ice_name().c_str(), strSourceURL.c_str());
		return;
	}
	setReponseWithoutBody(request, response, 200, "Server accepted the distribution request");
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupTransferContent() : leave fixupTransferContent()"));
}

void A3MsgHandler::fixupCancelTransfer(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupCancelTransfer() : do fixupCancelTransfer()"));

	// parse and valide request
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) ||
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume) ||
		xmlElement[assetID].empty() || xmlElement[providerID].empty())
	{
		setReponseWithoutBody(request, response, 404, "Unknown Provider Asset ID");
		return;
	}

	// open a3 content
	A3Module::A3ContentPrx a3ContentProxy  = _a3FacedeIPtr->openA3Content(xmlElement[providerID],
		xmlElement[assetID],  strNetId, strVolume);
	if (!a3ContentProxy)
	{
		setReponseWithoutBody(request, response, 404, "Unknown Provider Asset ID");
		return;
	}

	// cancel provision
	try
	{
		// cancel task and delete content file
		TianShanIce::Storage::ContentPrx contentProxy = a3ContentProxy->theContent();
		contentProxy->cancelProvision();
	}
	catch (const Ice::Exception& ex)
	{
		setReponseWithoutBody(request, response, 404, "Unknown Provider Asset ID");
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupCancelTransfer() : catch an ice exception [%s] when cancel content transfer"), ex.ice_name().c_str());
		return ;
	}
	setReponseWithoutBody(request, response, 200, "OK (Transfer canceled)");
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupCancelTransfer() : leave fixupCancelTransfer()"));
}

void A3MsgHandler::fixupDeleteContent(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupDeleteContent() : do fixupDeleteContent()"));

    // parse and valide request
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) ||
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume) ||
		xmlElement[assetID].empty() || xmlElement[providerID].empty())
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return; 
	}

	// open a3 content
	std::string strContentName = xmlElement[volumeName] + "/" + xmlElement[assetID] + "_" +xmlElement[providerID];
	A3Module::A3ContentPrx a3ContentProxy  = _a3FacedeIPtr->openA3Content( xmlElement[providerID],
		xmlElement[assetID], strNetId, strVolume);
	if (!a3ContentProxy)
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return;
	}

	// destroy content
	try
	{
		// if fail , APM client must be retry
		TianShanIce::Storage::ContentPrx contentProxy = a3ContentProxy->theContent();
		contentProxy->destroy();
	}
	catch (const TianShanIce::InvalidStateOfArt& ex)
	{
		setReponseWithoutBody(request, response, 409, "File in use.  Retry later");
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupDeleteContent() : catch an ice exception [%s] when destory content [%s]"), ex.ice_name().c_str(), strContentName.c_str());
		return ;
	}
	catch (const Ice::Exception& ex)
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupDeleteContent()) : catch an ice exception [%s] when destory content [%s]"), ex.ice_name().c_str(), strContentName.c_str());
		return ;
	}
	setReponseWithoutBody(request, response, 200, "OK");
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupDeleteContent() : leave fixupDeleteContent()"));
}

void A3MsgHandler::fixupGetContentChecksum(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgHandler, "fixupGetContentChecksum() : do fixupGetConentChecksum"));

	// parse and valide request
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) ||
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume) ||
		xmlElement[assetID].empty() || xmlElement[providerID].empty() || xmlElement[responseURL].empty())
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return; 
	}

	// open a3 content
	std::string strContentName = xmlElement[volumeName]+ "/" + xmlElement[assetID] + "_" + xmlElement[providerID];
	A3Module::A3ContentPrx contentProxy = _a3FacedeIPtr->openA3Content(xmlElement[providerID], 
		xmlElement[assetID],  strNetId, strVolume);
	if (!contentProxy)
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return;
	}

	// get content md5 checksum and send http request
	TianShanIce::Properties a3ContentMetaData;
	TianShanIce::Storage::ContentState contentState = TianShanIce::Storage::csOutService;
	int resultCode = 404;
	try
	{
		a3ContentMetaData = contentProxy->getMetaData();
		contentState = contentProxy->getState();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupGetContentChecksum() : catch an exception[%s] when get content checksum"), ex.ice_name().c_str());
	}
	std::ostringstream buf;
	buf << XML_HEADER ;
	buf << "<ContentChecksum ";
	buf <<    "providerID=\""   << xmlElement[providerID]   << "\" ";
	buf <<    "assetID=\""      << xmlElement[assetID]      << "\" ";
	buf <<    "volumeName=\""   << xmlElement[volumeName]   << "\" ";
	if (contentState == TianShanIce::Storage::csInService)
	{
		resultCode = 200;
		buf <<    "md5Checksum=\""  << a3ContentMetaData["sys.MD5CheckSum"] << "\" ";
		buf <<    "md5DateTime=\""  << GenerateUTCTime()                    << "\" ";
	}
	buf <<    "resultCode=\""   <<  resultCode             << "\" ";
	buf << "/>";
	std::string strContent = buf.str();
	std::string strResponseURL = xmlElement[responseURL] + "/ContentChecksum";
	_a3Client->SendRequest(strResponseURL, strContent);

    // set response 
	setReponseWithoutBody(request, response, 200, "OK (Async request accepted)");
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetContentChecksum() : leave fixupGetConentChecksum"));
}

void A3MsgHandler::fixupExposeContent(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupExposeContent() : do fixupExposeContent()"));

	// parse and valide request
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) ||
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume) ||
		xmlElement[assetID].empty() || xmlElement[providerID].empty())
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return;
	}
	if (xmlElement[protocol] != TianShanIce::Storage::potoFTP && 
		xmlElement[protocol] != TianShanIce::Storage::potoNFS && 
		xmlElement[protocol] != TianShanIce::Storage::potoCIFS)
	{
		setReponseWithoutBody(request, response, 451, "Unsupported transfer protocol");
		return;
	}

	// open a3 content
	std::string strContentName = xmlElement[volumeName]+ "/" + xmlElement[assetID] + "_" + xmlElement[providerID];
	A3Module::A3ContentPrx contentProxy = _a3FacedeIPtr->openA3Content(xmlElement[providerID], 
		xmlElement[assetID], strNetId, strVolume);
	if (!contentProxy)
	{
		setReponseWithoutBody(request, response, 404, "Content Not Found");
		return;
	}

	// get expose url
	int ttl;
	std::string strURL;
	TianShanIce::Properties addtionalInfos;
	try
	{
		strURL = contentProxy->theContent()->getExportURL(xmlElement[protocol], 
			atoi(xmlElement[transferBitRate].c_str()), ttl, addtionalInfos);
	}
	catch (const Ice::Exception& ex)
	{
		setReponseWithoutBody(request, response, 451, "Unsupported transfer protocol");
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupExposeContent() : catch an ice exception [%s] when expose content [%s]"), ex.ice_name().c_str(), strContentName.c_str());
		return ;
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgHandler, "fixupExposeContent() : success to get expose contetn [%s] infos"), strContentName.c_str());

	// ftp://aa:aa@192.168.81.98:21/filname
	// find user name
	size_t nStart = strURL.find("//");
	if (std::string::npos == nStart)
	{
		setReponseWithoutBody(request, response, 451, "Unsupported transfer protocol");
		return;
	}
	size_t nEnd = strURL.find(":", nStart);
	if (std::string::npos == nEnd)
	{
		setReponseWithoutBody(request, response, 451, "Unsupported transfer protocol");
		return;
	}
	nStart += strlen("//");
	std::string strUserName = strURL.substr(nStart, nEnd - nStart);

	// find password
	nStart = nEnd + strlen(":");
	nEnd = strURL.find("@", nStart);
	if (std::string::npos == nEnd)
	{
		setReponseWithoutBody(request, response, 451, "Unsupported transfer protocol");
		return;
	}
	std::string strPassword = strURL.substr(nStart, nEnd - nStart);

	// find url
	nStart = nEnd + strlen("@");
	std::string strOriURL = xmlElement[protocol] + "://" + strURL.substr(nStart);
	
	std::ostringstream buf;
	buf << XML_HEADER;
	buf << "<ExposeResponse " ;
	buf <<    "providerID=\""   << xmlElement[providerID]                          << "\" ";
	buf <<    "assetID=\""      << xmlElement[assetID]                             << "\" ";
	buf <<    "URL=\""          << strOriURL                                       << "\" ";
	buf <<    "userName=\""     << strUserName                                     << "\" ";
	buf <<    "password=\""     << strPassword                                     << "\" ";
	buf <<    "ttl=\""          << ttl                                             << "\" ";
	buf <<    "transferBitRate=\"" << addtionalInfos[TianShanIce::Storage::expTransferBitrate] << "\" ";
	buf << "/>";
	strMsgContent = buf.str();
	setResponseWithBody(request, response, 200, "OK", strMsgContent);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupExposeContent() : leave fixupExposeContent()"));
}

void A3MsgHandler::fixupGetTransferStatus(const CRG::IRequest* request, CRG::IResponse* response)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetTransferStatus() : do fixupGetTransferStatus() "));

	// parse and valide request 
	std::string strMsgContent("");
	std::string strNetId("");
	std::string strVolume("");
	StringMap xmlElement;
	request->getContent(strMsgContent);
	if (!parseMsgContent(strMsgContent.c_str(), strMsgContent.length(), xmlElement) ||
		!getVolumeNameAndId(xmlElement[volumeName], strNetId, strVolume) ||
		xmlElement[assetID].empty() || xmlElement[providerID].empty())
	{
		setReponseWithoutBody(request, response, 404, "Unknown Provider Asset ID");
		return;
	}

	// open a3 content
	std::string strContentName = xmlElement[volumeName]+ "/" + xmlElement[assetID] + "_" + xmlElement[providerID];
	A3Module::A3ContentPrx contentProxy = _a3FacedeIPtr->openA3Content(xmlElement[providerID], 
		xmlElement[assetID], strNetId, strVolume);
	if (!contentProxy)
	{
		setReponseWithoutBody(request, response, 404, "Unknown Provider Asset ID");
		return;
	}

	// get content status
	TianShanIce::Properties metaData;
	TianShanIce::Storage::ContentState contentState;
	int reasonCode = 200;
	try
	{
		metaData = contentProxy->getMetaData();
		contentState = contentProxy->getUpdateState();
	}
	catch (const Ice::Exception& ex)
	{
		reasonCode = 500;
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "fixupGetTransferStatus() : catch an exception [%s]"), ex.ice_name().c_str());
	}

    // output response
	std::ostringstream buf;
	buf << XML_HEADER ;
	buf << "<TransferStatus ";
	buf <<    "providerID=\""   << xmlElement[providerID]    << "\" ";
	buf <<    "assetID=\""      << xmlElement[assetID]       << "\" ";
	buf <<    "volumeName=\""   << xmlElement[volumeName]    << "\" ";
	buf <<    "state=\""        << convertState(contentState)<< "\" ";
	buf <<    "reasonCode=\""   << reasonCode                << "\" ";
	if (contentState == ::TianShanIce::Storage::csInService)
	{
		buf <<    "contentSize=\""       << metaData["sys.FileSize"]        << "\" ";
		buf <<    "supportFileSize=\""   << metaData["sys.SupportFileSize"] << "\" ";
		buf <<    "md5Checksum=\""       << metaData["sys.MD5CheckSum"]     << "\" ";
		buf <<    "md5DateTime=\""       << GenerateUTCTime()               << "\" ";
	}
	buf << "/>";
	strMsgContent = buf.str();
	setResponseWithBody(request, response, 200, "OK", strMsgContent);
    glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "fixupGetTransferStatus() : leave fixupGetTransferStatus() "));
}

bool A3MsgHandler::readXMLDoc(ZQ::common::XMLPreferenceDocumentEx& xmlDoc, const char* buffer, size_t bufLen)
{
	try
	{
		if(xmlDoc.read((void*)buffer, (int)bufLen, 1))//successful
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgHandler, "readXMLDoc() : read xml content successfully"));
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "readXMLDoc() : read xml content fail"));
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "readXMLDoc() : read xml catch a exception [%s]"), xmlEx.getString());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "readXMLDoc() : read xml catch a exception"));
		return false;
	}
	return true;
}

bool A3MsgHandler::parseMsgContent(const char* buffer, size_t bufLen, StringMap& xmlElement)
{
	if (!buffer || 0 == bufLen)
	{
		return false;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "parseMsgContent() : request content is [%s]"),
		buffer);
	::ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if (!readXMLDoc(xmlDoc, buffer, bufLen))
	{
		return false;
	}
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		xmlDoc.clear();
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "parseMsgContent() : getRootPreference error")); 
		return false;
	}
	xmlElement.clear();
	xmlElement = pXMLRoot->getProperties();
	pXMLRoot->free();
	xmlDoc.clear();
	return true;
}

bool A3MsgHandler::parseMsgContentEx(const char* buffer, size_t bufLen, 
									 StringMap& xmlElement, StringMap& metaDatas)
{
	if (!buffer || 0 == bufLen)
	{
		return false;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "parseMsgContent() : request content is [%s]"),
		buffer);
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if (!readXMLDoc(xmlDoc, buffer, bufLen))
	{
		return false;
	}
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		xmlDoc.clear();
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "parseMsgContent() : getRootPreference error")); 
		return false;
	}
	ZQ::common::XMLPreferenceEx* contentAsset = pXMLRoot->findChild("ContentAsset");
	if(NULL == contentAsset)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "parseMsgContent() : get ContentAsset Preference error")); 
		return false;
	}

	// get root properties
	xmlElement.clear();
	xmlElement = pXMLRoot->getProperties();

	// get optional metaDatas
	ZQ::common::XMLPreferenceEx* metaDatasProps = contentAsset->findChild("MetaDatas");
	if (metaDatasProps)
	{
		metaDatas.clear();
		metaDatas = metaDatasProps->getProperties();
	}	
	pXMLRoot->free();
	xmlDoc.clear();
	return true;
}

void A3MsgHandler::setReponseWithoutBody(const CRG::IRequest* request, CRG::IResponse* response, 
										 int statusCode, const char* reasonPhrase)
{
	response->setStatus(statusCode, reasonPhrase);
	response->setHeader("CSeq", request->header("CSeq"));
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "setReponseHeaderWithNoBody() : %d %s"), statusCode, reasonPhrase);
}

void A3MsgHandler::setResponseWithBody(const CRG::IRequest* request, CRG::IResponse* response, 
									   int statusCode, const char* reasonPhrase, std::string strMsgContent)
{
	char length[20];
	snprintf(length, sizeof(length), "%ld", strMsgContent.length());
	response->setStatus(statusCode, reasonPhrase);
	response->setHeader("Content-type", "text/xml");
	response->setHeader("Content-length", length);
	response->setHeader("CSeq", request->header("CSeq"));
	response->setContent(strMsgContent.data(), strMsgContent.length());
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "setResponseWithBody() : Response content is [%s]"), strMsgContent.c_str());
}

bool A3MsgHandler::getVolumeNameAndId(const std::string &strFullVol, std::string &strNetId, std::string& strVolume)
{
	if (strFullVol.empty())
	{
		return false;
	}
	size_t nPosition = strFullVol.find("/");
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
	return false;
}

