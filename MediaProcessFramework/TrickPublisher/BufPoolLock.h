// BufPoolLock.h: interface for the CBufPoolLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFPOOLLOCK_H__3689D356_EF2F_42B8_812A_B425FB93E569__INCLUDED_)
#define AFX_BUFPOOLLOCK_H__3689D356_EF2F_42B8_812A_B425FB93E569__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBufPoolLock  
{
public:
	CBufPoolLock(CRITICAL_SECTION&cri);
	virtual ~CBufPoolLock();
	CRITICAL_SECTION & m_Cri;
};

#endif // !defined(AFX_BUFPOOLLOCK_H__3689D356_EF2F_42B8_812A_B425FB93E569__INCLUDED_)
