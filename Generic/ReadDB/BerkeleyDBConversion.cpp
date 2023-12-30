// BerkeleyDBConversion.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Freeze/Freeze.h>
#include "StorageDict.h"
#include "StreamerDict.h"
#include "StorageToStorageLink.h"
#include "StreamerToStorageLink.h"
#include "StreamerToStreamLink.h"
#include "ServiceGroupToStreamLink.h"
#include "ServiceGroupDict.h"
#include <IceUtil/IceUtil.h>
#include <iostream>
#include <fstream>
#include "stroprt.h"
using namespace TianShanIce;
using namespace TianShanIce::Transport;
using namespace std;

#pragma warning(disable: 4018)

#define INDEXFILENAME(_IDX)				#_IDX "Idx"
#define NEW_INDEX(_IDX) _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "create index: " #_IDX)); \
		_idx##_IDX = new TianShanIce::Transport::##_IDX(INDEXFILENAME(_IDX))

#define _NOT_TEST_

namespace TianShanIce {

namespace Transport {

	class StreamLinkI : public StreamLinkEx , public IceUtil::AbstractMutexI<IceUtil::RecMutex>
	{
	public:
		StreamLinkI() {};
		virtual ~StreamLinkI() {};

		::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return ident;
		}
		
		::std::string getType(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return type;
		}
		
		::std::string getStreamerId(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return streamerId;
		}
		
		::Ice::Int getServiceGroupId(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return servicegroupId;
		}
		
		::TianShanIce::Transport::Streamer getStreamerInfo(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			Streamer stmer;
			return stmer;
		}
		
		::TianShanIce::Transport::ServiceGroup getServiceGroupInfo(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			ServiceGroup svg;
			return svg;
		}
		
		bool setPrivateData(const ::std::string&, const ::TianShanIce::Variant&, const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			return true;
		}
		
		::TianShanIce::ValueMap getPrivateData(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			return privateData;
		}
		
		void destroy(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
		}
		virtual ::Ice::Int getRevision(const ::Ice::Current& = ::Ice::Current()) const 
		{
			Lock lock(*this);
			return revision;
		}
		
		
		virtual void updateRevision(::Ice::Int _newRevision, const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			revision = _newRevision;
		}
		
		
		virtual bool updatePrivateData(const ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			return true;
		}

	};
	typedef ::IceInternal::Handle< StreamLinkI> StreamLinkIPtr;
//	typedef ::IceInternal::ProxyHandle< StreamLinkI> StreamLinkIPrx;

	class StorageLinkI: public StorageLinkEx , public IceUtil::AbstractMutexI<IceUtil::RecMutex>
	{
	public:
		StorageLinkI() {};
		virtual ~StorageLinkI() {};
		
		::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return ident;
		}
		
		::std::string getType(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return type;
		}
		
		::std::string getStorageId(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return storageId;
		}
		
		::std::string getStreamerId(const ::Ice::Current& = ::Ice::Current()) const
		{
			Lock lock(*this);
			return streamerId;
		}
		
		::TianShanIce::Transport::Storage getStorageInfo(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			Storage svg;
			return svg;
		}
		
		::TianShanIce::Transport::Streamer getStreamerInfo(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			Streamer stmer;
			return stmer;
		}
		
		bool setPrivateData(const ::std::string&, const ::TianShanIce::Variant&, const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			return true;
		}
		
		::TianShanIce::ValueMap getPrivateData(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			return privateData;
		}
		
		void destroy(const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
		}
		virtual ::Ice::Int getRevision(const ::Ice::Current& = ::Ice::Current()) const 
		{
			Lock lock(*this);
			return revision;
		}

		
		virtual void updateRevision(::Ice::Int _newRevision, const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			revision = _newRevision;
		}
		
		
		virtual bool updatePrivateData(const ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current())
		{
			Lock lock(*this);
			return true;
		}


	};
	typedef ::IceInternal::Handle< StorageLinkI> StorageLinkIPtr;
