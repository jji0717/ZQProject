#include "SMSContentProcessReq.h"

#define SEND_FAILED 0
#define NEED_NOT_SEND 1
#define NEED_SEND 2

#define WRONG_SEQUENCE 0
#define NOT_NEED_PSW 1
#define NOT_NEED_PROC 2

SMSContentProcessReq::SMSContentProcessReq(ZQ::common::NativeThreadPool& Pool, 
										   ModemGateway* pModemGateway, 
										   char* rawSMS)
:ThreadRequest(Pool)
{
	m_pModemGateway = pModemGateway;
	
	memset(m_TICPContent,	0x00,	500*sizeof(char));
	memset(m_RawContent,	0x00,	320*sizeof(char));
	memset(m_RawMsg,		0x00,	320*sizeof(char));
	memset(m_Send,			0x00,	320*sizeof(char));
	memset(m_pdu,			0x00,	512*sizeof(char));
	memset(m_ip,			0x00,	 20*sizeof(char));
	memset(m_Play,		    0x00,	 20*sizeof(char));
	memset(m_Chat,		    0x00,	 20*sizeof(char));
	memset(m_Reg,           0x00,    20*sizeof(char));
	memset(m_NickName,      0x00,    20*sizeof(char));

	strcpy(m_RawMsg, rawSMS);

	m_operationTimes = 0;
}


SMSContentProcessReq::SMSContentProcessReq(ZQ::common::NativeThreadPool& Pool, 
										   ModemGateway* pModemGateway, 
										   int   id,
										   int   mode,
										   char* telephone, 
										   char* entryTime, 
										   char* ticpContent,
										   int operationTimes)
:ThreadRequest(Pool)
{
	m_pModemGateway = pModemGateway;
	
	memset(m_TICPContent,  0x00,  500*sizeof(char));
	memset(m_Send,	       0x00,  320*sizeof(char));
	memset(m_pdu,	       0x00,  512*sizeof(char));
	memset(m_ip,	       0x00,   20*sizeof(char));
	memset(m_Play,         0x00,   20*sizeof(char));
	memset(m_Chat,         0x00,   20*sizeof(char));
	memset(m_Reg,          0x00,   20*sizeof(char));
	memset(m_NickName,     0x00,   20*sizeof(char));

	// difference from another
	m_id = id;
	m_mode = mode;
	strcpy(m_Telephone, telephone);
	strcpy(m_EntryTime, entryTime);
	strcpy(m_TICPContent, ticpContent);
	m_operationTimes = operationTimes;
}

SMSContentProcessReq::~SMSContentProcessReq()
{
}

