#include "EGConfig.h"
#include "FileSystemOp.h"
#include "strHelper.h"

namespace EventGateway{
    
using namespace ZQ::common;
// ModuleConfig
void ModuleConfig::structure(ZQ::common::Config::Holder<ModuleConfig> &holder)
{
    using namespace ZQ::common::Config;
    holder.addDetail("", "image", &ModuleConfig::image, NULL, optReadOnly);
}

/*
static void findWithWildcard(const std::string &filespec, std::vector<std::string> &result)
{
    result.clear();

    _finddata_t fileInfo;
    long hFind = ::_findfirst(filespec.c_str(), &fileInfo);
    if(-1 == hFind)
        return;
    // get the folder
    std::string folder = Config::parseFilePath(filespec).first;
    do
    {
        // save file path
        result.push_back(folder + fileInfo.name);
    }while(0 == ::_findnext(hFind, &fileInfo));
    ::_findclose(hFind);
    return;
}
*/

// PluginsConfig
void PluginsConfig::populate(const std::string &filespec)
{
    if(filespec.empty())
        return;
    
    std::vector<std::string> tmp = stringHelper::rsplit(filespec, FNSEPC, 1); 
    if(tmp.size() != 2) {
        return;
    }
    std::vector<std::string> files = FS::searchFiles(tmp.at(0), tmp.at(1));
//    findWithWildcard(filespec, files);
    
    std::copy(files.begin(), files.end(), std::inserter(modules, modules.end()));
}
void PluginsConfig::structure(ZQ::common::Config::Holder<PluginsConfig> &holder)
{
    using namespace ZQ::common::Config;
    holder.addDetail("", "populatePath", &PluginsConfig::populatePath, "", optReadOnly);
    holder.addDetail("", "configDir", &PluginsConfig::configDir, NULL, optReadOnly);
    holder.addDetail("", "logDir", &PluginsConfig::logDir, NULL, optReadOnly);
    holder.addDetail("Module", &PluginsConfig::readModule, &PluginsConfig::registerModule);
}
void PluginsConfig::readModule(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    using namespace ZQ::common::Config;
    Holder<ModuleConfig> moduleHolder;
    moduleHolder.read(node, hPP);
    modules.insert(moduleHolder.image);
}
void PluginsConfig::registerModule(const std::string &full_path)
{
}

// EGConfig
void EGConfig::structure(ZQ::common::Config::Holder<EGConfig> &holder)
{
    using namespace ZQ::common::Config;
    holder.addDetail("default/EventChannel", "endpoint", &EGConfig::iceStormEndpoint, NULL, optReadOnly);

    holder.addDetail("default/CrashDump", "path", &EGConfig::dumpPath, NULL, optReadOnly);
    holder.addDetail("default/CrashDump", "enabled", &EGConfig::dumpEnabled, "1", optReadOnly);
    holder.addDetail("default/CrashDump", "fullMemoryDumpEnabled", &EGConfig::fullMemoryDumpEnabled, "1", optReadOnly);

    holder.addDetail("default/IceTrace", "enabled", &EGConfig::iceTraceEnabled, "1", optReadOnly);
    holder.addDetail("default/IceTrace", "level", &EGConfig::iceTraceLevel, "7", optReadWrite);
    holder.addDetail("default/IceTrace", "size", &EGConfig::iceTraceFileSize, "10240000", optReadOnly);
    holder.addDetail("default/IceTrace", "count", &EGConfig::iceTraceCount, "5", optReadOnly);

    holder.addDetail("default/IceProperties/prop", &EGConfig::readIceProp, &EGConfig::registerNothing);

    holder.addDetail("EventGateway/ConnectionMaintenance", "checkIntervalMSec", &EGConfig::connCheckIntervalMSec, NULL, optReadOnly);

    holder.addDetail("EventGateway/Plugins", &EGConfig::readPlugins, &EGConfig::registerPlugins, Range(1, 1));
}
void EGConfig::readPlugins(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    plugins.read(node, hPP);
}
void EGConfig::registerPlugins(const std::string &full_path)
{
    plugins.snmpRegister(full_path);
}

void EGConfig::readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    using namespace ZQ::common::Config;
    Holder<NVPair> nvHolder;
    nvHolder.read(node, hPP);
    iceProperties[nvHolder.name] = nvHolder.value;
}
void EGConfig::registerNothing(const std::string &){}
} // namespace EventGateway 
