// LayoutCtx.h: interface for the LayoutCtx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYOUTCTX_H__95A0801E_34C7_4929_9F1E_A778A45B2F46__INCLUDED_)
#define AFX_LAYOUTCTX_H__95A0801E_34C7_4929_9F1E_A778A45B2F46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include <map>
#include <TsLayout.h>

namespace ZQTianShan
{
namespace Layout
{

class LayoutCtx : public ILayoutCtx
{
public:

	virtual bool setColumns(const char* colnames[], const int colcount);
	
	virtual bool addRow(const char* colvalues[], const char* ref=NULL);

	virtual void set(const char*key, const lvalue& val);

	virtual bool get(const char* key, lvalue& val);

	virtual int list(char* buf, const int bufsize, char*** pattrnames);
	
	virtual	void clear(const char* key);
public:
   LayoutCtx();	
	virtual ~LayoutCtx();

protected:
	typedef std::map<std::string , lvalue>	KEYVALUEPAIR;
	typedef std::vector<std::string>		ROWDATA;
	typedef struct _TableRow
	{
		ROWDATA			_row;
		std::string		_ref;
	}TableRow;
	typedef	std::vector<TableRow>			TABLEDATA;
	
	KEYVALUEPAIR				m_KeyValue;
	ROWDATA						m_ColumnName;
	TABLEDATA					m_tableData;
};

}}//namespace ZQTianShan::Layout
#endif // !defined(AFX_LAYOUTCTX_H__95A0801E_34C7_4929_9F1E_A778A45B2F46__INCLUDED_)
