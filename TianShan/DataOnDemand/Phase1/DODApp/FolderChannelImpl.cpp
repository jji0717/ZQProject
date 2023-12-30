#include "stdafx.h"
#include "DODAppImpl.h"
#include "FolderChannelImpl.h"
#include "DataPublisherImpl.h"
#include "global.h"

//////////////////////////////////////////////////////////////////////////
DataOnDemand::FolderChannelImpl::FolderChannelImpl()
{

}

DataOnDemand::FolderChannelImpl::~FolderChannelImpl()
{
/*	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]~FolderChannelImpl()",myInfo.name.c_str());*/	
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
	const Ice::Current& current)
{
	// updtamode= 1 , 6
	
	::std::string _CacheDir = myProps["Path"];
	bool ret ;
	if(clear)
	{
	    ret = DeleteDirectory(_CacheDir);
		
		if (!ret)
		{
			glog(ZQ::common::Log::L_ERROR,
		    	"[ChannelName = %s]notifyFullUpdate: DeleteDirectory error, \
		    	CachingDir = %s",myInfo.name.c_str(),_CacheDir.c_str());
			
			return;
		}
		glog(ZQ::common::Log::L_DEBUG,
		     "[ChannelName = %s]notifyFullUpdate: DeleteDirectory success, \
	       	 CachingDir = %s",myInfo.name.c_str(),_CacheDir.c_str());
		
		Sleep(3);
	}
	glog(ZQ::common::Log::L_DEBUG,
	  "[ChannelName = %s]notifyFullUpdate: MoveAllFile to  Caching  directory",
	  myInfo.name.c_str());

	if(!CopyAllFile(rootPath,_CacheDir))
	{
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName = %s]notifyFullUpdate: MoveAllFile to Cache Directory \
			error,CachingDir = %s, rootPath = %s",
			myInfo.name.c_str(),_CacheDir.c_str(),rootPath.c_str());
		return;
	}
	
	glog(ZQ::common::Log::L_INFO,
		"[ChannelName = %s]notifyFullUpdate: MoveAllFile to Cache Directory \
		success,CachingDir = %s, rootPath = %s",
		myInfo.name.c_str(),_CacheDir.c_str(),rootPath.c_str());
	
/*	if(deleteRootPath)
	{
		ret = DeleteDirectory(rootPath, true);
		if (!ret) 
		{
			glog( ZQ::common::Log::L_ERROR,
			"[ChannelName = %s]notifyFullUpdate::Delete remote Directory dir = %s \
			Error", myInfo.name.c_str(), rootPath.c_str());		
			return;
		}
		glog( ZQ::common::Log::L_INFO,
			"[ChannelName = %s]notifyFullUpdate::Delete remote Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());
	}*/
	NotityMuxItem(groupId);
}

void
DataOnDemand::FolderChannelImpl::notifyPartlyUpdate(const ::std::string& rootPath,
						 const ::std::string& paths,
						 ::Ice::Int groupId, 
						 const Ice::Current& current)//updatamode = 1;
{
	 glog(ZQ::common::Log::L_DEBUG,
		 "[ChannelName = %s]notifyPartlyUpdate: UpdateSubfolder remotePath = %s!",
		  myInfo.name.c_str(),rootPath.c_str());
     ::std::string _CacheDir = myProps["Path"];

     bool ret;
	 std::string strtemp,str1;

	 glog(ZQ::common::Log::L_DEBUG,
		 "[ChannelName = %s]notifyPartlyUpdate: remotePath = %s",
		 myInfo.name.c_str(),rootPath.c_str());

	 strtemp = _CacheDir + paths;
	 str1 = rootPath;

	 ret = DeleteDirectory(strtemp, true);
	 if (!ret)
	 {
	    glog(ZQ::common::Log::L_ERROR,
			"[ChannealName = %s]notifyPartlyUpdate: MoveAllFile to Cache \
			Directory error,CachingDir = %s, rootPath = %s",
			myInfo.name.c_str(),_CacheDir.c_str(),rootPath.c_str());
		 return;
	 }
	 glog( ZQ::common::Log::L_DEBUG,
		 "[ChannelName = %s]notifyPartlyUpdate::Delete remote Directory dir \
		 = %s OK", myInfo.name.c_str(), strtemp.c_str());

	strtemp = _CacheDir;

    ret = CopyAllFile(str1, strtemp);

	if (!ret)
	{ 	
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName = %s]notifyPartlyUpdate:Copy allfile to Cache \
			Directory error!",myInfo.name.c_str());
		return;
	}
    glog(ZQ::common::Log::L_ERROR,
		"[ChannealName = %s]notifyPartlyUpdate: MoveAllFile to Cache Directory \
		success,CachingDir = %s, rootPath = %s",
		myInfo.name.c_str(),_CacheDir.c_str(),rootPath.c_str());