//	typedef ::IceInternal::ProxyHandle< StorageLinkI> StorageLinkIPrx;

	class MyFactory : public Ice::ObjectFactory {
	public:
		virtual Ice::ObjectPtr create(const std::string&);
		virtual void destroy();
		typedef IceUtil::Handle<MyFactory> Ptr;
	};

	Ice::ObjectPtr MyFactory::create(const std::string& strType)
	{		
		if (StreamLink::ice_staticId() == strType)
			return new StreamLinkI();
		if(StreamLinkEx::ice_staticId() == strType )
			return new StreamLinkI();
		
		if (StorageLink::ice_staticId() == strType)
			return new StorageLinkI();
		if( StorageLinkEx::ice_staticId() == strType)
			return new StorageLinkI();
		return NULL;
	}

	void MyFactory::destroy()
	{
	}
	
} // namespace Transport

} // namespace TianShanIce

MyFactory::Ptr factory;

void writeVariant(ostream& os, const TianShanIce::Variant& vrt)
{
	TianShanIce::ValueType type = vrt.type;
	switch (type)
	{
	case TianShanIce::vtBin:
		{
			TianShanIce::ServerError ex;
			ex.message = "Not Implemented";
			ex.ice_throw();
			break;
		}
	case TianShanIce::vtInts:
		{
			os<<"TianShanIce::Variant::vtInts"<<endl;
			if (vrt.bRange)
			{
				os<<"TianShanIce::Variant::bRange=true"<<endl;
				if (vrt.ints.size() < 2)
				{
					TianShanIce::ServerError ex;
					ex.message = "TianShanIce::Variant::bRange = true, but ints.size() < 2";
					ex.ice_throw();
				}
				int size = 2;
				os<<"TianShanIce::Variant::Ints::size="<<size<<endl;
				os<<"int="<<vrt.ints[0]<<endl;
				os<<"int="<<vrt.ints[1]<<endl;
			}
			else 
			{
				os<<"TianShanIce::Variant::bRange=false"<<endl;
				int size = vrt.ints.size();
				os<<"TianShanIce::Variant::Ints::size="<<size<<endl;
				for (int i = 0; i < size; i++)
				{
					os<<"int="<<vrt.ints[i]<<endl;
				}
			}
			break;
		}
	case TianShanIce::vtLongs:
		{
			os<<"TianShanIce::Variant::vtLongs"<<endl;
			if (vrt.bRange)
			{
				os<<"TianShanIce::Variant::bRange=true"<<endl;
				if (vrt.lints.size() < 2)
				{
					TianShanIce::ServerError ex;
					ex.message = "TianShanIce::Variant::bRange = true, but lints.size() < 2";
					ex.ice_throw();
				}
				int size = 2;
				os<<"TianShanIce::Variant::Longs::size="<<size<<endl;
				os<<"long="<<vrt.lints[0]<<endl;
				os<<"long="<<vrt.lints[1]<<endl;
			}
			else 
			{
				os<<"TianShanIce::Variant::bRange=false"<<endl;
				int size = vrt.lints.size();
				os<<"TianShanIce::Variant::Longs::size="<<size<<endl;
				for (int i = 0; i < size; i++)
				{
					os<<"long="<<vrt.lints[i]<<endl;
				}
			}
			break;
		}
	case TianShanIce::vtStrings:
		{
			os<<"TianShanIce::Variant::vtStrings"<<endl;
			int size = vrt.strs.size();
			os<<"TianShanIce::Variant::Strings::size="<<size<<endl;
			for (int i = 0; i < size; i++)
			{
				string tmp_str = vrt.strs[i];
				ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\');
				os<<"string="<<tmp_str<<endl;
			}
			break;
		}
	default:
		{
			TianShanIce::ServerError ex;
			ex.message = "Not TianShanIce::Variant";
			ex.ice_throw();
		}
	}
}

