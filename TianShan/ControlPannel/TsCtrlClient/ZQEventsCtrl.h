#ifndef __ZQEVENTSCTRL_H
#define __ZQEVENTSCTRL_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>
#include <Windows.h>

#define  PROGRESS_INTERTIME  200
#define  INITICESTORM  1
#define  PTRMODE       1
#define  ITEMLEN      256 
#define  NAEMELEN      64
//#define  DEMOVER       1

#include <string>
using std::string;
#pragma warning(disable:4518  4786 4502)

typedef struct _tagEventAttribsData
{
	string  strIceStormEndPoint;
}EVENTATTRISDATA,*PEVENTATTRISDATA;

typedef struct _tagItemData
{
	char * szItemData;
}ITEMDATA,*PITEMDATA;

	#include <vector>
	#include <map>

	using std::vector;
	using std::map;
	using std::iterator;

	typedef vector<string>            STRVECTOR;
	typedef STRVECTOR::iterator       STRVECTORITOR;

	typedef map<int,STRVECTOR>        GRIDDATAARRAY;
	typedef GRIDDATAARRAY::iterator   GRIDDATAARRAYITOR;
	
	typedef map<string,GRIDDATAARRAY> GRIDDATAARRAY2;
	typedef GRIDDATAARRAY2::iterator  GRIDDATAARRAYITOR2;

	void InitIceStorm();
	typedef int (CALLBACK *RegEvent_Proc)( const string & strCategory,const  int &iLevel, const string & strCurTime, const string & strMessage);
	int  GetCateGoryDatas(ITEMDATA **pCategorys,int *iCount);
	int  OnEvent_Proc( const EVENTATTRISDATA & attribeData,RegEvent_Proc proc);

#ifdef PTRMODE
	typedef struct _tagAttribsData
	{
		char *szAttribsName;
		char *szAttisbsValue;
	}ATTRIBSDATA,*PATTRIBSDATA;

	typedef int (CALLBACK *RegProgrssProc)( const int  & iTotalNum, const int & iCurNum);
	typedef int (CALLBACK *RegPlayListStateProc)(const string & strUid, const string & strMsg, const string & strStateValue, const int & iCurCtrlNum );
	
	//ITEMDATA **  GetData_Proc(ATTRIBSDATA **pAttribsData, int *iAttCount,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
	//ITEMDATA **  GetData_Proc(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);

	int  GetPlayListData_Proc(const char * cUid,const char * cStateValue,ITEMDATA **pColumnNames,int *iCol);
	ITEMDATA **  GetData_Proc(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc = NULL,RegPlayListStateProc stateProc = NULL);
#else

	// Attrs's Data Struct
	typedef struct tagAttrisData
	{
		string  strAttrName;
		string  strAttrValue;
	}ATTRISDATA,*PATTRISDATA;

	typedef vector<ATTRISDATA>        ATTRISVECTOR;
	typedef ATTRISVECTOR::iterator    ATTRISITOR;
	int  GetData_Proc(ATTRISVECTOR & attribsData,int *ColumnCount,STRVECTOR &ColumnNames, int *RowCount, GRIDDATAARRAY & CellsData);
#endif
	

#endif

