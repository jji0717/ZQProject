module com {
module izq {
module am {
module facade {
module servicesForIce {
  
  struct AEInfo 
  {
    string aeUID;     // the uid of asset element
    int    bandWidth; // bandWidth
    int    cueIn;
    int    cueOut;
  };
  
  // the collection of AE is a sequence or vector
  sequence<AEInfo> AssetElementCollection;
  
  sequence<string> NetIDCollection;
  
  struct AECollectionWithNetID 
  {
    AssetElementCollection aeList;    // AE list
    NetIDCollection        netIDList; // The uid of net_id
  };
  
  sequence<string> StringCollection;
  
  dictionary<string, string> AttributesMap;
  
  struct AEInfo2 
  {
    string           aeUID;      // the uid of asset element
    int              bandWidth;  // bandWidth
    int              cueIn;
    int              cueOut;
    StringCollection nasUrls;    //NAS-URLs get by ALQ request from ALS
    AttributesMap    attributes; //for extending usage
  };

  sequence<AEInfo2> AssetElementCollection2;
  
  struct AEInfoListForReferenceOnlyAsset 
  {
    int                     useNasURL;  //1: yes, 0 : no
    AssetElementCollection2 aeInfoList;
  };
  
  exception LogicError 
  {
    int    errorCode;
    string errorMessage;
  };
  
  //Add for NGOD_2.0_SOW and NGOD_2.1_SOW
  //1. volumeList: a vector of unique volume names. 
  //Each unique volume name consists two parts: ContentStore netId and the volume name (optional) on the ContentStore. 
  //The unique volume name will be formatted as: <ContentStoreNetId>/<VolumeNameOnCS>
  //For the ContentStore that doesn't support multiple volumes, its netId will be directly used as the unique volume name
  //2. nasURL: left as empty if the type of content store is not Cache Type.
  struct AEInfo3 
  {
    string           name;       //only content name, without volume name
    int              bandWidth;  // bandWidth
    int              cueIn;
    int              cueOut;
    StringCollection nasUrls;
    StringCollection volumeList;
    AttributesMap    attributes; //for extending purpose   
  };
  sequence<AEInfo3> AEInfo3Collection;
  //~
  
  interface LAMFacade 
  {  
    AssetElementCollection getAEList(string assetUID);
    AssetElementCollection getAEListWithAppUID(string assetUID, string appUID);
    AssetElementCollection getAEListByProviderIdAndPAssetId(string providerId, string providerAUID);
    
    AECollectionWithNetID getAEListWithNetID(string assetUID);
    AECollectionWithNetID getAEListWithNetIDByPID(string providerId, string providerAUID);
    AECollectionWithNetID getAEListWithNetIDByAppUID(string assetUID, string appUID);
    AECollectionWithNetID getAEListByObjectUID(string objectUID, int objectType);
    
    AEInfoListForReferenceOnlyAsset getAEListByPIdAndPAIdForReferenceOnlyAsset(string providerId, string providerAUID) throws LogicError; 
    AEInfoListForReferenceOnlyAsset getAEListByPIdAndPAIdForRefOnlyAsset(string providerId, string providerAUID, string netID) throws LogicError; 

    //Add for NGOD_2.0_SOW and NGOD_2.1_SOW
    AEInfo3Collection getAEListByAUIdAppUId(string assetUID, string appUID) throws LogicError;
    AEInfo3Collection getAEListByPIdPAIdSId(string providerId, string providerAUID, string subscriberId) throws LogicError;
    //~
    
  };          
};
};
};
};
}; 
// DESCRIPTION
// 
// 
// RRVISION HISTORY
// --------------------------------------------------------------------------------------
// Version: LAM VERSION
//    Date: 2008-11-06
//  Person: Zhang Jian
// Changes: 1. define struct AEInfo3, define new return type AEInfo3Collection 
//          2. add new interface method getAEListByAUIdAppUId(string assetUID, string appUID)
//          3. add new interface method getAEListByPIdPAIdSId(string providerId, string providerAUID, string subscriberId)
// Purpose: Add for NGOD_2.0_SOW and NGOD_2.1_SOW
//-----------------------------------------------------------------------------------------
// Version: LAM VERSION
//    Date: 
//  Person: 
// Change1: 
//Purpose1: 
// Change2: 
//Purpose2: 
