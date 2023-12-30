// DupMIBSvc.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif//_MSC_VER

#include "TreeDupMIBSvc.hpp"
#include "getopt.h"

using namespace std;

void TreeDupMIBSvc::version(void) 
{
	std::cout << "Console for service version: " 
		<< 6 << "." 
		<< 1 << "." 
		<< 1 << "(build " 
		<< 1 << ")\n" 
		<< std::endl;
}

int TreeDupMIBSvc::parseCmdLine(int argc, char* argv[])
{
	int status = false;
	int ch = 0;
	if(1 == argc)
	{
	   version();
	   return  status;
	}

	if(8 > argc)
	{
		cerr<<"Command line input error"<<endl;
		return status;
	}

	status = true;
	while((ch = getopt(argc, argv, "f:p:s:n:o:c")) != EOF) 
	{
		if(ch == 's') 
		{
			_svcStr = optarg;
		}else if(ch == 'n') {
			_svcInstanceId = optarg;
			if (1 != _svcInstanceId.size())
			{
				std::cerr << "\tinvalid option: -n "<<_svcInstanceId <<std::endl;
				status = false;
				break;
			}
		}
		else if(ch == 'o') {
			_dstMibFilename = optarg;
			_srcMibFilename = argv[argc - 1];
		}else if(ch == 'c') {
			_splitWithoutTs = true;
		}else if(ch == 'f') {
			_parent = optarg;
		}else if(ch == 'p') {

		}else {
			std::cerr << "\tinvalid option" <<  std::endl;
			status = false;
			break;
		}
	}

	return status;
}

int TreeDupMIBSvc::usage(void)
{
	std::string outPut;
	outPut += "DupMIBSvc -s <ServiceName or ServiceOID> -n <SvcSeqNum> -o SvcXXX_SeqNum.MIB TianShan.MIB \n";
	outPut += " \t '-s' to specify a service type available in TianShan.MIB\n";
	outPut += " \t '-n' to specify the [0~9] sequence number of the service to duplicate as\n"; 
	outPut += " \t '-o' is the output new MIB file";

	cout<<outPut<<endl;

	return SUCCEED;
}

TreeDupMIBSvc::TreeDupMIBSvc()
:_splitWithoutTs(false)
{  
}

TreeDupMIBSvc::~TreeDupMIBSvc()
{  
}

ErrorCode TreeDupMIBSvc::init(int argc, char* argv[])
{
	ErrorCode initStatus = SUCCEED;
#ifdef DEBUG
	const int EXE_PATH_SIZE = 1024;
	char exePath[EXE_PATH_SIZE] = {0};
	getcwd(exePath, EXE_PATH_SIZE);
	cout<<"Current absolute path is "<<exePath<<endl;
#endif//DEBUG
	if (!parseCmdLine(argc, argv))
	{
		usage();
		initStatus = COMMAND_QUIT;
		return initStatus;
	}

	ifstream  srcFile;
	srcFile.open(_srcMibFilename.c_str(), ios::in | ios::binary);
	if (!srcFile)
	{
		initStatus = FUNCTION_FAILED;
		cerr<<"  Can not open file[\""<<_srcMibFilename<<"\"]";
		cerr<<"\terrorCode[\'"<<srcFile.rdstate()<<"\"]"<<endl;
		return initStatus;
	}

	srcFile.seekg(0, ios::end);
	int srcFileSize = srcFile.tellg();
	if (srcFileSize > 0)
	{															   
		_srcMibFileData.resize(srcFileSize);
		srcFile.seekg(0, ios::beg);
		srcFile.read((char *)_srcMibFileData.c_str(), srcFileSize);
	}else{
		initStatus = COMMAND_QUIT;
		cerr<<" File[\""<<_srcMibFilename<<"\"] is empty"<<endl;
		return initStatus;
	}

	srcFile.close();  

	return SUCCEED;
}

