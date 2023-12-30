// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// ===========================================================================

#include "ContentImpl.h"

extern "C"
{
//	#include <io.h>
};

using namespace ::TianShanIce::Storage;
using namespace ::TianShanIce;
using namespace ZQ::common;

#define MOLOG	(store._log)

namespace ZQTianShan {
namespace ContentStore {


std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
#if 0	
	std::string result = pathname;
	std::transform(result.begin(), result.end(), result.begin(), (int(*)(int)) toupper);

	return result;
#else
	return pathname;
#endif	
}


}}

