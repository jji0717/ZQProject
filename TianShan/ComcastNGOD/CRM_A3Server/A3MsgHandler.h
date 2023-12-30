// FileName : A3MsgHandler.h
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : handle A3 request message from APM client

#ifndef __CRG_PLUGIN_A3SERVER_A3MSG_HANDLER_H__
#define __CRG_PLUGIN_A3SERVER_A3MSG_HANDLER_H__

#include "CRMInterface.h"
#include "TsStorage.h"
#include "XMLPreferenceEx.h"
#include "A3ModuleImpl.h"
#include "A3Common.h"

namespace CRG
{
namespace Plugin
{
namespace A3Server
{

class A3Client;

class A3MsgHandler : public CRG::IContentHandler
{
public:
	A3MsgHandler(A3FacedeIPtr a3FacedeIPtr, A3Client* a3Client);
	~A3MsgHandler(void);

	/// A3 message handle entry point
	virtual void onRequest(const CRG::IRequest* request, CRG::IResponse* response);

private:
	/// fixup get volume info from APM client
	void fixupGetVolumeInfo(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup get Content info from APM client
	void fixupGetContentInfo(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup transfer content form APM client, asynchronous method
	void fixupTransferContent(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup cancel content from APM client
	void fixupCancelTransfer(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup delete content from APM client
	void fixupDeleteContent(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup get content checksum form APM client, asynchronous method
	void fixupGetContentChecksum(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup expose content from APM client
	void fixupExposeContent(const CRG::IRequest* request, CRG::IResponse* response);

	/// fixup get transfer status from APM client
	void fixupGetTransferStatus(const CRG::IRequest* request, CRG::IResponse* response);

private:
	bool getVolumeNameAndId(const std::string& strFullVol, std::string& strNetId, std::string& strVolume);

	/// read XML Doc
	inline bool readXMLDoc(ZQ::common::XMLPreferenceDocumentEx& xmlDoc, const char* buffer, size_t bufLen);

	/// parse XML message content
	bool parseMsgContent(const char* buffer, size_t bufLen, StringMap& xmlElement);

	/// parse XML message content
	bool parseMsgContentEx(const char* buffer, size_t bufLen, StringMap& xmlElement, StringMap& metaDatas);

	/// set http OK response with content body
	inline void setResponseWithBody(const CRG::IRequest* request, CRG::IResponse* response, 
		int statusCode, const char* reasonPhrase, std::string strMsgContent);

	/// set http response without content body
	inline void setReponseWithoutBody(const CRG::IRequest* request, CRG::IResponse* response, 
		int statusCode, const char* reasonPhrase);

	/// forbidden copy and assign
	A3MsgHandler(const A3MsgHandler& a3MsgHandler);
	A3MsgHandler& operator=(const A3MsgHandler& a3MsgHandler);

private:
	A3FacedeIPtr _a3FacedeIPtr;
	A3Client* _a3Client;
};


} // end for A3Server
} // end for Plugin
} // end for CRG

#endif

