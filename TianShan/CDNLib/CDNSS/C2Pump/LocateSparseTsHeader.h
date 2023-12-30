#ifndef __ZQ_LocateSparseTsHeader_H__
#define __ZQ_LocateSparseTsHeader_H__
#include <ZQ_common_conf.h>
// error code
#define SPARSE_E_INVAL -1 // invalid parameters
#define SPARSE_E_BADFILE -2 // bad file name
#define SPARSE_E_IOFAULT -3 // io fault
#define SPARSE_E_NOMEM -4 // memory allocation failure
// return positive value for success
int64 locateSparseTsHeader(const char* filename, int64 startOffset = 0, int64 readBufferSize = 32*1024, int64 reservedTailSize = 0, int64 packetSize = 188, char packetLeadByte = 0x47);
#endif
