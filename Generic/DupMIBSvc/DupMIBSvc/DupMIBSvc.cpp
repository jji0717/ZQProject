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

#include "DupMIBSvc.hpp"
#include "getopt.h"

using namespace std;

void DupMIBSvc::version(void) 
{
	std::cout << "Console for service version: " 
		<< 6 << "." 
		<< 1 << "." 
		<< 1 << "(build " 
		<< 1 << ")\n" 
		<< std::endl;
}

int DupMIBSvc::parseCmdLine(int argc, char* argv[])
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

int DupMIBSvc::usage(void)
{
	std::string outPut;
	outPut += "DupMIBSvc -s <ServiceName or ServiceOID> -n <SvcSeqNum> -o SvcXXX_SeqNum.MIB TianShan.MIB \n";
	outPut += " \t '-s' to specify a service type available in TianShan.MIB\n";
	outPut += " \t '-n' to specify the [0~9] sequence number of the service to duplicate as\n"; 
	outPut += " \t '-o' is the output new MIB file";

	cout<<outPut<<endl;

	return SUCCEED;
}

DupMIBSvc::DupMIBSvc()
:_splitWithoutTs(false)
{  
}

DupMIBSvc::~DupMIBSvc()
{  
}

ErrorCode DupMIBSvc::init(int argc, char* argv[])
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

ErrorCode DupMIBSvc::start(void)
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

	MibUtility mibTo;
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

void DupMIBSvc::stop(void)
{  
}

void DupMIBSvc::unInit(void)
{  
	cout<<"\tEnd of service"<<endl;
}	

int DupMIBSvc::forwardConvert(std::string& dstSvc, std::string& srcData, std::string& srcSvc, std::string& parent)
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
			cerr<<"\tServiceName or ServiceOID [\""<<srcSvc<<"\"] is in file [\""<<_srcMibFilename<<"\"]"<<";but file is illegal"<<endl;
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

int DupMIBSvc::splitData(std::string& restBeforeReplace, std::string& replaceRange, std::string& restAfterReplace, std::string& srcSliceData, std::string& dstService)
{
	int nRev = false;
	std::string newLineSymbol("\r\n");
	std::string tianShanService(_parent);
	std::string tianShanComponent(_parent);

	//restBeforReplace
	size_t tsComponentEndPos = srcSliceData.find(tianShanComponent) + tianShanComponent.size();
	size_t tsNextWordPos   =  srcSliceData.find_first_of("}", tsComponentEndPos + 1);
	if (std::string::npos == tsComponentEndPos || std::string::npos == tsNextWordPos)
		return nRev;
	else
		restBeforeReplace = srcSliceData.substr(0, tsNextWordPos + 1);

	//replaceRange
	size_t dstServicePos       = srcSliceData.find(dstService.c_str());
	size_t dstPrevLineEndPos   = srcSliceData.find_last_of("\r\n", dstServicePos - 1);
	size_t dstTianShanStartPos = srcSliceData.find(tianShanService, dstServicePos);
	size_t dstNextTianShanStartPos = srcSliceData.find(tianShanService, dstTianShanStartPos + tianShanService.size());

	if (std::string::npos == dstTianShanStartPos)
	{
		nRev =false;
	}else if (std::string::npos == dstNextTianShanStartPos)	{
		nRev = true;
		replaceRange   = srcSliceData.substr(dstPrevLineEndPos);
	}else{
		size_t dstServieEndPos = srcSliceData.find_last_of("}", dstNextTianShanStartPos);
		size_t dstNextLineEndPos = srcSliceData.find_first_of("\r\n", dstServieEndPos + 1) + newLineSymbol.size();
		if (std::string::npos != dstServieEndPos && std::string::npos != dstNextLineEndPos)
		{
		   nRev = true;
		   replaceRange = srcSliceData.substr(dstPrevLineEndPos, dstNextLineEndPos - dstPrevLineEndPos);
		   restAfterReplace = "\r\nEND";//restAfterReplace
		}
	} 

	return nRev;
}