bool readVariant(istream& is, TianShanIce::Variant& vrt)
{
	string type_str;
	is>>type_str;
	if (!type_str.size())
		return false;
	if (type_str == "TianShanIce::Variant::vtInts")
	{
		vrt.type = TianShanIce::vtInts;
		string tmp_str;
		is>>tmp_str;
		if (tmp_str == "TianShanIce::Variant::bRange=true")
		{
			vrt.bRange = true;/*TianShanIce::Variant::Ints::*/
			is>>tmp_str;
			ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
			if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::Variant::Ints::size")
			{
				TianShanIce::ServerError ex;
				ex.message = "Invalid TianShanIce::Variant::vtInts, no size.";
				ex.ice_throw();
			}
			int size;
			size = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
			if (size != 2)
			{
				TianShanIce::ServerError ex;
				ex.message = "Invalid TianShanIce::Variant::vtInts, bRange = true, but size != 2.";
				ex.ice_throw();
			}
			for (int i = 0; i < 2; i++)
			{
				is>>tmp_str;
				ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
				if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "int")
				{
					TianShanIce::ServerError ex;
					ex.message = "Invalid TianShanIce::Variant::vtInts, not a int value.";
					ex.ice_throw();
				}
				int tmp_int;
				tmp_int = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
				vrt.ints.push_back(tmp_int);
			}
		}
		else if (tmp_str == "TianShanIce::Variant::bRange=false")
		{
			vrt.bRange = false;
			is>>tmp_str;
			ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
			if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::Variant::Ints::size")
			{
				TianShanIce::ServerError ex;
				ex.message = "Invalid TianShanIce::Variant::vtInts, no size.";
				ex.ice_throw();
			}
			int size;
			size = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
			for (int i = 0; i < size; i++)
			{
				is>>tmp_str;
				ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
				if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "int")
				{
					TianShanIce::ServerError ex;
					ex.message = "Invalid TianShanIce::Variant::vtInts, not a int value.";
					ex.ice_throw();
				}
				int tmp_int;
				tmp_int = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
				vrt.ints.push_back(tmp_int);
			}
		}
		else 
		{
			TianShanIce::ServerError ex;
			ex.message = "Invliad TianShanIce::Variant::Ints, bRange value is invalid.";
			ex.ice_throw();
		}
	}
	else if (type_str == "TianShanIce::Variant::vtLongs")
	{
		vrt.type = TianShanIce::vtLongs;
		string tmp_str;
		is>>tmp_str;
		if (tmp_str == "TianShanIce::Variant::bRange=true")
		{
			vrt.bRange = true;
			is>>tmp_str;
			ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
			if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::Variant::Longs::size")
			{
				TianShanIce::ServerError ex;
				ex.message = "Invalid TianShanIce::Variant::vtLongs, no size.";
				ex.ice_throw();
			}
			int size;
			size = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
			if (size !=2)
			{
				TianShanIce::ServerError ex;
				ex.message = "Invalid TianShanIce::Variant::vtLongs, bRange = true, but size != 2.";
				ex.ice_throw();
			}
			for (int i = 0; i < 2; i++)
			{
				is>>tmp_str;
				ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
				if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "long")
				{
					TianShanIce::ServerError ex;
					ex.message = "Invalid TianShanIce::Variant::vtLongs, not a long value.";
					ex.ice_throw();
				}
				long tmp_lint;
				tmp_lint = atol(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
				vrt.lints.push_back(tmp_lint);
			}
		}
		else if (tmp_str == "TianShanIce::Variant::bRange=false")
		{
			vrt.bRange = false;
			is>>tmp_str;
			ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
			if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::Variant::Longs::size")
			{
				TianShanIce::ServerError ex;
				ex.message = "Invalid TianShanIce::Variant::vtLongs, no size.";
				ex.ice_throw();
			}
			int size;
			size = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
			for (int i = 0; i < size; i++)
			{
				is>>tmp_str;
				ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
				if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "long")
				{
					TianShanIce::ServerError ex;
					ex.message = "Invalid TianShanIce::Variant::vtLongs, not a long value.";
					ex.ice_throw();
				}
				long tmp_lint;
				tmp_lint = atol(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
				vrt.lints.push_back(tmp_lint);
			}
		}
		else 
		{
			TianShanIce::ServerError ex;
			ex.message = "Invliad TianShanIce::Variant::Longs, bRange value is invalid.";
			ex.ice_throw();
		}
	}
	else if (type_str == "TianShanIce::Variant::vtStrings")
	{
		vrt.type = TianShanIce::vtStrings;
		string tmp_str;
		is>>tmp_str;
		ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
		if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::Variant::Strings::size")
		{
			TianShanIce::ServerError ex;
			ex.message = "Invliad TianShanIce::Variant::Strings, no size.";
			ex.ice_throw();
		}
		int size;
		size = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
		for ( int i = 0; i < size; i++)
		{
			is>>tmp_str;
			ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
			if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "string")
			{
				TianShanIce::ServerError ex;
				ex.message = "Invliad TianShanIce::Variant::Strings, not a string value.";
				ex.ice_throw();
			}
			vrt.strs.push_back(ZQ::StringOperation::getRightStr(tmp_str, "=", true));
		}
	}
	else if (type_str == "TianShanIce::Variant::vtBin")
	{
		TianShanIce::ServerError ex;
		ex.message = "Not Implemented.";
		ex.ice_throw();
	}
	else 
	{
		TianShanIce::ServerError ex;
		ex.message = "Not TianShanIce::Variant.";
		ex.ice_throw();
	}
	return true;
}

