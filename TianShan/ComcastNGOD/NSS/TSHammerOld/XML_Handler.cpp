#include "XML_Handler.h"

XML_Handler::XML_Handler()
{

}

XML_Handler::~XML_Handler()
{

}

bool XML_Handler::getAttributeValue(::ZQ::common::XMLUtil::XmlNode node, const void *attrname, void *value, int maxvaluesize,  int charsize)
{
	if (NULL == node)
		return false;
	else
		return node->getAttributeValue(attrname, value, maxvaluesize, charsize);
}

bool XML_Handler::getAttributeValue(::ZQ::common::XMLPreferenceEx *node, const void *attrname, void *value, int maxvaluesize,  int charsize)
{
	if (NULL == node)
		return false;
	else
		return node->getAttributeValue(attrname, value, maxvaluesize, charsize);
}