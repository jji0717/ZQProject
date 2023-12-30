#include "./PingHandler.h"

namespace TianShanS1
{

bool HandlePing::process()
{
	Ice::Identity ident;
	ident.name = _session;
	ident.category = ServantType;
	if (false == openSessionCtx(ident))
	{
		//composeErrorResponse();
		composeResponse(_statusCode);
		return false;
	}

	if (1 == _cltSessCtx.requestType) // SeaChange
		_pRequestWriter->setHeader(HeaderFormatType, SeaChangeFormat);
	else 
	{
		_pRequestWriter->setHeader(HeaderFormatType, TianShanFormat);
		_bNeedModifyResponse = false;
	}

	if (false == getWeiwooPrx() || false == getStreamPrx() || false == getPurchasePrx() || false == renewSession())
	{
		//composeErrorResponse();
		composeResponse(_statusCode);
		return false;
	}

	_statusCode = 200;
	composeResponse(_statusCode);
	//composeRightResponse();

	return true;
}

} // namespace TianShanS1

