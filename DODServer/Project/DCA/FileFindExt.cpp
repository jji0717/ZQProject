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
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess::GetLastError() = %d ,ErrorDescription =  %s "),nError,strError);	
		return FALSE;
	}

	Dstroot+="\\"; 
	CString DstDIRTemp=Dstroot;
	
	//Clog( LOG_DEBUG,_T("CFileFindExt::DstDIRTemp =%s"),DstDIRTemp);

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

			//Clog( LOG_DEBUG,_T("CFileFindExt::strPath =%s"),strPath);

			BOOL NotEndofCurDir = TRUE;
			if(FindFile(strPath) == FALSE)
			{
				int nError=GetLastError();
			    char strError[500];

				GetErrorDescription(nError, strError);
				Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess::GetLastError() = %d ,ErrorDescription =  %s "),nError,strError);
				return FALSE;
			}
			
		//	Clog( LOG_DEBUG,_T("CFileFindExt::NotEndofCurDir = FindFile(strPath) =%d"),NotEndofCurDir);

			while (NotEndofCurDir)
			{
				NotEndofCurDir = FindNextFile();

				if (IsDots())
					continue;
				//Clog( LOG_DEBUG,_T("CFileFindExt::DoProcess:Is not Dots"));

				if (IsDirectory())
				{
					strPath = GetFilePath();
				//	Clog( LOG_DEBUG,_T("CFileFindExt::IsDirectory strPath=%s "),strPath);

					int len=strPath.GetLength();
					strPath = strPath.Right(len-rootlen);

					CString TempStrDir;
					TempStrDir=DstDIRTemp+strPath;//				stsdr="\n"+strPath;				TRACE(stsdr);

					CreateDirectory(TempStrDir,NULL);

					len-=rootlen;
					if ((tail+1)%MAXQUEUELEN==head)
					{
						for (int i=head; head!=tail; )
						{
							str=Queue[head];
							if(strcmp(str,"")!=0)
								delete []str;
							head=(head+1)%MAXQUEUELEN;
						}
						return false;
					}
					str=new char[len+1];
					strcpy(str,(LPCTSTR)strPath);				
					Queue[tail]=str;

				//	Clog( LOG_DEBUG,_T("CFileFindExt::CreateDirectory tail=%d,StrDir=%s "),tail,strPath);
					tail=(tail+1)%MAXQUEUELEN;
					if (!NotEndofCurDir)
						break;

				}
				else
				{		
					CString FileName=GetFilePath(); 
					CString DstFileDir=GetRoot();

				//	Clog( LOG_DEBUG,_T("CFileFindExt::NON_Directory FileName=%s ,root=%s"),FileName,DstFileDir);

					int len=DstFileDir.GetLength();
					DstFileDir = DstFileDir.Right(len-rootlen);
					if (DstFileDir.Right(1)=="\\") 
						DstFileDir=DstFileDir.Left(DstFileDir.GetLength()-1);
					if (DstFileDir!="")
						DstFileDir=Dstroot+DstFileDir+"\\";	
					else
						DstFileDir=Dstroot+DstFileDir;	

					CString FileTitle=GetFileName();
					CString DstFile=DstFileDir+FileTitle;
				
					if (!CopyFile(FileName,DstFile,FALSE))
					{
						int nError = GetLastError();
						char strError[500];

						GetErrorDescription(nError, strError);
						Clog( LOG_DEBUG,_T("CopyFile(SrcFileName(%s),DstFileName(%s)); GetLastError() = %d, ErrorDescription = %s"),FileName,DstFile,nError,strError);						
					}
				}
			} //End of while (NotEndofCurDir)

			//Clog( LOG_DEBUG,_T("CFileFindExt::process End of while (NotEndofCurDir)"));

			try
			{	
				str=Queue[head];
				//Clog( LOG_DEBUG,_T("CFileFindExt::delete head=%d,StrDir=%s "),head,str);
				if(strcmp(str,"")!=0)
					delete []str;
				head=(head+1)%MAXQUEUELEN;	
			}
			catch (...) 
			{	
				int nError = 0;
				Clog( LOG_DEBUG,_T("CFileFindExt::Do Processing delete [] str::GetLastError=%d error"),nError);
				return FALSE;
			}
		} while (head!=tail);
	}
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog( LOG_DEBUG,_T("CFileFindExt::Do Processing error::GetLastError() = %d ,ErrorDescription = %s"),nError,strError);
		return FALSE;
	}
	Close();

	//Clog( LOG_DEBUG,_T("CFileFindExt::process End"));

	return true;	
}