int DupMIBSvc::process(std::string& dstData, std::string& srcData, std::string& dstSvc, std::string& SvcSeqNum, MibUtility* howTo)
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
	if (!forwardConvert(dstSvcConv, srcData, dstSvc, parent) ||
		!splitData(restBeforeReplace, replaceRange, restAfterReplace, srcData, dstSvcConv))	
	{
		cerr<<"\r\tProcessing error[file data illegal], abort"<<endl;
        return nRev;
	}	

	try
	{
		cout<<"========";
		Node replaceNode(parent, dstSvcConv);
		if (howTo->parseStr(replaceNode, replaceRange, dstSvcConv) &&
			howTo->replaceStr(replaceNode, replaceRange, SvcSeqNum))
		{
			cout<<"===============]"<<endl;
			nRev = true;
			if (!_splitWithoutTs)
			{  
				dstData += restBeforeReplace;
				dstData += replaceRange;
				dstData += restAfterReplace;
			}else{
				dstData += replaceRange;
			}
		}
	}
	catch (...)
	{
		nRev = false;
		cerr<<"\tCatch unknown exception while processing, abort"<<endl;
	} 

	return nRev;
}

int MibUtility::parseStr(Node& node, std::string& dst, std::string& dstSvc)
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
		{
			lastNodePos = objectPos + 1;
			continue;
		}

		size_t myNamePos    = dst.find_last_of(" ", lastNodePos - 1) + 1;
		size_t myNameSize   = lastNodePos - myNamePos + 1;
		size_t parentPos    = dst.find_first_not_of("  ", doubleQuoPos + 1);
		size_t parentEnd    = dst.find_first_of("  ", parentPos + 1);
		size_t myNumPos     = dst.find_first_not_of("  ", parentEnd + 1);
		size_t nextSpacePos = dst.find_first_of("  ", myNumPos + 1);

		lastNodePos = revDoubleQuePos + 1;
		if (std::string::npos == myNamePos ||
			std::string::npos == parentPos ||
			std::string::npos == parentEnd ||
			std::string::npos == myNumPos)
		{
			continue;
		}

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
		nRev = node.insertNode(parent, myName, myNum);
	}

	_dstSvc = dstSvc;
	_dstSvcParent = node._parent;
	return nRev;
}

int MibUtility::replaceStr(Node& node, std::string& dst, std::string & SvcSeqNum)
{
	int nRev = true;

	if(!replaceCurrentNode(node, dst, SvcSeqNum, _dstSvc))
	{
		nRev = false;
		cerr<<"exec error at file node parent:"<<node._parent<<" my name:"<<node._myName<<" my number:"<<node._myNum;
		return nRev;
	}				

	for (std::list<Node >::iterator it = node._son.begin(); it != node._son.end(); ++it)
	{
		if (!this->replaceStr(*it, dst, SvcSeqNum))
		{
			nRev = false;
			cerr<<"exec error at file node parent:"<<it->_parent<<" my name:"<<it->_myName<<" my number:"<<it->_myNum;
			break;
		}
	}

	return nRev;
}

