#ifndef DSMCC_ERROR_CODE_H
#define DSMCC_ERROR_CODE_H

// this is a set of faked error code in addition to no error code defined in LSC
enum {
	lscErr_OK            = ZQ::DSMCC::RsnOK,
	lscErr_BadRequest    = 400,
	lscErr_NoSession     = ZQ::DSMCC::RspNeNoSession,
	lscErr_InvalidMethod = 456,
	lscErr_ServerErr     = 500,

};
#endif  //DSMCC_ERROR_CODE_H