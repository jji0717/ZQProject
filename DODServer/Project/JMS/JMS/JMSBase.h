#ifndef __JMSBASE_H__
#define __JMSBASE_H__

class CActiveJMS;

#include<list>
class IParse;
class CJMSMessage;
class CJMSBase
{
public:
	//
	CJMSBase(CActiveJMS* pActiveJMS,long  handler ,_bstr_t m_key,_bstr_t value,  IParse* parse=NULL/*if recev, not NULL */);
	~CJMSBase(void);

private:
   _bstr_t m_key; //queue key name 

   _bstr_t m_value;//queue key value

   long    m_handler;//queue handler

   CActiveJMS* m_pActiveJMS;

   IParse*  m_parse;

public:
	///get queue key name 
	char* GetKeyValue(void) { return m_value;}

	///get queue key  
	char* GetKey(void){  return m_key;}


	///get queue handler  
	long GetHandle(void){   return  m_handler; }

	///get parser pointer
	IParse* GetParse(void){  return m_parse; }

	void TopicSend(CJMSMessage* message);

    void QueueSend(CJMSMessage* message);
};


typedef std::list< CJMSBase > CJMSBaseList;
#endif