int MibUtility::replaceCurrentNode(Node& node, std::string& dst, std::string & SvcSeqNum, std::string& dstSvc)
{
	int nRev = true;
	std::string objectStr("OBJECT");
	std::string equalMarkStr("::=");
	size_t objectPos   = 0;
	size_t lastNodePos = dst.find(node._myName);
	while(std::string::npos != objectPos &&	false != nRev)
	{
		std::string myName;
		std::string parent;
		std::string myNum; 

		// common node
		objectPos = dst.find(objectStr, lastNodePos);
		lastNodePos = dst.find_last_not_of("  ", objectPos - 1);						 		
		size_t equalMarkPos    = dst.find(equalMarkStr, objectPos);
		size_t doubleQuoPos    = dst.find_first_of("{", equalMarkPos);
		size_t revDoubleQuePos = dst.find_first_of("}", doubleQuoPos);
		if (std::string::npos == lastNodePos ||	 std::string::npos == equalMarkPos ||
			std::string::npos == doubleQuoPos || std::string::npos == revDoubleQuePos ||
			doubleQuoPos < objectPos)
		{
			lastNodePos = objectPos + 1;
			continue;
		}

		size_t myNamePos    = dst.find_last_of(" ", lastNodePos - 1) + 1;
		size_t myNameSize   = lastNodePos - myNamePos + 1;
		size_t parentPos    = dst.find_first_not_of("  ", doubleQuoPos + 1);
		size_t parentEnd    = dst.find_first_of("  ", parentPos + 1);
		size_t myNumPos     = dst.find_first_not_of("  ", parentEnd + 1);
		size_t nextSpacePos = dst.find_first_of("  ", myNumPos + 1);

		lastNodePos = revDoubleQuePos + 1;
		if (std::string::npos == myNamePos || std::string::npos == parentPos ||
			std::string::npos == parentEnd || std::string::npos == myNumPos)
		{
			continue;
		}

		myName = dst.substr(myNamePos, myNameSize );
		parent = dst.substr(parentPos, parentEnd - parentPos);
		if (nextSpacePos > revDoubleQuePos)						   
			myNum  = dst.substr(myNumPos, revDoubleQuePos - myNumPos);
		else
			myNum  = dst.substr(myNumPos, nextSpacePos - myNumPos);

		if (0 == myName.compare(node._myName) && 0 == parent.compare(node._parent))
		{
			if (0 != node._parent.compare(_dstSvcParent))
			{
				std::string replaceMyParent = (0 == SvcSeqNum.compare("0")) ? parent : (parent + SvcSeqNum); //if ServiceOid = 0, ignore
				dst.replace(parentPos, parent.size(), replaceMyParent);
			}else{
				if(myNum.size() > 1) //myNum.size() - 2
					dst.replace(myNumPos + myNum.size() - 2, SvcSeqNum.size(), SvcSeqNum);  //just replace service oid tens digit, as source mib instance id number would not be zero
			}

			std::string replaceMyName = (0 == SvcSeqNum.compare("0")) ? myName : (myName + SvcSeqNum); //if ServiceOid = 0, ignore;
			dst.replace(myNamePos, myName.size(), replaceMyName);
		}														 

		nRev = replaceMibTable(node, dst, SvcSeqNum);
#if defined(DEBUG)
		size_t parentSize = parent.size();
		size_t myNumSize = myNum.size();
#endif //DEBUG
	}

	return nRev;
}
int MibUtility::replaceMibTable(Node& node, std::string& dst, std::string & SvcSeqNum)
{
	//table node => INDEX => SEQUENCE
	int nRev = true;
	std::string sequence("SEQUENCE");
	size_t seqPos = dst.find(sequence);
	size_t seqMyName = dst.find(node._myName);
	while (std::string::npos != seqPos && std::string::npos != seqMyName)
	{	
#if defined(DEBUG)
		size_t myNameSize = node._myName.size();
#endif //DEBUG
		size_t seqMyNameEnd       = dst.find_first_of("  \r\n", seqMyName);
		size_t seqDoubleQuoPos    = dst.find_first_of("{", seqPos);
		size_t seqRevDoubleQuePos = dst.find_first_of("}", seqPos);
		if (string::npos == seqDoubleQuoPos ||	string::npos == seqRevDoubleQuePos || string::npos == seqMyNameEnd )
			break;

		if (seqMyNameEnd < seqMyName ||	seqDoubleQuoPos > seqMyName ||	seqRevDoubleQuePos < seqMyNameEnd )
		{    //if after INDEX, step next 
			std::string index("INDEX");
			size_t mayIndexEnd = dst.find_last_not_of("  {", seqMyName - 1);
			size_t indexStart  = dst.rfind(index, seqMyName);
#ifdef DEBUG
			size_t indexSize = index.size();
			size_t indexEnd  = indexSize + indexStart;
#endif//DEBUG
			if (std::string::npos != mayIndexEnd &&	std::string::npos != indexStart &&	indexStart + index.size() == mayIndexEnd + 1)
			{
				if (node._myName.size() == seqMyNameEnd - seqMyName) //replace after INDEX
				{
					std::string replaceIndexMyName = (0 == SvcSeqNum.compare("0")) ? node._myName : (node._myName + SvcSeqNum);
					dst.replace(seqMyName, node._myName.size(), replaceIndexMyName);
				}

				seqMyName = dst.find(node._myName, seqMyName + node._myName.size());
			}else{ 	
				seqPos = dst.find(sequence, seqPos + sequence.size()); 
			}

			continue; 
		}

		if (node._myName.size() == seqMyNameEnd - seqMyName)
		{   //replace after SEQUENCE
			std::string replaceSeqMyName = (0 == SvcSeqNum.compare("0")) ? node._myName : (node._myName + SvcSeqNum); //if ServiceOid = 0, ignore
			dst.replace(seqMyName, node._myName.size(), replaceSeqMyName);
		}

		break;
	}

	return nRev;
}