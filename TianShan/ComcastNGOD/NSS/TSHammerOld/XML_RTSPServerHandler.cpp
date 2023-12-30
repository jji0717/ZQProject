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
		return false;

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(RTSPSERVERIP, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),RTSPSERVERIP, tmpAttr);
		_rtspServerNode.ip = tmpAttr;
	}

	if (xmlNode->getAttributeValue(RTSPSERVERPORT, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),RTSPSERVERPORT, tmpAttr);
		_rtspServerNode.port = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(CONNECTTYPE, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),CONNECTTYPE, tmpAttr);
		_rtspServerNode.type = tmpAttr;
	}

	if (xmlNode->getAttributeValue(BUFFERSIZE, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),BUFFERSIZE, tmpAttr);
		_rtspServerNode.bufferSize = atoi(tmpAttr);
	}
	return true;
}

bool XML_RtspServerHandler::readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode)
{
	if (NULL == xmlNode)
		return false;

	//read attribute from xml node
	char tmpAttr[tmpAttrSize];
	if (xmlNode->getAttributeValue(RTSPSERVERIP, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <RTSPServer> attribute %s=\"%s\""),RTSPSERVERIP, tmpAttr);
		_rtspServerNode.ip = tmpAttr;
	}

	if (xmlNode->getAttributeValue(RTSPSERVERPORT, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <RTSPServer> attribute %s=\"%s\""),RTSPSERVERPORT, tmpAttr);
		_rtspServerNode.port = atoi(tmpAttr);
	}

	if (xmlNode->getAttributeValue(CONNECTTYPE, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),CONNECTTYPE, tmpAttr);
		_rtspServerNode.type = tmpAttr;
	}

	if (xmlNode->getAttributeValue(BUFFERSIZE, tmpAttr, tmpAttrSize))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_RtspServerHandler,"get <Server> attribute %s=\"%s\""),BUFFERSIZE, tmpAttr);
		_rtspServerNode.bufferSize = atoi(tmpAttr);
	}
	return true;
}

bool XML_RtspServerHandler::initSessionSocket()
{
	_sessionSocket.m_Socket = CreateSocket(TCPSOCKET);
	if (_sessionSocket.m_Socket < 0)
		return false;
	else
		return true;
}

bool XML_RtspServerHandler::connectServer()
{
	//try to connect server
	_sessionSocket.m_Status = bConnection(_rtspServerNode.ip, _rtspServerNode.port, _sessionSocket.m_Socket, 5);
	return _sessionSocket.m_Status;
}