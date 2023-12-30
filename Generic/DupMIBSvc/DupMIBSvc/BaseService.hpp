#ifndef  _BASESERVICE_HPP
#define  _BASESERVICE_HPP

#include <list>
#include <string>

typedef enum _ErrorCode
{
	SUCCEED,
	COMMAND_QUIT,
	FUNCTION_FAILED,
	EXIT_FAILED,
	ERROR_COUNT
} ErrorCode;

class BaseService 
{
public:
	BaseService(){};
	virtual ~BaseService(){};

	virtual ErrorCode init(int argc, char* argv[]) = 0;
	virtual ErrorCode start(void) = 0;
	virtual void stop(void) = 0;
	virtual void unInit(void) = 0;
};

typedef class _BaseNode
{
public:
	_BaseNode(){}
	virtual ~_BaseNode(){}

	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum) = 0;
	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString) =  0;
	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString, std::string& restBeforeParent){return true;}
}BaseNode;

class _Node : public _BaseNode
{
public:
	_Node(){}

	_Node(std::string& parent, std::string& myName, std::string& myNum)
		:_parent(parent), _myName(myName), _myNum(myNum)
	{}

	_Node(std::string parent, std::string myName)
		:_parent(parent), _myName(myName)
	{}

	virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum, std::string& restString)
	{
		return insertNode(parent, myName, myNum);
	}

    virtual int insertNode(std::string& parent, std::string& myName, std::string& myNum)
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
				nRev = true;
			}
		}else if (0 == _myName.compare(parent)){
			_son.push_back(_Node(parent, myName, myNum));
			nRev = true;
		}else{
			for(std::list<_Node >::iterator it = _son.begin(); it != _son.end(); it++)
				if (it->insertNode(parent, myName, myNum))
					nRev = true;  
		}

		return nRev;
	}

public:
	std::string _parent;
	std::string _myName;
	std::string _myNum;

	std::list<_Node > _son;
};

typedef _Node Node;

#endif   //_BASESERVICE_HPP