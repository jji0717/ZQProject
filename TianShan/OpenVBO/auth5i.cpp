//extern "C"
//{
#include "hmac_md5.h"
//};
#include "XMLPreferenceEx.h"
#include "auth5i.h"
#include "urlstr.h"

#include <algorithm>

extern "C" {
#include <time.h>
}

Authen5i::Authen5i(ZQ::common::Log& log):_log(log)
{
}
Authen5i::~Authen5i()
{
}

bool Authen5i::loadKeyFile(const char* pathname)
{
	// Sample key file:
	// <?xml version="1.0" encoding="utf-8" ?>
	// <KeyStore>
	//   <Key id="1" >00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff</Key>
	//   <Key id="2" >20112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff</Key>
	//   <Key id="3" >30112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff</Key>
	// </KeyStore>

	try {

	ZQ::common::XMLPreferenceDocumentEx keyfile;
	if (!keyfile.open(pathname))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(Authen5i, "failed to load keyfile[%s]"), pathname);
		return false;
	}

	ZQ::common::XMLPreferenceEx* root = keyfile.getRootPreference();
	char buf[200];

	if (NULL == root || 0 != strcmp("KeyStore", root->name(buf, sizeof(buf)-2)))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(Authen5i, "No root elem <KeyStore> in keyfile[%s]"), pathname);
		return false;
	}

	for (ZQ::common::XMLPreferenceEx* keyElem = root->firstChild("Key"); NULL != keyElem; keyElem= root->nextChild())
	{
		int keyID=-1;
		std::string keyStr;
		if (keyElem->getPreferenceText(buf, sizeof(buf)-2))
			keyStr = buf;

		if (keyElem->getAttributeValue("id", buf, sizeof(buf)-2))
			keyID = atoi(buf);

		if (keyID >=0 && keyStr.size()>=128)
			setKey(keyID, keyStr.c_str());
	
		keyElem->free();
	}

	root->free();

	return true;
	}
	catch(const ZQ::common::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(Authen5i, "parse keyfile[%s] caught exception: %s"), pathname, ex.getString());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(Authen5i, "parse keyfile[%s] caught exception"), pathname);
	}

	return false;
/*
	ZQ::common::element_map_t::iterator elementIter=expatXml.m_element_map.begin();
	ZQ::common::element_data_t keyData;
	int keyID=0;
	std::string keyStr="";
	for (elementIter;elementIter != expatXml.m_element_map.end();elementIter++)
	{
		if ((*elementIter).first == "/KeyStore/Key")
		{
			keyData = (*elementIter).second;
			keyID = atoi((*(keyData.attributes.begin())).second.c_str());
			///< ??? ZKT: what if the "id" was not the first attribute of the element??

			keyStr=keyData.content;
			if(!setKey(keyID,keyStr.c_str()))
			{
				//the key is not a valid value  and don't add to the _keyList
			}
		}
	}
	*/
}

bool Authen5i::isExpired(const std::string& url)
{
	size_t pos = url.find("&E=");
	if (std::string::npos == pos)
		return true;

	std::string exp = url.substr(pos +sizeof("&E=")-1);
	pos = exp.find("&");
	if (std::string::npos != pos)
		exp = exp.substr(0, pos);

	// determine expiration
	char buf[100];
	time_t tExp = time(NULL); // now
	struct tm gmt = *gmtime(&tExp);
	strftime(buf, sizeof(buf)-2, "%Y%m%d%H%M%S", &gmt);
	return (strcmp(exp.c_str(), buf) <=0);
}


bool Authen5i::sign(std::string& url, int keyId/* =-1 */, const std::string& sessId/* =std::string */, const std::string& ip/*=std::string()*/, const std::string& expire/*=std::string()*/)
{
	ZQ::common::URLStr urlstr(url.c_str());
	std::string path, K, I, E, A, H;

	path = urlstr.getPath();

	if (keyId <0)
	{
		K = urlstr.getVar("k");
		if (!K.empty())
			keyId = atoi(K.c_str());
	}

	if (keyId <0 || _keyList.end() == _keyList.find(keyId)/* || keyId  > _keyList.size()*/)
	{
		_log("invalid keyId[%d] specified", keyId);
		return false;
	}

	I = sessId.empty() ? urlstr.getVar("i") : sessId;
	A = ip.empty() ? urlstr.getVar("a") : ip;
	E = expire.empty() ? urlstr.getVar("e") : expire;

	// TODO: return false for invalid I, A, E
	if (I.empty() || A.empty() || E.empty())
	{
		return false;
	}

	H = calculateSignature(path, keyId, I, A, E);
	char buf[256]="";
	snprintf(buf, sizeof(buf)-2, "?I=%s&K=%d&E=%s&A=%s&H=%s", I.c_str(), keyId, E.c_str(), A.c_str(), H.c_str());
	size_t pos = url.find('?');
	if (std::string::npos != pos)
		url = url.substr(0, pos);
	url += buf;
	return true;
}

