// ActiveData.h: interface for the ActiveData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ActiveData_H__C9CF1B97_EE1B_4688_816D_F08153879566__INCLUDED_)
#define AFX_ActiveData_H__C9CF1B97_EE1B_4688_816D_F08153879566__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ActiveData  
{
public:
	ActiveData();
	virtual ~ActiveData();

	void GetCurrentDatatime(std::string& strTime);

	virtual bool initchannel();
	virtual void uninit();

};

#endif // !defined(AFX_ActiveData_H__C9CF1B97_EE1B_4688_816D_F08153879566__INCLUDED_)
