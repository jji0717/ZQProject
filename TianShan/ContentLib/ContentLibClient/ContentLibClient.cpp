// ContentLibClient.cpp : Defines the entry point for the console application.
//

#include <sstream>
#include <cstdio>
#include "getopt.h"
#include "strHelper.h"
#include "TimeUtil.h"

#include "ZQResource.h"
#include "ContentLibClient.h"
#include "SystemUtils.h"

#define TRY_BEGIN \
	try {
#define TRY_END \
	} \
	catch(const TianShanIce::InvalidParameter& e) { \
	std::cerr << e.message << std::endl; \
	return; \
	} \
	catch(const TianShanIce::InvalidStateOfArt& e) { \
	std::cerr << e.message << std::endl; \
	return; \
	} \
	catch(const TianShanIce::NotSupported& e) { \
	std::cerr << e.message << std::endl; \
	return; \
	} \
	catch(const Ice::Exception& e) { \
	std::cerr << e.ice_name() << std::endl; \
	return; \
	} \
	catch(const char* str) { \
	std::cerr << str << std::endl; \
	} \
	catch(...) { \
	std::cerr << "unknown error" << std::endl; \
	}

namespace {

//	const char* BITRATE = "bitrate";
//	const char* SRCTYPE = "sourceType";
//	const char* STARTTIME = "startTime";
//	const char* ENDTIME = "endTime";
	const char* TIMEOUT = "timeout";
	const char* LISTMD = "lookForMetaData";

	unsigned defaultTimeout = 50000;
}

ContentLibClient::ContentLibClient()
{
	int argc = 0;
	Ice::PropertiesPtr _properties = Ice::createProperties();;
	_properties->setProperty("Ice.Trace.Network", "0");
	_properties->setProperty("Ice.Trace.Protocol", "0");
	_properties->setProperty("Ice.Trace.Retry", "1");
	_properties->setProperty("Ice.Warn.Connections", "1");
	_properties->setProperty("Ice.Warn.Endpoints", "1");
	_properties->setProperty("Ice.Override.ConnectTimeout", "10000");
	_properties->setProperty("Ice.Override.Timeout", "3600000");
	_properties->setProperty("Ice.MessageSizeMax", "409600");
	Ice::InitializationData initData;
	initData.properties = _properties;
	ic_ = Ice::initialize(argc, 0, initData);
}

ContentLibClient::~ContentLibClient() {

}

void ContentLibClient::usage(const std::string& key) const {
	// ruler         "-------------------------------------------------------------------------------"
	std::cout << "Console client for ContentLib version: " 
		<< ZQ_PRODUCT_VER_MAJOR << "." 
		<< ZQ_PRODUCT_VER_MINOR << "." 
		<< ZQ_PRODUCT_VER_PATCH << "(build " 
		<< ZQ_PRODUCT_VER_BUILD << ")\n\n"

		<< "connect <endpoint>           ICE endpoint, eg: \"tcp -h 10.0.0.1 -p 11900\"\n"
		<< "                             refer to variable <timeout>, require a \n"
		<< "                             reconnection to take effect\n" 
		<< "close                        disconnect with ContentLib\n"
		<< "exit|quit                    exit shell\n"
		<< "clear                        clear screen\n"
		<< "help                         display this screen\n"
		<< "current                      display the current console context\n"
		<< "set [<var>=<value>]          set a variable in the context, show if no args\n"
		<< "reset                        reset Properties\n"
		<< "open <name>                  open StoreReplica\n"
		<< "open volume <name>           open MetaVolume <NetId$VolName>\n"
		<< "open content <name>          open ContentReplica <NetId$VolName/ContentName>\n"
		<< "list volume                  list volumes\n"                 
		<< "list content                 list contents by Properties\n"
		<< "info                         show info of current store\n"
		<< std::endl;
}

void ContentLibClient::prompt() const
{
	std::cout << "ContentLib";
	std::cout << "> ";
}

