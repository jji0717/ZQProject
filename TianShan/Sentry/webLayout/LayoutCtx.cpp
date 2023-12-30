// LayoutCtx.cpp: implementation of the LayoutCtx class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LayoutCtx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace ZQTianShan
{
namespace Layout
{

LayoutCtx::LayoutCtx()
{
}

LayoutCtx::~LayoutCtx()
{
}


bool LayoutCtx::setColumns(const char* colnames[], const int colcount)
{
	//clear the column name first
	m_ColumnName.clear();
	for(int i=0;i<colcount;i++)
	{
		m_ColumnName.push_back(colnames[i]);
	}
	return true;
}
bool LayoutCtx::addRow(const char* colvalues[], const char* ref/*=NULL*/)
{
	TableRow	row;
	if( ref && strlen(ref)>0 )
	{
		row._ref	=	ref;
	}
	int	iColCount = (int) m_ColumnName.size();
	for( int i=0 ; i<iColCount ; i++ )
	{
		row._row.push_back(colvalues[i]);
	}

    m_tableData.push_back(row);
	return true;
}
void LayoutCtx::set(const char*key, const lvalue& val)
{
	if (key&&strlen(key)>0) 
	{
		/*strncpy((char*)val.oid, key,SNMPATTR_OID_MAXLEN-1);*/
		m_KeyValue[key]	=	val;
	}
}

bool LayoutCtx::get(const char* key, lvalue& val)
{
	KEYVALUEPAIR::const_iterator it = m_KeyValue.find(key);
	if(it != m_KeyValue.end())
	{
		val=it->second;
		return true;
	}
	else
	{
		return false;
	}
}
int LayoutCtx::list(char* buf, const int bufsize, char*** pattrnames)
{
	//we think the buf is enough	
	char** pAttr = (char**)buf;
	int nVarCount = m_KeyValue.size();
	char* pPtr = buf + sizeof(char*) * nVarCount;

	int nRetCount = 0;
	KEYVALUEPAIR::iterator it=m_KeyValue.begin();
	while(it!=m_KeyValue.end())
	{
		int nStrLen = it->first.length();

		if (pPtr - buf + nStrLen + 1 > bufsize)
		{
			//error, buffer size is not enough, need print out error log
			break;
		}

		pAttr[nRetCount] = pPtr;
		pPtr += sprintf(pPtr, "%s", it->first.c_str()) + 1;	// also add a 0 terminate
		nRetCount++;

		it++;
	}
	*pattrnames = pAttr;

	return nRetCount;
}

void LayoutCtx::clear(const char* key)
{
	if(!key)
	{
		m_KeyValue.clear();
	}
	else
	{
		KEYVALUEPAIR::iterator it = m_KeyValue.find(key);
		if(it!=m_KeyValue.end())
		{
			m_KeyValue.erase(it);
		}
	}
}

}}//namespace ZQTianShan::Layout