ErrorCode TreeDupMIBSvc::start(void)
{
	ofstream  dstFile;
	dstFile.open(_dstMibFilename.c_str(), ios::in | ios::out | ios::trunc);
	if (!dstFile)
	{	  
		cerr<<"Can not create file[\""<<_dstMibFilename<<"\"]";
		cerr<<"\terrorCode[\'"<<dstFile.rdstate()<<"\"]"<<endl;
		return FUNCTION_FAILED;
	}

#ifdef DEBUG
	size_t srcDataSize = _srcMibFileData.size();
#endif//DEBUG

	TreeMibUtility mibTo;
	std::string dstData;
	if(process(dstData, _srcMibFileData, _svcStr, _svcInstanceId, &mibTo))
	{ 
		dstFile << dstData;
		dstFile.close();
		cout<<"\tService process succeed,";
		cout<<" bytes["<<dstData.size()<<"] data write to file[\""<<_dstMibFilename<<"\"]"<<endl;
	}else{	
		dstFile.close();
		cout<<"\tservice process failed, no data write to file[\""<<_dstMibFilename<<"\"]"<<endl;
	}

	_srcMibFileData.clear();

	return SUCCEED;
}

void TreeDupMIBSvc::stop(void)
{  
}

void TreeDupMIBSvc::unInit(void)
{  
	cout<<"\tEnd of service"<<endl;
}	

int TreeDupMIBSvc::forwardConvert(std::string& dstSvc, std::string& srcData, std::string& srcSvc, std::string& parent)
{  //verdict ServiceName or ServiceOID, if it is ServiceOID, convent it to ServiceName
	int nRev         = false;
	int markConflict = 0;
	size_t currentPos = 0;
	size_t pos = srcData.find(srcSvc.c_str());
	if (std::string::npos == pos)
		cerr<<"\tServiceName or ServiceOID [\""<<srcSvc<<"\"] is not in file [\""<<_srcMibFilename<<"\"]"<<endl;

	std::string object("OBJECT");
	while(std::string::npos != pos)
	{
		currentPos = pos;
		size_t prevWord        = srcData.find_first_not_of("  ", currentPos + srcSvc.size());
		size_t prevTianShan    = srcData.rfind(parent, currentPos);
		size_t nextTianShan    = srcData.find(parent, currentPos);
		size_t prevDoubleQuoPos    = srcData.find_last_of("{", currentPos);
		size_t nextDoubleQuoPos    = srcData.find_first_of("{", currentPos);
		size_t revDoubleQuePos      = srcData.find_first_of("}", currentPos);
		size_t prevSpaceOrDoubleQue = srcData.find_first_of("  }", currentPos);
		size_t prevObjectStartPos   = srcData.rfind("OBJECT", currentPos);
		size_t nextObjectStartPos   = srcData.find("OBJECT",  currentPos);
		
		pos = srcData.find(srcSvc.c_str(), currentPos + srcSvc.size());
		if (std::string::npos == prevTianShan && std::string::npos == nextTianShan)//file is not TianShan.mib
		{
			cerr<<"\tError: ServiceName or ServiceOID [\""<<srcSvc<<"\"] is in file [\""<<_srcMibFilename<<"\"]; but parent[\""<<parent<<"\"] is not in"<<endl;
			break;
		}

		if (std::string::npos == revDoubleQuePos || std::string::npos == nextDoubleQuoPos)//ServiceName or ServiceOID can not be npos for {}
		{
			cerr<<"\tServiceName or ServiceOID [\""<<srcSvc<<"\"] format error in file [\""<<_srcMibFilename<<"\"]"<<endl;
			break;
		}
		
		if ( nextTianShan < revDoubleQuePos && currentPos < nextDoubleQuoPos &&
			 nextTianShan > nextDoubleQuoPos    && nextObjectStartPos > currentPos   && 
			 nextTianShan > nextObjectStartPos  && nextDoubleQuoPos > nextObjectStartPos  && 
		     nextDoubleQuoPos < revDoubleQuePos && srcSvc.size() == prevSpaceOrDoubleQue - currentPos) 
		{  //   ServiceName
			if (prevWord == nextObjectStartPos)// as svc name may be in notes
			{
				++markConflict;
				nRev = true;
				dstSvc = srcSvc;
			}
		}
		else if (prevTianShan < revDoubleQuePos  && prevTianShan > prevDoubleQuoPos && 
			     prevTianShan < currentPos       && prevObjectStartPos < prevTianShan &&
			     prevObjectStartPos < currentPos && srcSvc.size() == prevSpaceOrDoubleQue - currentPos )
		{	//ServiceOID convert to ServiceName
			size_t srcEndPos = srcData.find_last_not_of("  ", prevObjectStartPos - 1);
			size_t srcStartPos = srcData.find_last_of("  ", srcEndPos - 1) + 1;
			dstSvc = srcData.substr(srcStartPos, srcEndPos - srcStartPos + 1);
			nRev = true;
			++markConflict;
		}
	}

	if (markConflict > 1)
	{
		nRev = false;
		cerr<<"\tServiceName or ServiceOID [\""<<srcSvc<<"\"] conflict error in file [\""<<_srcMibFilename<<"\"]"<<endl;
	}

	return nRev;
}

