// ActiveDataMgr.h: interface for the ActiveDataMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ActiveDataMGR_H__6252A679_DFC4_44BA_9B66_AF87D8155B42__INCLUDED_)
#define AFX_ActiveDataMGR_H__6252A679_DFC4_44BA_9B66_AF87D8155B42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ActiveData.h"
#include "DataAppEx.h"

class ActiveDataMgr  
{
public:
	ActiveDataMgr();
	virtual ~ActiveDataMgr();

	ActiveData* create(
		TianShanIce::Application::DataOnDemand::DataPublishPointPrx& ch);
	
	bool remove(const std::string& name, bool destoryObj = true);

	ActiveData* get(const std::string& name);

protected:
	typedef std::map<std::string, ActiveData* > ActiveDataMap;
	ActiveDataMap	_ActiveDatas;
};

#endif // !defined(AFX_ActiveDataMGR_H__6252A679_DFC4_44BA_9B66_AF87D8155B42__INCLUDED_)
