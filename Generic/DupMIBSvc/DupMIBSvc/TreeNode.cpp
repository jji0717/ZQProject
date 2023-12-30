
#include "TreeNode.hpp"

_TreeNode::~_TreeNode()
{
	for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
	{
		_TreeNode *itVal = *it;
        delete itVal; 
	}
};


int _TreeNode::insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString)
{
#if defined(DEBUG)
	size_t parentLen = parent.size();
	size_t myNameLen = myName.size();
	size_t iParentLen = _parent.size();
	size_t iMyNameLen = _myName.size();
#endif//DEBUG
	int nRev = false;
	if (0 == _parent.compare(parent))
	{
		if (0 == _myName.compare(myName))
		{
			_myNum = myNum;
			_rest  = restString;
			_restReplace   = _rest;
			nRev = true;
		}
	}else if (0 == _myName.compare(parent)){
		nRev = true;
		if (std::string::npos != restString.find("INDEX") && std::string::npos != restString.find("{"))
			_leaf.push_back(new _TreeTableNode(parent, myName, myNum, restString));
		else
			_leaf.push_back(new _TreeNode(parent, myName, myNum, restString));

	}else{
		for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
		{
			_TreeNode *itVal = *it;
			if (itVal->insertNode(parent, myName, myNum, restString))
				nRev = true;  
		}
	}

	return nRev;
}

int  _TreeNode::insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString, std::string& sequenceStr)
{
#if defined(DEBUG)
	size_t parentLen = parent.size();
	size_t myNameLen = myName.size();
	size_t iParentLen = _parent.size();
	size_t iMyNameLen = _myName.size();
#endif//DEBUG
	int nRev = false;
	for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
	{
		_TreeNode *itVal = *it;
		if (itVal->insertNode(parent, myName, myNum, restString, sequenceStr))
			nRev = true;  
	}

	return nRev;
}

int _TreeNode::replaceNode(std::string& dst, std::string & SvcSeqNum)
{
	int nRev = true;
	_myNumReplace  = _myNum;
	_parentReplace = _parent;
	_myNameReplace = _myName;
	if(0 != SvcSeqNum.compare("0"))
	{
		if (0 == dst.compare(_myName))
		{
			_myNameReplace = _myName + SvcSeqNum;
			if(_myNumReplace.size() > 1)
				_myNumReplace.replace(_myNumReplace.size() - 2, SvcSeqNum.size(), SvcSeqNum);
		}else{
			_parentReplace += SvcSeqNum;
			_myNameReplace += SvcSeqNum;
		}
	}

	for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
	{
		_TreeNode *itVal = *it;
		if (itVal->replaceNode(dst, SvcSeqNum))
			nRev = true;  
	}

	return nRev;
}


int _TreeNode::toStrByReplace(std::string& dstStr)
{
	dstStr += "    ";
	dstStr += _myNameReplace;
	dstStr += _restReplace;
	dstStr += " ::= {";
	dstStr += _parentReplace;
	dstStr += "  ";
	dstStr += _myNumReplace;
	dstStr += " } \n\n";

	for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
	{
		_TreeNode *itVal = *it;
		itVal->toStrByReplace(dstStr);  
	}

	return true;
}

int _TreeNode::toStrByCurrent(std::string& dstStr)
{
	dstStr += "    ";
	dstStr += _myName;
	dstStr += _rest;
	dstStr += " ::= {";
	dstStr += _parent;
	dstStr += "  ";
	dstStr += _myNum;
	dstStr += " } \n\n";

	return true;
}

int TreeRoot::insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString)
{
#if defined(DEBUG)
	size_t parentLen = parent.size();
	size_t myNameLen = myName.size();
	size_t iParentLen = _parent.size();
	size_t iMyNameLen = _myName.size();
#endif//DEBUG
	int nRev = true;
	if (TreeNode::insertNode(parent, myName, myNum, restString))
		return nRev;

	int storeRootLeaf = true;
	for(std::list<_TreeNode* >::iterator it = _leafNotInTreeNode.begin(); it != _leafNotInTreeNode.end(); it++)
	{
		_TreeNode *itVal = *it;
		if (itVal->insertNode(parent, myName, myNum, restString))
			storeRootLeaf = false;  
	}

	if(!storeRootLeaf)
	{
		if (std::string::npos != restString.find("INDEX") && std::string::npos != restString.find("{"))
			_leafNotInTreeNode.push_back(new _TreeTableNode(parent, myName, myNum, restString));
		else
			_leafNotInTreeNode.push_back(new _TreeNode(parent, myName, myNum, restString));
	}

	for(std::list<_TreeNode* >::iterator it = _leafNotInTreeNode.begin(); it != _leafNotInTreeNode.end(); it++)
	{
		_TreeNode *itVal = *it;
		TreeNode::insertNode(itVal->_parent, itVal->_myName, itVal->_myNum, itVal->_rest);//retry insert to tree
	}

	return true;
}


int TreeTableNode::replaceNode(std::string& dst, std::string & SvcSeqNum)
{
	_myNumReplace  = _myNum;
	_parentReplace = _parent;
	_myNameReplace = _myName;
	_sequenceStrReplace = _sequenceStr;
	if(0 != SvcSeqNum.compare("0"))
	{
		for(std::list<_TreeNode* >::iterator it = _leaf.begin(); it != _leaf.end(); it++)
		{
			_TreeNode *itVal = *it;
			std::string columnName = itVal->_myName;
			std::string colunmReplace = columnName + SvcSeqNum;
#ifdef DEBUG
			size_t columnNameSize = columnName.size();
			size_t colunmReplaceSize = colunmReplace.size();
#endif
			size_t nameInRestPos = _restReplace.find(columnName);
			size_t markInRest    = _restReplace.find_first_of(" \t }", nameInRestPos);
			if (std::string::npos != nameInRestPos && std::string::npos != markInRest && columnName.size() == markInRest - nameInRestPos )
			{
			   _restReplace.replace(nameInRestPos, columnName.size(), colunmReplace);
			}

			size_t nameInSeqPos  = _sequenceStrReplace.find(columnName);
			size_t markInSeq     = _sequenceStrReplace.find_first_of("  \t \r\n", nameInSeqPos);
			if (std::string::npos != nameInSeqPos && std::string::npos != markInSeq && columnName.size() == markInSeq - nameInSeqPos )
			{
				_sequenceStrReplace.replace(nameInSeqPos, columnName.size(), colunmReplace);
			}
		}
	}

	return TreeNode::replaceNode(dst, SvcSeqNum);
}