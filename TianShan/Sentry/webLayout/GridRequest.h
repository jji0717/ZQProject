// GridRequest.h: interface for the GridRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDREQUEST_H__7BD4069E_6CB6_4453_B666_0161A8E46D4B__INCLUDED_)
#define AFX_GRIDREQUEST_H__7BD4069E_6CB6_4453_B666_0161A8E46D4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LayoutCtx.h"
#include "../httpdInterface.h"

namespace ZQTianShan
{
namespace Layout
{

class GridRequest : public LayoutCtx  
{
public:
	GridRequest(IHttpRequestCtx *pHttpRequestCtx);
	virtual ~GridRequest();

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
private:
    IHttpRequestCtx *_pHttpRequestCtx;
};

}}//namespace ZQTianShan::Layout
#endif // !defined(AFX_GRIDREQUEST_H__7BD4069E_6CB6_4453_B666_0161A8E46D4B__INCLUDED_)
