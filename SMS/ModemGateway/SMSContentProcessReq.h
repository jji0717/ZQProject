#if !defined(SMSCONTENTPROCESSREQ_H)
#define SMSCONTENTPROCESSREQ_H

#include "afx.h"
#include "SMSComPortWriteThread.h"
#include "SMSProcessRawMsgThread.h"
#include "NativeThreadPool.h"
#include "Log.h"
#include "ScLog.h"
#include "winsock2.h"
#include "SMSXmlProc.h"
#include "ModemGateway.h"
#include "SMSDB.h"
#include "TicpProc.h"

class ModemGateway;
class TicpProc;

#define    NATIONAL_TELEPHONE_FORMAT          1
#define    INTERNATIONAL_TELEPHONE_FORMAT     2
#define    NOT_EXITED_TELEPHONE_FORMAT        0
#define    UNICODE_CODE                       1
#define    SEVEN_BIT_CODE                     2

class SMSContentProcessReq : public ZQ::common::ThreadRequest
{
public:
	SMSContentProcessReq(ZQ::common::NativeThreadPool& Pool, 
						 ModemGateway* pModemGateway, 
						 char* rawSMS);

	SMSContentProcessReq(ZQ::common::NativeThreadPool& Pool, 
						 ModemGateway* pModemGateway, 
						 int   id, 
						 int   mode,
						 char* telephone, 
						 char* entryTime, 
						 char* ticpContent, 
						 int operationTimes);
	
	~SMSContentProcessReq();

protected:
	int  run();
	void final(int retcode, bool bCancelled);


private:
	//  Parse Raw Content ////////////////////////////////////////////////////////////
	bool ParseRawMsg();

	//分解raw msg并且根据日期来判断是否需要继续进行处理
	// @param[in]  m_rawContent
	// @param[out] m_rawTelephoneNumber
	// @param[out] m_entryTime
	// @param[out] m_rawSMSContent
	// @param[out] m_codeMode
	bool ParseMsgAndJudgeProcess();

	// @param[in] content
	// @param[in] start
	// @param[in] length
	// @param[out] dest
	void getCharSegment(char* dest, char* content, int start, int length);

	//int  IsNativeTelephoneNumber(char *RawContent);
	int  IsNativeTelephoneNumber();

	//int  getCodeMode(char *RawContent);
	int  getCodeMode();

	// @param[in]  RawContent
	// @param[out] rawTelephoneNumber
	//void getTelephoneNumber(char* RawContent, char* rawTelephoneNumber);
	void getTelephoneNumber();
	
	// @param[in]  m_szRawMsg
	// @param[out] m_szEntryTime
	//void getEntryTime(char* RawContent, char* rawEntryTime);
	void getEntryTime();
	
	// @param[in]  RawContent
	// @param[out] rawSMSContent
	//void getContent(char* RawContent, char* rawSMSContent);
	void getContent();

	// exchange time format from char to CTime
	// @param[in]  rawEntryTime
	//void changeTimeFormat(char* rawEntryTime);
	void changeTimeFormat();

	// judge sms whether is overtime
	// origin: m_entryTime
	// return value: true is overtime, false is not overtime
	bool IsOvertime();

	// judge sms whether is processed
	// origin: m_operationTimes
	bool IsParsed();
	
	void TwoCharsExchange(char* InAndOutChar);

	// Unicode Decode ////////////////////////////////////////////////////////////
	//把字符中16进制的值转换为10进制（Int），例：A -> 10，但是ASCII码是65，所以得-55即(-48-7)
	int CharHexToInt(char inputChar);
	
	//主要解码函数
	// @param[in]  m_szRawContent
	// @param[out] m_wszContent
	//bool DecodeUnicode(char* rawSMSContent, wchar_t* wContent);
	bool DecodeUnicode();


	// UTF7 Decode ////////////////////////////////////////////////////////////////
	//把UTF7编码转换为正常值
	// return value is index of dest array
	// @param[in]  UTF7Content
	// @param[in]  UTF7Start
	// @param[in]  UTF7End
	// @param[in]  ANSIIndex
	// @param[out] ANSIContent
	int UTF7ToInt(int* UTF7, int UTF7Start, int UTF7End, int* ANSI, int ANSIIndex);
	
