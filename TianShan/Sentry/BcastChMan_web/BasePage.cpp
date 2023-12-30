#include "BasePage.h"

namespace BcastWebPage
{
	BasePage::BasePage(IHttpRequestCtx* pHttpRequestCtx) : 
		_reqCtx(pHttpRequestCtx), 
		chnlPub(NULL), 
		_gComm(NULL)
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

			std::string pubep = _varMap[PublisherKey];
			IHttpRequestCtx::RequestVars::iterator pubsh_itor;
			pubsh_itor = _varMap.find(PublisherKey);
			if (pubep.empty())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "No channel publisher end point");
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			snprintf(szBuf, sizeof(szBuf) - 1, "<p><B>BcastChannel:</B> %s</p>", _varMap[PublisherKey].c_str());
			responser<<szBuf;

			try
			{
				int i=0;
				_gComm = Ice::initialize(i, NULL);
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "init ice environment caught %s", ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}

			// add channel to db
			try
			{
				chnlPub = TianShanIce::Application::Broadcast::BcastPublisherPrx::checkedCast(_gComm->stringToProxy(pubep));
			}
			catch (const Ice::ObjectNotExistException&)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "BcastChannel not started or endpoint is wrong");
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "get channel publisher proxy caught %s", ex.ice_name().c_str());
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

