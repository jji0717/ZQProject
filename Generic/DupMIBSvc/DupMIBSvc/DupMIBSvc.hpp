#ifndef _DUP_MIB_SVC_HPP
#define _DUP_MIB_SVC_HPP

#include <list>
#include "BaseService.hpp"

class MibUtility
{
public:
	virtual int parseStr(Node& node, std::string& dst, std::string& dstSvc);
	virtual int replaceStr(Node& node, std::string& dst, std::string & SvcSeqNum);

	int replaceCurrentNode(Node& node, std::string& dst, std::string & SvcSeqNum, std::string& dstSvc);
	int replaceMibTable(Node& node, std::string& dst, std::string & SvcSeqNum);

private:
	std::string _dstSvc;
	std::string _dstSvcParent;
};


class DupMIBSvc : public BaseService
{
public:
	DupMIBSvc();
	virtual ~DupMIBSvc();

	virtual ErrorCode init(int argc, char* argv[]);
	virtual ErrorCode start(void);
	virtual void stop(void);
	virtual void unInit(void);

private:
	int  usage(void);
	void version(void);
	int  parseCmdLine(int argc, char* argv[]);
	int  forwardConvert(std::string& dstSvc, std::string& srcData, std::string& srcSvc, std::string& parent);
	int  process(std::string& dstData, std::string& srcData, std::string & dstSvc, std::string & SvcSeqNum, MibUtility* howTo);
	int  splitData(std::string& restBeforeReplace, std::string& replaceRange, std::string& restAfterReplace, std::string& srcSliceData, std::string& dstService);
	
private:
	int _splitWithoutTs;
	std::string _svcStr;
	std::string _parent;
	std::string _srcMibFileData;
	std::string _svcInstanceId;
	std::string _srcMibFilename;
	std::string _dstMibFilename;
};
#endif//_DUP_MIB_SVC_HPP