int TreeDupMIBSvc::splitData(std::string& restBeforeReplace, std::string& restAfterReplace, std::string& srcSliceData, std::string& dstService, TreeRoot& root)
{
	int nRev = true;
	restAfterReplace = "\n\nEND";
	std::string moduleStr("MODULE-IDENTITY");
	std::string equalMarkStr("::=");
	size_t modulePos   = 0;
	size_t nextModulePos = 0;
	size_t lastNodePos = 0;
	while(std::string::npos != modulePos &&
		false != nRev)
	{
		std::string myName;
		std::string parent;
		std::string myNum;
		std::string restString;

		modulePos = srcSliceData.find(moduleStr, lastNodePos);//last MODULE-IDENTITY is the second tree node
		nextModulePos = srcSliceData.find(moduleStr, modulePos + moduleStr.size());
		lastNodePos = srcSliceData.find_last_not_of("  ", modulePos - 1);						 		
		size_t equalMarkPos    = srcSliceData.find(equalMarkStr, modulePos);
		size_t doubleQuoPos    = srcSliceData.find_first_of("{", equalMarkPos);
		size_t revDoubleQuePos = srcSliceData.find_first_of("}", doubleQuoPos);
		if (std::string::npos == lastNodePos ||
			std::string::npos == equalMarkPos ||
			std::string::npos == doubleQuoPos ||
			std::string::npos == revDoubleQuePos ||
			std::string::npos != nextModulePos  ||
			doubleQuoPos < modulePos)
		{
			lastNodePos = modulePos + 1;
			continue;
		}

		size_t myNamePos    = srcSliceData.find_last_of(" ", lastNodePos - 1) + 1;
		size_t myNameSize   = lastNodePos - myNamePos + 1;
		size_t parentPos    = srcSliceData.find_first_not_of("  ", doubleQuoPos + 1);
		size_t parentEnd    = srcSliceData.find_first_of("  ", parentPos + 1);
		size_t myNumPos     = srcSliceData.find_first_not_of("  ", parentEnd + 1);
		size_t nextSpacePos = srcSliceData.find_first_of("  ", myNumPos + 1);

		lastNodePos = revDoubleQuePos + 1;
		if (std::string::npos == myNamePos ||
			std::string::npos == parentPos ||
			std::string::npos == parentEnd ||
			std::string::npos == myNumPos)
		{
			continue;
		}
		
		restBeforeReplace  = srcSliceData.substr(0, myNamePos - 1);
#if defined(DEBUG)
		size_t size = restBeforeReplace.size();
#endif //DEBUG 	
	}

	std::string parents;
	TreeNode * dstNode = root.findNode(dstService);
	dstNode = root.findNode(dstNode->_parent);
	while (0 != dstNode)
	{
		std::string currentParent;
	    dstNode->toStrByCurrent(currentParent);
		parents.insert(0, currentParent);
		dstNode = root.findNode(dstNode->_parent);
	}											  

	restBeforeReplace += parents;

	return nRev;
}

