#include "XML_SleepHandler.h"

XML_SleepHandler::XML_SleepHandler()
{
	
}

XML_SleepHandler::~XML_SleepHandler()
{

}

bool XML_SleepHandler::readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
		return false;

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(SLEEPWAIT, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SleepHandler,"get <Sleep> attribute %s=\"%s\""),SLEEPWAIT, tmpAttr);
		_sleepNode.wait = atoi(tmpAttr);
	}

	return true;
}

bool XML_SleepHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
		return false;

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(SLEEPWAIT, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SleepHandler,"get <Sleep> attribute %s=\"%s\""),SLEEPWAIT, tmpAttr);
		_sleepNode.wait = atoi(tmpAttr);
	}

	return true;
}