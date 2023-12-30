// SnmpRequest.h: interface for the SnmpRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNMPREQUEST_H__BF26E25F_7A95_47A8_ADE8_41A2BE6E8567__INCLUDED_)
#define AFX_SNMPREQUEST_H__BF26E25F_7A95_47A8_ADE8_41A2BE6E8567__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LayoutCtx.h"
#include "../httpdInterface.h"

namespace ZQTianShan
{
namespace Layout
{

class SnmpRequest : public LayoutCtx  
{
public:
	SnmpRequest(IHttpRequestCtx *pHttpRequestCtx);
	virtual ~SnmpRequest();

    bool process();
private:
    bool init();
    bool prepareLayoutCtx();
    bool format();
private:
    //page info
    const char* _template;
    const char* _uri;
    //dll info
    const char* _datasource;
    const char* _entry;
    const char* _logfilepath;
    //procedure parameters
    const char* _baseoid;
    const char* _serverip;
    const char* _community;

    // mode
    const char* _mode; // extend for ngod
    const char* _authURL;

    // filter
    const char* _filter; // group filter
private:
    IHttpRequestCtx *_pHttpRequestCtx;
};

}}//namespace ZQTianShan::Layout
#endif // !defined(AFX_SNMPREQUEST_H__BF26E25F_7A95_47A8_ADE8_41A2BE6E8567__INCLUDED_)
