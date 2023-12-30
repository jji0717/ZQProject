// Util.h: interface for the Util class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTIL_H__5A31B637_E858_4CC0_85E9_47C0C87C2E42__INCLUDED_)
#define AFX_UTIL_H__5A31B637_E858_4CC0_85E9_47C0C87C2E42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataStream.h"
#include <list>
#include <string>
#include <vector>
::Ice::Identity createStreamIdentity(const std::string& space, 
									 const std::string& name);
::Ice::Identity createMuxItemIdentity(const std::string& space, 
									  const std::string& strmName, 
									  const std::string& itemName);

::Ice::ObjectPrx createObjectWithEndPoint(const Ice::CommunicatorPtr& ic, 
										  const Ice::Identity& ident, 
										  const std::string& endPoint);
bool DeleteDirectory(std::string DeleteDir,bool IsDel = false);
bool CopyAllFile(std::string DeleteDir, std::string srcPath);
int  ListFile(const char *argv, std::list<std::string> &File);
bool ListDir(const char *lpSrcFile, std::list<std::string> &Directory);

class NameToStream {
public:
	NameToStream(const Ice::CommunicatorPtr& ic) :
	  _ic(ic)
	{

	}

	DataOnDemand::DataStreamPrx operator()(
		const std::string& endPoint, 
		const std::string& space, 
		const std::string& name)
	{
		Ice::Identity ident = createStreamIdentity(space, name);

		return DataOnDemand::DataStreamPrx::checkedCast(
			createObjectWithEndPoint(_ic, ident, endPoint));
	}

private:

	const Ice::CommunicatorPtr _ic;
};

class NameToMuxItem {
public:
	NameToMuxItem(const Ice::CommunicatorPtr& ic) :
		_ic(ic)
	{

	}

	DataOnDemand::MuxItemPrx operator()(
		const std::string& endPoint, 
		const std::string& space, 
		const std::string& strmName, 
		const std::string& itemName)
	{
		Ice::Identity ident = createMuxItemIdentity(space, 
			strmName, itemName);

		return DataOnDemand::MuxItemPrx::checkedCast(
			createObjectWithEndPoint(_ic, ident, endPoint));
	}

private:

	const Ice::CommunicatorPtr _ic;
};


#endif // !defined(AFX_UTIL_H__5A31B637_E858_4CC0_85E9_47C0C87C2E42__INCLUDED_)
