#pragma warning (disable:4786)

#include "windows.h"
#include "itv.h"
#include "ids.h"
#include "getopt.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

#include <iomanip>

namespace {
	ITVVERSION itvVersion;
	IDSSESS idsSession;
	ITVSTATUSBLOCK itvStatus;

	const wchar_t* ITV_USER_NAME = L"MD4File";
	const wchar_t* METADATA_NAME = L"*";

	wchar_t* defaultPID = 0;
	wchar_t* prefix = 0;

	const int METADATA_COUNT = 1;

	bool verbose = false;
	bool withTitle = false;

	enum TYPE {ELEMENT, ASSET};

	typedef std::map<std::wstring, METADATA>::const_iterator DATASET_ITER;

	typedef struct {
		DWORD id;
		std::string name;
	} Element;
}



/*
output format:
"AE name","element md","element md","asset md","asset md"
*/

void usage() {

		printf("\nQuery IDS for the specified metadata, exports as csv format\n\n");
		printf("Usage: MD4File -i <IDS ip address>\n");
		printf("       -f <file [path:stdin]> file containing one element ID per line or standard input\n");
		printf("       -e <element name>\n");
		printf("       -m <asset metadata names>      the asset metadata names, delimited by comma, to query\n");
		printf("       -l <element metadata names>    the element metadata names, delimited by comma, to query\n");
		printf("       -d <default PID>               the default PID for element when it is empty\n");
		printf("       -p <prefix>                    the prefix used to generate PAID for element when it is empty\n");
		printf("       -t with title\n");
		printf("       -v with statistics\n");
		printf("       -h display this help\n\n"); 
		printf("sample: MD4File -i 10.11.0.22 -e 0038e4c9 -l ProviderId,ProviderAssetId \n");       
		printf("sample: MD4File -i 10.11.0.22 -f aelist.txt -l ProviderId,ProviderAssetId -m ProviderId,ActivateTime\n"); 
		printf("sample: MD4File -i 10.11.0.22 -f stdin -m ProviderId,ActivateTime\n");       
}

std::vector<std::string> split(const std::string& src, char sep, size_t counts=0) {
	std::vector<std::string> result;
	
	if(src.empty()) {
		return result;
	}
	
	std::string sub = src;
	std::string::size_type pos = sub.find_first_of(sep);
	
	size_t cnt = 0;
	while(pos != std::string::npos) {
		result.push_back(sub.substr(0, pos));
		sub = sub.substr(pos+1); 	
		
		if(++cnt == counts) {
			break;
		}
		
		pos = sub.find_first_of(sep);
	}
	if(!sub.empty()) {
		result.push_back(sub);
	}
	
	if(result.empty()) {
		result.push_back(sub);
	}
	
	return result;
}

bool init(const std::string& server) {
	DWORD res = 0;

	itvVersion.VersionComponents.byteMajor = 1;
	itvVersion.VersionComponents.byteMinor = 0;

	res = IdsInitialize(&itvVersion, NULL);
	if (res != 0) {
		std::cerr << "failed to initialize Ids: (" << res << ")" << std::endl;
		return false;
	}
	
	wchar_t* idsAddr = 0;
	size_t s = mbstowcs(0, server.c_str(), 0);
	
	idsAddr = new wchar_t[(s+1)*sizeof(wchar_t)];
	mbstowcs(idsAddr, server.c_str(), s+1);			

	res = IdsBind(idsAddr, const_cast<wchar_t*>(ITV_USER_NAME), 1, 0, &idsSession, &itvStatus, 0,	NULL);

	delete idsAddr;

	if (res != 0) {
		std::cerr << "failed to bind with Ids: (" << res << ")" << std::endl;
		return false;
	}

	return true;
}

bool uninit() {
	DWORD res = 0;	
	
	res = IdsUnbind(&idsSession, IDS_GRACEFUL, &itvStatus, 0);
	if (res != 0) {
		std::cerr << "failed to unbind with Ids" << std::endl;
		return false;
	}

// 	res = IdsUninitialize();
// 	if (res != 0) {
// 		std::cerr << "failed to uninit Ids" << std::endl;
// 		return false;
// 	}
	
	return true;
}


