#ifndef __ZQ_SimpleXMLParser_H__
#define __ZQ_SimpleXMLParser_H__
#include <expatxx.h>
#include <map>
#include <list>
#include <stack>
#include <vector>

/// a simple xml parser
// usage:
// 1. create a parser instance.
// 2. parse xml file or xml text.(see ExpatBase)
// 3. access the parsed content in the document tree. (through parser.document())
// 4. invoke clear() to reset the parser and go on from step 2.

class SimpleXMLParser: public ZQ::common::ExpatBase
{
public:
    struct Node
    {
        std::string name;
        std::string content;
        std::map<std::string, std::string> attrs;
        std::list<Node> children;
    };

    SimpleXMLParser();
    virtual ~SimpleXMLParser();
    /// reset the parser
    void clear();
    /// get the parsed xml document
    const Node& document();

protected:
    virtual void OnStartElement(const XML_Char* name, const XML_Char** atts);
    virtual void OnEndElement(const XML_Char*);
    virtual void OnCharData(const XML_Char* data, int len);
    virtual void OnLogicalClose();

private:
    Node _doc;
    std::stack<Node*> _nodePath;
};

//////////////////////////////////////////////////////////////////
/// helper functions

// find the target node in the specified path from a root node
// return the target node's pointer or NULL for the failure
// note that only the first node that match the tag name in the path can be selected
const SimpleXMLParser::Node* findNode(const SimpleXMLParser::Node* root, const std::string& path);

// build xml document with sub node replaced
void buildXMLDocument(std::string& output, const SimpleXMLParser::Node& root, const std::string& path, const SimpleXMLParser::Node& sub);

class SiblingNode : public SimpleXMLParser::Node
{
public:
	SiblingNode( ):
	  mIdx(-1)
	{
	}
	virtual ~SiblingNode( ){;}
public:
	///get next sibling node
	/// return NULL if no more sibling node available;
	const SimpleXMLParser::Node*	first()
	{
		mIdx = 0;
		if( mSibling.size() <= 0 )
			return NULL;
		return mSibling[mIdx]; 
	}
	const SimpleXMLParser::Node*	next( )
	{
		++mIdx;
		if ( mIdx >= static_cast<int>(mSibling.size()) )
		{
			return NULL;
		}
		else
		{
			return mSibling[mIdx];
		}
	}

	 size_t							count() const
	 {
		 return mSibling.size();
	 }

private:
	std::vector<SimpleXMLParser::Node*>		mSibling;
	int										mIdx;
	friend SiblingNode childNodes( const SimpleXMLParser::Node* , const std::string& );

};
/// more helper function here
/// get child node of parentNode, if there are more than one child node with the same name, you can walk them by using SiblingNode
///
SiblingNode childNodes( const SimpleXMLParser::Node* parentNode, const std::string& name);


#endif