void writeValueMap(ostream& os, const TianShanIce::ValueMap& vMap)
{
	string tmp_str;
	os<<"TianShanIce::ValueMap"<<endl;
	int map_size = vMap.size();
	os<<"TianShanIce::ValueMap::size="<<map_size<<endl;
	std::map<std::string, ::TianShanIce::Variant>::const_iterator map_itor;
	for (map_itor = vMap.begin(); map_itor != vMap.end(); map_itor++)
	{
		tmp_str = map_itor->first;
		ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\');
		os<<"TianShanIce::ValueMap::key="<<tmp_str<<endl;
		writeVariant(os, map_itor->second);
	}
}

bool readValueMap(istream& is, TianShanIce::ValueMap& vMap)
{
	string tmp_str;
	is>>tmp_str;
	if (!tmp_str.size())
		return false;
	if (tmp_str != "TianShanIce::ValueMap")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid TianShanIce::ValueMap data.";
		ex.ice_throw();
	}
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::ValueMap::size")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid TianShanIce::ValueMap data, no TianShanIce::ValueMap::size.";
		ex.ice_throw();
	}
	int map_size;
	map_size = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
	for (int i = 0; i < map_size; i++)
	{
		TianShanIce::Variant _vrt;
		is>>tmp_str;
		ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
		if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "TianShanIce::ValueMap::key")
		{
			TianShanIce::ServerError ex;
			ex.message = "Invalid TianShanIce::ValueMap data, no TianShanIce::ValueMap::key.";
			ex.ice_throw();
		}
		readVariant(is, _vrt);
		vMap.insert(TianShanIce::ValueMap::value_type(ZQ::StringOperation::getRightStr(tmp_str, "=", true), _vrt));
	}
	return true;
}

string typestr(const TianShanIce::Variant& vrt)
{
	TianShanIce::ValueType type = vrt.type;
	switch (type)
	{
	case TianShanIce::vtInts:
		{
			return "Ints";
			break;
		}
	case TianShanIce::vtStrings:
	{
		return "Strings";
		break;
	}
	case TianShanIce::vtLongs:
	{
		return "Longs";
		break;
	}
	case TianShanIce::vtBin:
	{
		return "vtBin";
		break;
	}
	default:
	{
		TianShanIce::ServerError ex;
		ex.message = "Unknown variant type.";
		ex.ice_throw();
	}
	}
}

void writeServiceGroup(ostream& os, const TianShanIce::Transport::ServiceGroup& _group)
{
	os<<"TianShanIce::Transport::ServiceGroup"<<endl;
	os<<"service-group-id="<<_group.id<<endl;
	string tmp_str;
	tmp_str = _group.type;
	os<<"type="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _group.desc;
	os<<"description="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
}

