#include "stdafx.h"
#include "FileFindExt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CFileFindExt::CFileFindExt()
{
	m_FileNames.SetSize(MAX_PATH,MAX_PATH/2);
	m_FileNames.RemoveAll();
}

CFileFindExt::~CFileFindExt()
{
	Close();
	m_FileNames.RemoveAll();	
}

void CFileFindExt::SetSize(int nNewSize, int nGrowBy)
{
	m_FileNames.SetSize(nNewSize, nGrowBy);
}

int CFileFindExt::GetSize() const
{
	return m_FileNames.GetSize();
}

CString CFileFindExt::GetAt(int nFileIndex) const
{
	return m_FileNames.GetAt(nFileIndex);
}


BOOL CFileFindExt::SearchFiles(CString DirName, CString FileExtName)
{
	const int MAXQUEUELEN=MAX_PATH<<4; //Maxsubdirnumber in same direcrory
	char* Queue[MAXQUEUELEN];
	
	CString root;
	int rootlen=DirName.GetLength();
	// All Dir deinclude "
	if(DirName.Right(1) != "\\")
		root=DirName; 
	else
		root=DirName.Left(--rootlen);
	rootlen++;

	int head=0;
	int tail=0;
	Queue[tail++]=""; 
	char* str;

	CString strPath;		
	do{
		strPath=Queue[head];
		if (strPath!="")
			strPath=root+"\\"+strPath+_T("\\*.*");
		else
			strPath=root+strPath+_T("\\*.*");
		BOOL NotEndofCurDir = FindFile(strPath);
		while (NotEndofCurDir)
		{
			NotEndofCurDir = FindNextFile();
			
			if (IsDots())
				continue;
			
			//Is subdirectory
			if (IsDirectory())
			{
				// save them to relatively direcorry 
				strPath = GetFilePath();
				int len=strPath.GetLength();
				strPath = strPath.Right(len-rootlen);
				len-=rootlen;
				if ((tail+1)%MAXQUEUELEN==head)
				{
					// the queue is fulled 
				//	AfxMessageBox("The queue is fulled!");
					//Close and delete all memory of the queue
					for (int i=head; head!=tail; )
					{
						str=Queue[head];
						if (str!="")
							delete [] str;
						head=(head+1)%MAXQUEUELEN;
					}
					return false;
				}
				str=new char[len+1];
				strcpy(str,(LPCTSTR)strPath);
				Queue[tail]=str;
				tail=(tail+1)%MAXQUEUELEN;
				if (!NotEndofCurDir)
					break;
			}
			else
			{
				//Deal with current file
				CString FileName=GetFilePath(); 
				FileName = FileName.Right(FileName.GetLength()-rootlen);
				if (FileExtName.Compare("*.*"))
				{
					CString ext=FileName.Right(3);
					if (!ext.CompareNoCase(FileExtName.Right(3)))
						m_FileNames.Add(FileName);
				}
				else
					m_FileNames.Add(FileName);
			}
		} //End of while (NotEndofCurDir)
		str=Queue[head];
		if (str!="")
			delete [] str;
		head=(head+1)%MAXQUEUELEN;	
	} while (head!=tail);
	Close();
	//m_iFiles=m_FileNames.GetSize();
	return true;
}

BOOL CFileFindExt::DoProcess(CString DirName, PROFUN1 process, CString FileExtName)
{
	const int MAXQUEUELEN=MAX_PATH<<4; 
	//MaxSubdirectory in the same DIR
	char* Queue[MAXQUEUELEN];
	
	CString root;
	int rootlen=DirName.GetLength();
	//all root_path exclude "\\"
	if(DirName.Right(1) != "\\")
		root=DirName; 
	else
		root=DirName.Left(--rootlen);
	rootlen++;
	
	int head=0;
	int tail=0;
	Queue[tail++]=""; 
	char* str;
	
	CString strPath;		
	do{
		strPath=Queue[head];
		if (strPath!="")
			strPath=root+"\\"+strPath+_T("\\*.*");
		else
			strPath=root+strPath+_T("\\*.*");
		BOOL NotEndofCurDir = FindFile(strPath);
		while (NotEndofCurDir)
		{
			NotEndofCurDir = FindNextFile();
			
			if (IsDots())
				continue;
			
			if (IsDirectory())
			{
				strPath = GetFilePath();
				int len=strPath.GetLength();
				strPath = strPath.Right(len-rootlen);
				len-=rootlen;
				if ((tail+1)%MAXQUEUELEN==head)
				{
				//	AfxMessageBox("There are overfull subdirectory in current DIR!");
					for (int i=head; head!=tail; )
					{
						str=Queue[head];
						if (str!="")
							delete [] str;
						head=(head+1)%MAXQUEUELEN;
					}
					return false;
				}
				str=new char[len+1];
				strcpy(str,(LPCTSTR)strPath);				
				Queue[tail]=str;
				tail=(tail+1)%MAXQUEUELEN;
				if (!NotEndofCurDir)
					break;
			}
			else
			{
				CString FileName=GetFilePath(); 
				if (FileExtName.Compare("*.*"))
				{
					CString ext=FileName.Right(3);
					if (!ext.CompareNoCase(FileExtName.Right(3)))
					{
						if (!process(FileName))
							return false;
					}
				}
				else
				{
					if (!process(FileName))
						return false;
				}
			}
		} //End of while (NotEndofCurDir)
		str=Queue[head];
		if (str!="")
			delete [] str;
		head=(head+1)%MAXQUEUELEN;	
	} while (head!=tail);
	Close();
	return true;
}

