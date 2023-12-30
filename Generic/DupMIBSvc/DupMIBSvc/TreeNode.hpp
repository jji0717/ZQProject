#ifndef  _TREE_NODE_HPP
#define  _TREE_NODE_HPP

#include <list>
#include "BaseService.hpp"

class _TreeTableNode;

class _TreeNode : public Node
{
public:	
	_TreeNode(){}

	_TreeNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restStr)
		:_Node(parent, myName, myNum),_rest(restStr), _restReplace(restStr) 
	{}

	_TreeNode(std::string parent, std::string myName)
		:_Node(parent, myName)
	{}

	virtual ~_TreeNode();


	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString);

	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString, std::string& sequenceStr);

	virtual int replaceNode(std::string& dst, std::string & SvcSeqNum);

	virtual _TreeNode* findNode(std::string& dst)
	{
		_TreeNode* nodePtr = 0;
		if (0 == dst.compare(_myName))
			return this;

		for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
		{
			_TreeNode *itVal = *it;
			nodePtr = itVal->findNode(dst);
			if (0 != nodePtr)
			{
				break;  
			}
		}

		return nodePtr;
	}

	virtual int toStrByCurrent(std::string& dstStr);
	virtual int toStrByReplace(std::string& dstStr);

public:
	std::list<_TreeNode *> _leaf;
	std::string _rest;
	std::string _restReplace;
	std::string _parentReplace;
	std::string _myNameReplace;
	std::string _myNumReplace;
};	

typedef _TreeNode  TreeNode;

class _TreeRoot : public TreeNode
{
public:
	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString);
public:
	std::list<_TreeNode *> _leafNotInTreeNode;   // not in tree node
};

typedef _TreeRoot TreeRoot;

class _TreeTableNode : public TreeNode
{
public:
	_TreeTableNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restStr)
		:TreeNode(parent, myName, myNum, restStr)
	{}

	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString, std::string& sequenceStr )
	{
		if (0 == parent.compare(_parent) && 0 == myName.compare(_myName))
		{
			_sequenceStr = sequenceStr;
			return true;
		}
	   
	   return TreeNode::insertNode(parent, myName, myNum, restString, sequenceStr);
	} 

	virtual int toStrByReplace(std::string& dstStr)
	{
		int nRev = TreeNode::toStrByReplace(dstStr);
		dstStr += _sequenceStrReplace;
		dstStr += "\n\n";

		return nRev;
	}

	virtual int toStrByCurrent(std::string& dstStr)
	{
		int nRev = TreeNode::toStrByCurrent(dstStr);
		dstStr += _sequenceStrReplace;
		dstStr += "\n\n";

		return nRev;
	}

	virtual int replaceNode(std::string& dst, std::string & SvcSeqNum);

public:
	std::string _sequenceStr;
	std::string _sequenceStrReplace;
};

typedef _TreeTableNode TreeTableNode;

#endif//_TREE_NODE_HPP