#ifndef __GRIDHANDLER_H__
#define __GRIDHANDLER_H__

#include "TsLayout.h"

typedef int (*GridHandler)(::ZQTianShan::Layout::PLayoutCtx ctx);
GridHandler GetGridHandler(const char*);

#endif