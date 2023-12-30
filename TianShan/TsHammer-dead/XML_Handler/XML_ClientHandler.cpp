#include "XML_ClientHandler.h"

XML_CLIENTHandler::XML_CLIENTHandler()
{
	
}

XML_CLIENTHandler::~XML_CLIENTHandler()
{

}

bool XML_CLIENTHandler::readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(RECEIVETHREADS, tmpAttr, tmpAttrSize))
	{
		receiveThreads = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), RECEIVETHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(PROCESSTHREADS, tmpAttr, tmpAttrSize))
	{
		processThreads = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), PROCESSTHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONHREADS, tmpAttr, tmpAttrSize))
	{
		sessionThreads = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), SESSIONHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(TAILORTYPECFG, tmpAttr, tmpAttrSize))
	{
		strTailorType = tmpAttr;
		//XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), SESSIONHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(TAILORRANGE, tmpAttr, tmpAttrSize))
	{
		tailorRange = atoi(tmpAttr);
		//XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), SESSIONHREADS, tmpAttr);
	}

	return true;
}

bool XML_CLIENTHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(RECEIVETHREADS, tmpAttr, tmpAttrSize))
	{
		receiveThreads = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), RECEIVETHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(PROCESSTHREADS, tmpAttr, tmpAttrSize))
	{
		processThreads = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), PROCESSTHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(SESSIONHREADS, tmpAttr, tmpAttrSize))
	{
		sessionThreads = atoi(tmpAttr);
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), SESSIONHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(TAILORTYPECFG, tmpAttr, tmpAttrSize))
	{
		strTailorType = tmpAttr;
		//XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), SESSIONHREADS, tmpAttr);
	}

	if (xmlNode->getAttributeValue(TAILORRANGE, tmpAttr, tmpAttrSize))
	{
		tailorRange = atoi(tmpAttr);
		//XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_CLIENTHandler,"get <Client> attribute %s=\"%s\""), SESSIONHREADS, tmpAttr);
	}

	return true;
}
