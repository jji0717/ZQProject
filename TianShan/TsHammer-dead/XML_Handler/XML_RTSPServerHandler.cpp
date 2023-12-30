#include "XML_RTSPServerHandler.h"

XML_RtspServerHandler::XML_RtspServerHandler()
{
	
}

XML_RtspServerHandler::~XML_RtspServerHandler()
{

}

bool XML_RtspServerHandler::readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(RTSPSERVERIP, tmpAttr, tmpAttrSize))
	{
	    ip = tmpAttr;
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),RTSPSERVERIP, tmpAttr);
	
	}

	if (xmlNode->getAttributeValue(RTSPSERVERPORT, tmpAttr, tmpAttrSize))
	{
		port = tmpAttr;
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),RTSPSERVERPORT, tmpAttr);
		
	}

	if (xmlNode->getAttributeValue(CONNECTTYPE, tmpAttr, tmpAttrSize))
	{
		connectType = tmpAttr;
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),CONNECTTYPE, tmpAttr);
	}
	return true;
}

bool XML_RtspServerHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(RTSPSERVERIP, tmpAttr, tmpAttrSize))
	{
		ip = tmpAttr;
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <RTSPServer> attribute %s=\"%s\""),RTSPSERVERIP, tmpAttr);

	}

	if (xmlNode->getAttributeValue(RTSPSERVERPORT, tmpAttr, tmpAttrSize))
	{
		port = tmpAttr;
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <RTSPServer> attribute %s=\"%s\""),RTSPSERVERPORT, tmpAttr);

	}

	if (xmlNode->getAttributeValue(CONNECTTYPE, tmpAttr, tmpAttrSize))
	{
		connectType = tmpAttr;
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),CONNECTTYPE, tmpAttr);
	}
	return true;
}
