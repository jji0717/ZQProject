// AdminCtrlUtil.h: interface for the AdminCtrlUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADMINCTRLUTIL_H__8DDAD4CE_C20E_4858_96B2_F8F7DCD823E8__INCLUDED_)
#define AFX_ADMINCTRLUTIL_H__8DDAD4CE_C20E_4858_96B2_F8F7DCD823E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../httpdInterface.h"

using namespace ZQ::common;


class AdminCtrlUtil  
{
public:
    static bool importFile(IHttpResponse &out, const char *path);

    static std::string bool2str(bool b);
    static std::string int2str(int i);
    static std::string long2str(int64 l);
    static int str2int(const std::string &s);
    static int64 str2long(const std::string &s);
    static unsigned char str2byte(const std::string &s);
    
    static std::string vartype(const TianShanIce::Variant &var);
    static std::string var2str(const TianShanIce::Variant &var);
    static bool str2var(const std::string &str, const std::string &type, TianShanIce::Variant &var);

    static std::string trimString(const std::string &s, const std::string &chs = " ");
    static std::string trimWS(const std::string &s);
    static void splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter);

    typedef std::map< std::string, std::string > KVData_t;
    static bool extractKVData(const std::string &keyPrefix, IHttpRequestCtx *pRqstCtx, KVData_t &kvdata);

    typedef std::vector< std::string > StringVector;
    static bool updateXML(const std::string& filePath, const std::string& nodePath, const StringVector &content);

    static std::string escapeString(const std::string& str);
private:
    static std::string unquoteString(const std::string &s);
};

#endif // !defined(AFX_ADMINCTRLUTIL_H__8DDAD4CE_C20E_4858_96B2_F8F7DCD823E8__INCLUDED_)
