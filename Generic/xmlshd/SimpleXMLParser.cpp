#include "SimpleXMLParser.h"
#include <boost/algorithm/string.hpp>
#include <sstream>

SimpleXMLParser::SimpleXMLParser()
{
    _nodePath.push(&_doc);
}
SimpleXMLParser::~SimpleXMLParser()
{
}
void SimpleXMLParser::OnStartElement(const XML_Char* name, const XML_Char** atts)
{
    Node node;
    node.name = name;
    for(int i = 0; atts[i]; i += 2)
    {
        node.attrs[atts[i]] = atts[i + 1];
    }
    Node* container = _nodePath.top();
    container->children.push_back(node);
    _nodePath.push(&container->children.back());
}
void SimpleXMLParser::OnEndElement(const XML_Char*)
{
    _nodePath.pop();
}
void SimpleXMLParser::OnCharData(const XML_Char* data, int len)
{
    // we are simple XML parser
    // so we simply merge the multi-lines content
    _nodePath.top()->content.append(boost::trim_copy(std::string(data, len)));
}
void SimpleXMLParser::OnLogicalClose()
{
}

/// reset the parser
void SimpleXMLParser::clear()
{
    _doc.name.clear();
    _doc.content.clear();
    _doc.children.clear();
    _nodePath = std::stack<Node*>();
    _nodePath.push(&_doc);
}

/// get the parsed xml document
const SimpleXMLParser::Node& SimpleXMLParser::document()
{
    return _doc;
}

/// find the target node in the spicified path from a root node
const SimpleXMLParser::Node* findNode(const SimpleXMLParser::Node* root, const std::string& path)
{
    if(NULL == root || path.empty())
        return root;

    typedef std::list<std::string> StandardPath;
    StandardPath stdPath;
    boost::split(stdPath, path, boost::is_any_of("/"));

    const SimpleXMLParser::Node* current = root;
    for(StandardPath::const_iterator it = stdPath.begin(); it != stdPath.end(); ++it)
    {
        // search the child nodes
        const SimpleXMLParser::Node* target = NULL;
        std::list<SimpleXMLParser::Node>::const_iterator itChild;
        for(itChild = current->children.begin(); itChild != current->children.end(); ++itChild)
        {
            if(itChild->name == *it)
            { // get the target node
                target = &(*itChild);
                break;
            }
        }

        if(target)
        { // try the next node
            current = target;
            continue;
        }
        else
        { // can't find the node
            return NULL;
        }
    }
    return current;
}


SiblingNode childNodes( const SimpleXMLParser::Node* parentNode, const std::string& name)
{
	SiblingNode nodes;
	if(NULL == parentNode || name.empty())
		return nodes;
	const std::list<SimpleXMLParser::Node>& children = parentNode->children;	
	std::list<SimpleXMLParser::Node>::const_iterator it = children.begin();
	for( ; it != children.end() ; it ++ )
	{
		if( it->name == name )
		{
			nodes.mSibling.push_back( const_cast<SimpleXMLParser::Node*>(&(*it)) );
		}
	}
	return nodes;
}


// helper function to help build the xml document
typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::string> StringVector;
static void printStartTag(const SimpleXMLParser::Node& el, std::ostream& out)
{
    if(el.name.empty())
        return;

    out << "<" << el.name;
    for(StringMap::const_iterator it = el.attrs.begin(); it != el.attrs.end(); ++it)
    {
        out << " " << it->first << "=\"" << it->second << "\"";
    }
    out << " >";
}
static void printContent(const SimpleXMLParser::Node& el, std::ostream& out)
{
    if(el.name.empty())
        return;

    out << el.content;
}
static void printEndTag(const SimpleXMLParser::Node& el, std::ostream& out)
{
    if(el.name.empty())
        return;
    out << "</" << el.name << ">";
}

static void printXMLElement(const SimpleXMLParser::Node& el, std::ostream& out)
{
    if(el.name.empty())
        return;

    // print the start tag
    printStartTag(el, out);
    printContent(el, out); // content

    // enumerate the child nodes
    std::list<SimpleXMLParser::Node>::const_iterator itChild;
    for(itChild = el.children.begin(); itChild != el.children.end(); ++itChild) {
        printXMLElement(*itChild, out);
    }

    // end tag
    printEndTag(el, out);
}

// print xml element with sub node replaced
static void printXMLElement(const SimpleXMLParser::Node& el, std::ostream& out, const StringVector& targetPath, const SimpleXMLParser::Node& sub)
{
    if(!targetPath.empty() && targetPath[0] == el.name)
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
        printStartTag(el, out);
        printContent(el, out);

        // enumerate the child nodes
        std::list<SimpleXMLParser::Node>::const_iterator itChild;
        for(itChild = el.children.begin(); itChild != el.children.end(); ++itChild) {
            printXMLElement(*itChild, out, rel_path, sub);
        }

        if(rel_path.size() == 1)
        { // we are the parent node
            // insert the subnode here
            printXMLElement(sub, out);
        }
        printEndTag(el, out);
    }
    else
    { // just print with no alter
        printXMLElement(el, out);
    }
}

void buildXMLDocument(std::string& output, const SimpleXMLParser::Node& root, const std::string& path, const SimpleXMLParser::Node& sub) {
    output.clear();
    StringVector targetPath;
    boost::split(targetPath, path, boost::is_any_of("/"));
    std::ostringstream buf;
    printXMLElement(root, buf, targetPath, sub);
    buf.str().swap(output);
}