//////////////////////////////////////////////////////////////////////////
int SMSContentProcessReq::run(void)
{
	//��д����	
	glog(Log::L_DEBUG, "SMSContentProcessReq::run");
	
	m_pComWrite = m_pModemGateway->getComWrite();
	if (m_pComWrite == NULL)
	{
		glog(Log::L_DEBUG, "Get Com point failed");
	}

	m_pRawMessage = m_pModemGateway->getRawThread();
	if (m_pRawMessage == NULL)
	{
		glog(Log::L_DEBUG, "Get raw message point failed");
	}

	m_db = m_pRawMessage->getDB();
	if (m_db == NULL)
	{
		glog(Log::L_DEBUG, "Get db point failed");
	}
	
	strcpy(m_Play,      m_pModemGateway->getPlayFlag());
	strcpy(m_Chat,      m_pModemGateway->getChatFlag());
	strcpy(m_Reg,       m_pModemGateway->getRegFlag());
	strcpy(m_NickName,  m_pModemGateway->getNCFlag());
	
	_strlwr(m_Play);
	_strlwr(m_Chat);
	_strlwr(m_Reg);
	_strlwr(m_NickName);

	wcstombs(m_ip,	 m_pModemGateway->getIP(), 20);
	
	m_mostOpertionTimes = m_pModemGateway->getTimes();
	m_errorResponse     = m_pModemGateway->getErrRsp();
	m_overtime          = m_pModemGateway->getOvertime();
	m_response          = m_pModemGateway->getResponse();
	m_timeout           = m_pModemGateway->getTimeout();
	m_port	            = m_pModemGateway->getIpPort();

	m_ticpProc = new TicpProc(m_ip, m_port, m_timeout);

	m_sequenceID = m_pRawMessage->getSequenceID();
	glog(Log::L_DEBUG, "Get Sequence ID <%d>", m_sequenceID);
	
	if (!IsParsed())
	{
		deleteSMS();
		Sleep(1000);
		
		if (ParseRawMsg())
		{
			if(!processContent())
			{
				glog(Log::L_INFO, "ID: %d, get content failed" , m_sequenceID);
				return 1;
			}
			//ֻ����ȷ�ı�־�Ż���ִ����ȥ,���Ҳ������ݿ�			
			m_id = m_pRawMessage->getDBID();

			InsertSMSToDB();
		}
		else
		{
			glog(Log::L_INFO, "ID: %d, need not continue!", m_sequenceID);
			return 1;
		}
	}

	m_ret = m_ticpProc->TicpProcess(m_mode, 
									m_Telephone,
									m_sequenceID,
									m_EntryTime,
									m_TICPContent);

	if (m_ret >= 0)
	{
		//TICP process success
		SetMesssageContent(m_ret);
	}
	else
	{
		//TICP process failed
		if (IsOprFull())
		{
			// ��TICP�����Ѵ�������
			SetMesssageContent(0);
			return 1;
		}
		// ��TICP����δ��������
		return 0;
	}

	UpdateDB(m_id, 1);//�������ݿ�����ɹ� 1 = finish
	return 1;
}

void SMSContentProcessReq::final(int retcode, bool bCancelled)
{
	//because m_ticpProc new is in the Run()
	if (m_ticpProc)
	{
		delete m_ticpProc;
		m_ticpProc = NULL;
	}

	if (retcode == 1)
	{
		glog(Log::L_DEBUG, "ID: %d, SMSContentProcessReq::final", m_sequenceID);
		delete this;
	}
	else
	{
		this->start();
	}
}

//////////////////////////////////////////////////////////////////////////
bool SMSContentProcessReq::ParseRawMsg()
{
	if(ParseMsgAndJudgeProcess())
	{
		if (m_codeMode == UNICODE_CODE)
		{
			if (!DecodeUnicode())
			{
				glog(Log::L_INFO, "ID: %d, Unicode ����ʧ��!", m_sequenceID);
				return false;
			}	
		}
		else if (m_codeMode == SEVEN_BIT_CODE)
		{
			if (!DecodeUTF7())
			{
				glog(Log::L_INFO, "ID: %d, UTF7 ����ʧ��!", m_sequenceID);
				return false;
			}
		}
	}
	else
	{
		glog(Log::L_INFO, "ID: %d, �����ڵı����ʽ!", m_sequenceID);
		return false;
	}
	return true;
}

//process Raw SMS
bool SMSContentProcessReq::ParseMsgAndJudgeProcess()
{
	glog(Log::L_DEBUG, "ID: %d, ԭʼ������: %s", m_sequenceID , m_RawMsg);

	m_telephoneNumberMode = IsNativeTelephoneNumber();

	if (m_telephoneNumberMode == NOT_EXITED_TELEPHONE_FORMAT)
	{
		return false;
	}
	
	getEntryTime();

	getTelephoneNumber();
	
	if (IsOvertime())
	{
		return false;
	}
	
	m_codeMode = getCodeMode();

	getContent();

	return true;
}