bool readServiceGroup(istream& is, TianShanIce::Transport::ServiceGroup& _group)
{
	string temp_str;
	is>>temp_str;
	if (!temp_str.size())
		return false;
	if (temp_str != "TianShanIce::Transport::ServiceGroup")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid ServiceGroup data";
		ex.ice_throw();
	}
	// DO: read service group id
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "service-group-id")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid ServiceGroup data, no service-group-id field.";
		ex.ice_throw();
	}
	_group.id = atoi(ZQ::StringOperation::getRightStr(temp_str, "=", true).c_str());
	
	// DO: read type
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "type")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid ServiceGroup data, no type field.";
		ex.ice_throw();
	}
	_group.type = ZQ::StringOperation::getRightStr(temp_str, "=", true);

	// DO: read description
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "description")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid ServiceGroup data, no description field.";
		ex.ice_throw();
	}
	_group.desc = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	return true;
}

void writeStorage(ostream& os, const TianShanIce::Transport::Storage& _storage)
{
	os<<"TianShanIce::Transport::Storage"<<endl;
	string tmp_str;
	tmp_str = _storage.netId;
	os<<"netId="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _storage.type;
	os<<"type="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _storage.desc;
	os<<"description="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _storage.ifep;
	os<<"interface-endpoint="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	writeValueMap(os, _storage.privateData);
}

bool readStorage(istream& is, TianShanIce::Transport::Storage& _storage)
{
	string temp_str;
	is>>temp_str;
	if (!temp_str.size())
		return false;
	if (temp_str != "TianShanIce::Transport::Storage")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Storage data.";
		ex.ice_throw();
	}
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "netId")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Storage data, no netId field.";
		ex.ice_throw();
	}
	_storage.netId = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "type")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Storage data, no type field.";
		ex.ice_throw();
	}
	_storage.type = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "description")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Storage data, no description field.";
		ex.ice_throw();
	}
	_storage.desc = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "interface-endpoint")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Storage data, no interface-endpoint field.";
		ex.ice_throw();
	}
	_storage.ifep = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	_storage.privateData.clear();
	readValueMap(is, _storage.privateData);
	return true;
}

void writeStreamer(ostream& os, const TianShanIce::Transport::Streamer& _streamer)
{
	os<<"TianShanIce::Transport::Streamer"<<endl;
	string tmp_str;
	tmp_str = _streamer.netId;
	os<<"netId="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _streamer.type;
	os<<"type="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _streamer.desc;
	os<<"description="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _streamer.ifep;
	os<<"interface-endpoint="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	writeValueMap(os, _streamer.privateData);
}

bool readStreamer(istream& is, TianShanIce::Transport::Streamer& _streamer)
{
	string temp_str;
	is>>temp_str;
	if (!temp_str.size())
		return false;
	if (temp_str != "TianShanIce::Transport::Streamer")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Streamer data.";
		ex.ice_throw();
	}
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "netId")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Streamer data, no netId field.";
		ex.ice_throw();
	}
	_streamer.netId = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "type")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Streamer data, no type field.";
		ex.ice_throw();
	}
	_streamer.type = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "description")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Streamer data, no description field.";
		ex.ice_throw();
	}
	_streamer.desc = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	is>>temp_str;
	ZQ::StringOperation::replaceStr(temp_str, "\\", " ");
	if (ZQ::StringOperation::getLeftStr(temp_str, "=", true) != "interface-endpoint")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid Streamer data, no interface-endpoint field.";
		ex.ice_throw();
	}
	_streamer.ifep = ZQ::StringOperation::getRightStr(temp_str, "=", true);
	_streamer.privateData.clear();
	readValueMap(is, _streamer.privateData);
	return true;
}

