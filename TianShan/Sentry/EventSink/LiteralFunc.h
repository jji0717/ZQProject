#ifndef __LiteralFunction_H__
#define __LiteralFunction_H__
#include <string>
#include <vector>
#include <map>
#include <list>
namespace Literal
{
    void split(std::vector<std::string>& v, const std::string& s, const std::string d = " ");

    typedef std::vector<std::string> Arguments;
    typedef std::string (*Func)(const Arguments&);
    typedef std::map<std::string, Func> Library;

    struct SimpleExpression
    {
        Func func;
        Arguments args;
        SimpleExpression():func(0){}
    };
    typedef std::list<SimpleExpression> Expression;

    Expression compile(const std::string& s, const Library& lib);
    std::string exec(const Expression& expr);
};
#endif
