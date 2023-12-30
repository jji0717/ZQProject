// AdminCtrlUtil.cpp: implementation of the AdminCtrlUtil class.
//
//////////////////////////////////////////////////////////////////////
#include <fstream>
#include "StdAfx.h"
#include "AdminCtrlUtil.h"
#include <XMLPreferenceEx.h>

using namespace ZQ::common;
using namespace TianShanIce;

#define LOG_MODULE_NAME         "ACUtil"
bool AdminCtrlUtil::importFile(IHttpResponse &out, const char *path)
{
    if(NULL == path)
        return false;
    
    std::ifstream fl(path);
    if(fl.fail())
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to load file [%s]."), path);
        return false;
    }
    char ch;
    while(fl.get(ch))
    {
        out << ch;
    }
    
    if(fl.eof())
    {
        glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "import file [%s] successfully."), path);
        return true;
    }
    else
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "error occurred during read file [%s]."), path);
        return false;
    }
}
std::string AdminCtrlUtil::bool2str(bool b)
{
    return b ? "true" : "false";
}
std::string AdminCtrlUtil::int2str(int i)
{
    char buf[12] = {0};
    return itoa(i, buf, 10);
}
std::string AdminCtrlUtil::long2str(int64 l)
{
    char buf[22] = {0};
    snprintf(buf,22,"%lld",l);
    return buf;
  //  return _i64toa(l, buf, 10);
}
int AdminCtrlUtil::str2int(const std::string &s)
{
    return atoi(s.c_str());
}
int64 AdminCtrlUtil::str2long(const std::string &s)
{
#ifdef ZQ_OS_MSWIN
    return _atoi64(s.c_str());
#else
	return atoll(s.c_str());
#endif
}
std::string quoteString(const std::string &s)
{
    return (std::string("\"") + s + "\"");
}
const std::string& selfString(const std::string &s)
{
    return s;
}
std::string AdminCtrlUtil::unquoteString(const std::string &s)
{
    return trimString(s, "\"'");
}
unsigned char AdminCtrlUtil::str2byte(const std::string &s)
{
    if(s.size() != 2)
        return 0;
    int i = 0;
    sscanf(s.c_str(), "%x", &i);
    return (unsigned char)i;
}
template< class _Iter , class _fnToStr>
std::string joinString(_Iter itFirst, _Iter itLast, const std::string &delimiter, _fnToStr toStr)
{
    std::string str;
    while(itFirst != itLast)
    {
        str += toStr(*itFirst);
        str += delimiter;
        ++itFirst;
    }
    if(!str.empty())
    {
        return str.substr(0, str.size() - delimiter.size());
    }
    else
    {
        return "";
    }
}
void AdminCtrlUtil::splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter)
{
    using namespace std;
    result.clear();
    string::size_type pos_from = 0;
    while((pos_from = str.find_first_not_of(delimiter, pos_from)) != string::npos)
    {
        string::size_type pos_to = str.find_first_of(delimiter, pos_from);
        if(pos_to != string::npos)
        {
            result.push_back(str.substr(pos_from, pos_to - pos_from));
        }
        else
        {
            result.push_back(str.substr(pos_from));
            break;
        }
        
        pos_from = pos_to;
    }
}
std::string AdminCtrlUtil::trimString(const std::string &s, const std::string &chs)
{
    std::string::size_type pos_beg = std::string::npos;
    std::string::size_type pos_end = std::string::npos;
    pos_beg = s.find_first_not_of(chs);
    if(std::string::npos == pos_beg)
        return "";
    pos_end = s.find_last_not_of(chs);
    return s.substr(pos_beg, pos_end - pos_beg + 1);
}
std::string AdminCtrlUtil::trimWS(const std::string &s)
{
    return trimString(s, " \f\n\r\t\v");
}
//////////////////////////////////////////////////////////////////////////
std::string AdminCtrlUtil::vartype(const TianShanIce::Variant &var)
{
    std::string type;
    type += (var.bRange ? "R" : "E");
    switch(var.type)
    {
    case vtInts:
        type += "I";
        break;
    case vtLongs:
        type += "L";
        break;
    case vtStrings:
        type += "S";
        break;
    case vtBin:
        type += "B";
        break;
    case vtFloats:
        type += "F";
        break;
    default:
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad var type [%d]."), var.type);
    }
    return type;
}
std::string AdminCtrlUtil::var2str(const TianShanIce::Variant &var)
{
    std::string delimiter = var.bRange ? "~" : ";";
    switch(var.type)
    {
    case vtInts:
        return joinString(var.ints.begin(), var.ints.end(), delimiter, int2str);
    case vtLongs:
        return joinString(var.lints.begin(), var.lints.end(), delimiter, long2str);
    case vtStrings:
        return joinString(var.strs.begin(), var.strs.end(), delimiter, selfString);
#pragma message(__MSGLOC__"TODO: support type vtBin and vtFloats.")
    case vtBin:
    case vtFloats:
        break;
    }
    return "";
}
#define VARIANT_TYPESTR_LEN     2
#define VARIANT_TYPESTR_IDX_RANGE   0
#define VARIANT_TYPESTR_IDX_TYPE    1
bool AdminCtrlUtil::str2var(const std::string &str, const std::string &type, TianShanIce::Variant &var)
{
    // check type
    if(type.size() != VARIANT_TYPESTR_LEN)
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad var type [%s]."), type.c_str());
        return false;
    }
    switch(type[VARIANT_TYPESTR_IDX_RANGE])
    {
    case 'R':
        var.bRange = true;
        break;
    case 'E':
        var.bRange = false;
        break;
    default:
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad var type [%s]."), type.c_str());
        return false;
    }
    
    StrValues vals;
    splitString(vals, str, (var.bRange ? "~" : ";"));
    std::transform(vals.begin(), vals.end(), vals.begin(), trimWS);
    switch(type[VARIANT_TYPESTR_IDX_TYPE])
    {
    case 'I':
        {
            var.type = vtInts;
            var.ints.clear();
            var.ints.resize(vals.size());
            std::transform(vals.begin(), vals.end(), var.ints.begin(), str2int);
        }
        break;
    case 'L':
        {
            var.type = vtLongs;
            var.lints.clear();
            var.lints.resize(vals.size());
            std::transform(vals.begin(), vals.end(), var.lints.begin(), str2long);
        }
        break;
    case 'S':
        {
            var.type = vtStrings;
            var.strs.clear();
            var.strs.resize(vals.size());
            std::transform(vals.begin(), vals.end(), var.strs.begin(), unquoteString);
        }
        break;
#pragma message(__MSGLOC__"TODO: support type vtBin and vtFloats.")
    case 'B':
    case 'F':
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "var type [%s] not supported."), type.c_str());
        return false;
    default :
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad var type [%s]."), type.c_str());
        return false;
    }
    return true;
}
bool AdminCtrlUtil::extractKVData(const std::string &keyPrefix, IHttpRequestCtx *pRqstCtx, KVData_t &kvdata)
{
    kvdata.clear();
    if(NULL == pRqstCtx)
    {
        return false;
    }
    IHttpRequestCtx::RequestVars vars;
    pRqstCtx->GetRequestVars().swap(vars);
    IHttpRequestCtx::RequestVars::const_iterator cit_var;
    for(cit_var = vars.begin(); cit_var != vars.end(); ++cit_var)
    {
        if(cit_var->first.compare(0, keyPrefix.size(), keyPrefix))
            continue; // ignore other variables
        
        std::string pdKey = cit_var->first.substr(keyPrefix.size());
        if(pdKey.empty())
        {
            glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad request var name. [%s]"), cit_var->first.c_str());
            return false;
        }
        kvdata[pdKey] = cit_var->second;
    }
    return true;
}

