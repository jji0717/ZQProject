#include <C2Request.h>
#include <FileLog.h>

int main(int argc, char const *argv[])
{
    ZQ::common::FileLog g_log(strLogPath.c_str(), ZQ::common::Log::L_DEBUG);
    ZQ::StreamService::RequestParams params;
    params.alignment = 1;
    std::string senddata1;
    for (int i = 0; i < 110; ++i)
    {
        senddata1.append("1234567890");
    }

    ZQ::StreamService::C2ReadFile::Ptr  readPtr = new ZQ::StreamService::C2ReadFile(NULL, g_log, params);
    readPtr->onData("A", 1);
    readPtr->onData(senddata2, senddata2.length(), true);
    readPtr->onData("B", 1);

    return 0;
}