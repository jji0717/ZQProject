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
		return false;

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(SESSIONSEQID, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONSEQID, tmpAttr);
		_sessionNode.seqId = tmpAttr;
	}

	if (xmlNode->getAttributeValue(SESSIONDESC, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONDESC, tmpAttr);
		_sessionNode.desc = tmpAttr;
	}

	if (xmlNode->getAttributeValue(SESSIONITERATION, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONITERATION, tmpAttr);
		_sessionNode.iteration = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONINTERVAL, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONINTERVAL, tmpAttr);
		_sessionNode.interval = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONLOOP, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONLOOP, tmpAttr);
		_sessionNode.loop = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONTIMEOUT, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONTIMEOUT, tmpAttr);
		_sessionNode.timeout = atoi(tmpAttr);
	}
	return true;
}

bool XML_SessionHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
		return false;

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(SESSIONSEQID, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONSEQID, tmpAttr);
		_sessionNode.seqId = tmpAttr;
	}

	if (xmlNode->getAttributeValue(SESSIONDESC, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONDESC, tmpAttr);
		_sessionNode.desc = tmpAttr;
	}

	if (xmlNode->getAttributeValue(SESSIONITERATION, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONITERATION, tmpAttr);
		_sessionNode.iteration = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONINTERVAL, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONINTERVAL, tmpAttr);
		_sessionNode.interval = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONTIMEOUT, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Session> attribute %s=\"%s\""),SESSIONTIMEOUT, tmpAttr);
		_sessionNode.timeout = atoi(tmpAttr);
	}
	return true;
}