/*	if(deleteRootPath)
	{
		ret = DeleteDirectory(rootPath,true);
		if (!ret) 
		{
			glog( ZQ::common::Log::L_ERROR,
			"[ChannelName = %s]notifyPartlyUpdate::Delete remote Directory dir = \
			%s Error", myInfo.name.c_str(), rootPath.c_str());		
			return;
		}
		glog( ZQ::common::Log::L_INFO,
			"[ChannelName = %s]notifyPartlyUpdate::Delete remote Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());
	}*/

	NotityMuxItem(groupId);

	return ;
}

void
DataOnDemand::FolderChannelImpl::notifyFolderDeleted(
	const ::std::string& paths,
	::Ice::Int groupId, 
	const Ice::Current& current)//updatamode = 2;
{
    glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]notifyFolderDeleted: DeleteFullPath!",
		myInfo.name.c_str());

    ::std::string _CacheDir = myProps["Path"];

	bool ret;
	std::string str;
	str = _CacheDir;
	
	ret = DeleteDirectory(str);
	 if (!ret)
	 {
		 glog( ZQ::common::Log::L_ERROR,
			 "[ChannelName = %s]notifyFolderDeleted::Delete Directory dir = %s \
			 Error", myInfo.name.c_str(), str.c_str());		 
		 return;
	 }
	 glog( ZQ::common::Log::L_INFO,
		 "[ChannelName = %s]notifyFolderDeleted::Delete  Directory dir = %s OK",
		 myInfo.name.c_str(), str.c_str());

	 NotityMuxItem(groupId);

	return;
}

void
DataOnDemand::FolderChannelImpl::notifyFileDeleted(
	const ::std::string& paths,
	::Ice::Int groupId, 
	const Ice::Current& current)//updatamode = 3;
{
    glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]notifyFileDeleted: DeleteDIRorFile!",
		myInfo.name.c_str());
   ::std::string _CacheDir = myProps["Path"];

	bool ret;
	std::string str;
	str = _CacheDir + paths;
	ret = DeleteFileA(str.c_str());
	if (!ret)
	{
		glog(ZQ::common::Log::L_DEBUG,
			"[ChannelName = %s]notifyFileDeleted: DeleteDIRorFile dir = %s \
			error!",myInfo.name.c_str(), str.c_str());
		return;
	}
	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]notifyFileDeleted: DeleteDIRorFile success!",
		myInfo.name.c_str(), str.c_str());

	NotityMuxItem(groupId);
	return ;
}

void
DataOnDemand::FolderChannelImpl::notifyFileModified(const ::std::string& rootPath,
						 const ::std::string& paths,
						 ::Ice::Int groupId, 
						 const Ice::Current& current)//updataMode = 4;
{
	::std::string _CacheDir = myProps["Path"];
	if(!CopyAllFile(rootPath, _CacheDir))
	{
	    glog(ZQ::common::Log::L_ERROR,
			"[ChannealName = %s]notifyFileModified: MoveAllFile to Cache \
			Directory error,CachingDir = %s, rootPath = %s",
			myInfo.name.c_str(),_CacheDir.c_str(),rootPath.c_str());
		return;
	}
    glog(ZQ::common::Log::L_ERROR,
		"[ChannealName = %s]notifyFileModified: MoveAllFile to Cache Directory \
		success,CachingDir = %s, rootPath = %s",
		myInfo.name.c_str(),_CacheDir.c_str(),rootPath.c_str());
	
/*	if(deleteRootPath)
	{
		bool ret = DeleteDirectory(rootPath, true);
		if (!ret) 
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannelName = %s]notifyFileModified::Delete Directory dir = %s Error",
				myInfo.name.c_str(), rootPath.c_str());		 
			return ;
		}
		glog(ZQ::common::Log::L_DEBUG,
			"[ChannelName = %s]notifyFileModified::Delete Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());	
	}*/
	
	NotityMuxItem(groupId);
}