void ContentLibClient::connect(const std::string& endpoint) {
	init();

	std::ostringstream oss;
	oss << "ContentLibApp" << ":" << endpoint;

	std::string::size_type pos = std::string::npos;
	if((pos = endpoint.find_last_of(" -t ")) == std::string::npos) {
		oss << " -t " << defaultTimeout;
	}
	else {
		std::istringstream iss(endpoint.substr(pos, endpoint.find_first_of(' ', pos)));
		iss >> defaultTimeout;
	}

	try {
		ctx_.clPrx_ = ::TianShanIce::Repository::ContentLibPrx::checkedCast(ic_->stringToProxy(oss.str()));

		if (!ctx_.clPrx_) {
			std::cerr << "failed connecting with (" << oss.str() << ")" << std::endl;
			return;
		}
	}
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
		return;
	}

	std::cout << "connected with (" << oss.str() << ")" << std::endl;
}

void ContentLibClient::close() {
	if(checkConnection()) {
		init();
	}
}

void ContentLibClient::exit() {
	try{
		ic_->destroy();
	} 
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch(...) {
		std::cerr << "unknown error" << std::endl;  /* not necessary but a place holder */
	}

	quit_ = true;
}

bool ContentLibClient::quit() const {
	return quit_;
}

void ContentLibClient::init() {
	ctx_.csPrx_ = 0;
	ctx_.mvPrx_ = 0;
	ctx_.ctPrx_ = 0;
	ctx_.ctName_.clear();
	ctx_.mvName_.clear();
	ctx_.csName_.clear();

	prop_.clear();
}

bool ContentLibClient::checkConnection() const {
	if(!ctx_.clPrx_) {
		std::cerr << "not connected with any ContentLib server" << std::endl;
		return false;
	}
	return true;
}

bool ContentLibClient::isInteractive() const {
	return interactive_;
}

void ContentLibClient::setInteractive(bool val) {
	interactive_ = val;
}

void ContentLibClient::setProperty(const std::string& key, const std::string& val) {
	if(key == LISTMD) {
		listMetadata_ = ZQ::common::stringHelper::split(val, ',');
		/* not show as a context variable */
		return;
	}
	if(key == TIMEOUT) {
		std::istringstream iss(val);
		iss >> defaultTimeout;
	}
	prop_[key] = val;
}

void ContentLibClient::reset()
{
	listMetadata_.clear();
	prop_.clear();
	ctx_.csPrx_ = 0;
	ctx_.mvPrx_ = 0;
	ctx_.ctPrx_ = 0;
	ctx_.ctName_.clear();
	ctx_.mvName_.clear();
	ctx_.csName_.clear();
}

void ContentLibClient::info() const {
	if(!checkConnection()) {
		return;
	} 
}

void ContentLibClient::current() const {
	std::ostringstream oss;

	if(ctx_.csPrx_) {
		oss <<  "ContentStore Name: " << ctx_.csName_ << "\n";
	}
	if(ctx_.mvPrx_) {
		oss <<  "Volume Name: " << ctx_.mvName_ << "\n";
	}
	if(ctx_.ctPrx_) {
		oss <<  "Content Name: " << ctx_.ctName_ << "\n";
	}

	TianShanIce::Properties::const_iterator iter = prop_.begin();
	for(; iter != prop_.end(); ++iter) {
		oss << "var[" << iter->first << "]: " << iter->second << "\n";
	}

	if(!oss.str().empty()) {
		std::cout << oss.str();
	}
}

void ContentLibClient::list()
{
	TRY_BEGIN
		if(ctx_.clPrx_)
		{
			TianShanIce::StrValues expectMetaData;
			TianShanIce::Properties searchForMetaData;
			searchForMetaData.insert(TianShanIce::Properties::value_type("objectType", "ContentStoreReplica"));
			expectMetaData.push_back("endpoint");
			expectMetaData.push_back("netId");
			TianShanIce::Repository::MetaObjectInfos infos = ctx_.clPrx_->locateContent(searchForMetaData, expectMetaData, 0, false);
			for(TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin(); it != infos.end(); it++)
			{
				std::cout << it->id << ": " << it->metaDatas["endpoint"].value << "\n";
			}
		}
		else
		{
			std::cerr << "no connect to ContentLib" << std::endl;
			return;
		}

	TRY_END
}