void writeStorageLink(ostream& os, TianShanIce::Transport::StorageLinkExPrx _storageLinkPrx)
{
	os<<"TianShanIce::Transport::StorageLink"<<endl;
	string tmp_str;
	tmp_str = _storageLinkPrx->getIdent().name;
	os<<"identity="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\');
	os<<"/";
	tmp_str = _storageLinkPrx->getIdent().category;
	os<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _storageLinkPrx->getStorageId();
	os<<"storageId="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _storageLinkPrx->getStreamerId();
	os<<"streamerId="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _storageLinkPrx->getType();
	os<<"type="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	os<<"revision="<<_storageLinkPrx->getRevision()<<endl;
	TianShanIce::ValueMap vMap = _storageLinkPrx->getPrivateData();
	writeValueMap(os, vMap);
}

bool readStorageLink(istream& is, TianShanIce::Transport::StorageLinkI& _storageLink)
{
	string tmp_str;
	is>>tmp_str;
	if (!tmp_str.size())
		return false;
	if (tmp_str != "TianShanIce::Transport::StorageLink")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StorageLink data.";
		ex.ice_throw();
	}
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "identity")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StorageLink data, no identity field.";
		ex.ice_throw();
	}
	_storageLink.ident.name = ZQ::StringOperation::getLeftStr(ZQ::StringOperation::getRightStr(tmp_str, "=", true), "/", true);
	_storageLink.ident.category = ZQ::StringOperation::getRightStr(ZQ::StringOperation::getRightStr(tmp_str, "=", true), "/", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "storageId")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StorageLink data, no storageId field.";
		ex.ice_throw();
	}
	_storageLink.storageId = ZQ::StringOperation::getRightStr(tmp_str, "=", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "streamerId")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StorageLink data, no streamerId field.";
		ex.ice_throw();
	}
	_storageLink.streamerId = ZQ::StringOperation::getRightStr(tmp_str, "=", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "type")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StorageLink data, no type field.";
		ex.ice_throw();
	}
	_storageLink.type = ZQ::StringOperation::getRightStr(tmp_str, "=", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "revision")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StorageLink data, no revision field.";
		ex.ice_throw();
	}
	_storageLink.revision = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
	_storageLink.privateData.clear();
	readValueMap(is, _storageLink.privateData);
	return true;
}

void writeStreamLink(ostream& os, TianShanIce::Transport::StreamLinkExPrx _streamLinkPrx)
{
	os<<"TianShanIce::Transport::StreamLink"<<endl;
	string tmp_str;
	tmp_str = _streamLinkPrx->getIdent().name;
	os<<"identity="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\');
	os<<"/";
	tmp_str = _streamLinkPrx->getIdent().category;
	os<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	os<<"service-group-id="<<_streamLinkPrx->getServiceGroupId()<<endl;
	tmp_str = _streamLinkPrx->getStreamerId();
	os<<"streamerId="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	tmp_str = _streamLinkPrx->getType();
	os<<"type="<<ZQ::StringOperation::replaceChar(tmp_str, ' ', '\\')<<endl;
	os<<"revision="<<_streamLinkPrx->getRevision()<<endl;
	TianShanIce::ValueMap vMap = _streamLinkPrx->getPrivateData();
	writeValueMap(os, vMap);
}

