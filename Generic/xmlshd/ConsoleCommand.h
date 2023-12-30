#include <sstream>
namespace Console
{
    typedef std::ostringstream DataBuffer;
    bool execute(DataBuffer &output, const char* pExe, const char* pCmdLine);
}
