#include "stdafx.h"
#include "DODAppImpl.h"
#include "FolderChannelImpl.h"
#include "DataPublisherImpl.h"
#include "global.h"
#include "ActiveFolderChannel.h"
#include <DODContentStore.h>
//////////////////////////////////////////////////////////////////////////
DataOnDemand::FolderChannelImpl::FolderChannelImpl()
{

}

DataOnDemand::FolderChannelImpl::~FolderChannelImpl()
{
/*	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName: %s]~FolderChannelImpl()",myInfo.name.c_str());*/	
}

bool 
DataOnDemand::FolderChannelImpl::init()
{
	/*
	long lastupdate;
    time(&lastupdate);
    myLastUpdate = lastupdate;
	*/
	return true;
}

void
DataOnDemand::FolderChannelImpl::notifyFullUpdate(
	const ::std::string& rootPath,
	bool clear,
	::Ice::Int groupId, 
	int verNumber,
	const Ice::Current& current)
{
	// updatemode= 1 , 6
	char grouptemp[255];
	::std::string _CacheDir;

	sprintf(grouptemp,"%s\\%d\\", myProps["Path"].c_str(), groupId);
//    sprintf(grouptemp,"%s//%d//", myProps["Path"].c_str(), groupId);


	if (  ( (_access (grouptemp, 0 )) != 0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,
		"[ChannelName: %s]notifyFullUpdate() no port match with groupID (%d) or  cache folder (%s)is not exist",
		myInfo.name.c_str(),groupId, grouptemp);
		return;
	}
    _CacheDir = grouptemp;

	bool ret ;
	if(clear)
	{
	    ret = DeleteDirectory(_CacheDir);
		
		if (!ret)
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannelName: %s]notifyFullUpdate() Delete Directory [%s] error",myInfo.name.c_str(),_CacheDir.c_str());
			
			return;
		}
		glog(ZQ::common::Log::L_INFO,
		     "[ChannelName: %s]notifyFullUpdate() DeleteDirectory [%s] success",myInfo.name.c_str(),_CacheDir.c_str());
		
		Sleep(3);
	}
	glog(ZQ::common::Log::L_INFO,
	  "[ChannelName: %s]notifyFullUpdate() Move all files to  cache  directory",
	  myInfo.name.c_str());

	if(!CopyAllFile(rootPath,_CacheDir))
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFullUpdate() Move all files(%s) to cache directory (%s) error",
			myInfo.name.c_str(),rootPath.c_str(),_CacheDir.c_str());
		return;
	}
	
	glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyFullUpdate() Move all files(%s) to cache directory (%s) success",
		myInfo.name.c_str(), rootPath.c_str(), _CacheDir.c_str());
	
/*	if(deleteRootPath)
	{
		ret = DeleteDirectory(rootPath, true);
		if (!ret) 
		{
			glog( ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFullUpdate::Delete remote Directory dir = %s "
			"Error", myInfo.name.c_str(), rootPath.c_str());		
			return;
		}
		glog( ZQ::common::Log::L_INFO,
			"[ChannelName: %s]notifyFullUpdate::Delete remote Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());
	}*/
    NotifyMuxItem(groupId,verNumber);
}

void
DataOnDemand::FolderChannelImpl::notifyPartlyUpdate(const ::std::string& rootPath,
						 const ::std::string& paths,
						 ::Ice::Int groupId,
						 int verNumber,
						 const Ice::Current& current)//updatemode = 1;
{
	 glog(ZQ::common::Log::L_INFO,
		 "[ChannelName: %s]notifyPartlyUpdate() Update sub folder remotePath (%s)",
		  myInfo.name.c_str(),rootPath.c_str());

    char grouptemp[255];
	::std::string _CacheDir;

	sprintf(grouptemp,"%s\\%d\\", myProps["Path"].c_str(), groupId);
 //   sprintf(grouptemp,"%s//%d//", myProps["Path"].c_str(), groupId);
	if (  ( (_access (grouptemp, 0 )) != 0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,
		"[ChannelName: %s]notifyFullUpdate() no port match with groupID (%d) or  cache folder(%s) is not exist",
		myInfo.name.c_str(), groupId, grouptemp);
		return;
	}
    _CacheDir = grouptemp;

     bool ret;
	 std::string strtemp,str1;

	 strtemp = _CacheDir + paths;
	 str1 = rootPath;

	 ret = DeleteDirectory(strtemp, true);
	 if (!ret)
	 {
	    glog(ZQ::common::Log::L_ERROR,
			"[ChannealName: %s]notifyPartlyUpdate() Move all files (%s) to Cache Directory (%s)",
			myInfo.name.c_str(), rootPath.c_str(), _CacheDir.c_str());
		 return;
	 }
	 glog( ZQ::common::Log::L_DEBUG,
		 "[ChannelName: %s]notifyPartlyUpdate() Delete remote directory (%s) success",
		 myInfo.name.c_str(), strtemp.c_str());

	strtemp = _CacheDir;

	if (!CopyAllFile(str1, strtemp))
	{ 	
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyPartlyUpdate() Copy all file to Cache Directory error",
			myInfo.name.c_str());
		return;
	}
    glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyPartlyUpdate() Move all files (%s) to Cache Directory(%s) success ",
		myInfo.name.c_str(), rootPath.c_str(), _CacheDir.c_str());

