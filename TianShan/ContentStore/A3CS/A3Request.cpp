#include "A3Request.h"

#define A3REQLOG if(_pLog) (*_pLog) 

A3Request::A3Request()
{
	_pLog = NULL;
	_http.init();
	_http.setRecvTimeout(20);
}

A3Request::A3Request(std::string& strHost, ZQ::common::Log* pLog)
:_strHost(strHost),_pLog(pLog)
{
	size_t ns = _strHost.length();
	if(_strHost[ns-1] != '/')
		_strHost += '/';

	_http.init();
	_http.setRecvTimeout(20);
	_http.setLog(pLog);
}

void A3Request::setHost(std::string& strHost)
{
	_strHost = strHost;
	size_t ns = _strHost.length();
	if(_strHost[ns-1] != '/')
		_strHost += '/';
}

void A3Request::setLog(ZQ::common::Log* pLog)
{
	_pLog = pLog;
	_http.setLog(pLog);
}

A3Request::~A3Request(void)
{
}

int A3Request::TransferContent(const TransferInfo& info) {
	std::string str = _xmlOp.makeTransferContent(info);
	std::string url = _strHost + "TransferContent";

	return SendRequest(url, str);
}

int A3Request::GetTransferStatus(TransferStatus& info) {
	std::string str = _xmlOp.makeGetTransferStatus(info);
	std::string url = _strHost + "GetTransferStatus";
	
	int code = SendRequest(url, str);
	if(code == 200) {
		if(!_xmlOp.parseGetTransferStatus(info, str.c_str(), str.length())) {
			return (-1);
		}
	}
	
	return code;
}

int A3Request::CancelTransfer(const DeleteCancelContent& info) {
	std::string str = _xmlOp.makeCancelTransfer(info);
	std::string url = _strHost + "CancelTransfer";

	return SendRequest(url, str);
}

int A3Request::ExposeContent(const ExposeContentInfo& info, ExposeResponse& response) {
	std::string str = _xmlOp.makeExposeContent(info);
	std::string url = _strHost + "ExposeContent";
	
	int code = SendRequest(url, str);
	if(code == 200) {
		if(!_xmlOp.parseExposeContent(response, str.c_str(), str.length())) {
			return (-1);
		}
	}
	return code;
}

int A3Request::GetContentChecksum(const ContentChecksumInfo& info) {
	std::string str = _xmlOp.makeGetContentChecksum(info);
	std::string url = _strHost + "GetContentChecksum";

	return SendRequest(url, str);
}

int A3Request::GetContentInfo(const ContentInfo& info, std::vector<ContentInfo>& contentInfo) {
	std::string str = _xmlOp.makeGetContentInfo(info);
	std::string url = _strHost + "GetContentInfo";
	
	int code = SendRequest(url, str);
	if(code == 200) {
		if(!_xmlOp.parseGetContentInfo(contentInfo, str.c_str(), str.length())) {
			return (-1);
		}
	}
	return code;
}

int A3Request::DeleteContent(const DeleteCancelContent& info) {
	std::string str = _xmlOp.makeDeleteContent(info);
	std::string url = _strHost + "DeleteContent";

	return SendRequest(url, str);
}

int A3Request::GetVolumeInfo(struct VolumeInfo& info) {
	std::string str = _xmlOp.makeGetVolumeInfo(info);
	std::string url = _strHost + "GetVolumeInfo";

	int code = SendRequest(url, str);
	if(code == 200) {
		if(!_xmlOp.parseGetVolumeInfo(info, str.c_str(), str.length())) {
			return (-1);	
		}
	}
	return code;
}

std::string A3Request::getStatusMessage() {
	return _http.getMsg();
}

int A3Request::SendRequest(const std::string& url, std::string& buffer) {

	if(_http.httpConnect(url.c_str())) {
		A3REQLOG(ZQ::common::Log::L_ERROR,"_strHost() connect failed,endpoint: %s",url.c_str());
		_http.uninit();
		return (-1);
	}

	if(_http.httpSendContent(buffer.data(), buffer.length()) || _http.httpEndSend()) {
		A3REQLOG(ZQ::common::Log::L_ERROR,"_strHost() send errorcode: %d",_http.getErrorcode());
		_http.uninit();
		return (-1);
	}

	if(_http.httpBeginRecv()) {
		A3REQLOG(ZQ::common::Log::L_ERROR,"_strHost() receive errorstring:%s",_http.getErrorstr());
		_http.uninit();
		return (-1);
	}

	while(!_http.isEOF()) {

		if(_http.httpContinueRecv()) {
			A3REQLOG(ZQ::common::Log::L_ERROR,"_strHost() continue receive errorstring:%s",_http.getErrorstr());
			_http.uninit();	
			return (-1);
		}
	}

	_http.getContent(buffer);

	if (_pLog)
		_pLog->hexDump(ZQ::common::Log::L_DEBUG, buffer.c_str(), buffer.length(), NULL);

	_http.uninit();
	
	return _http.getStatusCode();
}