void initMeta (METADATA** data) {
	*data = new METADATA;
	memset(*data, 0, sizeof(METADATA));
	(*data)->Version = itvVersion;
	memcpy((*data)->wszMdName, METADATA_NAME, sizeof(METADATA_NAME));
}


void printMeta (const METADATA* data) {
	switch (data->wMdType) {
	case IDS_INTEGER:
		std::cout << data->iVal;
		break;

	case IDS_REAL:
		std::cout << data->rVal;
		break;

	case IDS_FLOAT:
		std::cout << std::setprecision(0) << std::setiosflags(std::ios::fixed) << data->fVal;
		break;

	case IDS_STRING:
		std::wcout << data->sVal;
		break;
	
	case IDS_DATETIME:
		char buffer[20];
		strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&data->tVal));
		std::cout << buffer;
		break;

	case IDS_BINARY:
		{
			unsigned char* p = data->bVal;
			DWORD len = data->wMdLength;
			while(len--) {
				std::cout << std::hex << (unsigned int)(*p++);
			}
		}
		break;

	case IDS_BOOLEAN:
		std::wcout << std::boolalpha << (bool)(data->bitVal);
		break;
	
	case IDS_ASN1:
		std::cout << data->aVal;
		break;
	}
}


void populateMeta(IN const std::vector<std::string> mds, OUT std::vector<std::wstring>& meta) {
	
	for(size_t i = 0; i < mds.size();  ++i) {
		std::string tmp = mds.at(i);
		size_t len = tmp.length();

		size_t s = mbstowcs(0, tmp.c_str(), 0);
		wchar_t* md = new wchar_t[(s+1)*sizeof(wchar_t)];
		
		mbstowcs(md, tmp.c_str(), s+1);
		meta.push_back(std::wstring(md));
		delete md;
	}
}

DWORD String2ID(const std::string& id) {
	DWORD elementID = 0;
	std::istringstream iss(id);
	iss >> std::hex >> elementID;

	return elementID;
}

std::string ID2String(DWORD id) {
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(8) << std::hex << id;
	
	return oss.str();
}

bool parseArgs(IN int argc, 
			   IN char** argv,
			   OUT std::string& idsAddr, 
			   OUT std::vector<Element>& elements, 
			   OUT std::vector<std::wstring>& elementMeta,
			   OUT std::vector<std::wstring>& assetMeta) {

	if(argc < 2) {
		usage();
		return false;
	}

	bool fromFile = false;

	int ch;
	while((ch = getopt(argc, argv, "he:i:l:m:f:d:p:tv")) != EOF)	{
		
		switch (ch) {
			
		case 'h':
			
			usage();
			return false;
			
		case 'e':
			
			if(fromFile) {
				std::cerr << "can't specify elementID and file at the same time";
				return false;
			}
			
			{
				DWORD id = String2ID(optarg);
				if(!id) {
					std::cerr << "invalid element ID: (" << optarg << ")" << std::endl;
					return false;
				}
				
				Element e;
				e.id = id;
				e.name = optarg;

				elements.push_back(e);
			}
			
			break;
			
		case 'i':
			
			idsAddr = optarg;
			break;
		
		case 't':
			withTitle = true;
			break;

		case 'l':
			
			populateMeta(split(optarg, ','), elementMeta);
			break;
			
		case 'm':
			
			populateMeta(split(optarg, ','), assetMeta);
			break;
			
		case 'f':
			
			if(!elements.empty()) {
				std::cerr << "can't specify elementID and file at the same time";
				return false;
			}
			
			fromFile = true;
			
			{
				if(!stricmp(optarg, "stdin")) {
					std::string line;

					while(std::cin >> line) {
						DWORD id = String2ID(line);
						if(!id) {
							std::cerr << "invalid element ID: (" << line << ")" << std::endl;
							continue;
						}
						Element e;
						e.id = id;
						e.name = line;
						
						elements.push_back(e);	
					}
				}
				else {
					std::ifstream src(optarg);
					if(!src) {
						std::cerr << "bad source file" << std::endl;
						return false;
					}
				
					std::string line;
					while(src >> line) {
						DWORD id = String2ID(line);
						if(!id) {
							std::cerr << "invalid element ID: (" << line << ")" << std::endl;
							continue;
						}
						Element e;
						e.id = id;
						e.name = line;

						elements.push_back(e);
					}
				}
			}
			
			break;
		
		case 'd':
			{
				size_t s = mbstowcs(0, optarg, 0);;
				defaultPID = new wchar_t[(s+1)*sizeof(wchar_t)];
				mbstowcs(defaultPID, optarg, s+1);
			}
			
			break;

		case 'p':
			{
				size_t s = mbstowcs(0, optarg, 0);
				prefix = new wchar_t[(s+1)*sizeof(wchar_t)];
				mbstowcs(prefix, optarg, s+1);
			}
			
			break;

		case 'v':

			verbose = true;
			break;
			
		default:
			
			std::cerr << "unknown option (" << ch << ")" << std::endl;
			return false;
		}
	}
	
	if(idsAddr.empty()) {
		std::cerr << "must specify IDS server" << std::endl;
		return false;
	}
	
	if(elements.empty()) {
		std::cerr << "must specify element name" << std::endl;
		return false;
	}
	
	if(elementMeta.empty() && assetMeta.empty()) {
		std::cerr << "must specify metadata" << std::endl;
		return false;
	}

	return true;
}

