#include "XML_RequestHandler.h"

XML_RequestHandler::XML_RequestHandler()
{

}

XML_RequestHandler::~XML_RequestHandler()
{

}

bool XML_RequestHandler::getContent(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//get node attribute
	::ZQ::common::XMLPreferenceEx *current = xmlNode->firstChild();
	//::ZQ::common::XMLPreferenceEx *current = xmlNode;

	vector<::std::string> RequestVec;
	while (NULL != current)
	{
		const int iBufLen = 8192;
		char tmpBuf[iBufLen];
		if (current->getPreferenceText(tmpBuf, iBufLen))
		{
			RequestVec.push_back(string(tmpBuf));
		}
		current->free();
		current = xmlNode->nextChild();
	}

	for (vector<string>::iterator iter = RequestVec.begin(); iter != RequestVec.end(); iter++)
	{
		strRequest += (*iter);
	}
	return true;
}

bool XML_RequestHandler::getContent(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//get node attribute
	::ZQ::common::XMLPreferenceEx *current = xmlNode->firstChild();

	vector<::std::string> RequestVec;
	while (NULL != current)
	{
		const int iBufLen = 1024;
		char tmpBuf[iBufLen];
		if (current->getPreferenceText(tmpBuf, iBufLen))
		{
			RequestVec.push_back(string(tmpBuf));
		}
		current = xmlNode->nextChild();
	}
	for (vector<string>::iterator iter = RequestVec.begin(); iter != RequestVec.end(); iter++)
	{
		strRequest += (*iter);
	}
	return true;
}

bool XML_RequestHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(REQUESTSKIP, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Request> attribute %s=\"%s\""),REQUESTSKIP, tmpAttr);
		if (tmpAttr[0] == '0')
		{
			bSkip = false;
		}
		else
		{
			bSkip = true;
		}
	}
	return true;
}

bool XML_RequestHandler::readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
	{
		return false;
	}

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(REQUESTSKIP, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessionHandler,"get <Request> attribute %s=\"%s\""),REQUESTSKIP, tmpAttr);
		if (tmpAttr[0] == '0')
		{
			bSkip = false;
		}
		else
		{
			bSkip = true;
		}
	}

	return true;
}

//::std::string XML_RequestHandler::formatRequest()
//{
//	::std::string strRequest;
//	for (vector<string>::iterator iter = RequestVec.begin(); iter != RequestVec.end(); iter++)
//	{
//		strRequest += (*iter);
//	}
//	return strRequest;
//}


//bool XML_RequestHandler::getRequest(ZQ::common::XMLPreferenceEx *xmlNode)
//{
//	if (!readAttribute(xmlNode))
//	{
//		return false;
//	}
//	if (!getContent(xmlNode))
//	{
//		return false;
//	}
//	formatRequest();
//}