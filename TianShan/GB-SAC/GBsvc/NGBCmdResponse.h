#if !defined(__ZQTIANSHAN_NGB_CMD_RESPONSE_H__)
#define __ZQTIANSHAN_NGB_CMD_RESPONSE_H__

#include <string>
#include <map>
#include "boost/shared_ptr.hpp"

namespace ZQTianShan {	  
namespace GBServerNS { 

typedef enum _NGBCmdCode
{
	AO_CDN_A4_FILE_PROPAGATION_REQ     ,
	AO_CDN_A4_FILE_BAT_PROPAGATION_REQ ,
	AO_CDN_A4_FILE_STATE_REQ           ,
	CDN_AO_A4_FILE_STATE_NOTIFY        ,
	AO_CDN_A4_FILE_PROPAGATION_CANCEL  ,
	AO_CDN_A4_FILE_DELETE              ,
	A4_cmd_end                         ,
	AO_CDN_A5_STREAM_INGEST_REQ        ,
	AO_CDN_A5_STREAM_BAT_INGEST_REQ    ,
	CDN_AO_A5_STREAM_STATE_NOTIFY      ,
	AO_CDN_A5_STREAM_STATE_REQ         ,
	AO_CDN_A5_STREAM_INGEST_CANCEL     ,
	A5_cmd_end
} NGBCmdCode;

const struct  
{
    NGBCmdCode  _ngbCmdCode;
	char*       _ngbCmdStr;
}ngbCmdTbl[] = {
	{AO_CDN_A4_FILE_PROPAGATION_REQ,      "AO_CDN_A4_FILE_PROPAGATION_REQ"},
	{AO_CDN_A4_FILE_BAT_PROPAGATION_REQ,  "AO_CDN_A4_FILE_BAT_PROPAGATION_REQ"},
	{AO_CDN_A4_FILE_STATE_REQ,            "AO_CDN_A4_FILE_STATE_REQ"},
	{CDN_AO_A4_FILE_STATE_NOTIFY,         "CDN_AO_A4_FILE_STATE_NOTIFY"},
	{AO_CDN_A4_FILE_PROPAGATION_CANCEL,   "AO_CDN_A4_FILE_PROPAGATION_CANCEL"},
	{AO_CDN_A4_FILE_DELETE,               "AO_CDN_A4_FILE_DELETE"},
	{A4_cmd_end,                          "A4_cmd_end"},
	{AO_CDN_A5_STREAM_INGEST_REQ,         "AO_CDN_A5_STREAM_INGEST_REQ"},
	{AO_CDN_A5_STREAM_BAT_INGEST_REQ,     "AO_CDN_A5_STREAM_BAT_INGEST_REQ"},
	{CDN_AO_A5_STREAM_STATE_NOTIFY,       "CDN_AO_A5_STREAM_STATE_NOTIFY"},
	{AO_CDN_A5_STREAM_STATE_REQ,          "AO_CDN_A5_STREAM_STATE_REQ"},
	{AO_CDN_A5_STREAM_INGEST_CANCEL,      "AO_CDN_A5_STREAM_INGEST_CANCEL"},
	{A5_cmd_end,                          "A5_cmd_end"}
};

class IGBSSCmd
{
public:
	IGBSSCmd(std::map<std::string, std::string >& xmlParseResult, std::string opCodeInXML)
		:_xmlParseResult(xmlParseResult), _opCodeInXML(opCodeInXML)
	{}

	virtual ~IGBSSCmd(){}

	virtual std::string  makeXmlContent(void);
	virtual std::string  getCmdStr(void){return _opCodeInXML;};	

private:
	virtual std::string  makeXmlBody(void);
	virtual std::string  makeXmlHeader(void);

private:
	std::string _opCodeInXML;
	std::map<std::string, std::string >& _xmlParseResult;
};

typedef IGBSSCmd  IGBA4Resp;
typedef IGBSSCmd  IGBA5Resp;

class A4FileStateNotifyResp : public IGBA4Resp
{
public:
	A4FileStateNotifyResp(std::map<std::string, std::string >& xmlParseResult, std::string opCodeInXML)
		:IGBA4Resp(xmlParseResult, opCodeInXML), _xmlParseResult(xmlParseResult)
	{}

	virtual ~A4FileStateNotifyResp(){}

private:
	virtual std::string  makeXmlBody(void)
	{
		std::string content("<Body>\n");

		content += "<ContentProState ";
		content += " contentID=\"";
		std::map<std::string, std::string >::iterator it = _xmlParseResult.find("contentID");
		if (it != _xmlParseResult.end()) 
			content += it->second;
		else
			content += "NULL";

		content += "\"  reasonCode=\"200\" ";

		content += "state=\"Pending\"  ";

		content += "volumeName=\"";
		it = _xmlParseResult.find("volumeName");
		if (it != _xmlParseResult.end())
			content += it->second;
		else
			content += "NULL";

		content += "\"> ";

		content+= "</ContentProState>\n";
		content += "</Body>\n";

		return content;
	}

private:
	std::map<std::string, std::string >& _xmlParseResult;
};

typedef A4FileStateNotifyResp A4FilePropagationResp;
typedef A4FileStateNotifyResp A4BatFilePropagationResp;
typedef A4FileStateNotifyResp A4FilePropagationCancelResp;
typedef A4FileStateNotifyResp A4FileDeleteResp;

typedef A4FileStateNotifyResp A4FileStateResp;

typedef A4FilePropagationResp A5StreamIngestResp;
typedef A4FileStateNotifyResp A5StreamBatIngestResp;
typedef A4FileStateNotifyResp A5StreamIngestCancelResp;
typedef A4FileStateResp       A5StreamStateResp;

typedef A4FileStateNotifyResp A5StreamStateNotifyResp;


typedef boost::shared_ptr< IGBSSCmd > IGBSSCmdPtr;

}//GBServerNS
}//	ZQTianShan

#endif// __ZQTIANSHAN_NGB_CMD_RESPONSE_H__