	//十六进制转换为二进制，char内容为0x61之类的
	int HexToBinar(char hexContent);
	
	//把2个char转换为一个二进制码（8位的）
	int TwoCharToOneBinar(char UTF7Content, char NextUTF7content);
	
	//二进制转换为十进制
	int BinarToDec(int ANSIResult);
	
	//主要解码函数
	// @param[in]  m_szRawContent
	// des:        m_szUTF7
	//bool DecodeUTF7(char* rawSMSContent);
	bool DecodeUTF7();
	

	// Unicode Coding ////////////////////////////////////////////////////
	// create Unicode to send
	// @param[in]  content
	// @param[out] send
	//void UnicodeCoding(wchar_t* content, char* send);//just for test
	void UnicodeCoding(wchar_t* content);//just for test

	// create sms cmd
	// @param[in]  Send
	// @param[in]  telephone
	// @param[out] pdu
	//void CreateCmd(char* send, char* telephone, char* pdu);
	void CreateCmd();

	// code telephone number
	// @param[in]   telephone
	// @param[out]  PDUTelephone
	void CodeTelephoneNumber(char* telephone, char* PDUTelephone);//just for test

	// UTF7 Coding ///////////////////////////////////////////////////////
	// UTF7 编码已经不需要了,因为发送的时候肯定是 UNICODE
	void UTF7Coding(char* UTF7Content, char* send);//just for test


	// create response SMS
	// @param[in] inputChar
	void createResponeSMS(wchar_t* inputChar);

	// get AssetCode and Delay
	// @param[out] assetCode
	// @param[out] delay
	// input: m_wszContent or m_szUTF7
	bool processContent();
	
	// 判断操作是否已达到操作最高次数
	bool IsOprFull();

	// delete SMS 
	void deleteSMS();

	//insert data to db
	void InsertSMSToDB();//把分解后的短信插入数据库

	// 根据 TICP 的返回值 ，设置给 TM 的短信内容
	void SetMesssageContent(int ret);

public:
	bool CodingUnicode();
	
	void UpdateDB(long id, bool flag);//处理完短信后更新DB中的字段
	void ParseResponseMsg();
	void GenerateResponseSMS();

	bool IsNeededToDecode();

	
private:
	char		m_RawMsg[320];//没有去掉短信头的
	char		m_RawContent[320];//没有解码的
	
	//UNICODE 存放解码后的全部短信内容
	wchar_t		m_wContent[160];//store chinese, one chinese = 4 char

	//UTF7 存放解码后的全部短信内容
	char		m_UTF7[160];//store ansi, 1:2
	
	char		m_TICPContent[500];// 从用户短信中的内容得来的，used by XML file

	char		m_Send[320];//存放从TICP交互后返回的结果内容
	
	char		m_pdu[512];

	int   m_nIndex;	  // DB index
	int   m_codeMode;//编码方式，1是Unicode，2是UTF7，0是错误
	int   m_telephoneNumberMode;//电话号码格式，国内，国际

	char  m_Telephone[13];
	char  m_EntryTime[20];//SMS time 
	CTime m_entryTime;//SMS time 

	// operation times
	int m_operationTimes;

	// sequence id,used by XML file
	int m_sequenceID;

	// 点播的字符
	char m_Play[20];

	// 聊天的字符
	char m_Chat[20];

	// 注册的字符
	char m_Reg[20];

	// 昵称的字符
	char m_NickName[20];

	int  m_mode;
	
	//select function time value
	DWORD m_timeout;
	
	//over time
	int m_overtime;

	// ip address
	char m_ip[20];
	
	// port
	int  m_port;
	
	// most operation times
	int m_mostOpertionTimes;

	// response or not
	DWORD m_response;
	
	// error response
	DWORD m_errorResponse;

	SMSProcessRawMsgThread*		m_pRawMessage;
	SMSComPortWriteThread*		m_pComWrite;
	ModemGateway*					m_pModemGateway;
	TicpProc*					m_ticpProc;
	SMSDB*						m_db;

	int m_ret;

	// db id
	long m_id;	
};
#endif  //!defined(SMSCONTENTPROCESSREQ_H)