int TreeDupMIBSvc::process(std::string& dstData, std::string& srcData, std::string& dstSvc, std::string& SvcSeqNum, TreeMibUtility* howTo)
{
	int nRev = false;
	std::string dstSvcConv;
	std::string replaceRange;
	std::string restBeforeReplace;
	std::string restAfterReplace;
	std::string parent("tianShanService");
	if (!_parent.empty())
		parent = _parent;

	_parent = parent;
	cout<<"\tService processing [===";
	if (!forwardConvert(dstSvcConv, srcData, dstSvc, parent))	
	{
		cerr<<"\r\tProcessing error[file data illegal], abort"<<endl;
        return nRev;
	}	

	try
	{
		cout<<"========";
		TreeRoot replaceNode;
		if (howTo->parseStr(replaceNode, srcData, dstSvcConv) &&
			howTo->replaceStr(replaceNode, dstSvcConv, SvcSeqNum) &&
			replaceNode._leafNotInTreeNode.empty())
		{
			cout<<"===============";
			TreeNode * dstNode = replaceNode.findNode(dstSvcConv);
			dstNode->toStrByReplace(replaceRange);
			if (!_splitWithoutTs)
			{		
				splitData(restBeforeReplace, restAfterReplace, srcData, dstSvcConv, replaceNode);
				dstData += restBeforeReplace;
				dstData += replaceRange;
				dstData += restAfterReplace;
			}else{
				dstData += replaceRange;
			}

			nRev = true;
			cout<<"===========]"<<endl;
		}else if (!replaceNode._leafNotInTreeNode.empty()){
			nRev = false;
			std::string errorNode;
			for(std::list<_TreeNode* >::iterator it = replaceNode._leafNotInTreeNode.begin(); it != replaceNode._leafNotInTreeNode.end(); it++)
			{
				_TreeNode *itVal = *it;
				itVal->toStrByReplace(errorNode);  
			}
			
			cerr<<"Error MIB file, the follow list of node not in tree"<<errorNode<<endl;
		}
	}
	catch (...)
	{
		nRev = false;
		cerr<<"\tCatch unknown exception while processing, abort"<<endl;
	} 

	return nRev;
}

int TreeMibUtility::parseTreeRoot(TreeNode& node, std::string& dst)
{
	int nRev = true;
	std::string moduleStr("MODULE-IDENTITY");
	std::string equalMarkStr("::=");
	size_t modulePos   = 0;
	size_t nextModulePos = 0;
	size_t lastNodePos = 0;
	while(std::string::npos != modulePos &&
		false != nRev)
	{
		std::string myName;
		std::string parent;
		std::string myNum;
		std::string restString;

		modulePos = dst.find(moduleStr, lastNodePos);//last MODULE-IDENTITY is the second tree node
		nextModulePos = dst.find(moduleStr, modulePos + moduleStr.size());
		lastNodePos = dst.find_last_not_of("  ", modulePos - 1);						 		
		size_t equalMarkPos    = dst.find(equalMarkStr, modulePos);
		size_t doubleQuoPos    = dst.find_first_of("{", equalMarkPos);
		size_t revDoubleQuePos = dst.find_first_of("}", doubleQuoPos);
		if (std::string::npos == lastNodePos ||
			std::string::npos == equalMarkPos ||
			std::string::npos == doubleQuoPos ||
			std::string::npos == revDoubleQuePos ||
			std::string::npos != nextModulePos  ||
			doubleQuoPos < modulePos)
		{
			lastNodePos = modulePos + 1;
			continue;
		}

		size_t myNamePos    = dst.find_last_of(" ", lastNodePos - 1) + 1;
		size_t myNameSize   = lastNodePos - myNamePos + 1;
		size_t parentPos    = dst.find_first_not_of("  ", doubleQuoPos + 1);
		size_t parentEnd    = dst.find_first_of("  ", parentPos + 1);
		size_t myNumPos     = dst.find_first_not_of("  ", parentEnd + 1);
		size_t nextSpacePos = dst.find_first_of("  ", myNumPos + 1);

		if (std::string::npos == myNamePos ||
			std::string::npos == parentPos ||
			std::string::npos == parentEnd ||
			std::string::npos == myNumPos)
		{
			lastNodePos = revDoubleQuePos + 1;
			continue;
		}

		restString = dst.substr(lastNodePos + 1, equalMarkPos - lastNodePos - 1);
		myName = dst.substr(myNamePos, myNameSize );
		parent = dst.substr(parentPos, parentEnd - parentPos);
		if (nextSpacePos > revDoubleQuePos)						   
			myNum  = dst.substr(myNumPos, revDoubleQuePos - myNumPos);
		else
			myNum  = dst.substr(myNumPos, nextSpacePos - myNumPos);

#if defined(DEBUG)
		size_t parentSize = parent.size();
		size_t myNumSize = myNum.size();
#endif //DEBUG
		lastNodePos = revDoubleQuePos + 1;
		node._parent = parent;
		node._myName = myName;
		nRev = node.insertNode(parent, myName, myNum, restString);
	}

	return nRev;
}

