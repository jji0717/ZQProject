
#include "StreamSmithEnv.h"

#include <memoryDebug.h>

namespace ZQ
{
namespace StreamService
{

StreamSmithEnv::StreamSmithEnv()
:mStreamerManager(this),
mSessionScaner(this,mStreamerManager)
{

}
StreamSmithEnv::~StreamSmithEnv()
{
}



}}
