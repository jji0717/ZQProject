#include "BasePage.h"

#define LOG_MODULE_NAME         "BasePage"


namespace ContentProvisionClusterweb
{

	BasePage::BasePage(IHttpRequestCtx* pHttpRequestCtx) : 
_reqCtx(pHttpRequestCtx), 
_ic(NULL), 
_cpc(NULL)
{
	_lastError.clear();
	memset(szBuf, 0, sizeof(szBuf));
}

BasePage::~BasePage()
{
	if (_ic) {
		try {
            _ic->destroy();
        } catch (...){}
    }
	_ic = 0;
}

const char* BasePage::getLastError() const
{
	return _lastError.c_str();
}

void BasePage::setLastError(const char* error)
{
	if (NULL != error)
		_lastError = error;
}

void BasePage::addToLastError(const char* error)
{
	if (NULL != error)
	{
		// ÀÛ¼Ó´íÎóÐÅÏ¢
		_lastError += error;
	}
}

bool BasePage::process()
{
	bool bRet = false;

	try
	{
		if (NULL == _reqCtx)
		{
			setLastError("Http request context pointer is null");
			glog(EmergLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			return false;
		}
		IHttpResponse& responser = _reqCtx->Response();
		_varMap = _reqCtx->GetRequestVars();

		if (_varMap[contentprovisonclusterAddressKey].empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "contentprovisonclusterAddressKey not configured");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		std::string endpoint = "ContentProvisionCluster: ";
		endpoint += _varMap[contentprovisonclusterAddressKey];
		snprintf(szBuf, sizeof(szBuf) - 1, "<p><B>ContentProvisionCluster:</B> %s</p>", endpoint.c_str());
		responser<<szBuf; //tcp -h 10.50.12.4 -p 22288

		try
		{
			int i=0;
			_ic = Ice::initialize(i, NULL);
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "init ice environment caught %s", ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		try
		{
			_cpc = TianShanIce::ContentProvision::ContentProvisionClusterPrx::checkedCast(_ic->stringToProxy(endpoint));
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get  proxy(%s) caught %s", endpoint.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		if (isPostBack())
			bRet = post();
		else 
			bRet = get();
	}
	catch (const TianShanIce::ClientError& ex)
	{
		std::string err = "Caught " + ex.ice_name() + " during the processing.";
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), err.c_str());
		_reqCtx->Response().SetLastError(err.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		std::string err = "Caught " + ex.ice_name() + " during the processing.";
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), err.c_str());
		_reqCtx->Response().SetLastError(err.c_str());
	}
	catch (...)
	{
		setLastError("caught global unexpect exception");
		glog(EmergLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		IHttpResponse& responser = _reqCtx->Response();
		responser.SetLastError(getLastError());
	}

	return bRet;
}

bool BasePage::isPostBack() const
{
	return (_reqCtx->GetMethodType() == M_POST) ? true : false;
}


}