int TreeMibUtility::parseTreeObject(TreeNode& node, std::string& dst)
{
	int nRev = true;
	std::string objectStr("OBJECT");
	std::string equalMarkStr("::=");
	size_t objectPos   = 0;
	size_t lastNodePos = 0;
	while(std::string::npos != objectPos &&
		false != nRev)
	{
		std::string myName;
		std::string parent;
		std::string myNum;
		std::string restString;

		objectPos = dst.find(objectStr, lastNodePos);
		lastNodePos = dst.find_last_not_of("  ", objectPos - 1);						 		
		size_t equalMarkPos    = dst.find(equalMarkStr, objectPos);
		size_t doubleQuoPos    = dst.find_first_of("{", equalMarkPos);
		size_t revDoubleQuePos = dst.find_first_of("}", doubleQuoPos);
		if (std::string::npos == lastNodePos ||
			std::string::npos == equalMarkPos ||
			std::string::npos == doubleQuoPos ||
			std::string::npos == revDoubleQuePos ||
			doubleQuoPos < objectPos)
		{   // may not distinguish like "enterprises, Counter64, NOTIFICATION-TYPE, MODULE-IDENTITY, OBJECT-TYPE"  in MIB, but code"if (0 == _myName.compare(myName))" in insertNode would work this
			lastNodePos = objectPos + 1;
			continue;
		}

		size_t myNamePos    = dst.find_last_of(" ", lastNodePos - 1) + 1;
		size_t myNameSize   = lastNodePos - myNamePos + 1;
		size_t parentPos    = dst.find_first_not_of("  ", doubleQuoPos + 1);
		size_t parentEnd    = dst.find_first_of("  ", parentPos + 1);
		size_t myNumPos     = dst.find_first_not_of("  ", parentEnd + 1);
		size_t nextSpacePos = dst.find_first_of("  ", myNumPos + 1);
		size_t restStringPos = lastNodePos + 1;

		lastNodePos = revDoubleQuePos + 1;
		if (std::string::npos == myNamePos ||
			std::string::npos == parentPos ||
			std::string::npos == parentEnd ||
			std::string::npos == myNumPos)
		{
			continue;
		}

		restString = dst.substr(restStringPos, equalMarkPos - restStringPos - 1);
		myName = dst.substr(myNamePos, myNameSize );
		parent = dst.substr(parentPos, parentEnd - parentPos);
		if (nextSpacePos > revDoubleQuePos)						   
			myNum  = dst.substr(myNumPos, revDoubleQuePos - myNumPos);
		else
			myNum  = dst.substr(myNumPos, nextSpacePos - myNumPos);

#if defined(DEBUG)
		size_t parentSize = parent.size();
		size_t myNumSize = myNum.size();
#endif //DEBUG
		nRev = node.insertNode(parent, myName, myNum, restString);
	}

	return nRev;
}

int TreeMibUtility::parseTreeSequence(TreeNode& node, std::string& dst)
{
	int nRev = true;
	std::string sequence("SEQUENCE");
	std::string equalMarkStr("::=");
	size_t lastSeqPos = dst.find(sequence);;

	while (std::string::npos != lastSeqPos)
	{
		size_t seqPos  = lastSeqPos;
		size_t markPos = dst.find_last_not_of("  \n\r\t", seqPos - 1);
		size_t equalMarkDesire = dst.find_last_of(" \n\r\t", markPos - 1) + 1;
		size_t equalMarkStartPos    = dst.rfind(equalMarkStr, seqPos);
		size_t doubleQuoPos    = dst.find_first_of("{", equalMarkStartPos);
		size_t revDoubleQuePos = dst.find_first_of("}", equalMarkStartPos);
		size_t fistColumnStartPos = dst.find_first_not_of("  \n\r\t", doubleQuoPos + 1);
		size_t fistColumnEndPos   = dst.find_first_of("  \n\r\t", fistColumnStartPos + 1);
		lastSeqPos = dst.find(sequence, lastSeqPos + 1);
		if (std::string::npos == seqPos || std::string::npos == equalMarkStartPos || std::string::npos == doubleQuoPos ||
			std::string::npos == revDoubleQuePos || seqPos > doubleQuoPos || doubleQuoPos > revDoubleQuePos || 
			std::string::npos == fistColumnStartPos || std::string::npos == fistColumnEndPos || equalMarkDesire != equalMarkStartPos)
		{
			continue;
		}

		std::string fistColumn = dst.substr(fistColumnStartPos, fistColumnEndPos - fistColumnStartPos);
		TreeNode*   columnNode = node.findNode(fistColumn);
		if (0 != columnNode)
		{
			size_t seqSegmentStart = dst.find_last_of("\n\r", equalMarkStartPos) + 1;
			size_t seqSegmentEnd   = dst.find_first_of("\n\r", revDoubleQuePos);
			TreeNode* columnParentNode = node.findNode(columnNode->_parent);
			if(0 != columnParentNode && std::string::npos != seqSegmentEnd && std::string::npos != seqSegmentStart)
			{
				std::string seqSegment = dst.substr(seqSegmentStart, seqSegmentEnd - seqSegmentStart);
				columnParentNode->insertNode(columnParentNode->_parent, columnParentNode->_myName, columnParentNode->_myNum, columnParentNode->_rest, seqSegment);
			}
		}else{
			nRev = false;
			cerr<<"MIB SEQUENCE index error:["<<fistColumn<<"]"<<endl;
			break;
		}
	}

    return nRev;
}