bool SMSContentProcessReq::DecodeUnicode()
{
	glog(Log::L_DEBUG, "ID: %d , Unicode ����", m_sequenceID);

	memset(m_wContent, 0x00, 160*sizeof(wchar_t));

	for (int i=0; i< strlen(m_RawContent)/4; i++)
	{
		m_wContent[i] =   CharHexToInt(m_RawContent[4*i]) *16*16*16  + 
						  CharHexToInt(m_RawContent[4*i + 1]) *16*16 + 
						  CharHexToInt(m_RawContent[4*i + 2]) *16    + 
						  CharHexToInt(m_RawContent[4*i + 3]);
	}
	glog(Log::L_DEBUG, "ID: %d , Unicode ����������: %s", m_sequenceID, m_wContent);
	return true;
}

bool SMSContentProcessReq::DecodeUTF7()
{
	glog(Log::L_DEBUG, "ID: %d , UTF7 ����", m_sequenceID);

	int charLength = strlen(m_RawContent);
	int binarLength = charLength/2;
	int UTF7SMSContent[140];//store UTF7, Ӣ�Ķ��ſ�����160����2λ�����1λӢ�ģ�������320��UTF7����280��2λ����1��Ӣ�ģ�����140
	int ANSIIntValue[160];//store ANSI int value
	int ANSIIndex = 0;//Array ANSIIntValue Index

	int k=0;
	while (k < binarLength)
	{
		if ((k+7) <= binarLength)//������������ڵ���7��
		{
			for (int i=k; i<k+7; i++)
			{
				UTF7SMSContent[i] = TwoCharToOneBinar(m_RawContent[2*i], m_RawContent[2*i+1]);
				glog(Log::L_DEBUG, "%d", UTF7SMSContent[i]);
			}
			ANSIIndex = UTF7ToInt(UTF7SMSContent, k, k+7, ANSIIntValue, ANSIIndex);
		}
		else//�������������7��������2������������ڱ߽�
		{
			for (int i=k; i<binarLength; i++)
			{
				UTF7SMSContent[i] = TwoCharToOneBinar(m_RawContent[2*i], m_RawContent[2*i+1]);
				glog(Log::L_DEBUG, "%d", UTF7SMSContent[i]);
			}
			ANSIIndex = UTF7ToInt(UTF7SMSContent, k, binarLength, ANSIIntValue, ANSIIndex);
		}
		k = k+7;
	}
	memset(m_UTF7, 0x00, 160*sizeof(char));
	for (int j=0; j<ANSIIndex; j++)
	{
		m_UTF7[j] = BinarToDec(ANSIIntValue[j]);
		glog(Log::L_DEBUG, "%08d  --  %c", ANSIIntValue[j], m_UTF7[j]);
	}
	m_UTF7[ANSIIndex] = 0;
	glog(Log::L_DEBUG, "ID: %d , UTF7������ <%s>" , m_sequenceID ,m_UTF7);
	return true;
}