static std::string tagName(ZQ::common::XMLPreferenceEx *pNode)
{
    if(NULL == pNode)
        return "";

    char nameBuf[256];
    nameBuf[0] = '\0';
    if(pNode->name(nameBuf, sizeof(nameBuf)))
    {
        return nameBuf;
    }
    else
    {
        return "";
    }

}
static void printStartTag(ZQ::common::XMLPreferenceEx *pNode, std::ostream& out)
{
    if(NULL == pNode)
        return;

    char nameBuf[256];
    nameBuf[0] = '\0';
    if(pNode->name(nameBuf, sizeof(nameBuf)))
    {
        out << "<" << nameBuf;
        typedef std::map<std::string, std::string> StringMap;
        StringMap attrs = pNode->getProperties();
        for(StringMap::iterator it = attrs.begin(); it != attrs.end(); ++it)
        {
            out << " " << it->first << "=\"" << it->second << "\"";
        }
        out << " >";
    }
}

static void printEndTag(ZQ::common::XMLPreferenceEx *pNode, std::ostream& out)
{
    if(NULL == pNode)
        return;

    char nameBuf[256];
    nameBuf[0] = '\0';
    if(pNode->name(nameBuf, sizeof(nameBuf)))
    {
        out << "</" << nameBuf << ">";
    }
}
static void printXMLElement(ZQ::common::XMLPreferenceEx *pNode, std::ostream& out)
{
    if(NULL == pNode)
        return;

    // print the start tag
    printStartTag(pNode, out);
    // enumerate the child nodes
    ZQ::common::XMLPreferenceEx *pChild = pNode->firstChild();
    while(pChild)
    {
        printXMLElement(pChild, out);
        pChild->free();

        pChild = pNode->nextChild();
    }
    // print the content if available
    {
        char contentBuf[1024];
        contentBuf[0] = '\0';
        if(pNode->getPreferenceText(contentBuf, sizeof(contentBuf)))
            out << contentBuf;
    }
    
    printEndTag(pNode, out);
}

