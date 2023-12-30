#include "afx.h"
#include "xmlproc.h"

bool XMLProc::GetParameters(char*		xml,
							int&		id,
							int&		type,
							wchar_t*	path)
{

	ZQ::common::XMLPrefDoc doc(*init);
	ZQ::common::PrefGuard  pRoot, pGuard;

	char nodeName[20];
	memset(nodeName, 0x00, 20*sizeof(char));
	
	char temp[BUFSIZ];
	memset(temp, 0x00, BUFSIZ*sizeof(char));

	bool bSuccess = true;

	try
	{
		if (!doc.read(xml, -1))
		{
			bSuccess = false;
		}
	}
	catch(...)
	{
		bSuccess = false;
	}

	if (bSuccess)
	{
		pRoot.pref(doc.root());
		if (!pRoot.valid())
		{
			doc.close();
			return false;
		}
		else
		{
			pRoot.pref()->name(nodeName);
			if(strcmp(nodeName, "Msg") != 0)
			{
				doc.close();
				return false;
			}
		}

		pGuard.pref(pRoot.pref()->firstChild());
		while (1)
		{
			if (!pGuard.valid())
			{
				doc.close();
				return true;
			}
			memset(nodeName, 0x00, 20*sizeof(char));
			pGuard.pref()->name(nodeName);
			if (strcmp(nodeName, "SyncCommand") == 0)
			{
				pGuard.pref()->get("id", temp);
				id = atoi(temp);
				
				memset(temp, 0x00, BUFSIZ*sizeof(char));
				pGuard.pref()->get("type", temp);
				type = atoi(temp);

				memset(temp, 0x00, BUFSIZ*sizeof(char));
				pGuard.pref()->getUnicode(L"path", path);
				WideCharToMultiByte(CP_ACP, 0, path, -1, temp, MAX_PATH, NULL, NULL);
				MultiByteToWideChar(CP_UTF8, 0, temp, -1, path, MAX_PATH);
			}

			pGuard.pref(pRoot.pref()->nextChild());
		}
	}

	doc.close();

	return bSuccess;
}

void XMLProc::GetBackupFile(char*		 xmlfile,
						    int*         id,
						    int*         type,
						    wchar_t**    path,
						    int*         count)
{	
	ZQ::common::XMLPrefDoc doc(*init);
	ZQ::common::PrefGuard  pRoot, pGuard;

	char nodeName[20];
	memset(nodeName, 0x00, 20*sizeof(char));
	
	char temp[50];
	memset(temp, 0x00, 50*sizeof(char));

	bool bSuccess = true;

	*count = 0;

	try
	{
		if (!doc.open(xmlfile, -1))
		{
			bSuccess = false;
		}
	}
	catch(...)
	{
		bSuccess = false;
	}

	if (bSuccess)
	{
		pRoot.pref(doc.root());
		if (!pRoot.valid())
		{
			doc.close();
			return;
		}
		else
		{
			pRoot.pref()->name(nodeName);
			if(strcmp(nodeName, "BACKUP") != 0)
			{
				doc.close();
				return;
			}
		}

		pGuard.pref(pRoot.pref()->firstChild());
		while (1)
		{
			if (!pGuard.valid())
			{
				doc.close();
				return;
			}
			memset(nodeName, 0x00, 20*sizeof(char));
			pGuard.pref()->name(nodeName);
			if (strcmp(nodeName, "INFO") == 0)
			{
				pGuard.pref()->get("id", temp);
				id[(*count)] = atoi(temp);
				
				memset(temp, 0x00, 50*sizeof(char));
				pGuard.pref()->get("type", temp);
				type[(*count)] = atoi(temp);

				memset(temp, 0x00, 50*sizeof(char));
				pGuard.pref()->getUnicode(L"path", path[(*count)]);
				WideCharToMultiByte(CP_ACP, 0, path[(*count)], -1, temp, MAX_PATH, NULL, NULL);
				MultiByteToWideChar(CP_UTF8, 0, temp, -1, path[(*count)], MAX_PATH);
				
				(*count) ++;
			}

			pGuard.pref(pRoot.pref()->nextChild());
		}
	}

	doc.close();
}

void XMLProc::DeleteNode(char*        xmlfile,
					     int          id)
{
	ZQ::common::XMLPrefDoc doc(*init);
	ZQ::common::PrefGuard  pRoot, pGuard;

	char nodeName[20];
	memset(nodeName, 0x00, 20*sizeof(char));
	
	char temp[50];
	memset(temp, 0x00, 50*sizeof(char));

	bool bSuccess = true;

	try
	{
		if (!doc.open(xmlfile, -1))
		{
			bSuccess = false;
		}
	}
	catch(...)
	{
		bSuccess = false;
	}

	if (bSuccess)
	{
		pRoot.pref(doc.root());
		if (!pRoot.valid())
		{
			doc.close();
			return;
		}
		else
		{
			pRoot.pref()->name(nodeName);
			if(strcmp(nodeName, "BACKUP") != 0)
			{
				doc.close();
				return;
			}
		}

		pGuard.pref(pRoot.pref()->firstChild());
		while (1)
		{
			if (!pGuard.valid())
			{
				doc.close();
				return;
			}
			memset(nodeName, 0x00, 20*sizeof(char));
			pGuard.pref()->name(nodeName);
			if (strcmp(nodeName, "INFO") == 0)
			{
				pGuard.pref()->get("id", temp);
				int tId = atoi(temp);
				
				if (tId == id)
				{
					pRoot.pref()->removeChild(pGuard.pref());
					doc.save();
					doc.close();
					return;
				}
			}
			pGuard.pref(pRoot.pref()->nextChild());
		}
	}
	doc.close();
}

void XMLProc::AddNode(char*        xmlfile,
					  int          id,
					  int          type,
					  wchar_t*     path)
{
	ZQ::common::XMLPrefDoc doc(*init);
	ZQ::common::PrefGuard  pRoot, pGuard;

	char nodeName[100];
	memset(nodeName, 0x00, 100*sizeof(char));

	bool bSuccess = true;

	try
	{
		if (!doc.open(xmlfile, -1))
		{
			bSuccess = false;
		}
	}
	catch(...)
	{
		bSuccess = false;
	}
	if (bSuccess)
	{
		pRoot.pref(doc.root());
		if (!pRoot.valid())
		{
			doc.close();
			return;
		}
		else
		{
			pRoot.pref()->name(nodeName);
			if(strcmp(nodeName, "BACKUP") != 0)
			{
				doc.close();
				return;
			}
		}

		ZQ::common::IPreference *pNew = NULL;
		
		pNew = doc.newElement("INFO");

		char temp[100];
		strnset(temp, 0, 100);
		sprintf(temp, "%d", id);
		pNew->set("id",   temp);
		
		strnset(temp, 0, 100);
		sprintf(temp, "%d", type);
		pNew->set("type", temp);
		
		strnset(temp, 0 ,100);
		int len = wcslen(path) * 2 + 1;
		WideCharToMultiByte(CP_ACP, 0, path, -1, temp, len, NULL, NULL);	
		pNew->set("path", temp);

		pRoot.pref()->addNextChild(pNew);

		doc.save();

		delete pNew;
	}
	doc.close();
}