////////////////////////////////////////////////////////
//////////////////// functions  ////////////////////////
// �ж��Ƿ����
bool SMSContentProcessReq::IsOvertime()
{
	CTime current = CTime::GetCurrentTime();
	if((m_entryTime + CTimeSpan(0 ,m_overtime ,0 ,0)) < current)
	{
		glog(Log::L_INFO, "ID: %d , ��ʱ %d Сʱ", m_sequenceID, m_overtime);
		wchar_t wTemp[] = L"�㲥��ʱ�޷����";
		createResponeSMS(wTemp);
		return true;
	}
	return false;
}
// ���ַ�����ȡ�������ַ���
void SMSContentProcessReq::getCharSegment(char* dest, char* content, int start, int length)
{
	for (int i=0; i<length; i++)
	{
		dest[i] = content[start+i];
	}
	dest[length] = '\0';
}
// �жϵ绰����ĸ�ʽ�����ʣ�OD�����߹��ڣ�OB��
int SMSContentProcessReq::IsNativeTelephoneNumber()
{
	char NativeFlag[3];

	// read telephone format from PDU Content
	getCharSegment(NativeFlag, m_RawMsg, 20, 2);
	
	if(strcmp(NativeFlag, "0B") == 0)
	{	
		glog(Log::L_DEBUG, "ID: %d , �����ֻ������ʽ: %s", m_sequenceID, NativeFlag);

		return NATIONAL_TELEPHONE_FORMAT;
	}
	else if (strcmp(NativeFlag, "0D") == 0)
	{
		wchar_t wszMsg[3];
		memset(wszMsg, 0x00, 3*sizeof(wchar_t));
		mbstowcs(wszMsg, NativeFlag, 3);
		glog(Log::L_DEBUG, "ID: %d , �����ֻ������ʽ: %s", m_sequenceID, NativeFlag);
		
		return INTERNATIONAL_TELEPHONE_FORMAT;
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , �����ڵ��ֻ������ʽ: %s", m_sequenceID, NativeFlag);
		return NOT_EXITED_TELEPHONE_FORMAT;
	}
}
//��ñ����ʽ
int SMSContentProcessReq::getCodeMode()
{
	char codeMode[3];
	if (m_telephoneNumberMode == NATIONAL_TELEPHONE_FORMAT)
	{
		getCharSegment(codeMode, m_RawMsg, 38, 2);
	}
	else if(m_telephoneNumberMode == INTERNATIONAL_TELEPHONE_FORMAT)
	{
		getCharSegment(codeMode, m_RawMsg, 40, 2);
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , ����� PDU ����!", m_sequenceID);
		return NULL;
	}

	if (strcmp(codeMode, "08") == 0)//Unicode
	{
		glog(Log::L_DEBUG, "ID: %d , %d �� UNICODE �����ʽ" , m_sequenceID, m_telephoneNumberMode);
		return UNICODE_CODE;
	}
	else if (strcmp(codeMode, "00") == 0)//7-bit
	{
		glog(Log::L_DEBUG, "ID: %d , %d �� UTF7 �����ʽ" , m_sequenceID, m_telephoneNumberMode);
		return SEVEN_BIT_CODE;
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , �����ڵı����ʽ!" , m_sequenceID);
		return NULL;
	}
}
//��õ绰����
void SMSContentProcessReq::getTelephoneNumber()
{
	if (m_telephoneNumberMode == NATIONAL_TELEPHONE_FORMAT) //���ڸ�ʽ
	{
		getCharSegment(m_Telephone, m_RawMsg, 24, 12);
		glog(Log::L_DEBUG, "ID: %d , PDU�ĵ绰������ %s" , m_sequenceID, m_Telephone);
	}
	else if (m_telephoneNumberMode == INTERNATIONAL_TELEPHONE_FORMAT)//���ʸ�ʽ
	{
		getCharSegment(m_Telephone, m_RawMsg, 26, 12);
		glog(Log::L_DEBUG, "ID: %d , PDU�ĵ绰������ %s" , m_sequenceID, m_Telephone);
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , �����PDU�绰����!" , m_sequenceID);
		return;
	}
	TwoCharsExchange(m_Telephone);
	m_Telephone[11] = 0;
	glog(Log::L_DEBUG, "ID: %d , �绰������: %s" , m_sequenceID, m_Telephone);
}