void ContentLibClient::toStoreReplica(const std::string& name)
{
	TRY_BEGIN
	if(ctx_.clPrx_)
	{
		ctx_.csPrx_  = ctx_.clPrx_->toStoreReplica(name);
		ctx_.csName_ = name;
	}
	else
	{
		std::cerr << "no connect to ContentLib" << std::endl;
		return;
	}

	::TianShanIce::ObjectInfo info = ctx_.csPrx_->getInfo(listMetadata_);

	std::cout << "========== " << info.ident.name << " ==========" << "\n";
	for(::TianShanIce::Properties::iterator it = info.props.begin(); it != info.props.end(); it++)
	{
		std::cout << it->first << ": " << it->second << "\n";
		if(it == info.props.end())
			std::cout << std::endl;
	}

	TRY_END

}

void ContentLibClient::toVolume(const std::string& name)
{
	TRY_BEGIN
		if(ctx_.clPrx_)
		{
			ctx_.mvPrx_  = ctx_.clPrx_->toVolume(name);
			ctx_.mvName_ = name;
		}
		else
		{
			std::cerr << "no connect to ContentLib" << std::endl;
			return;
		}

		::TianShanIce::StatedObjInfo info = ctx_.mvPrx_->getInfo(listMetadata_);
		::TianShanIce::Storage::VolumePrx volume = ctx_.mvPrx_->theVolume();

		std::cout << "========== " << info.ident.name << " ==========" << "\n";
		for(::TianShanIce::Properties::iterator it = info.props.begin(); it != info.props.end(); it++)
		{
			std::cout << it->first << ": " << it->second << "\n";
				if(it == info.props.end())
					std::cout << std::endl;
		}

	TRY_END
}

void ContentLibClient::toContentReplica(const std::string& name)
{
	TRY_BEGIN
		if(ctx_.clPrx_)
		{
			ctx_.ctPrx_  = ctx_.clPrx_->toContentReplica(name);
			ctx_.ctName_ = name;
		}
		else
		{
			std::cerr << "no connect to ContentLib" << std::endl;
			return;
		}

		::TianShanIce::StatedObjInfo info = ctx_.ctPrx_->getInfo(listMetadata_);

		std::cout << "========== " << info.ident.name << " ==========" << "\n";
		for(::TianShanIce::Properties::iterator it = info.props.begin(); it != info.props.end(); it++)
		{
			std::cout << it->first << ": " << it->second << "\n";
				if(it == info.props.end())
					std::cout << std::endl;
		}

	TRY_END
}

void ContentLibClient::listVolume()
{
	TRY_BEGIN
		if(ctx_.clPrx_)
		{
			if(!ctx_.csPrx_)
			{
				std::cerr << "no open StoreReplica" << std::endl;
				return;
			}
			else
			{
				::TianShanIce::Repository::ContentStoreReplicaExPrx csEx = ::TianShanIce::Repository::ContentStoreReplicaExPrx::checkedCast(ctx_.csPrx_);
				TianShanIce::Repository::MetaObjectInfos infos = csEx->listVolume();
				for(TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin(); it != infos.end(); it++)
				{
					std::cout << "========== " << it->id << " ==========" << "\n";
				}
				std::cout << infos.size() << " volumes in all";
				std::cout << std::endl;
			}
		}
		else
		{
			std::cerr << "no connect to ContentLib" << std::endl;
			return;
		}
	TRY_END
}