void
DataOnDemand::FolderChannelImpl::notifyFileAdded(
	const ::std::string& rootPath,
	const ::std::string& paths,
	::Ice::Int groupId, 
	const Ice::Current& current)//updata = 5;
{
    glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]notifyFileDeleted: DeleteDIRorFile!",
		myInfo.name.c_str());

    ::std::string _CacheDir = myProps["Path"];
	bool ret;
	std::string strdest, srcpath;
	srcpath = rootPath + paths;
	strdest = _CacheDir + paths;
	ret = CopyFileA(srcpath.c_str(), strdest.c_str(), false);

	if (!ret)
	{
		glog(ZQ::common::Log::L_DEBUG,
			"[ChannelName = %s]notifyFileAdded: Copyfile error filename = %s \
			error!",myInfo.name.c_str(), srcpath.c_str());
		return;
	}
	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]notifyFileAdded: Copyfile success!",
		myInfo.name.c_str(), strdest.c_str());

/*	if(deleteRootPath)
	{
		bool ret = DeleteDirectory(rootPath,true);
		if (!ret) 
		{
			glog(ZQ::common::Log::L_ERROR,
				"[ChannelName = %s]notifyFileAdded::Delete remote Directory dir = %s Error",
				myInfo.name.c_str(), rootPath.c_str());		 
			return ;
		}
		glog(ZQ::common::Log::L_ERROR,
			"[ChannelName = %s]notifyFileAdded::Delete remote Directory dir = %s OK",
			myInfo.name.c_str(), rootPath.c_str());	
	}*/

	NotityMuxItem(groupId);
	return ;
}
bool DataOnDemand::FolderChannelImpl::NotityMuxItem(int groupId)
{
	DestLinks::iterator iter;
    ::DataOnDemand::DestInfo destinfo;
	
    ::Ice::Identity ident;
	::Ice::ObjectPrx  objprx;
	::DataOnDemand::MuxItemPrx muxitemprx;
	try
	{
		for(iter = myDestLinks.begin(); iter != myDestLinks.end(); iter++)
		{		
			destinfo = iter->second.dest->getInfo();
			if(destinfo.groupId == groupId)
			{
				ident = createMuxItemIdentity(configSpaceName,(*iter).first, 
					myInfo.name);

				objprx = createObjectWithEndPoint(DataPublisherImpl::_ic,ident,
					DataPublisherImpl::_strmSvcEndPoint);
				muxitemprx = ::DataOnDemand::MuxItemPrx::checkedCast(objprx);

				glog(ZQ::common::Log::L_INFO, 
					"[%s, %s]::UpdateChannel will start!", 
					(*iter).first.c_str(), myInfo.name.c_str());
				
				std::string filename ="";
					muxitemprx->notifyFullUpdate(filename);
				
				glog(ZQ::common::Log::L_INFO, 
					"[%s, %s]::Update operation end!",
					(*iter).first.c_str(), myInfo.name.c_str());
			}
		}
	}
	catch(const Ice::ObjectNotExistException&) 
	{
		glog(ZQ::common::Log::L_CRIT, 
			"FolderChannelImpl::NotityMuxItem\tcannt connect to streamer");
		return false;
	}
	catch (const ::Ice::Exception & ex) 
	{
		glog(ZQ::common::Log::L_ERROR,
			"FolderChannelImpl::NotityMuxItem: Ice::Exception errorcode = %s",
			ex.ice_name().c_str());
		return false;
    } 

	return true;
}
