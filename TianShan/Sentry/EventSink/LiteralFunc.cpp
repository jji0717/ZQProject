#include "LiteralFunc.h"
#include <sstream>
#include <boost/regex.hpp>
namespace Literal
{
    void split(std::vector<std::string>& v, const std::string& s, const std::string d)
    {
        v.clear();
        
        std::string::size_type pos_from = 0;
        while((pos_from = s.find_first_not_of(d, pos_from)) != std::string::npos)
        {
            std::string::size_type pos_to = s.find_first_of(d, pos_from);
            if(pos_to != std::string::npos)
            {
                v.push_back(s.substr(pos_from, pos_to - pos_from));
            }
            else
            {
                v.push_back(s.substr(pos_from));
                break;
            }
            pos_from = pos_to;
        }
    }

    Expression compile(const std::string& s, const Library& lib)
    {
        Expression compiled;
        boost::regex re("\\$([\\w\\d_]+)\\(([^\\)]*)\\)");

        boost::smatch res;
        std::string::const_iterator begPos = s.begin();
        std::string::const_iterator endPos = s.end();
        while(boost::regex_search(begPos, endPos, res, re))
        {
            if(res.prefix().matched)
            { // identical text
                SimpleExpression ex;
                ex.args.push_back(res.prefix().str());
                compiled.push_back(ex);
            }
            SimpleExpression ex;
            Library::const_iterator func = lib.find(res.str(1));
            if(func != lib.end())
            {
                ex.func = func->second;
            } // ignore if the function invalid. treat as identical text

            split(ex.args, res.str(2), ", ");
            compiled.push_back(ex);

            // continue processing the rest
            begPos = res.suffix().first;
        }

        if(begPos != endPos)
        { // idential text
            SimpleExpression ex;
            ex.args.push_back(std::string(begPos, endPos));
            compiled.push_back(ex);
        }
        return compiled;
    }

    std::string exec(const Expression& expr)
    {
        std::ostringstream buf;

        for(Expression::const_iterator itExp = expr.begin(); itExp != expr.end(); ++itExp)
        {
            if(itExp->func)
            {
                buf << itExp->func(itExp->args);
            }
            else
            { // copy the arguments
                for(size_t i = 0; i < itExp->args.size(); ++i)
                    buf << itExp->args[i];
            }
        }
        return buf.str();
    }
};
