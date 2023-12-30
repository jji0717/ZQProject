#include "EventHelper.h"
#include "FileWriterConfig.h"

ZQ::common::Config::Loader<EventGateway::FileWriter::FileWriterConfig> fwConfig("EGH_FileWriter.xml");

int main() {
    fwConfig.loadInFolder("/opt/TianShan/etc");
    ZQ::common::FileLog flog("./rotate.log", 7);
    EventGateway::FileWriter::EventHelper eh(flog);
    if(!eh.init()) {
        printf("failed to init\n");
        return false;
    };

    std::map<std::string, std::string> test;
    test["content"] = "abc";
    int i = 0;
    for(; i < 1000; ++i) {
        eh.onEvent("Content", i, "Created", "2010", "local", test);
    }

    return 0;
}
