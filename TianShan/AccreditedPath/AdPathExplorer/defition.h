typedef struct  
{ 
	int id;
	std::string  type; 
    std::string  netId; 
    std::string  desc; 
    std::string  ifep ;
	BOOL flag;//flag update data or add data, true is update data,and false is add data;
}SData;

typedef struct  
{
	std::string storageNetId;
	std::string streamerNetId;
	int SGId;
	std::string linktype;
    int bStorage;
	TianShanIce::AccreditedPath::StorageLinkPrx storagelinkPrx;
	TianShanIce::AccreditedPath::StreamLinkPrx  streamlinkprx;
}LinkInfo;
