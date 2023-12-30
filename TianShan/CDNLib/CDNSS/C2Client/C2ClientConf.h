#ifndef _ZQ_C2CLIENT_CONF_H
#define _ZQ_C2CLIENT_CONF_H
#include <ConfigHelper.h>
#include <vector>
#include <string>

namespace ZQ{
    namespace StreamService{
        struct LogFile
        {
            std::string dir;
            int32 size;
            int32 level;
            int32 maxCount;
            int32 bufferSize;

            typedef ZQ::common::Config::Holder<LogFile> LogFileHolder;

            static void structure(LogFileHolder& holder);
        };
        typedef LogFile::LogFileHolder LogFileHolder;

        struct Statistic
        {
            int32 printInterval;
            typedef ZQ::common::Config::Holder<Statistic> StatisticHolder;

            static void structure(StatisticHolder& holder);
        };
        typedef Statistic::StatisticHolder StatisticHolder;

        struct RequestParamsConf
        {
            std::string upstreamIP;
            std::string url;
            std::string clientTransfer;
            std::string locateIP;
            int locatePort;
            int defaultGetPort;
            std::string transferRate;
            std::string ingressCapacity;
            std::string exclusionList;
            std::string transferDelay;
            std::string range;
            int indexTimeout;
            int indexRetry;
            int mainfileTimeout;
            int mainfileRetry;
            int enableTransferDelete;
            int alignment;

            typedef ZQ::common::Config::Holder<RequestParamsConf> RequestParamsConfHolder;

            static void structure(RequestParamsConfHolder& holder);
        };
        typedef RequestParamsConf::RequestParamsConfHolder RequestParamsHolder;

        struct Item
        {
            std::string name;
            typedef ZQ::common::Config::Holder<Item> ItemHolder;

            static void structure(ItemHolder& holder);
        };
        typedef Item::ItemHolder ItemHolder;

        struct Files
        {
            std::vector<ItemHolder> files;

            typedef ZQ::common::Config::Holder<Files> FilesHolder;

            static void structure(FilesHolder& holder);

            void readFiles(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

            void registerFiles(const std::string &full_path);
        };
        typedef Files::FilesHolder FilesHolder;

        struct C2ClientConf
        {
            int client;
            int loop;
            int interval;
            int32 eventloop;
            int32 file;
            LogFileHolder _logFileHolder;
            StatisticHolder _statisticHolder;
            RequestParamsHolder _requestParamsHolder;
            FilesHolder _filesHolder;
            typedef ZQ::common::Config::Holder<C2ClientConf> C2ClientConfHolder;

            static void structure(C2ClientConfHolder& holder);

            void readLogFile( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
            void regsiterLogFile(const std::string& full_path );
            void readStatistic( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
            void regsiterStatistic(const std::string& full_path );
            void readRequestParams( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
            void regsiterRequestParams(const std::string& full_path );
            void readFiles( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
            void regsiterFiles(const std::string& full_path );
        };

        //extern ZQ::common::Config::Loader<C2ClientConf> _c2conf;
    }
}

#endif