bool readStreamLink(istream& is, TianShanIce::Transport::StreamLinkI& _streamLink)
{
	string tmp_str;
	is>>tmp_str;
	if (!tmp_str.size())
		return false;
	if (tmp_str != "TianShanIce::Transport::StreamLink")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StreamLink data.";
		ex.ice_throw();
	}
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "identity")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StreamLink data, no identity field.";
		ex.ice_throw();
	}
	_streamLink.ident.name = ZQ::StringOperation::getLeftStr(ZQ::StringOperation::getRightStr(tmp_str, "=", true), "/", true);
	_streamLink.ident.category = ZQ::StringOperation::getRightStr(ZQ::StringOperation::getRightStr(tmp_str, "=", true), "/", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "service-group-id")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StreamLink data, no service-group-id field.";
		ex.ice_throw();
	}
	_streamLink.servicegroupId = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "streamerId")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StreamLink data, no streamerId field.";
		ex.ice_throw();
	}
	_streamLink.streamerId = ZQ::StringOperation::getRightStr(tmp_str, "=", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "type")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StreamLink data, no type field.";
		ex.ice_throw();
	}
	_streamLink.type = ZQ::StringOperation::getRightStr(tmp_str, "=", true);
	is>>tmp_str;
	ZQ::StringOperation::replaceChar(tmp_str, '\\', ' ');
	if (ZQ::StringOperation::getLeftStr(tmp_str, "=", true) != "revision")
	{
		TianShanIce::ServerError ex;
		ex.message = "Invalid StreamLink data, no revision field.";
		ex.ice_throw();
	}
	_streamLink.revision = atoi(ZQ::StringOperation::getRightStr(tmp_str, "=", true).c_str());
	_streamLink.privateData.clear();
	readValueMap(is, _streamLink.privateData);
	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cerr<<"Invalid number of param!"<<endl;
		return 0;
	}