// ���ʱ��
void SMSContentProcessReq::getEntryTime()
{
	if (m_telephoneNumberMode == NATIONAL_TELEPHONE_FORMAT) //���ڸ�ʽ
	{
		getCharSegment(m_EntryTime, m_RawMsg, 40, 12);
		glog(Log::L_DEBUG, "ID: %d , PDU�ķ���ʱ��: %s" , m_sequenceID, m_EntryTime);
	}
	else if (m_telephoneNumberMode == INTERNATIONAL_TELEPHONE_FORMAT)//���ʸ�ʽ
	{
		getCharSegment(m_EntryTime, m_RawMsg, 42, 12);
		glog(Log::L_DEBUG, "ID: %d , PDU�ķ���ʱ��: %s" , m_sequenceID, m_EntryTime);
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , �����PDUʱ��" , m_sequenceID);
		return;
	}
	TwoCharsExchange(m_EntryTime);
	glog(Log::L_DEBUG, "ID: %d , ����ʱ��: %s" , m_sequenceID, m_EntryTime);
	
	// get time format (year-month-day hour:minute:second)
	changeTimeFormat();
}
//��ö�������
void SMSContentProcessReq::getContent()
{
	if (m_telephoneNumberMode == NATIONAL_TELEPHONE_FORMAT)
	{
		getCharSegment(m_RawContent, m_RawMsg, 56, strlen(m_RawMsg)-56);
	}
	else if(m_telephoneNumberMode == INTERNATIONAL_TELEPHONE_FORMAT)
	{
		getCharSegment(m_RawContent, m_RawMsg, 58, strlen(m_RawMsg)-58);
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , �����PDU����!" , m_sequenceID);
		return;
	}
	glog(Log::L_DEBUG, "ID: %d , ԭʼ��PDU����: %s" , m_sequenceID, m_RawContent);
}

//convert char time to CTime time
void SMSContentProcessReq::changeTimeFormat()
{
	char entrytime[13];
	strncpy(entrytime, m_EntryTime, 12);
	int year   = (entrytime[0]-48) * 10 + (entrytime[1]-48) + 2000;
	int month  = (entrytime[2]-48) * 10 + (entrytime[3]-48);
	int day    = (entrytime[4]-48) * 10 + (entrytime[5]-48);
	int hour   = (entrytime[6]-48) * 10 + (entrytime[7]-48);
	int minute = (entrytime[8]-48) * 10 + (entrytime[9]-48);
	int second = (entrytime[10]-48) * 10 + (entrytime[11]-48);
	m_entryTime = CTime(year ,month ,day ,hour ,minute ,second);

	memset(m_EntryTime, 0x00, 20*sizeof(char));
	sprintf(m_EntryTime, "%d-%d-%d %d:%d:%d", year, month, day, hour ,minute ,second);
}

