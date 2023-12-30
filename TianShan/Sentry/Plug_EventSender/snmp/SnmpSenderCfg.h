#if !defined __SNMPSENDERCFG_H__
#define __SNMPSENDERCFG_H__
#include <ConfigHelper.h>
#include <string>
#include <vector>
using namespace ZQ::common;

struct Filter
{
	std::string strCategory;
	std::string strEventName;

	static void structure(Config::Holder< Filter > &holder)
	{
		holder.addDetail("", "category", &Filter::strCategory, "", Config::optReadOnly);
		holder.addDetail("", "eventName", &Filter::strEventName, "", Config::optReadOnly);
	}
};

struct Target 
{
	int				nEnabled;
	std::string		ipAddress;
	int32			port;
	std::string		community;

	typedef std::vector< Config::Holder< Filter > > Filters;
    Filters		filters;
	
	static void structure(Config::Holder< Target > &holder)
    {
		holder.addDetail("", "enabled", &Target::nEnabled, "1", Config::optReadOnly);
        holder.addDetail("", "ipAddress", &Target::ipAddress, NULL, Config::optReadOnly);
        holder.addDetail("", "port", &Target::port, NULL, Config::optReadOnly);
        holder.addDetail("", "community", &Target::community, NULL, Config::optReadOnly);

		holder.addDetail("Filters/Filter", &Target::readFilter, &Target::registerFilter);

    }

	void readFilter(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Filter> filterholder("");
        filterholder.read(node, hPP);
        filters.push_back(filterholder);
    }

    void registerFilter(const std::string &full_path)
    {
        for (Filters::iterator it = filters.begin(); it != filters.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
};

struct SnmpSenderInfo
{
	//log item
	std::string		logPath;
	int32			logSize;
	int32			logLevel;
	int32			logNumber;

	std::string		agentIp;

	typedef std::vector< Config::Holder< Target > > Targets;
    Targets		targets;

	static void structure(Config::Holder< SnmpSenderInfo > &holder)
    {
		//log detail
        holder.addDetail("EventSender/Log", "logPath", &SnmpSenderInfo::logPath, NULL, Config::optReadOnly);
        holder.addDetail("EventSender/Log", "logFileSize", &SnmpSenderInfo::logSize, "10240000", Config::optReadOnly);
		holder.addDetail("EventSender/Log", "logLevel", &SnmpSenderInfo::logLevel, "7", Config::optReadOnly);
		holder.addDetail("EventSender/Log", "logNumber", &SnmpSenderInfo::logNumber, "2", Config::optReadOnly);

		holder.addDetail("EventSender/SnmpSender/Agent", "ipAddress", &SnmpSenderInfo::agentIp, NULL, Config::optReadOnly);

		holder.addDetail("EventSender/SnmpSender/Targets/Target", &SnmpSenderInfo::readTarget, &SnmpSenderInfo::registerTarget);
	}

	void readTarget(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Target> targetholder("");
        targetholder.read(node, hPP);
        targets.push_back(targetholder);
    }

    void registerTarget(const std::string &full_path)
    {
        for (Targets::iterator it = targets.begin(); it != targets.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }

};







#endif//__SNMPSENDERCFG_H__