bool Authen5i::authen(const std::string& url)
{
	ZQ::common::URLStr urlstr(url.c_str());
	std::string path, K, I, E, A, H, H2;
	int keyId =0;

	path = urlstr.getPath();
	I = urlstr.getVar("i");
	A = urlstr.getVar("a");
	E = urlstr.getVar("e");
	K = urlstr.getVar("k");
	if (!K.empty())
		keyId = atoi(K.c_str());

	H = urlstr.getVar("h");
	H2 = calculateSignature(path, keyId, I, A, E);

	std::transform(H.begin(), H.end(), H.begin(), toupper);
	std::transform(H2.begin(), H2.end(), H2.begin(), toupper);

	return (0 == H2.compare(H));
}

std::string Authen5i::calculateSignature(const std::string& uri_path, int keyId, const std::string& sessId, const std::string& ip, const std::string& expire)
{
	char buf[512]="";
	snprintf(buf, sizeof(buf)-2, "%s?I=%s&K=%d&E=%s&A=%s", uri_path.c_str(), sessId.c_str(), keyId, expire.c_str(),ip.c_str());

		// convert to lower case
	for (char *p=buf; *p; p++)
		*p = tolower(*p);

    //TODO
   // 4.   Convert the string to bytes using UTF-8 encoding 
	std::string uri(buf),utf8uri;
	EncodeToUTF8(uri,utf8uri);
   // 5.  Perform the HMAC-MD5 signature using the key bytes matching the key id. The result will be 16 bytes. 
	Key5i_t keyValue = {0};
	KeyList::iterator keyIter = _keyList.find(keyId);
	if (keyIter == _keyList.end())
	{
		keyIter = _keyList.begin();
	}
	keyValue = (*keyIter).second;
	//keyId   should return
	keyId = (*keyIter).first;

	uint8 res[MD5_BYTE_LEN+1];

	hmac_md5((uint8*)utf8uri.c_str(),(uint32)utf8uri.length(),(uint8*)keyValue.b,(uint32)sizeof(keyValue.b), (uint8*)res);
	res[MD5_BYTE_LEN]='\0';
	
	std::string H;
	for (int i =0; i<MD5_BYTE_LEN; i++)
	{
		char buffer[3];
		sprintf(buffer,"%02X\0", res[i]);
		H.append(buffer);
 	}

	return H;
}

int hashString( const std::string& keyStr, size_t keySize )
{
	const char *str = keyStr.c_str();		
	int hash = 0;
	while (*str)
	{
		// equivalent to: hash = 65599*hash + (*str++);
		hash = (*str++) + (hash << 6) + (hash << 16) - hash;
	}

	int keyId = (hash & 0x7FFFFFFF) % keySize;
	return keyId;
}

bool Authen5i::authC2( const std::string& paid, const std::string& pid, 
			const std::string& txnId, const std::string& clientSessId,
			const std::string& expiration, const std::string& signature ) 
{
	std::string keystr = paid + pid + txnId + clientSessId + expiration;
	int keyId = hashString( keystr, keySize() );
	return calculateSignature(paid,keyId,txnId,clientSessId,expiration) == signature ;
}

std::string Authen5i::signC2PlaylistItem(const std::string& providerAssetId, const std::string providerId, const std::string& txnId, const std::string& clientSessionId, const std::string& expire)
{
	if(keySize() <= 0)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(Authen5i, "The keyList is empty."));
		return "";
	}
	//generate keyId			
	std::string keystr = providerAssetId + providerId + txnId + clientSessionId + expire;
	
	int keyId = hashString(keystr, keySize() );

	return calculateSignature(providerAssetId, keyId, txnId, clientSessionId, expire);
}

