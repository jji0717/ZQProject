#include "NGBCmdResponse.h"

namespace ZQTianShan {	  
namespace GBServerNS { 

const std::string  XML_HEADER("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");

std::string  IGBSSCmd::makeXmlContent(void)
{
	std::string content(XML_HEADER);
	content += "<Message>\n";
	content += makeXmlHeader();
	content += makeXmlBody();
	content += "</Message>";

	return content;
}

std::string  IGBSSCmd::makeXmlHeader(void)
{
	std::map<std::string, std::string >::iterator itMap;
	std::string head("<Header>\n");
	
	head += "<TransactionID>";
	itMap = _xmlParseResult.find("TransactionID");
	if (itMap != _xmlParseResult.end())	{ head += itMap->second;}
	head += "</TransactionID>\n";

	head += "<Time>";
	itMap = _xmlParseResult.find("Time");
	if (itMap != _xmlParseResult.end())	{ head += itMap->second;}
	head += "</Time>\n";

	head += "<OpCode>";
    head +=  getCmdStr();
	head += "</OpCode>\n";

	head += "<MsgType>RESP</MsgType>\n";
	head += "<ReturnCode>0200</ReturnCode>\n";
	head += "<ErrorMessage>OK</ErrorMessage>\n";

	head += "</Header>\n";

    return head;
}

std::string  IGBSSCmd::makeXmlBody(void)
{
	return std::string("<Body></Body>\n");
}

}//GBServerNS
}//	ZQTianShan