/*	if(deleteRootPath)
	{
		ret = DeleteDirectory(rootPath,true);
		if (!ret) 
		{
			glog( ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyPartlyUpdate::Delete remote Directory dir = \
			%s Error", myInfo.name.c_str(), rootPath.c_str());		
			return;
		}
		glog( ZQ::common::Log::L_INFO,
			"[ChannelName: %s]notifyPartlyUpdate::Delete remote Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());
	}*/

    NotifyMuxItem(groupId,verNumber);

	return ;
}

void
DataOnDemand::FolderChannelImpl::notifyFolderDeleted(
	const ::std::string& paths,
	::Ice::Int groupId, 
	int verNumber,
	const Ice::Current& current)//updatemode = 2;
{
    glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName: %s]notifyFolderDeleted() Delete Full Path (%s)",
		myInfo.name.c_str(), paths.c_str());

    char grouptemp[255];
	::std::string _CacheDir;

	sprintf(grouptemp,"%s\\%d\\", myProps["Path"].c_str(), groupId);
	  //sprintf(grouptemp,"%s//%d//", myProps["Path"].c_str(), groupId);

	if (  ( (_access (grouptemp, 0 )) != 0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFolderDeleted() no port match with groupID (%d) or  cache folder(%s) is not exist",
			myInfo.name.c_str(), groupId, grouptemp);
		return;
		return;
	}
    _CacheDir = grouptemp;

	bool ret;
	std::string str;
	str = _CacheDir;
	
	ret = DeleteDirectory(str);
	if (!ret)
	{
		glog( ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFolderDeleted() Delete Directory (%s) error", myInfo.name.c_str(), str.c_str());		 
		return;
	}
	 glog( ZQ::common::Log::L_INFO,
		 "[ChannelName: %s]notifyFolderDeleted() Delete Directory (%s) success",
		 myInfo.name.c_str(), str.c_str());

      NotifyMuxItem(groupId,verNumber);

	return;
}

void
DataOnDemand::FolderChannelImpl::notifyFileDeleted(
	const ::std::string& paths,
	::Ice::Int groupId, 
	int verNumber,
	const Ice::Current& current)//updatemode = 3;
{
    glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyFileDeleted() Delete directory or file (%s)",
		myInfo.name.c_str(), paths.c_str());

    char grouptemp[255];
	::std::string _CacheDir;

	sprintf(grouptemp,"%s\\%d\\", myProps["Path"].c_str(), groupId);
	 // sprintf(grouptemp,"%s//%d//", myProps["Path"].c_str(), groupId);

	if (  ( (_access (grouptemp, 0 )) != 0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileDeleted() no port match with groupID (%d) or  cache folder(%s) is not exist",
			myInfo.name.c_str(), groupId, grouptemp);
		return;
		return;
	}
    _CacheDir = grouptemp;

	bool ret;
	std::string str;
	str = _CacheDir + paths;
	ret = DeleteFileA(str.c_str());
	if (!ret)
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileDeleted() Delete DIR or File (%s) error",
			 myInfo.name.c_str(), str.c_str());
		return;
	}
	glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyFileDeleted() Delete DIR or File (%s) success",
		myInfo.name.c_str(), str.c_str());

    NotifyMuxItem(groupId,verNumber);
}

