#include "BasePage.h"

namespace ngod2view
{
	BasePage::BasePage(IHttpRequestCtx* pHttpRequestCtx) : 
		_reqCtx(pHttpRequestCtx), 
		_gComm(NULL), 
		_viewPrx(NULL)
	{
		_lastError.clear();
		memset(szBuf, 0, sizeof(szBuf));
	}

	BasePage::~BasePage()
	{
		if (_gComm != NULL)
			try {_gComm->destroy();} catch (...){}
		_gComm = NULL;
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
				glog(EmergLog, CLOGFMT(BasePage, "%s"), getLastError());
				return false;
			}
			IHttpResponse& responser = _reqCtx->Response();
			_varMap = _reqCtx->GetRequestVars();

			if (_varMap[Ngod2BindAddressKey].empty())
			{
				SNPRINTF(szBuf, sizeof(szBuf) - 1, "Ngod2BindAddress not configured");
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}

			std::string endpoint = "Ngod2View:";
			endpoint += _varMap[Ngod2BindAddressKey];
			SNPRINTF(szBuf, sizeof(szBuf) - 1, "<p><B>Ngod2:</B> %s</p>", endpoint.c_str());
			//responser<<szBuf;

			try
			{
				int i=0;
				_gComm = Ice::initialize(i, NULL);
			}
			catch (const Ice::Exception& ex)
			{
				SNPRINTF(szBuf, sizeof(szBuf) - 1, "init ice environment caught %s", ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}

			try
			{
				_viewPrx = NGOD::SessionViewPrx::checkedCast(_gComm->stringToProxy(endpoint));
			}
			catch (const Ice::Exception& ex)
			{
				SNPRINTF(szBuf, sizeof(szBuf) - 1, "get session view proxy(%s) caught %s", endpoint.c_str(), ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}

			if (isPostBack())
				bRet = post();
			else 
				bRet = get();
		}
		catch (...)
		{
			setLastError("caught global unexpect exception");
			glog(EmergLog, CLOGFMT(BasePage, "%s"), getLastError());
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