//�ַ���������
void SMSContentProcessReq::TwoCharsExchange(char* InAndOutChar)
{
	int inputCharLength = strlen(InAndOutChar);
	if (inputCharLength%2 == 1)
	{
		glog(Log::L_INFO, "ID: %d , ������������ݳ���Ӧ����˫��!" , m_sequenceID);
		return;
	}
	char temp;
	for(int i=0; i< inputCharLength; i=i+2)
	{
		temp = InAndOutChar[i];
		InAndOutChar[i] = InAndOutChar[i+1];
		InAndOutChar[i+1] = temp;
	}
}
//////////////// Unicode Decode ////////////////////////////
//16���Ƶ��ַ�ת��ΪInt
int SMSContentProcessReq::CharHexToInt(char inputChar)
{
	if (((inputChar-48) >= 0) && ((inputChar-48) <= 9) )
	{
		return (inputChar - 48);
	}
	else if ((inputChar-48) > 9)
	{
		return (inputChar - 55);
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , Wrong input char!" , m_sequenceID);//0123456789ABCDEF
		return false;
	}
}
///////////////// UTF7 Decode ///////////////////////////////
//////////////////////////////////////////////////////////////////////////
//ת��UTF7����
int SMSContentProcessReq::UTF7ToInt(int* UTF7, int UTF7Start, int UTF7End, int* ANSI, int ANSIIndex)
{
	printf("DecodeUTF7IntContent!\n");
	int nUTF7[8];
//	nUTF7[0] = 1100001; //1100001
//	nUTF7[1] = 11110001;//1100010
//	nUTF7[2] = 10011000;//1100011
//	nUTF7[3] = 1011100; //1100100
//	nUTF7[4] = 110110;  //1100101
//	nUTF7[5] = 10011111;//1100110
//	nUTF7[6] = 11010001;//1100111
//	nUTF7[7] = 0;       //1101000
	int interval = UTF7End - UTF7Start;
	if (interval > 7)
	{
		glog(Log::L_INFO, "ID: %d , UTF7 content is too long, should not be more than 7!" , m_sequenceID);
		return 0;
	}
	for (int k=0; k < interval; k++)
	{
		nUTF7[k] = UTF7[UTF7Start+k];
	}
	int temp1 = 0;
	int temp2 = 0;
	int temp = 0;
	int p = 10000000; 
	int q = 1;
	for (int i=0; i < interval+1; i++)//�Ժ�8����interval
	{
		temp = nUTF7[i];//

		temp1 = nUTF7[i]/p;
		
		nUTF7[i]%=p;

		temp = nUTF7[i];//
		
		nUTF7[i] *= q;
		
		temp = nUTF7[i];//
		
		nUTF7[i] += temp2;
		
		temp2 = temp1;
		
		p /= 10;
		
		q *= 10;
		
		temp = nUTF7[i];//
	}
	for (int j=0; j<interval; j++)
	{
		ANSI[ANSIIndex + j] = nUTF7[j];
		TRACE(L"%d\n", nUTF7[j]);
	}
	if (interval == 7)
	{
		ANSI[ANSIIndex + interval] = nUTF7[interval];
		ANSIIndex = ANSIIndex + interval + 1;
	}
	else
	{
		ANSIIndex = ANSIIndex + interval;
	}
	return ANSIIndex;
}
//////////////////////////////////////////////////////////////////////////
//16����ת����2����
int SMSContentProcessReq::HexToBinar(char hexContent)
{
	int result = 0;
	switch (hexContent)
	{
	case '0':
		result = 0;
		break;
	case '1':
		result = 1;
		break;
	case '2':
		result = 10;
		break;
	case '3':
		result = 11;
		break;
	case '4':
		result = 100;
		break;
	case '5':
		result = 101;
		break;
	case '6':
		result = 110;
		break;
	case '7':
		result = 111;
		break;
	case '8':
		result = 1000;
		break;
	case '9':
		result = 1001;
		break;
	case 'A':
		result = 1010;
		break;
	case 'B':
		result = 1011;
		break;
	case 'C':
		result = 1100;
		break;
	case 'D':
		result = 1101;
		break;
	case 'E':
		result = 1110;
		break;
	case 'F':
		result = 1111;
		break;
	default:
		break;
	}
	return result;
}
//////////////////////////////////////////////////////////////////////////
//2���ַ�ת����1��
int SMSContentProcessReq::TwoCharToOneBinar(char UTF7Content, char NextUTF7content)
{
	int firstContent = HexToBinar(UTF7Content);
	int secondContent = HexToBinar(NextUTF7content);
	return (firstContent*10000 + secondContent);
}
//////////////////////////////////////////////////////////////////////////
//������ת����ʮ����
int SMSContentProcessReq::BinarToDec(int ANSIIntValue)
{
	if (ANSIIntValue <0)
	{
		glog(Log::L_INFO, "ID: %d , Int value should be more than 0!" , m_sequenceID);
		return false;
	}
	int ANSIValue = ANSIIntValue;
	int result = 0;
	int indexOfBinar = 1;
	while(ANSIValue > 0)
	{
		result += (ANSIValue%10)*indexOfBinar;
		indexOfBinar *= 2;
		ANSIValue/=10;
	}
	return result;	
}
////////////// Unicode Code ////////////////////////////////////
void SMSContentProcessReq::CreateCmd()
{
	int totalLength = 15;
	//add 
	char cmd[400];
	char PDUTelephoneNumber[13];
	memset(cmd, 0x00, 400*sizeof(char));
	memset(PDUTelephoneNumber, 0x00, 13*sizeof(char));
	strcpy(cmd, "0031000D9168");//00 31 00 0D 91 68�����ʸ�ʽ
	//add telephone number
	CodeTelephoneNumber(m_Telephone ,PDUTelephoneNumber);
	strcat(cmd, PDUTelephoneNumber);//cmd = 0031000BA13118989364F2
	//add 
	strcat(cmd, "0008A7");//result = 0031000BA13118989364F20008A7
	//add content length
	int contentLength = strlen(m_Send);//12����Ҫ����2,��Ϊ��WORD
	char contentPDULength[3];
	memset(contentPDULength, 0x00, 3*sizeof(char));
	sprintf(contentPDULength, "%x", contentLength/2);
	if (strlen(contentPDULength) ==1 )
	{
		strcat(cmd, "0");
		strcat(cmd, contentPDULength);
	}
	else
	{
		strcat(cmd, contentPDULength);
	}
	//add content,SMS Content is in m_szSend
	strcat(cmd, m_Send);
	//put cmd to write com queue
	totalLength += contentLength/2;
	sprintf(m_pdu, "AT+CMGS=%d\n", totalLength);
	strcat(m_pdu, cmd);
	strcat(m_pdu, "\x01a");
	glog(Log::L_DEBUG, "ID: %d , PDUCmd: %s" , m_sequenceID, m_pdu);
}
void SMSContentProcessReq::CodeTelephoneNumber(char* telephone, char* PDUTelephone)
{
	char temp[13];
	memset(temp, 0x00, 13*sizeof(char));
	strcpy(temp, telephone);
	strcat(temp, "F");
	strcpy(PDUTelephone, temp);
	TwoCharsExchange(PDUTelephone);

	glog(Log::L_DEBUG, "ID: %d , PDU�ĵ绰����: %s" , m_sequenceID, PDUTelephone);
}
void SMSContentProcessReq::UnicodeCoding(wchar_t* content)
{//m_wszContent -> m_Send
	wchar_t wTemp[10];
	wchar_t wUnicodeContent[320];
	memset(wTemp,			0x00, 10*sizeof(wchar_t));
	memset(wUnicodeContent, 0x00, 320*sizeof(wchar_t));
	
	for (int i = 0; i < wcslen(content); i++)
	{
		swprintf(wTemp, L"%04x", content[i]);
		wcscat(wUnicodeContent, wTemp);
	}

	wcstombs(m_Send, wUnicodeContent, 320);
	glog(Log::L_DEBUG, "ID: %d , %s" , m_sequenceID, m_Send);
}