#ifdef _NOT_TEST_
	try
	{		
		ofstream of_StorageLink, of_StreamLink, of_Storage, of_Streamer, of_Group;
		of_StorageLink.open("StorageLink.txt");
		of_StreamLink.open("StreamLink.txt");
		of_Storage.open("Storage.txt");
		of_Streamer.open("Streamer.txt");
		of_Group.open("Group.txt");
		if (!of_StorageLink.is_open() || !of_Storage.is_open() || !of_StreamLink.is_open() || !of_Streamer.is_open() || !of_Group.is_open())
		{
			cout<<"Can't create files needed!"<<endl;
			exit(1);
		}
		
		::Ice::CommunicatorPtr _ic;
		_ic = ::Ice::initialize(argc,argv);
		::Ice::ObjectAdapterPtr myAdapter = _ic->createObjectAdapterWithEndpoints("myAdapter","default -p 2345");
		
		factory = new MyFactory();
		_ic->addObjectFactory(factory,StreamLink::ice_staticId());
		_ic->addObjectFactory(factory,StreamLinkEx::ice_staticId());
		_ic->addObjectFactory(factory,StorageLink::ice_staticId());
		_ic->addObjectFactory(factory,StorageLinkEx::ice_staticId());
		string envName = argv[1];
		string storageDictName = "Storages";
		string strmDictName = "Streamers";
		string groupDictName = "ServiceGroups";
		string storageIndexName = "StorageLink";
		string strmIndexName = "StreamLink";
		::Freeze::ConnectionPtr _conn;
		_conn = ::Freeze::createConnection(_ic,envName);
		
		// Read Storage dictionary from berkeley database, it maps the StorageId to Object Storage
		StorageDict _storageDict(_conn,storageDictName);
		
		// Read Streamer dictionary from berkeley database, it maps the StreamerId to Object Streamer
		StreamerDict _strmDict(_conn,strmDictName);

		ServiceGroupDict _groupDict(_conn, groupDictName);
	
//		NEW_INDEX(StorageToStorageLink);
//		NEW_INDEX(StreamerToStorageLink);
		/*TianShanIce :: Transport :: StorageToStorageLinkPtr _idxStorageToStorageLink;*/
		TianShanIce :: Transport :: StreamerToStorageLinkPtr _idxStreamerToStorageLink;
		TianShanIce :: Transport :: StreamerToStreamLinkPtr _idxStreamerToStreamLink;
		/*TianShanIce :: Transport :: ServiceGroupToStreamLinkPtr _idxServiceGroupToStreamLink;*/
		/*_idxStorageToStorageLink= new TianShanIce::Transport::StorageToStorageLink("StorageToStorageLinkIdx");*/
		_idxStreamerToStorageLink = new TianShanIce::Transport::StreamerToStorageLink("StreamerToStorageLinkIdx");
		_idxStreamerToStreamLink = new TianShanIce :: Transport :: StreamerToStreamLink("StreamerToStreamLinkIdx");
		/*_idxServiceGroupToStreamLink = new TianShanIce::Transport::ServiceGroupToStreamLink("ServiceGroupToStreamLinkIdx");*/
		{
			std::vector<Freeze::IndexPtr> indices;
			/*indices.push_back(_idxStorageToStorageLink);*/
			indices.push_back(_idxStreamerToStorageLink);
			/*indices.push_back(_idxStreamerToStreamLink);*/
			/*indices.push_back(_idxServiceGroupToStreamLink);*/
			::Freeze::EvictorPtr _storageEvictor = Freeze::createEvictor(myAdapter, envName, storageIndexName, 0, indices);
			myAdapter->addServantLocator(_storageEvictor, "StorageLink");
			
			::Freeze::EvictorIteratorPtr _storageIdxItor = _storageEvictor->getIterator("", 10000);
			while (_storageIdxItor->hasNext())
			{
				// find a StorageLink proxy
				Ice::Identity ident = _storageIdxItor->next();
				StorageLinkExPrx _storageLinkPrx = StorageLinkExPrx::uncheckedCast(myAdapter->createProxy(ident));
				
				// build a StoregeLinkStruct which stores all the data members of class StorageLink
				writeStorageLink(of_StorageLink, _storageLinkPrx);
				
				//
				string _storageId = _storageLinkPrx->getStorageId();
				StorageDict::iterator _storageDictItor = _storageDict.find(_storageId);
				if (_storageDictItor != _storageDict.end()) 
				{
					const Storage& _storageRef = _storageDictItor->second;
					writeStorage(of_Storage, _storageRef);
				}
				
				//
				string _strmId = _storageLinkPrx->getStreamerId();
				StreamerDict::iterator _strmDictItor = _strmDict.find(_strmId);
				if (_strmDictItor != _strmDict.end()) 
				{
					const ::TianShanIce::Transport::Streamer& _strmRef = _strmDictItor->second;
					writeStreamer(of_Streamer, _strmRef);
				}
			}
		}

		// DO: 
		{
			std::vector<Freeze::IndexPtr> indices;
			/*indices.push_back(_idxStorageToStorageLink);*/
			/*indices.push_back(_idxStreamerToStorageLink);*/
			indices.push_back(_idxStreamerToStreamLink);
			/*indices.push_back(_idxServiceGroupToStreamLink);*/
			::Freeze::EvictorPtr _strmEvictor = Freeze::createEvictor(myAdapter, envName, strmIndexName, 0, indices);
			myAdapter->addServantLocator(_strmEvictor, "StreamLink");
			
			::Freeze::EvictorIteratorPtr _strmIdxItor = _strmEvictor->getIterator("", 10000);
			while (_strmIdxItor->hasNext())
			{
				// find a StorageLink proxy
				Ice::Identity ident = _strmIdxItor->next();
				StreamLinkExPrx _strmLinkPrx = StreamLinkExPrx::uncheckedCast(myAdapter->createProxy(ident));
				
				// build a StoregeLinkStruct which stores all the data members of class StorageLink
				writeStreamLink(of_StreamLink, _strmLinkPrx);

				// DO: 
				Ice::Int _groupId = _strmLinkPrx->getServiceGroupId();
				ServiceGroupDict::iterator _groupDictItor = _groupDict.find(_groupId);
				if (_groupDictItor != _groupDict.end()) 
				{
					const ::TianShanIce::Transport::ServiceGroup& _groupRef = _groupDictItor->second;
					writeServiceGroup(of_Group, _groupRef);
				}
			}
		}

		// Close all the files
		if (of_StorageLink.is_open()) 
		{
			of_StorageLink.close();
		}
		if (of_Storage.is_open())
		{
			of_Storage.close();
		}
		if (of_StreamLink.is_open())
		{
			of_StreamLink.close();
		}
		if (of_Streamer.is_open()) 
		{
			of_StreamLink.close();
		}

	}
	catch(Ice::Exception& ex)
	{
		cout<<ex.ice_name()<<endl;
	}
#endif
	
	return 0;
}
