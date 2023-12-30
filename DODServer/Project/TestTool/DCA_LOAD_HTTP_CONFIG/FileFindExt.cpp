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

BOOL CFileFindExt::DoProcess(CString DirName, PROFUN1 process, CString FileExtName)
{
	return true;
}

BOOL CFileFindExt::DoProcess(CString DirName, PROFUN2 process, CString DstDir,
							 CString SrcFileExt, CString DstFileExt)
{
	CString Dstroot;
	const int MAXQUEUELEN=MAX_PATH<<4; 
	char* Queue[MAXQUEUELEN];
	CString root;
	int rootlen=DirName.GetLength();

	try{	

		if(DirName.Right(1) != "\\")
			root=DirName; 
		else
			root=DirName.Left(--rootlen);
		rootlen++;

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
	}
	catch (...) 
	{
		int nError=GetLastError();
		Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess::GetLastError=%d error"),nError);
		return FALSE;
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
	try
	{
		do{
			strPath=Queue[head];
			if (strPath!="")
				strPath=root+"\\"+strPath+_T("\\*.*");
			else
				strPath=root+strPath+_T("\\*.*");
			BOOL NotEndofCurDir = FindFile(strPath);
	//		Clog( LOG_DEBUG,_T("CFileFindExt::NotEndofCurDir = FindFile(strPath):"));

			while (NotEndofCurDir)
			{
				NotEndofCurDir = FindNextFile();

//				Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess:IsDot before "));

				if (IsDots())
					continue;
//				Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess:IsDots"));

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
							//if (str!="")
							if(strcmp(str,"")!=0)
								delete []str;
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

				//	Clog( LOG_DEBUG,_T("CFileFindExt::if (SrcFileExt.Compare(*.*)) before:"));

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
					//	Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess:process(FileName,DstFile"));
						if (!process(FileName,DstFile))
							return false;
					}
				}
			} //End of while (NotEndofCurDir)
			try
			{	
				str=Queue[head];
				//if (str!="")
				if(strcmp(str,"")!=0)
					delete []str;
				head=(head+1)%MAXQUEUELEN;	
			}
			catch (...) 
			{	int nError=0;
				Clog( LOG_DEBUG,_T("CFileFindExt::Do Processing delete [] str::GetLastError=%d error"),nError);
				return FALSE;
			}
		} while (head!=tail);
	}
	catch (...) 
	{
		int nError=GetLastError();
		Clog( LOG_DEBUG,_T("CFileFindExt::Do Processing ::GetLastError=%d error"),nError);
		return FALSE;
	}
	Close();
	return true;	
}