// UTF7 coding ///////////////////////////////////////////////////////////////
// UTF7 �����Ѿ�����Ҫ��,��Ϊ���͵�ʱ��϶��� UNICODE
void SMSContentProcessReq::UTF7Coding(char* UTF7Content, char* send)
{//m_szUTF7 -> m_wszContent
	mbstowcs(m_wContent, m_UTF7, 160);
	UnicodeCoding(m_wContent);
}

bool SMSContentProcessReq::IsParsed()
{
	if (m_operationTimes == 0)
	{
		return false;
	}
	return true;
}


bool SMSContentProcessReq::processContent()
{
	char  temp[160];
	char  Para2[10] = "";
	char  Para3[10] = "";
	char  tempFlag[10];
	memset(tempFlag, 0x00, 10*sizeof(char));

	if (m_codeMode == UNICODE_CODE)
	{
		WideCharToMultiByte(CP_ACP, 0, m_wContent, -1, temp, 160, 0, 0);

//		sscanf(szContent, "%s%s%s", Para1, Para2, Para3);
	}
	else if(m_codeMode == SEVEN_BIT_CODE)
	{
//		sscanf(m_szUTF7, "%s%s%s", Para1, Para2, Para3);
		strcpy(temp, m_UTF7);
	}
	
	_strlwr(temp);
	
	//ȥ���ո�
	int len = 0;
	while(temp[len] == 32)
	{
		len ++;
	}

	char *p = strstr((temp + len), m_Play);
	char *q = strstr((temp + len), m_Chat);
	char *r = strstr((temp + len), m_Reg);
	char *s = strstr((temp + len), m_NickName);
	if (p == (temp + len))
	{
		m_mode = PLAY;
		strcpy(tempFlag, m_Play);
		glog(Log::L_INFO, L"�㲥��־");
	}
	else if (q == (temp + len))
	{
		m_mode = CHAT;
		strcpy(tempFlag, m_Chat);
		glog(Log::L_INFO, L"�����־");
	}
	else if (r == (temp + len))
	{
		m_mode = REGISTER;
		strcpy(tempFlag, m_Reg);
		glog(Log::L_INFO, L"ע���־");
	}
	else if (s == (temp + len))
	{
		m_mode = NICKNAME;
		strcpy(tempFlag, m_NickName);
		glog(Log::L_INFO, L"�ǳƱ�־");
	}
	else
	{
		glog(Log::L_INFO, "ID: %d , play flag is wrong" , m_sequenceID);
		wchar_t wTemp[] = L"��־����";
		createResponeSMS(wTemp);
		return false;
	}

	// ����
	len = len + strlen(tempFlag);	
	//ȥ����־������Ŀո�
	while (temp[len] == 32)
	{
		len ++;
	}
	//���ȥ���ո���û���ˣ����˳�
	if (temp[len] == '\0')
	{
		glog(Log::L_DEBUG, L"��־������û������");
		return false;
	}

	if (m_mode == PLAY	    || m_mode == CHAT || 
		m_mode == REGISTER  || m_mode == NICKNAME)
	{
		strcpy(m_TICPContent, temp + len);
		glog(Log::L_INFO, "����<%s>" , m_TICPContent);
	}
	return true;
}

