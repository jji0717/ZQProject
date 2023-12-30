#ifndef __ZQ_PRIVATEDATADEFINE_H__
#define __ZQ_PRIVATEDATADEFINE_H__




#define DEFAULT_TAG_LENGTH			        4
#define DEFAULT_OBJECT_KEY_LENGTH		    4
#define DEFAULT_INDEX_TABLE_DESC_LENGTH		16

#define ENCRYPT_MODE_NONE				0x00
#define ENCRYPT_MODE_ILP_NORMAL			0x01
#define ENCRYPT_MODE_ILP_SIMPLE			0x02
#define ENCRYPT_MODE_COMPRESS_SIMPLE	0x01


typedef struct _ObjectHeader
{
	char		szObjectTag[DEFAULT_TAG_LENGTH];
	char		nReserved[4];
	char		nObjectKeyLength;
	char		szObjectKey[DEFAULT_OBJECT_KEY_LENGTH];
	char		nObjectContentLength[4];

	_ObjectHeader()
	{
		memset( szObjectTag, 0, DEFAULT_TAG_LENGTH );
		memset( nReserved, 0xFF, 4 );
		nObjectKeyLength = DEFAULT_OBJECT_KEY_LENGTH;
		memset( szObjectKey, 0, DEFAULT_OBJECT_KEY_LENGTH );
		memset( nObjectContentLength, 0, 4 );
	}
} ObjectHeader;		// sizeof(ObjectHeader) = 17


typedef struct _IndexTableDescriptor
{
	BYTE	byTag;
	BYTE	byLength;
	BYTE	byTableCount;
	BYTE	byIDLength;									// = DEFAULT_OBJECT_KEY_LENGTH
	char	szPID[DEFAULT_OBJECT_KEY_LENGTH];
	BYTE	byDescLength;								// = DEFAULT_OBJECT_KEY_LENGTH
	char	szDesc[DEFAULT_INDEX_TABLE_DESC_LENGTH];
	_IndexTableDescriptor()
	{
		byTag = 0x80;
		byLength = sizeof(_IndexTableDescriptor) - 2;
		byTableCount = 0;
		byIDLength = DEFAULT_OBJECT_KEY_LENGTH;
		byDescLength = DEFAULT_INDEX_TABLE_DESC_LENGTH;
	}
} IndexTableDescriptor;


#define MAX_OBJECT_LENGTH               (64 * 1024 * 16 * 4)

#define MAX_TABLE_PAYLOAD_LENGTH		(4084 * 256)    // table payload.
#define MAX_OBJECT_PAYLOAD_LENGTH	    (MAX_TABLE_PAYLOAD_LENGTH-sizeof(ObjectHeader)) // TABLEPAYLOADMAXLEN - sizeof(ObjectHeader)

#define MAX_ONE_TABLE_TS_LEN   		    (23 * 188 * 256)    // 4096 bytes need 23 ts packets to load.

//
// DOD related Content Properties
//
const std::string CNTPRY_TS_OBJECT_NAME   = "ObjectName";
const std::string CNTPRY_TS_PID           = "PID";
const std::string CNTPRY_TS_TABLE_COUNT   = "TableCount";
const std::string CNTPRY_TS_TABLE_ID      = "TableId";
const std::string CNTPRY_TS_TABLE_IDEXT   = "TableIdExt";
const std::string CNTPRY_TS_OBJKEY        = "ObjectKey";
const std::string CNTPRY_TS_VERSION_NUM   = "VersionNumber";
const std::string CNTPRY_TS_OBJTAG        = "ObjectTag";


#define BUFF_WAIT_TIMEOUT   500    // ms


#endif