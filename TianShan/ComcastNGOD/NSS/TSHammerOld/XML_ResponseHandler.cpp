#include "XML_ResponseHandler.h"
#include "RTSP_MessageParser.h"
#include <boost/regex.hpp>

XML_ResponseHandler::XML_ResponseHandler()
{

}

XML_ResponseHandler::~XML_ResponseHandler()
{

}

bool XML_ResponseHandler::getResponseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
		return false;

	if (!ResponseVec.empty())
		ResponseVec.clear();

	//get node attribute
	::ZQ::common::XMLPreferenceEx *current = xmlNode->firstChild();

	XML_SessCtxHandler xml_SessCtxHandler;
	xml_SessCtxHandler.setLogger(_log);

	char name[iNameLen];
	ResponseNode tmpNode;

	while (NULL != current)
	{
		if (current->name(name, iNameLen))
		{
			if (string(name).compare("Header") == 0)
			{
				char tmpAttr[tmpAttrSize];
				if (current->getAttributeValue(RESPONSENAME, tmpAttr, tmpAttrSize))
				{
					XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_ResponseHandler,"get <Response/Header> attribute %s=\"%s\""),RESPONSENAME, tmpAttr);
					tmpNode.name = tmpAttr;
				}
				if (current->getAttributeValue(RESPONSESYNTAX, tmpAttr, tmpAttrSize))
				{
					XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_ResponseHandler,"get <Response/Header> attribute %s=\"%s\""),RESPONSESYNTAX, tmpAttr);
					tmpNode.syntax = tmpAttr;
				}
			}

			//parse <SessCtx> to update variables
			::ZQ::common::XMLPreferenceEx *child = current->firstChild("SessCtx");
			if (NULL != child)
			{
				MapValue tmpValue;
				if (xml_SessCtxHandler.parseSessCtx(child, tmpNode.key, tmpValue, strLocalType))
				{
					tmpNode.value = tmpValue.value;
					ResponseVec.push_back(tmpNode);
				}
			}
			child->free();
		}
		
		current->free();
		current = xmlNode->nextChild();
	}

	return true;
}

bool XML_ResponseHandler::getResponseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode)
{
	if (NULL == xmlNode)
		return false;

	//get node attribute
	::ZQ::common::XMLPreferenceEx *current = xmlNode->firstChild();

	XML_SessCtxHandler xml_SessCtxHandler;
	xml_SessCtxHandler.setLogger(_log);

	char name[iNameLen];
	ResponseNode tmpNode;

	while (NULL != current)
	{
		if (current->name(name, iNameLen))
		{
			if (string(name).compare("Header") == 0)
			{
				char tmpAttr[tmpAttrSize];
				if (current->getAttributeValue(RESPONSENAME, tmpAttr, tmpAttrSize))
				{
					XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_ResponseHandler,"get <Response/Header> attribute %s=\"%s\""),RESPONSENAME, tmpAttr);
					tmpNode.name = tmpAttr;
				}
				if (current->getAttributeValue(RESPONSESYNTAX, tmpAttr, tmpAttrSize))
				{
					XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_ResponseHandler,"get <Response/Header> attribute %s=\"%s\""),RESPONSESYNTAX, tmpAttr);
					tmpNode.syntax = tmpAttr;
				}
			}

			//parse <SessCtx> to update variables
			::ZQ::common::XMLPreferenceEx *child = current->firstChild("SessCtx");
			if (NULL != child)
			{
				MapValue tmpValue;
				if (xml_SessCtxHandler.parseSessCtx(child, tmpNode.key, tmpValue, strLocalType))
				{
					tmpNode.value = tmpValue.value;
					ResponseVec.push_back(tmpNode);
				}
			}
			child->free();
		}

		current->free();
		current = xmlNode->nextChild();
	}

	return true;
}

void XML_ResponseHandler::updateSessCtxHandler(XML_SessCtxHandler &xml_SessCtxHandler)
{
	for (ResponseNodeVector::iterator iter = ResponseVec.begin(); iter != ResponseVec.end(); iter++)
	{
		MapValue value;
		value.type = strLocalType;
		value.value = (*iter).value;
		xml_SessCtxHandler._sessCtxMap[(*iter).key] = value;
		xml_SessCtxHandler.updateMacro((*iter).key, (*iter).value);
	}
}

bool XML_ResponseHandler::parseResponse(const ::std::string &strMsg)
{
	boost::regex _regex;
	::std::vector<::std::string> msgLine;

	//split message to line by line
	RTSPMessageParser::splitMsg2Line(strMsg.c_str(), (uint16)strMsg.length(), msgLine);
	::std::string strValue;

	//try to match each regular expression
	for (ResponseNodeVector::iterator iter = ResponseVec.begin(); iter != ResponseVec.end(); iter++)
	{
		::std::string strSyntax;		
		try
		{
			strSyntax = (*iter).name + (*iter).syntax;
			_regex.assign(strSyntax);
			boost::cmatch results;
			for (::std::vector<::std::string>::iterator strIter = msgLine.begin(); strIter != msgLine.end(); strIter++)
			{
				if (boost::regex_match((*strIter).c_str(), results, _regex))
				{
					strValue.assign(results[0].first, results[0].second);
					XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_ResponseHandler,"match regular expression(%s) with value %s"), strSyntax.c_str(), strValue.c_str());

					//get value
					int iLen = (int)(*iter).name.length();
					while (strValue[iLen] == ' ')
						iLen++;
					strValue = strValue.substr(iLen);

					if ((*iter).value.compare("${1}") == 0)
						(*iter).value = strValue;
				}
			}
		}
		catch(boost::bad_expression& ex)
		{
			XMLLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(XML_ResponseHandler, "Syntax [%S] error at %S"),strSyntax.c_str(), ex.what());
			return false;
		}
		catch(...)
		{
			XMLLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(XML_ResponseHandler, "Initialize expression %s catch a exception"),strSyntax.c_str());
			return false;
		}
	}

	return true;
}