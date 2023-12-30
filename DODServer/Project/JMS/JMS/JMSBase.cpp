#include "StdAfx.h"
#include "JMSBase.h"
#include"IParse.h"

CJMSBase::CJMSBase(CActiveJMS* pActiveJMS,long  handler ,_bstr_t key,_bstr_t value,  IParse* parse/*if recev, not NULL */)
{
   m_pActiveJMS=pActiveJMS;

   m_key = key;
   m_value = value;
   m_handler = handler;
   m_parse = parse;

}

CJMSBase::~CJMSBase(void)
{
 
}


void CJMSBase::TopicSend(CJMSMessage* message)
{
	m_pActiveJMS->TopicSend(message,m_handler);

}
 
void CJMSBase::QueueSend(CJMSMessage* message)
{
	m_pActiveJMS->QueueSend(message,m_handler);
}