bool readMetadata(
		TYPE type, 
		DWORD id, 
		std::map<std::wstring, METADATA>& dataset, 
		METADATA** md, 
		DWORD* count,
		OBJECTLIST** objs,
		DWORD* count2) {

//	METADATA* md = 0, *tmp = 0;
	METADATA* tmp = 0;
	initMeta(&tmp);
	
//	DWORD count = 0, res = 0;
	DWORD res = 0;

	if(type == ELEMENT) {
		res = IdsReadAtomicElement(&idsSession, id, &tmp, METADATA_COUNT, md, count, &itvStatus, 0);
	
		if (!res) {

			for(short i = 0; i < (*count); ++i) {
				if(!wcscmp((*md+i)->wszMdName, L"ProviderId") && !((*md+i)->wMdLength) && defaultPID) {
					(*md+i)->sVal = new wchar_t[wcslen(defaultPID)+sizeof(wchar_t)];
					wcscpy((*md+i)->sVal, defaultPID);
					(*md+i)->wMdLength = wcslen((*md+i)->sVal);
				}
				if(!wcscmp((*md+i)->wszMdName, L"ProviderAssetId") && !((*md+i)->wMdLength) && prefix) {
					std::string str = ID2String(id);
					size_t s = mbstowcs(0, str.c_str(), 0);

					wchar_t* t = new wchar_t[(s+1)*sizeof(wchar_t)];
					mbstowcs(t, str.c_str(), s+1);
					
					(*md+i)->sVal = new wchar_t[wcslen(prefix)+wcslen(t)+sizeof(wchar_t)];
					swprintf((*md+i)->sVal, L"%s%s", prefix, t);
					(*md+i)->wMdLength = wcslen((*md+i)->sVal);

					delete t;
				}
				dataset.insert(std::make_pair(std::wstring((*md+i)->wszMdName), *(*md+i)));
			}
//			IdsFreeMd(md, count);
		}
		else {
			delete tmp;
			return false;
 		}
	}
	else if(type == ASSET) {

//		OBJECTLIST* objs = 0; 
		
		res = IdsFindAssetByMember(&idsSession, IDS_ATOMIC_ELEMENT, id, objs, count2, &itvStatus, 0);
		if(!res) {
		
			for(short i = 0; i < (*count2); ++i) {
				METADATA* tmp = 0;
				initMeta(&tmp);
				
				res = IdsReadAsset(&idsSession, (*objs)[i].dwUid, &tmp, METADATA_COUNT, &((*objs)[i].pMd), &((*objs)[i].dwNumMd), &itvStatus, 0);			
				if(!res) {
					for(short j = 0; j < (*objs)[i].dwNumMd; ++j) {
						dataset.insert(std::make_pair(std::wstring((*objs)[i].pMd[j].wszMdName), (*objs)[i].pMd[j]));
					}
//					IdsFreeMd(objs[i].pMd, objs[i].dwNumMd);
		 		}
				else {
					std::cerr << "failed to get metadata for asset (" << ID2String((*objs)[i].dwUid) << ")" << std::endl;
				}
			}
		}
		else {
			std::cerr << "failed to get asset by element (" << ID2String(id) << ")" << std::endl;
			delete tmp;

			return false;
		}
	}

	delete tmp;
	return true;
}

