//////////////////////////////////////////////////////////////////////
#if !defined(AFX_FILEFINDEXT_H)
#define AFX_FILEFINDEXT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/*

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê7ÔÂ19ÈÕ 15:23:23
This article was found out in www.csdn.com.

major fouctions which are test successful are some ones.

	DoProcess()		
		{	delDIR();
			delFile();
		}
*/


//typedef BOOL (* PROFUN )(CString SrcFile, CString DstFile="");

typedef BOOL (* PROFUN1 )(CString SrcFile);

typedef BOOL (* PROFUN2 )(CString SrcFile, CString DstFile);


class CFileFindExt : public CFileFind  //inherit CFileFind class
{
protected:

	CStringArray m_FileNames;
	
public:

	CFileFindExt();
	virtual ~CFileFindExt();

	void SetSize(int nNewSize, int nGrowBy = -1);

	int GetSize() const;
	
	CString GetAt(int nFileIndex) const;

	BOOL DoProcess(CString DirName, PROFUN1 process, CString FileExtName="*.*");

	BOOL DoProcess(CString DirName, PROFUN2 process, CString DstDir,
		CString SrcFileExt="*.*", CString DstFileExt="*.*");
	
};

#endif // !defined(AFX_FILEFINDEXT_H_)





















