// ConsoleCommand.h: interface for the ConsoleCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONSOLECOMMAND_H__674BDC9E_506E_47B4_8E0C_F0E25FCED53A__INCLUDED_)
#define AFX_CONSOLECOMMAND_H__674BDC9E_506E_47B4_8E0C_F0E25FCED53A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../httpdInterface.h"

namespace ZQTianShan
{
namespace Layout
{

typedef IHttpResponse DataBuffer;

class ConsoleCommand  
{
public:
    static bool execute(DataBuffer &output, const char* pExe, const char* pCmdLine);
    static bool execute(std::string &output, const char* pExe, const char* pCmdLine);
};

}}// namespace ZQTianShan::Layout
#endif // !defined(AFX_CONSOLECOMMAND_H__674BDC9E_506E_47B4_8E0C_F0E25FCED53A__INCLUDED_)
