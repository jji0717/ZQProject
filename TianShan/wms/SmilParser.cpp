// SmilParser.cpp: implementation of the SmilParser class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SmilParser.h"

using namespace ZQ::common;

namespace SMIL {

_bstr_t MediaItemId(_T("media"));
_bstr_t MediaSrc(_T("src"));
_bstr_t MediaId(_T("id"));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SmilParser::SmilParser(IXMLDOMDocumentPtr doc)
{
	_doc = doc;
}

SmilParser::~SmilParser()
{

}

bool SmilParser::createPlaylist(const std::string& fileName, 
								const std::string& srcRoot, 
								const std::vector<std::string>& items)
{
	IXMLDOMNodePtr wsxNode = _doc->createNode(
		_variant_t((long )MSXML::NODE_PROCESSING_INSTRUCTION), 
		_bstr_t(_T("wsx")), _bstr_t());

	_doc->appendChild(wsxNode);
	wsxNode->text = "version = \'1.0\'";
	IXMLDOMElementPtr smilElem = _doc->createElement(_bstr_t(_T("smil")));
	_doc->appendChild(smilElem);
	IXMLDOMElementPtr mediaElem;
	long len = items.size();
	TCHAR src[MAX_PATH];
	for (long i = 0; i < len; i ++) {
		mediaElem = _doc->createElement(_bstr_t(_T("media")));
		wsprintf(src, _T("%s\\%s"), srcRoot.c_str(), items[i].c_str());
		mediaElem->setAttribute(_bstr_t(_T("src")), src);
		smilElem->appendChild(mediaElem);
	}

	_doc->save(_bstr_t(fileName.c_str()));
	return true;
}

IXMLDOMElementPtr SmilParser::createMedia(const _bstr_t& src, int id)
{
	IXMLDOMElementPtr newElem = _doc->createElement(
		SMIL::MediaItemId);
	newElem->setAttribute(SMIL::MediaSrc, src);
	TCHAR buf[128];
	wsprintf(buf, _T("%d"), id);
	newElem->setAttribute(SMIL::MediaId, _variant_t(buf));
	return newElem;
}

IXMLDOMElementPtr SmilParser::getElementById(int id)
{
	IXMLDOMElementPtr elem;
	elem = _doc->firstChild->firstChild;
	while(true) {
		if (getElementId(elem) == id) {
			return elem;
		}

		elem = elem->nextSibling;
	}
}

IXMLDOMElementPtr SmilParser::insertBefore(int whereId, int id, 
										   const _bstr_t& src)
{
	IXMLDOMElementPtr elem;
	IXMLDOMElementPtr newElem = createMedia(src, id);
	elem = getElementById(whereId);
	elem->parentNode->insertBefore(newElem, _variant_t(elem));
	return elem;
}

IXMLDOMElementPtr SmilParser::apppendMedia(int id, const _bstr_t& src)
{
	IXMLDOMElementPtr root;
	IXMLDOMElementPtr newElem = createMedia(src, id);
	
	root = _doc->firstChild;
	return root->appendChild(newElem);
}

int SmilParser::getElementCount()
{
	IXMLDOMNodePtr procInst = _doc->firstChild;
	IXMLDOMElementPtr smil = procInst->nextSibling;
	printf("%s", (LPCTSTR )smil->text);
	return smil->childNodes->length;
}

int SmilParser::getNextElementsCount(IXMLDOMElementPtr elem)
{
	IXMLDOMElementPtr next = elem->nextSibling;
	int count;
	while (next) {
		count ++;
		next = next->nextSibling;
	}

	return count;
}

bool SmilParser::empty()
{
	IXMLDOMElementPtr root = _doc->firstChild;
	return root->childNodes->length <= 0;
}

int SmilParser::getElementId(IXMLDOMElementPtr elem)
{
	int id;
	_variant_t attr = elem->getAttribute(MediaId);
	id = _ttoi((LPCTSTR )attr.bstrVal);
	return id;
}

bool SmilParser::erase(int id)
{
	IXMLDOMElementPtr elem = getElementById(id);
	return elem->parentNode->removeChild(elem);
}

} // namespace SMIL {
