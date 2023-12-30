#ifndef _TIANSHAN_ONGOD_SENTRY_PAGE_HEADER_FILE_H__
#define _TIANSHAN_ONGOD_SENTRY_PAGE_HEADER_FILE_H__

#include "BasePage.h"
namespace ngod2view
{

class NgodPage : public BasePage
{
public:
	NgodPage( IHttpRequestCtx* pRequest );
	~NgodPage(void);

protected: 

	virtual bool get();
	virtual bool post();

protected:
	
	bool	displayNGODResUsage( );

private:
	static void splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter);

};
}
#endif//_TIANSHAN_ONGOD_SENTRY_PAGE_HEADER_FILE_H__
