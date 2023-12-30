#ifndef __ZQ_XmlRPC_Config_H__
#define __ZQ_XmlRPC_Config_H__
#include <ConfigHelper.h>

struct AllowedCommand
{
    std::string pattern;
    static void structure(ZQ::common::Config::Holder<AllowedCommand>& holder) {
        using namespace ZQ::common;
        holder.addDetail("", "pattern", &AllowedCommand::pattern);
    }
};
struct XmlRPCConf
{
    std::string crashDumpPath;
    int32 crashDumpEnabled;

    std::string bindAddress;
    int32 bindPort;
    int32 capacity;
    int32 hexDump;
    std::vector<std::string> allowedCommands;

    static void structure(ZQ::common::Config::Holder<XmlRPCConf>& holder)
    {
        using namespace ZQ::common;
        holder.addDetail("default/CrashDump", "path", &XmlRPCConf::crashDumpPath, NULL, Config::optReadOnly);
        holder.addDetail("default/CrashDump", "enabled", &XmlRPCConf::crashDumpEnabled, "1", Config::optReadOnly);

        holder.addDetail("xmlshd/Http", "address", &XmlRPCConf::bindAddress, NULL, Config::optReadOnly);
        holder.addDetail("xmlshd/Http", "port", &XmlRPCConf::bindPort, NULL, Config::optReadOnly);
        holder.addDetail("xmlshd/Http", "threads", &XmlRPCConf::capacity, "5", Config::optReadOnly);
        holder.addDetail("xmlshd/Http", "hexDump", &XmlRPCConf::hexDump, "0", Config::optReadOnly);

        holder.addDetail("xmlshd/AllowedCommands/Cmd", &XmlRPCConf::readAllowedCommand, &XmlRPCConf::registerNothing);
    }
    void readAllowedCommand(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        ZQ::common::Config::Holder<AllowedCommand> reader;
        reader.read(node, hPP);
        allowedCommands.push_back(reader.pattern);
    }
    void registerNothing(const std::string&){}
};

extern ZQ::common::Config::Loader<XmlRPCConf> gConfig;
#endif