void
DataOnDemand::FolderChannelImpl::notifyFileModified(const ::std::string& rootPath,
						 const ::std::string& paths,
						 ::Ice::Int groupId, 
						 int verNumber,
						 const Ice::Current& current)//updatemode = 4;
{
	glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyFileModified() file modify (%s)",
		myInfo.name.c_str(), paths.c_str());

	char grouptemp[255];
	::std::string _CacheDir;
	
	sprintf(grouptemp,"%s\\%d\\", myProps["Path"].c_str(), groupId);
//  sprintf(grouptemp,"%s//%d//", myProps["Path"].c_str(), groupId);
	
	if (  ( (_access (grouptemp, 0 )) != 0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileModified() no port match with groupID (%d) or  cache folder(%s) is not exist",
			myInfo.name.c_str(), groupId, grouptemp);
		return;
	}
    _CacheDir = grouptemp;

	if(!CopyAllFile(rootPath, _CacheDir))
	{
	    glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileModified() Move all files (%s) to cache directory (%s) error",
			myInfo.name.c_str(), rootPath.c_str(), _CacheDir.c_str());
		return;
	}
    glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyFileModified() Move all files (%s) to cache directory (%s) success",
		myInfo.name.c_str(),rootPath.c_str(), _CacheDir.c_str());
	
/*	if(deleteRootPath)
	{
		bool ret = DeleteDirectory(rootPath, true);
		if (!ret) 
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannelName: %s]notifyFileModified::Delete Directory dir = %s Error",
				myInfo.name.c_str(), rootPath.c_str());		 
			return ;
		}
		glog(ZQ::common::Log::L_DEBUG,
			"[ChannelName: %s]notifyFileModified::Delete Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());	
	}*/
    NotifyMuxItem(groupId,verNumber);
}

void
DataOnDemand::FolderChannelImpl::notifyFileAdded(
	const ::std::string& rootPath,
	const ::std::string& paths,
	::Ice::Int groupId, 
	int verNumber,
	const Ice::Current& current)//updatemode = 5;
{
    glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName: %s]notifyFileAdded() add files rootpath(%s) path(%s)",
		myInfo.name.c_str(), rootPath.c_str(), paths.c_str());

    char grouptemp[255];
	::std::string _CacheDir;

	sprintf(grouptemp,"%s\\%d\\", myProps["Path"].c_str(), groupId);
	//sprintf(grouptemp,"%s//%d//", myProps["Path"].c_str(), groupId);

	if (  ( (_access (grouptemp, 0 )) != 0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileAdded() no port match with groupID (%d) or  cache folder(%s) is not exist",
			myInfo.name.c_str(), groupId, grouptemp);
		return;
	}
    _CacheDir = grouptemp;

	bool ret;
	std::string strdest, srcpath;
	srcpath = rootPath + paths;
	strdest = _CacheDir + paths;
	ret = CopyFileA(srcpath.c_str(), strdest.c_str(), false);

	if (!ret)
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileAdded() Copy file error filename (%s) error",
			myInfo.name.c_str(), srcpath.c_str());
		return;
	}
	glog(ZQ::common::Log::L_INFO,
		"[ChannelName: %s]notifyFileAdded() Copy file (%s) success!",
		myInfo.name.c_str(), strdest.c_str());

/*	if(deleteRootPath)
	{
		bool ret = DeleteDirectory(rootPath,true);
		if (!ret) 
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannelName: %s]notifyFileAdded::Delete remote Directory dir = %s Error",
				myInfo.name.c_str(), rootPath.c_str());		 
			return ;
		}
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName: %s]notifyFileAdded::Delete remote Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());	
	}*/
	
    NotifyMuxItem(groupId,verNumber);

	return ;
}
::std::string 
DataOnDemand::FolderChannelImpl::getContentName(::Ice::Int groupId,const Ice::Current&)
{
	DataOnDemand::GroupIdContentName::iterator it = myGroupCName.find(groupId);
	if (it == myGroupCName.end())
		throw TianShanIce::InvalidParameter();

	return it->second;
}
void
DataOnDemand::FolderChannelImpl::setContentName(::Ice::Int groupId,
												const ::std::string&  contentname ,
												const Ice::Current&)
{	
    myGroupCName[groupId] = contentname;
}
void
DataOnDemand::FolderChannelImpl::NotifyMuxItem(int groupId, int verNumber)
{	
    ActiveSharedFolderChannel* pAShareFoldChanel = (ActiveSharedFolderChannel*)getActiveChannel();
	if(pAShareFoldChanel == NULL)
		return;
	pAShareFoldChanel->NotifyMuxItem(groupId,verNumber);
}