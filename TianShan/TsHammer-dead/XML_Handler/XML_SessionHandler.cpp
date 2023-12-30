#include "XML_SessionHandler.h"

XML_SessionHandler::XML_SessionHandler()
{
	
}

XML_SessionHandler::~XML_SessionHandler()
{

}

bool XML_SessionHandler::readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(SESSIONITERATION, tmpAttr, tmpAttrSize))
	{
		iteration = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONITERATION, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONINTERVAL, tmpAttr, tmpAttrSize))
	{
		interval = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONINTERVAL, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONLOOP, tmpAttr, tmpAttrSize))
	{
		loop = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONLOOP, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONTIMEOUT, tmpAttr, tmpAttrSize))
	{
		timeout = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONTIMEOUT, tmpAttr);
	}


	
	return true;
}

bool XML_SessionHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(SESSIONITERATION, tmpAttr, tmpAttrSize))
	{
		iteration = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONITERATION, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONINTERVAL, tmpAttr, tmpAttrSize))
	{
		interval = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONINTERVAL, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONLOOP, tmpAttr, tmpAttrSize))
	{
		loop = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONLOOP, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONTIMEOUT, tmpAttr, tmpAttrSize))
	{
		timeout = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONTIMEOUT, tmpAttr);
	}
	return true;
}