void outputHeader(const std::vector<std::wstring>& elementMeta, const std::vector<std::wstring>& assetMeta) {
	std::cout << "\"Ae\"";

	std::vector<std::wstring>::const_iterator iter = elementMeta.begin();
	for(; iter != elementMeta.end(); ++iter) {
		std::wcout << L"," << L"\"" << (*iter) << L"\"";
	}
	std::vector<std::wstring>::const_iterator iter2 = assetMeta.begin();
	for(; iter2 != assetMeta.end(); ++iter2) {
		std::wcout << L"," << L"\"" << (*iter2) << L"\"";
	}
	std::cout << std::endl;
}

void outputMeta(const std::map<std::wstring, METADATA>& dataset, const std::vector<std::wstring>& meta) {
 //	DATASET_ITER ds = dataset.begin();
 //	for(;ds != dataset.end(); ++ds) {
 //		std::wcout << (ds->first) << std::endl;
 //	}
	std::vector<std::wstring>::const_iterator iter2 = meta.begin();
	for(; iter2 != meta.end(); ++iter2) {
//		std::wcout << L"meta: " << (*iter2) << std::endl;
		DATASET_ITER ds = dataset.find(*iter2);
		if(ds != dataset.end()) {
			std::cout << "," << "\"";
			printMeta(&(ds->second));
			std::cout << "\"";
		}
		else {
			std::cout << "," << "\"\"";
		}
	}
}

int main(int argc, char* argv[]) {

	std::string idsAddr;

	std::vector<Element> elements;
	std::vector<std::wstring> elementMeta, assetMeta;

	if(!parseArgs(argc, argv, idsAddr, elements, elementMeta, assetMeta)) {
		return (1);
	}

	if(!init(idsAddr)) {
		return (1);
	}

	if(withTitle) {
		outputHeader(elementMeta, assetMeta);
	}
	
	std::vector<Element>::iterator iter = elements.begin();

	size_t effective = elements.size();

	DWORD start = GetTickCount();
	for(; iter != elements.end(); ++iter) {
		
		if(!elementMeta.empty()) {
			std::map<std::wstring, METADATA> dataset;
			
			METADATA* md = 0; 
			DWORD count = 0;
			if(readMetadata(ELEMENT, iter->id, dataset, &md, &count, 0, 0)) {
				std::cout << "\"" << iter->name << "\"";
				outputMeta(dataset, elementMeta);

				IdsFreeMd(md, count);	
			}
			else {
				std::cerr << "invalid element ID: (" << iter->name << ")" << std::endl;
				--effective;
				continue;
			}
		}

		if(!assetMeta.empty()) {
			std::map<std::wstring, METADATA> dataset;

			OBJECTLIST* objs = 0;
			DWORD count = 0;
			readMetadata(ASSET, iter->id, dataset, 0, 0, &objs, &count);
		
			if(elementMeta.empty()) {
				std::cout << "\"" << iter->name << "\"";
			}
			outputMeta(dataset, assetMeta);

 			for(DWORD i = 0; i < count; ++i) {
 				IdsFreeMd(objs[i].pMd, objs[i].dwNumMd);
 			}
		}
		
		if(iter+1 != elements.end()) {
			std::cout << std::endl;
		}
	}

	DWORD end = GetTickCount();

	if(verbose) {
		std::cerr << "total: " << elements.size() << std::endl
				  << "effective: " << effective << std::endl 
				  << "metadata: " << (elementMeta.size()+assetMeta.size()) << std::endl
				  << "takes: " << (end-start) << " ms" << std::endl;
	}

	if(defaultPID) {
		delete defaultPID;
	}
	if(prefix) {
		delete prefix;
	}

	uninit();
	return (effective != 0);
}