void  Authen5i::EncodeToUTF8(const std::string& szSource, std::string& szFinal)
{
	unsigned short ch;
	unsigned char bt1, bt2, bt3, bt4, bt5, bt6;
	//int n, nMax = szSource.length();
	szFinal.clear();
//	std::string::iterator n = szSource.begin();
	unsigned int n;
	for (n = 0; n < szSource.length(); ++n)
	{
		ch = (unsigned short)szSource.at(n);
		if (ch <128)
		{
			char szTemp[2];
			szTemp[0] = (unsigned char)ch;
			szTemp[1] = ('\0');
			//strcat(szFinal, szTemp);
			szFinal.append(szTemp);
		}
		else if (ch <= 2047)
		{
			char szTemp[256];
			bt1 = (unsigned char)(192 + (ch / 64));
			bt2 = (unsigned char)(128 + (ch % 64));
			sprintf(szTemp, ("%02X%02X"), bt1, bt2);
			//strcat(szFinal, szTemp);
			szFinal.append(szTemp);
		}
		else if (ch <= 65535)
		{
			char szTemp[256];
			bt1 = (unsigned char)(224 + (ch / 4096));
			bt2 = (unsigned char)(128 + ((ch / 64) % 64));
			bt3 = (unsigned char)(128 + (ch % 64));
			sprintf(szTemp, ("%02X%02X%02X"), bt1, bt2, bt3);
			//strcat(szFinal, szTemp);
			szFinal.append(szTemp);
		}
		else if (ch <= 2097151)
		{
			char szTemp[256];
			bt1 = (unsigned char)(240 + (ch / 262144));
			bt2 = (unsigned char)(128 + ((ch / 4096) % 64));
			bt3 = (unsigned char)(128 + ((ch / 64) % 64));
			bt4 = (unsigned char)(128 + (ch % 64));
			sprintf(szTemp, ("%02X%02X%02X%02X"), bt1, bt2, bt3, bt4);
			//strcat(szFinal, szTemp);
			szFinal.append(szTemp);
		}
		else if (ch <=67108863)
		{
			char szTemp[256];
			bt1 = (unsigned char)(248 + (ch / 16777216));
			bt2 = (unsigned char)(128 + ((ch / 262144) % 64));
			bt3 = (unsigned char)(128 + ((ch / 4096) % 64));
			bt4 = (unsigned char)(128 + ((ch / 64) % 64));
			bt5 = (unsigned char)(128 + (ch % 64));
			sprintf(szTemp, ("%02X%02X%02X%02X%02X"), bt1, bt2, bt3, bt4, bt5);
			//strcat(szFinal, szTemp);
			szFinal.append(szTemp);
		}
		else if (ch <=2147483647)
		{
			char szTemp[256];
			bt1 = (unsigned char)(252 + (ch / 1073741824));
			bt2 = (unsigned char)(128 + ((ch / 16777216) % 64));
			bt3 = (unsigned char)(128 + ((ch / 262144) % 64));
			bt4 = (unsigned char)(128 + ((ch / 4096) % 64));
			bt5 = (unsigned char)(128 + ((ch / 64) % 64));
			bt6 = (unsigned char)(128 + (ch % 64));
			sprintf(szTemp, ("%02X%02X%02X%02X%02X%02X"), bt1, bt2, bt3, bt4, bt5, bt6);
			//strcat(szFinal, szTemp);
			szFinal.append(szTemp);
		}        
	}    
}

bool Authen5i::setKey(int idx, const char* keystr)
{
	Key5i_t key;
	memset(key.b, 0, sizeof(key.b));
	int i=0;
	for(i=0; i <64 && *keystr; i++)
	{
		char ch = tolower(*keystr++);
		if (ch >='0' && ch <='9')
			key.b[i] = ch - '0';
		else if (ch >='a' && ch <='f')
			key.b[i] = ch - 'a' +10;
		else break;

		ch = tolower(*keystr++);
		if (ch >='0' && ch <='9')
			ch = ch - '0';
		else if (ch >='a' && ch <='f')
			ch = ch - 'a' +10;
		else break;

		key.b[i] <<=4;
		key.b[i] |= ch;
	}

	if (i <64)
		return false;
	_keyList.insert(KeyList::value_type(idx, key));
	return true;
}