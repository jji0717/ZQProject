#include "C2ClientConf.h"

namespace ZQ{
    namespace StreamService{
        void LogFile::structure(LogFileHolder& holder)
        {
            holder.addDetail("", "dir", &LogFile::dir, "", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "size", &LogFile::size, "10000000", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "level", &LogFile::level, "6", ZQ::common::Config::optReadOnly);	
            holder.addDetail("", "maxCount", &LogFile::maxCount, "5", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "bufferSize", &LogFile::bufferSize, "16000", ZQ::common::Config::optReadOnly);
        }

        void Statistic::structure(StatisticHolder& holder)
        {
            holder.addDetail("", "printInterval", &StatisticHolder::printInterval, "1000", ZQ::common::Config::optReadOnly);
        }

        void RequestParamsConf::structure(RequestParamsHolder& holder)
        {
            holder.addDetail("", "UpStreamIP",          &RequestParamsConf::upstreamIP,         "",             ZQ::common::Config::optReadOnly);
            holder.addDetail("", "url",                 &RequestParamsConf::url,                "",             ZQ::common::Config::optReadOnly);
            holder.addDetail("", "clientTransfer",      &RequestParamsConf::clientTransfer,     "",             ZQ::common::Config::optReadOnly);
            holder.addDetail("", "httpCRGAddr",         &RequestParamsConf::locateIP,           "",             ZQ::common::Config::optReadOnly);
            holder.addDetail("", "httpCRGPort",         &RequestParamsConf::locatePort,         "10080",        ZQ::common::Config::optReadOnly);
            holder.addDetail("", "defaultGetPort",      &RequestParamsConf::defaultGetPort,     "12000",        ZQ::common::Config::optReadOnly);
            holder.addDetail("", "transferRate",        &RequestParamsConf::transferRate,       "3750000",      ZQ::common::Config::optReadOnly);
            holder.addDetail("", "ingressCapacity",     &RequestParamsConf::ingressCapacity,    "16512000000",  ZQ::common::Config::optReadOnly);
            holder.addDetail("", "exclusionList",       &RequestParamsConf::exclusionList,      "",             ZQ::common::Config::optReadOnly);
            holder.addDetail("", "transferDelay",       &RequestParamsConf::transferDelay,      "-2000",        ZQ::common::Config::optReadOnly);
            holder.addDetail("", "range",               &RequestParamsConf::range,              "0-",           ZQ::common::Config::optReadOnly);
            holder.addDetail("", "indexTimeout",        &RequestParamsConf::indexTimeout,       "1000",         ZQ::common::Config::optReadOnly);
            holder.addDetail("", "indexRetry",          &RequestParamsConf::indexRetry,         "2",            ZQ::common::Config::optReadOnly);
            holder.addDetail("", "mainfileTimeout",     &RequestParamsConf::mainfileTimeout,    "3000",         ZQ::common::Config::optReadOnly);
            holder.addDetail("", "mainfileRetry",       &RequestParamsConf::mainfileRetry,      "2",            ZQ::common::Config::optReadOnly);
            holder.addDetail("", "enableTransferDelete",&RequestParamsConf::enableTransferDelete,"0",           ZQ::common::Config::optReadOnly);
            holder.addDetail("", "alignment",           &RequestParamsConf::alignment,           "4",           ZQ::common::Config::optReadOnly);
        }

        void Item::structure(ItemHolder& holder)
        {
            holder.addDetail("", "name", &Item::name, "0", ZQ::common::Config::optReadOnly);
        }

        void Files::structure(FilesHolder& holder)
        {
            holder.addDetail("Item", &Files::readFiles, &Files::registerFiles);
        }

        void Files::readFiles(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ItemHolder itemHolder("name");
            itemHolder.read(node, hPP);
            files.push_back(itemHolder);
        }

        void Files::registerFiles(const std::string &full_path)
        {
            for (std::vector<ItemHolder>::iterator 
                it = files.begin(); 
                it != files.end(); it ++)
            {
                it->snmpRegister(full_path);
            }
        }

        void C2ClientConf::structure(C2ClientConfHolder& holder)
        {
            holder.addDetail("", "client", &C2ClientConf::client, "0", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "file", &C2ClientConf::file, "0", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "loop", &C2ClientConf::loop, "0", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "interval", &C2ClientConf::interval, "0", ZQ::common::Config::optReadOnly);
            holder.addDetail("", "eventloop", &C2ClientConf::eventloop, "2", ZQ::common::Config::optReadOnly);
            holder.addDetail("LogFile", &C2ClientConf::readLogFile, &C2ClientConf::regsiterLogFile);
            holder.addDetail("Statistic", &C2ClientConf::readStatistic, &C2ClientConf::regsiterStatistic);
            holder.addDetail("RequestParams", &C2ClientConf::readRequestParams, &C2ClientConf::regsiterRequestParams);
            holder.addDetail("Files", &C2ClientConf::readFiles, &C2ClientConf::regsiterFiles);
        }

        void C2ClientConf::readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            _logFileHolder.read(node,hPP);
        }

        void C2ClientConf::regsiterLogFile(const std::string &full_path)
        {
            _logFileHolder.snmpRegister(full_path);
        }

        void C2ClientConf::readStatistic( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
        {
            _statisticHolder.read(node, hPP);
        }

        void C2ClientConf::regsiterStatistic(const std::string& full_path )
        {
            _statisticHolder.snmpRegister(full_path);
        }

        void C2ClientConf::readFiles(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            _filesHolder.read(node,hPP);
        }

        void C2ClientConf::regsiterFiles(const std::string &full_path)
        {
            _filesHolder.snmpRegister(full_path);
        }

        void C2ClientConf::readRequestParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            _requestParamsHolder.read(node,hPP);
        }

        void C2ClientConf::regsiterRequestParams(const std::string &full_path)
        {
            _requestParamsHolder.snmpRegister(full_path);
        }
    }
}