typedef std::vector<std::string> StringVector;
static void printXMLElement(ZQ::common::XMLPreferenceEx *pNode, std::ostream& out, const StringVector& targetPath, const StringVector &content)
{
    if(NULL == pNode)
        return;

    if(!targetPath.empty() && targetPath[0] == tagName(pNode))
    { // hit the target path
        if(targetPath.size() == 1)
        { // we are the target node, just ignore
            return;// no output
        }

        // generate the relative path
        StringVector rel_path;
        rel_path.reserve(targetPath.size() - 1);
        std::copy(targetPath.begin() + 1, targetPath.end(), std::back_inserter(rel_path));

        // start the printing
        printStartTag(pNode, out);

        // enumerate the child nodes
        ZQ::common::XMLPreferenceEx *pChild = pNode->firstChild();
        while(pChild)
        {
            printXMLElement(pChild, out, rel_path, content);
            pChild->free();

            pChild = pNode->nextChild();
        }

        if(rel_path.size() == 1)
        { // we are the parent node
            // insert the content here
            for(StringVector::const_iterator it = content.begin(); it != content.end(); ++it)
                out << "<" << rel_path[0] << ">" << (*it) << "</" << rel_path[0] << ">";
        }
        printEndTag(pNode, out);
    }
    else
    { // just print with no alter
        printXMLElement(pNode, out);
    }
}
// update a XML file 
bool AdminCtrlUtil::updateXML(const std::string& filePath, const std::string& nodePath, const StringVector &content)
{
    // load xml file into memory
    ZQ::common::XMLPreferenceDocumentEx doc;
    bool bValidXML = false;
    try{
        bValidXML = doc.open(filePath.c_str());
    }catch(XMLException &e)
    { // failed 
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encouter XMLExcption [%s] during loading the server load template file [%s]."), e.getString(), filePath.c_str());
        bValidXML = false;
    }catch(...)
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter unknown exception during loading the server load template file [%s]."), filePath.c_str());
        bValidXML = false;
    }
    if(!bValidXML)
        return false;

    std::ofstream out; 
    out.open(filePath.c_str());
    if(out.good())
    {
        StringVector targetPath;
        splitString(targetPath, nodePath, "/");

        try{ // print the doc
            ZQ::common::XMLPreferenceEx *rootNode = doc.getRootPreference();
            printXMLElement(rootNode, out, targetPath, content);
            rootNode->free();
        }catch(XMLException &e)
        { // failed 
            glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encouter XMLExcption [%s] during updating xml document [%s]."), e.getString(), filePath.c_str());
            return false;
        }catch(...)
        {
            glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter unknown exception during update xml document [%s]."), filePath.c_str());
            return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

std::string AdminCtrlUtil::escapeString(const std::string& str)
{ // only escape the '\' to fit our need
    std::string escaped;
    escaped.reserve(str.size() * 2);

    for(size_t i = 0; i < str.size(); ++i)
    {
        char ch = str.at(i);
        switch(ch)
        {
        case '\\':
            escaped += "\\\\";
            break;
        default:
            escaped += ch;
            break;
        }
    }
    return escaped;
}