void SMSContentProcessReq::createResponeSMS(wchar_t* inputChar)
{
	if (m_response == 1)
	{
		UnicodeCoding(inputChar);
	
		CreateCmd();
	
		m_pComWrite->AddWriteMsg(m_pdu);
	}
}

bool SMSContentProcessReq::IsOprFull()
{
	if (m_operationTimes < m_mostOpertionTimes)
	{
		m_operationTimes++;
		glog(Log::L_DEBUG, "ID: %d , Update DB" , m_sequenceID);
		UpdateDB(m_id, 0);
		return false;
	}

	UpdateDB(m_id, true);
	
	wchar_t wTemp[] = L"�㲥�����޷����";
	createResponeSMS(wTemp);
	return true;
}


void SMSContentProcessReq::deleteSMS()
{
	m_pRawMessage->deleteSMS();
}

void SMSContentProcessReq::InsertSMSToDB()
{
	char content[160];
	memset(content, 0x00, 160*sizeof(char));
	if (m_codeMode == UNICODE_CODE)
	{
		WideCharToMultiByte(CP_ACP, 0, m_wContent, -1, content, 160, 0, 0);
	}
	else if (m_codeMode == SEVEN_BIT_CODE)
	{
		strcpy(content, m_UTF7);
	}

	m_db->Insert(0, 
				 m_mode,
				 m_id,
				 m_Telephone,
				 m_Telephone,
				 m_EntryTime,
				 content,
				 m_TICPContent,
				 0, 0,
				 NULL);
	
	glog(Log::L_DEBUG, "ID: %d , insert db, db id: %d" , m_sequenceID, m_id);
}

void SMSContentProcessReq::UpdateDB(long id, bool flag)
{
	m_db->UpdateState(id, flag, flag, m_Send);
}

void SMSContentProcessReq::SetMesssageContent(int ret)
{	
	memset(m_Send, 0x00, 320*sizeof(char));

	if (m_pModemGateway->GetReturnCode(ret, m_Send))
	{
		wchar_t send[320];
		memset(send,	0x00, 320*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_Send, -1, send, 320);
		
		glog(Log::L_DEBUG, "return text <%s>", m_Send);

		createResponeSMS(send);
	}
}