int TreeMibUtility::parseTreeNotification(TreeNode& node, std::string& dst)
{   
	int nRev = true;
	std::string notifyStr("NOTIFICATION-TYPE");
	std::string statusInNode("STATUS");
	std::string equalMarkStr("::=");
	size_t notifyPos   = 0;
	size_t lastNodePos = 0;
	while(std::string::npos != notifyPos &&
		false != nRev)
	{
		std::string myName;
		std::string parent;
		std::string myNum;
		std::string restString;

		notifyPos = dst.find(notifyStr, lastNodePos);
		lastNodePos = dst.find_last_not_of("  ", notifyPos - 1);
		size_t statusInNodePos = dst.find(statusInNode, notifyPos);
		size_t equalMarkPos    = dst.find(equalMarkStr, notifyPos);
		size_t doubleQuoPos    = dst.find_first_of("{", equalMarkPos);
		size_t revDoubleQuePos = dst.find_first_of("}", doubleQuoPos);
		if (std::string::npos == lastNodePos ||	std::string::npos == equalMarkPos || std::string::npos == doubleQuoPos || 
			std::string::npos == revDoubleQuePos ||	std::string::npos == statusInNodePos || doubleQuoPos < notifyPos ||
			statusInNodePos > doubleQuoPos)
		{
			lastNodePos = notifyPos + 1;
			continue;
		}

		size_t myNamePos    = dst.find_last_of(" ", lastNodePos - 1) + 1;
		size_t myNameSize   = lastNodePos - myNamePos + 1;
		size_t parentPos    = dst.find_first_not_of("  ", doubleQuoPos + 1);
		size_t parentEnd    = dst.find_first_of("  ", parentPos + 1);
		size_t myNumPos     = dst.find_first_not_of("  ", parentEnd + 1);
		size_t nextSpacePos = dst.find_first_of("  ", myNumPos + 1);
		size_t restStringPos = lastNodePos + 1;

		lastNodePos = revDoubleQuePos + 1;
		if (std::string::npos == myNamePos ||
			std::string::npos == parentPos ||
			std::string::npos == parentEnd ||
			std::string::npos == myNumPos)
		{
			continue;
		}

		restString = dst.substr(restStringPos, equalMarkPos - restStringPos - 1);
		myName = dst.substr(myNamePos, myNameSize );
		parent = dst.substr(parentPos, parentEnd - parentPos);
		if (nextSpacePos > revDoubleQuePos)						   
			myNum  = dst.substr(myNumPos, revDoubleQuePos - myNumPos);
		else
			myNum  = dst.substr(myNumPos, nextSpacePos - myNumPos);

#if defined(DEBUG)
		size_t parentSize = parent.size();
		size_t myNumSize = myNum.size();
#endif //DEBUG
		nRev = node.insertNode(parent, myName, myNum, restString);
	}

	return true;
};

int TreeMibUtility::parseStr(TreeNode& node, std::string& dst, std::string& dstSvc)
{
	int nRoot   = parseTreeRoot(node, dst);
	int nObject = parseTreeObject(node, dst);
	int nSeq    = parseTreeSequence(node, dst);
	int nNotif  = parseTreeNotification(node, dst);

    int nRev    = nRoot && nObject && nSeq && nNotif;

	return nRev;
}

int TreeMibUtility::replaceStr(TreeNode& node, std::string& dst, std::string & SvcSeqNum)
{
	int nRev = true;
	TreeNode * dstNode = node.findNode(dst);

	if (0 != dstNode)
	{
		nRev = dstNode->replaceNode(dst, SvcSeqNum);
	}else{
		nRev = false;
	}

	return nRev;
}