BOOL CFileFindExt::DoProcess(CString DirName, PROFUN2 process, CString DstDir,
							 CString SrcFileExt, CString DstFileExt)
{
	const int MAXQUEUELEN=MAX_PATH<<4; 
	char* Queue[MAXQUEUELEN];

	CString root;
	int rootlen=DirName.GetLength();
	if(DirName.Right(1) != "\\")
		root=DirName; 
	else
		root=DirName.Left(--rootlen);
	rootlen++;

	CString Dstroot;
	int loc=DstDir.Find(':');
	if (loc==1) //DstDir is absolutely path
		Dstroot=DstDir;
	else
	{   
		loc=root.ReverseFind('\\');
		Dstroot=root.Left(loc)+"\\"+DstDir;
		if (Dstroot.Right(1)=="\\")
			Dstroot=Dstroot.Left(Dstroot.GetLength()-1);
	}
/*
	BOOL	bReturn =CreateDirectory(Dstroot,NULL);
	if (bReturn==0)
	{
		return FALSE;
	}
*/
	Dstroot+="\\"; 
	CString DstDIRTemp=Dstroot;
	
	
	int head=0;
	int tail=0;
	Queue[tail++]=""; 
	char* str;
	
	CString strPath;		
	do{
		strPath=Queue[head];
		if (strPath!="")
			strPath=root+"\\"+strPath+_T("\\*.*");
		else
			strPath=root+strPath+_T("\\*.*");
		BOOL NotEndofCurDir = FindFile(strPath);
		while (NotEndofCurDir)
		{
			NotEndofCurDir = FindNextFile();
			
			if (IsDots())
				continue;
			
			if (IsDirectory())
			{
				strPath = GetFilePath();
				int len=strPath.GetLength();
				strPath = strPath.Right(len-rootlen);

				CString stsdr111;
				stsdr111=DstDIRTemp+strPath;//				stsdr="\n"+strPath;				TRACE(stsdr);
				CreateDirectory(stsdr111,NULL);

				len-=rootlen;
				if ((tail+1)%MAXQUEUELEN==head)
				{
			///		AfxMessageBox("11!");
					for (int i=head; head!=tail; )
					{
						str=Queue[head];
						if (str!="")
							delete [] str;
						head=(head+1)%MAXQUEUELEN;
					}
					return false;
				}
				str=new char[len+1];
				strcpy(str,(LPCTSTR)strPath);				
				Queue[tail]=str;
				tail=(tail+1)%MAXQUEUELEN;
				if (!NotEndofCurDir)
					break;
			}
			else
			{		
				CString FileName=GetFilePath(); 
				CString DstFileDir=GetRoot();
				
				int len=DstFileDir.GetLength();
				DstFileDir = DstFileDir.Right(len-rootlen);
				if (DstFileDir.Right(1)=="\\") 
					DstFileDir=DstFileDir.Left(DstFileDir.GetLength()-1);
				if (DstFileDir!="")
					DstFileDir=Dstroot+DstFileDir+"\\";	
				else
					DstFileDir=Dstroot+DstFileDir;	
					
				CString FileTitle=GetFileName();
		//		CString DstFile=DstFileDir+FileTitle+DstFileExt.Right(4);
				CString DstFile=DstFileDir+FileTitle;
				if (SrcFileExt.Compare("*.*"))
				{
					CString ext=FileName.Right(3);
					if (!ext.CompareNoCase(SrcFileExt.Right(3)))
					{
						CreateDirectory(DstFileDir,NULL);
						if (!process(FileName,DstFile))
							return false;
					}
				}
				else 
				{

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê8ÔÂ19ÈÕ 15:14:06
				//	CString tmepstr=DstFileDir;
		//			BOOL	bReturn =CreateDirectory(DstFileDir,NULL);
					if (!process(FileName,DstFile))
						return false;
				}
			}
		} //End of while (NotEndofCurDir)
		str=Queue[head];
		if (str!="")
			delete [] str;
		head=(head+1)%MAXQUEUELEN;	
	} while (head!=tail);
	Close();
	return true;	
}


