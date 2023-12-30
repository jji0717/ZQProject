// SmilParser.h: interface for the SmilParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMILPARSER_H__93C018AF_D039_4E1E_9183_646A7E503AC9__INCLUDED_)
#define AFX_SMILPARSER_H__93C018AF_D039_4E1E_9183_646A7E503AC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(push)
#pragma warning(disable:4146)
#import "msxml.dll" named_guids
using namespace MSXML;
#pragma warning(pop)

namespace SMIL {

class SmilParser  
{
public:
	SmilParser(IXMLDOMDocumentPtr doc);
	virtual ~SmilParser();

	bool createPlaylist(const std::string& fileName, 
		const std::string& srcRoot, 
		const std::vector<std::string>& items);

	IXMLDOMElementPtr insertBefore(int nNum, int id, 
		const _bstr_t& src);

	IXMLDOMElementPtr apppendMedia(int id, const _bstr_t& src);

	int getElementCount();

	int getNextElementsCount(IXMLDOMElementPtr elem);

	bool empty();

	int getElementId(IXMLDOMElementPtr elem);

	bool erase(int id);

	IXMLDOMElementPtr getElementById(int id);
	
	// IXMLDOMElementPtr getFirstMedia();
protected:
	IXMLDOMElementPtr createMedia(const _bstr_t& src, int id);

protected:
	IXMLDOMDocumentPtr _doc;
};

} // namespace SMIL {

#endif // !defined(AFX_SMILPARSER_H__93C018AF_D039_4E1E_9183_646A7E503AC9__INCLUDED_)