void ContentLibClient::listContent()
{
	TRY_BEGIN
		if(!ctx_.clPrx_)
		{			
			std::cerr << "no connect to ContentLib" << std::endl;
			return;
		}
		// locate content by netid and volume
		if(prop_.empty() || prop_.find("netId") != prop_.end() || prop_.find("volumeName") != prop_.end())
		{
			std::string netId, volumeName;
			if(!ctx_.csName_.empty())
			{
				netId = ctx_.csName_;
			}
			if(!ctx_.mvName_.empty())
			{
				netId = ctx_.mvName_.substr(0, ctx_.mvName_.find_first_of('$'));
				volumeName = ctx_.mvName_.substr(ctx_.mvName_.find_first_of('$')+1);
			}
			if(prop_.find("netId") != prop_.end())
			{
				netId = prop_["netId"];
			}
			if(prop_.find("volumeName") != prop_.end())
			{
				volumeName = prop_["volumeName"];
			}
			int totalCount = 0;
			std::string nextCommand;
			TianShanIce::StrValues expectedMetadata;
			expectedMetadata.clear();
			TianShanIce::Repository::MetaObjectInfos AllInfos = ctx_.clPrx_->locateContentByNetIDAndVolume(netId, volumeName, expectedMetadata, 0, -1, totalCount);
			std::cout << totalCount << " contents in all\n";
			SYS::sleep(1000);
			TianShanIce::Repository::MetaObjectInfos::iterator it = AllInfos.begin();
			for(; it < AllInfos.end(); it++)
			{
				std::string fullName = it->id;
				std::cout << "========== " << fullName << " ==========" << "\n";
				std::string contentName =  fullName.substr(0, fullName.find("@"));
				std::string strNetId = fullName.substr(fullName.find("@") + 1, fullName.find_first_of("$") - fullName.find("@") - 1);
				std::string volName = fullName.substr(fullName.find_first_of("$") + 1);

				try
				{
					::TianShanIce::Repository::ContentReplicaPrx contentPrx = ctx_.clPrx_->toContentReplica(strNetId + "$" + volName + "/" + contentName);
					::TianShanIce::Repository::MetaDataMap metaDataMap = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(contentPrx)->getMetaDataMap();
					for (TianShanIce::StrValues::const_iterator iter = listMetadata_.begin(); iter != listMetadata_.end(); iter++)
					{
						if(metaDataMap.find(*iter) != metaDataMap.end())
						{
							std::cout << *iter << "--" << metaDataMap[*iter].value << "\n";
						}
					}
				}
				catch (...)
				{
					continue;
				}
			}
		}
		else
		{
			TianShanIce::Repository::MetaObjectInfos infos = ctx_.clPrx_->locateContent(prop_, listMetadata_, 0, false);
			for(TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin(); it != infos.end(); it++)
			{
				std::cout << "========== " << it->id << " ==========" << "\n";
				for (::TianShanIce::Repository::MetaDataMap::const_iterator iter = it->metaDatas.begin(); iter != it->metaDatas.end(); iter++)
				{
					std::cout << iter->first << "--" << iter->second.value << "\n";
				}
			}
			std::cout << infos.size() << " contents in all";
			std::cout << std::endl;
		}
	TRY_END
}

extern FILE *yyin;
extern int yyparse();

extern bool isEOF;

extern ContentLibClient client;

void usage() {
	std::cout << "Usage: ContentLibClient [option] [arg]\n\n"
		<< "Options:\n"
		<< "  -h              show this message\n"
		<< "  -e <endpoint>   ICE endpoint to be connected with\n"
		<< "  -f <file>       read instruction from file, for batch jobs\n"
		<< "  -v              output product version\n"
		<< std::endl;
}

int main(int argc, char* argv[])
{
	std::string file;

	if(argc > 1) {
		int ch = 0;
		while((ch = getopt(argc, argv, "he:f:v")) != EOF) {
			if(ch == 'h') {
				usage();
				return (0);
			}			
			else if(ch == 'e') {
				client.connect(optarg);
			}
			else if(ch == 'f') {
				file = optarg;
			}
			else if(ch == 'v') {
				std::cout << "Console client for ContentLib version: " 
					<< ZQ_PRODUCT_VER_MAJOR << "." 
					<< ZQ_PRODUCT_VER_MINOR << "." 
					<< ZQ_PRODUCT_VER_PATCH << "(build " 
					<< ZQ_PRODUCT_VER_BUILD << ")\n"
					<< std::endl;
				return (0);
			}
			else {
				std::cerr << "invalid option" <<  std::endl;
				return (0);
			}
		}
	}

	if(!file.empty()) {
		FILE* fp = std::fopen(file.c_str(), "r");
		yyin = fp;

		client.setInteractive(false);
		while(!isEOF) {
			yyparse();
		}
		if(!client.quit()) {
			client.exit();
		}
		fclose(fp);
	}	
	else {
		client.setInteractive(true);

		while(!client.quit()) {
			client.prompt();
			yyparse();
		}
	}

	return 0;
}

