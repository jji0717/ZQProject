#ifndef _TREE_DUP_MIB_SVC_HPP
#define _TREE_DUP_MIB_SVC_HPP

#include "TreeNode.hpp"

class TreeMibUtility
{
public:
	virtual int parseStr(TreeNode& node, std::string& dst, std::string& dstSvc);
	virtual int replaceStr(TreeNode& node, std::string& dst, std::string & SvcSeqNum);

	int parseTreeRoot(TreeNode& node, std::string& dst);  // search MODULE-IDENTITY key word as tree sub root
	int parseTreeObject(TreeNode& node, std::string& dst); //include "OBJECT-TYPE" | "OBJECT IDENTIFIER"    
	int parseTreeSequence(TreeNode& node, std::string& dst); //include "::= SEQUENCE {...} "    
	int parseTreeNotification(TreeNode& node, std::string& dst); //include "NOTIFICATION-TYPE"    
};


class TreeDupMIBSvc : public BaseService
{
public:
	TreeDupMIBSvc();
	virtual ~TreeDupMIBSvc();

	virtual ErrorCode init(int argc, char* argv[]);
	virtual ErrorCode start(void);
	virtual void stop(void);
	virtual void unInit(void);

private:
	int  usage(void);
	void version(void);
	int  parseCmdLine(int argc, char* argv[]);
	int  forwardConvert(std::string& dstSvc, std::string& srcData, std::string& srcSvc, std::string& parent);
	int  process(std::string& dstData, std::string& srcData, std::string & dstSvc, std::string & SvcSeqNum, TreeMibUtility* howTo);
	int  splitData(std::string& restBeforeReplace, std::string& restAfterReplace, std::string& srcSliceData, std::string& dstService, TreeRoot& root);
	
private:
	int _splitWithoutTs;
	std::string _svcStr;
	std::string _parent;
	std::string _srcMibFileData;
	std::string _svcInstanceId;
	std::string _srcMibFilename;
	std::string _dstMibFilename;
};

#endif//_TREE_DUP_MIB_SVC_HPP