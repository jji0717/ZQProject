// BaseSender.cpp: implementation of the BaseSender class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseSender.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BaseSender::BaseSender()
{

}

BaseSender::~BaseSender()
{

}

ZQ::common::Log* plog = NULL;
Config::Loader< EventSender>* pEventSenderCfg = NULL;

IMsgSender* g_pIMsgSender = NULL;

