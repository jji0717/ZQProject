#include "globa.h"
#include <algorithm>
int AssetCount = 0;
int AEcount = 0;
std::string strProduct;
vector_type ADIAMS;
vector_type ADIAMSAPPMD;
vector_type ASSETMD;
vector_type ASSETCLASS;
vector_type ASSETClASSTYPE;
ELEMENTMDMP ElementMDmap;
ELEMENTMDMP KeyValuemap;
STRMAP      SysDefineKey;

std::string     Version_Major;
std::string     Version_Minor;

void initDefaultKey()
{
	SysDefineKey["SYS_Asset_ID"] = " ";
	SysDefineKey["SYS_Provider_ID"] = " ";
	SysDefineKey["SYS_Asset_Class"] = " ";
	SysDefineKey["SYS_Languages"] = " ";
	SysDefineKey["SYS_Creation_Date"] = " ";
	SysDefineKey["SYS_Audio_Type"] = " ";
	SysDefineKey["SYS_Screen_Format"] = " ";
}
///Convert ITV File to ADI File
///@param[in] FilePath  : ITV file path
///@param[in] exportPath: the Fold of the Export ADI file
void ConvertITVtoADI(std::string FilePath, std::string exportPath)
{
    vector_type     VersionInfo;
    vector_type		AssetIDName; 
	std::string		AppName;
	std::string     strLanguage ="";
	std::string     strPrivodeID="";
	std::string     strAssetID = "";
	std::string     ReleaseYear="";
	std::string     AutioType;
	std::string     ScreenFormat;

	char strFormat[1024];

	FILE*		fpwrite = NULL;

	if(!GetSessionValue("version", VersionInfo,FilePath))
	{
		printf("Get this Session = [version] error!\n");
	};
	Version_Major = VersionInfo[0].second;
	Version_Minor = VersionInfo[1].second;

	if(!GetSessionValue("uid", AssetIDName,FilePath))
	{
       printf("Get this Session = [uid] error!\n");
	};

	VECSTR result;
    compartStr(AssetIDName[0].second, ',',result);
    AppName = result[1];
	result.clear();

    vector_type::iterator it;
	vector_type::iterator  itkey;
	for(it = AssetIDName.begin(); it!= AssetIDName.end();++it) 
	{
	   vector_type		AssetMetadataInfo; 
	   std::string AssetID = it->first;
	   std::string AssetName;

       if(it->second[it->second.size() -1] == ',')
	   {
		   AssetName = " ";
	   }
	   else
	   {
		   compartStr( it->second, ',',result);
		   AssetName = result[result.size() -1];
	   }

       if(AssetID[0] == '2')
	   {
		   printf("\tAssetID = %s, AssetName = %s \n",
			   AssetID.c_str(), AssetName.c_str());
		   if(!GetSessionValue("metadata_"+AssetID, AssetMetadataInfo,FilePath))
		   {
			   printf("Get this Session = [%s] error!\n", AssetID.c_str());
			   continue;
		   };
		   
		   std::string ADIFilePath = exportPath + AssetID + 
													"_" + AssetName + ".xml";
		   if(fpwrite != NULL)
		   {
			   fprintf(fpwrite, "</Asset>\n</ADI>");
			   fclose(fpwrite);
			   fpwrite = NULL;
			   strLanguage = "";
			   strPrivodeID="";
			   strAssetID = "";
		   }
		   
		   if((fpwrite = fopen(ADIFilePath.c_str(), "w")) == NULL)
		   {
			   printf("Create ADIFile error! FilePath = %d \n",ADIFilePath.c_str());
		   }
			VECSTR strLas;
			GetKeyValue(AssetMetadataInfo,"Languages","Language",strLas,ASSETMD);
		    strLanguage = strLas[0];
			strLas.clear();

			GetKeyValue(AssetMetadataInfo,"Creation_Date","CreateTime",strLas,ASSETMD);
			ReleaseYear= strLas[0];
			strLas.clear();
			if(ReleaseYear.size() > 0)
			{
				struct tm *deltime;	
				char strdeltime[20] = "0";

				long Createtime = atoi(ReleaseYear.c_str());

				deltime = localtime(&Createtime);
				
				sprintf(strdeltime,"%04d-%02d-%02d",
					deltime->tm_year + 1900, deltime->tm_mon + 1,
					deltime->tm_mday);
				ReleaseYear = strdeltime;
			}

			GetKeyValue(AssetMetadataInfo,"Audio_Type","AudioType",strLas,ASSETMD);
			AutioType= strLas[0];
			strLas.clear();

			GetKeyValue(AssetMetadataInfo,"Screen_Format","ScreenFormat",strLas,ASSETMD);
			ScreenFormat = strLas[0];
			strLas.clear();

		    GetKeyValue(AssetMetadataInfo,"Asset_ID","ProviderAssetId",strLas,ASSETMD);
		    strAssetID = strLas[0];
			if(strAssetID == " ")
				strAssetID = AssetID.substr(1, AssetID.size() -1);
			strLas.clear();

			SysDefineKey["SYS_Asset_ID"] = "pkge" +  strAssetID;
			SysDefineKey["SYS_Asset_Name"] = AssetName+"package";
			SysDefineKey["SYS_Asset_Class"] = "package";
			SysDefineKey["SYS_Languages"] = strLanguage;
			SysDefineKey["SYS_Creation_Date"] = ReleaseYear;
			SysDefineKey["SYS_Audio_Type"] = AutioType;

			if(ScreenFormat == " ")
				SysDefineKey["SYS_Screen_Format"] = "Standard";
			else
			 SysDefineKey["SYS_Screen_Format"] = ScreenFormat;
			
		   /*最上面的ADI_MetaData信息*/
		   fprintf(fpwrite, "<ADI>\n<Metadata>\n	<AMS \n");
		   
		   for(itkey = ADIAMS.begin(); itkey!= ADIAMS.end(); itkey++)
		   {
			   VECSTR strValue;
			   GetKeyValue(AssetMetadataInfo,itkey->first,itkey->second,
				   strValue,ASSETMD);

			   if (itkey->first == "Asset_Class" && strValue[0] == " ")
			   {
				   strValue.clear();
				   strValue.push_back("package");
			   }
			   else if (itkey->first  == "Asset_Name" && strValue[0] == " ")
			   {
				   strValue.clear();
				   strValue.push_back(AssetName+"package" );
			   }				   
			   else if (itkey->first  == "Asset_ID" && strValue[0] == " ")
			   {
				   strValue.clear();
				   strValue.push_back("pkge" +  strAssetID);
			   }
			   else if(itkey->first  == "Creation_Date")
			   {
				   strValue.clear();
				   strValue.push_back(ReleaseYear);
			   }
				   
			   VECSTR::iterator itor;
			   for (itor = strValue.begin(); itor != strValue.end(); itor++)
			   {
				   if(*itor ==" ")
				   {
					   fprintf(fpwrite, "		%s=\"\"\n", itkey->first.c_str());
				   }
				   else
				   {
					   fprintf(fpwrite, "		%s=\"%s\"\n", itkey->first.c_str(), itor->c_str());
				   }
			   }
			   strValue.clear();	
		   }
		   
		   fprintf(fpwrite, "	/>\n");

		   /*最上面的ADI_App_data信息*/

		   for(itkey = ADIAMSAPPMD.begin(); itkey!= ADIAMSAPPMD.end(); itkey++)
		   {
			   VECSTR strValue;
			   GetKeyValue(AssetMetadataInfo,itkey->first,itkey->second,strValue,ASSETMD);
			   VECSTR::iterator itor;
			   for (itor = strValue.begin(); itor != strValue.end(); itor++)
			   {
				   memset(strFormat,0, 1024);
				   sprintf(strFormat,"	  <App_Data app=\"%s\" Name=\"%s\"",
										strProduct.c_str(), itkey->first.c_str());
				   if(*itor == " ")
				   {
					   fprintf(fpwrite, "%-60s  Value=\"\"/>\n",strFormat);	
				   }
				   else
				   {
					   fprintf(fpwrite, "%-60s  Value=\"%s\"/>\n",strFormat, itor->c_str());	
				   }
			   }
			   strValue.clear();
		   }
		   fprintf(fpwrite, "</Metadata>\n");
		   
		   /*Asset MetaData信息*/
		   fprintf(fpwrite, "<Asset>\n <Metadata>\n	<AMS \n");
		   
		   SysDefineKey["SYS_Asset_ID"] = "titl" +  strAssetID;
		   SysDefineKey["SYS_Asset_Name"] = AssetName+"title";
		   SysDefineKey["SYS_Asset_Class"] = "title";
		   
		   for(itkey = ADIAMS.begin(); itkey!= ADIAMS.end(); itkey++)
		   {
			   VECSTR strValue;

			   GetKeyValue(AssetMetadataInfo,itkey->first,
				   itkey->second,strValue,ASSETMD);
			   
			   if (itkey->first == "Asset_Class"  && strValue[0] == " ")
			   {
				   strValue.clear();
				   strValue.push_back("title");
			   }
			   else if (itkey->first == "Asset_Name" && strValue[0] == " ")
			   {
				   strValue.clear();
				   strValue.push_back(AssetName+"title");
			   }				   
			   else if (itkey->first  == "Asset_ID" && strValue[0] == " ")
			   {
				   strValue.clear();
				   strValue.push_back("titl" +  strAssetID);
			   }
			   else if(itkey->first  == "Creation_Date")
			   {
				   strValue.clear();
				   strValue.push_back(ReleaseYear);
			   }
			   
			   VECSTR::iterator itor;
			   for (itor = strValue.begin(); itor != strValue.end(); itor++)
			   {
				   if(*itor == " ")
				   {
					   fprintf(fpwrite, "		%s=\"\"\n", itkey->first.c_str());
				   }
				   else
				   {
					   fprintf(fpwrite, "		%s=\"%s\"\n", itkey->first.c_str(), itor->c_str());
				   }
			   }
			   strValue.clear();	
		   }
		   fprintf(fpwrite, "	/>\n");
		   	/*每个Asset  App_Data信息*/
		   for(itkey = ASSETMD.begin(); itkey!= ASSETMD.end(); itkey++)
		   {
			   VECSTR strValue;	 
			   VECSTR::iterator itor;
			   strValue.clear();

			   GetKeyValue(AssetMetadataInfo,itkey->first,itkey->second,strValue,ASSETMD);
			   			   
			   for (itor = strValue.begin(); itor != strValue.end(); itor++)
			   {
				   memset(strFormat,0, 1024);
				   sprintf(strFormat,"	  <App_Data app=\"%s\" Name=\"%s\"",
										strProduct.c_str(), itkey->first.c_str());
				   if(*itor == " ")
				   {
					   fprintf(fpwrite, "%-60s  Value=\"\"/>\n",strFormat);	
				   }
				   else
				   {
					   fprintf(fpwrite, "%-60s  Value=\"%s\"/>\n",strFormat, itor->c_str());	
				   }
			   }
			   strValue.clear();
		   }
		   fprintf(fpwrite, " </Metadata>\n");
		   AssetCount++;
	   }
	   else   
		   if( AssetID[0] == '3')
		   {
			   printf("\tAssetID = %s, AssetName = %s \n",
				   AssetID.c_str(), AssetName.c_str());

			   if(!GetSessionValue("metadata_"+AssetID, AssetMetadataInfo,FilePath))
			   {
				   printf("Get this Session = [%s] error!\n", AssetID.c_str());
				   continue;
			   };
			   if(fpwrite == NULL)
			   {
				   printf("the ITV file error,no asset!");
				   continue;
			   }
				/*每个Element MetaData信息*/
			   fprintf(fpwrite, "<Asset>\n <Metadata>\n	<AMS\n");
			   VECSTR strName;
			   VECSTR strValue;

			   if(!getKeyString(AssetMetadataInfo,"ContentClass",ASSETCLASS,strName))
			   {
				   printf("Get ContentClass error ! filepath = %s!",FilePath.c_str());
				   if(fpwrite != NULL)
				   {
					   fclose(fpwrite);
					   fpwrite = NULL;
				   }
				   return ;
			   }

			   VECSTR strLas;
			   GetKeyValue(AssetMetadataInfo,"Asset_ID","ProviderAssetId",strLas,ADIAMS);
			   strAssetID = strLas[0];
			   if(strAssetID == " ")
				   strAssetID = AssetID.substr(1, AssetID.size() -1);
			   strLas.clear();
			   strValue.push_back(strName[1] + strAssetID);
			   
			   SysDefineKey["SYS_Asset_ID"] = strName[1] +  strAssetID;
			   SysDefineKey["SYS_Asset_Name"] = AssetName+strName[0];
			   SysDefineKey["SYS_Asset_Class"] = strName[0];

			   for(itkey = ADIAMS.begin(); itkey!= ADIAMS.end(); itkey++)
			   {
				   strValue.clear();

				   GetKeyValue(AssetMetadataInfo,itkey->first,itkey->second,
					   strValue,ADIAMS);
				   
				   if (itkey->first == "Asset_Class" && strValue[0] == " ")
				   {
					   strValue.clear();
					   strValue.push_back(strName[0]);
				   }
				   else if (itkey->first  == "Asset_Name" && strValue[0] == " ")
				   {
					   strValue.clear();
					   strValue.push_back(AssetName+strName[0]);
				   } 
				   else if (itkey->first  == "Asset_ID" && strValue[0] == " ")
				   {
					   strValue.clear();
					   strValue.push_back(strName[1] + strAssetID);
				   }
				   else if(itkey->first  == "Creation_Date")
				   {
					   strValue.clear();
					   strValue.push_back(ReleaseYear);
				   }

				   VECSTR::iterator itor;
				   for (itor = strValue.begin(); itor != strValue.end(); itor++)
				   {
					   if(*itor == " ")
					   {
						   fprintf(fpwrite, "		%s=\"\"\n", itkey->first.c_str());
					   }
					   else
					   {
						   fprintf(fpwrite, "		%s=\"%s\"\n", itkey->first.c_str(), itor->c_str());
					   }
				   }
				   strValue.clear();	
			   }
			   fprintf(fpwrite, "	/>\n");
               
			   ELEMENTMDMP::iterator itMD = ElementMDmap.find(strName[2]);
			   if(itMD == ElementMDmap.end())
			   {
				   printf("Can not find AssetClass = %s Metadata Key!", strName[0].c_str());
				   return;
			   }
			   vector_type ELEMENTMD;
			   ELEMENTMD = itMD->second;
		    	/*每个Element  App_Data信息*/
			   for(itkey = ELEMENTMD.begin(); itkey!= ELEMENTMD.end(); itkey++)
			   {
				   VECSTR strValue;

				   if (itkey->first == "Type")
				   {
					   strValue.clear();
					   strValue.push_back(strName[0]);
				   }
				   else if (itkey->first == "Languages")
				   {
					   strValue.clear();
					   strValue.push_back(strLanguage);
				   }
				   else	if(itkey->first =="Audio_Type")
				   {
					   strValue.clear();
					   strValue.push_back(AutioType);
				   }
				   else if (itkey->first == "Screen_Format")
				   {
					   strValue.clear();
					   strValue.push_back(ScreenFormat);
				   }else
				   {
					   GetKeyValue(AssetMetadataInfo,itkey->first,itkey->second,strValue,ELEMENTMD);					   
				   }
				   VECSTR::iterator itor;
				   for (itor = strValue.begin(); itor != strValue.end(); itor++)
				   {
					   memset(strFormat,0, 1024);
					   sprintf(strFormat,"	  <App_Data app=\"%s\" Name=\"%s\"",
						   strProduct.c_str(), itkey->first.c_str());
					   if(*itor == " ")
					   {
						   fprintf(fpwrite, "%-60s  Value=\"\"/>\n",strFormat);	
					   }
					   else
					   {
						   fprintf(fpwrite, "%-60s  Value=\"%s\"/>\n",strFormat, itor->c_str());	
					   }
				   }
				   strValue.clear();
			   }
/*			   GetKeyValue(AssetMetadataInfo,"","FileName",
				   strValue,ADIAMS);
			   fprintf(fpwrite, " </Metadata>\n  <Content Value=\"%s\"/>\n</Asset>\n",strValue[0].c_str());
*/
			   fprintf(fpwrite, " </Metadata>\n</Asset>\n");

			   AEcount++;
		   }
		   AssetMetadataInfo.clear();
	}

	if(fpwrite != NULL)
	{
		fprintf(fpwrite, "</Asset>\n</ADI>\n");
		fclose(fpwrite);
		fpwrite = NULL;
	}
//	printf("AppName = %s \n", AppName.c_str());
	printf("Asset Count %d, Asset Element Count %d\n", AssetCount,AEcount);
    printf("ITV File convert ADI File Success\n\n", FilePath.c_str());
	AssetCount = 0;
    AEcount = 0;
   
}

///Convert ITV File to ADI File
///@param[in] SessionName  : the name of the section in the ini file
///@param[in] SessionResult: key_Value Vector of this Session
///@param[in] strFilePath  : specifies the name of the initialization file
bool GetSessionValue(std::string SessionName, vector_type& SessionResult, 
												std::string strFilePath)
{
	char   tstr[1024 * MAX_KEY_NUM];
	DWORD  ret;
    
    ret = GetPrivateProfileSection(SessionName.c_str(), tstr, 
									  1024 * MAX_KEY_NUM,strFilePath.c_str());

	if(ret <= 0)
	{
		printf("This Session = [%s] is not exist or no Key value\n",
											SessionName.c_str());
		return false;
	}
	
	char*   key=tstr;   
	int     i=0;   
	char   Kvalue[1024]; 
	std::string temp;
    VECSTR result;
	while(*key!=0)   
	{ 		
		strncpy(Kvalue,   key,   1024);  
		temp = Kvalue;
        compartStr(temp, '=', result);
		key=key+strlen(key)+1; 
		
		SessionResult.push_back(spair__(result[0],result[1])); 
#ifdef _DEBUG
		printf("%-5dkey  = %-20s     value= %-20s\n", i,result[0].c_str(), result[1].c_str());
#endif
		result.clear();
		i++;
	}  
	return true;
}

///Get Key define from KeyDefine.ini
bool getKeyConfig()
{
   	std::string  strCurDir;
	char           sModuleName[1025];
	std::string	 strConfigFile;
	
	DWORD dSize = GetModuleFileName(NULL,sModuleName,1024);
	sModuleName[dSize] = L'\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.rfind('\\');
	strCurDir = strCurDir.substr(0,nIndex + 1);
    strConfigFile = strCurDir + "KeyDefine.ini";
    vector_type Proudct;


	if (_access (strConfigFile.c_str(), 0 ) != 0  )
	{
		printf("KeyDefine.ini File is not exist!\n");
		return false;
	}

#ifdef _DEBUG
	printf("Get ADI_AMS Key info!\n");
#endif

	if(!GetSessionValue("ADI_AMS", ADIAMS,strConfigFile))
	{
		printf("Get this Session = [ADI_AMS] error!\n");
				return false;
	}
#ifdef _DEBUG
	printf("\nGet ADI_AMS_APP_DATA Key info!\n");
#endif

	if(!GetSessionValue("ADI_AMS_APP_DATA", ADIAMSAPPMD,strConfigFile))
	{
		printf("Get this Session = [ADI_AMS_APP_DATA] error!\n");
		return false;
	}
	
#ifdef _DEBUG
	printf("\nGet ASSET_METADATA Key info!\n");
#endif
	
	if(!GetSessionValue("ASSET_METADATA", ASSETMD,strConfigFile))
	{
		printf("Get this Session = [ASSET_METADATA] error!\n");
		return false;
	}

#ifdef _DEBUG
	printf("\nGet ASSET_CLASS Key info!\n");
#endif

	if(!GetSessionValue("ASSET_CLASS", ASSETCLASS,strConfigFile))
	{
		printf("Get this Session = [ASSET_CLASS] error!\n");
		return false;
	}

#ifdef _DEBUG
	printf("\nGet ASSET_CLASS_TYPE Key info!\n");
#endif

	if(!GetSessionValue("ASSET_CLASS_TYPE", ASSETClASSTYPE,strConfigFile))
	{
		printf("Get this Session = [ASSET_CLASS_TYPE] error!\n");
		return false;
	}

	vector_type::iterator itor;
	for(itor = ASSETClASSTYPE.begin(); itor != ASSETClASSTYPE.end(); itor++)
	{
		vector_type elementMD;

    #ifdef _DEBUG
		printf("\nGet %s Key info!\n",itor->second.c_str());
    #endif
		
		if(!GetSessionValue(itor->second, elementMD,strConfigFile))
		{
			printf("Get this Session = [%s] error!\n", itor->second.c_str());
			return false;
		}
		ElementMDmap[itor->first] =  elementMD;	
	}

#ifdef _DEBUG
	printf("\nGet ITV_ASSET_KEY_DEFINE Key info!\n");
#endif

	vector_type keyvalue;
	if(!GetSessionValue("ITV_ASSET_KEY_DEFINE", keyvalue,strConfigFile))
	{
		printf("Get this Session = [ITV_ASSET_KEY_DEFINE] error!\n");
		return false;
	}

	for(itor = keyvalue.begin(); itor != keyvalue.end(); itor++)
	{
		vector_type elementMD;

    #ifdef _DEBUG
		printf("\nGet %s Key info!\n",itor->second.c_str());
    #endif
		
		if(!GetSessionValue(itor->second, elementMD,strConfigFile))
		{
			printf("Get this Session = [%s] error!\n", itor->second.c_str());
			return false;
		}
		KeyValuemap[itor->first] =  elementMD;	
	}

	#ifdef _DEBUG
	printf("\nGet PROUDCT_INFO Key info!\n");
    #endif

	if(!GetSessionValue("PROUDCT_INFO", Proudct,strConfigFile))
	{
		printf("Get this Session = [PROUDCT_INFO] error!\n");
		return false;
	}

	strProduct = Proudct[0].second;
	return true;
}

///Get value according to ADIkey and ITVKey 
///@param[in] MetaData  : the Session name of ITV File
///@param[in] strADIkey : ADI key
///@param[in] strITVkey : ITV key
///@param[in] result    : 
bool GetKeyValue(vector_type& MetaData, std::string strADIkey,std::string strITVkey, 
				 VECSTR& result, vector_type& ADITVMAP)
{

    if(strADIkey == "Version_Major")
	{
		result.push_back(Version_Major);	
		return true;
	}
	else if (strADIkey == "Version_Minor")
	{
		result.push_back(Version_Minor);	
		return true;
	}
	else if (strADIkey == "Product")
	{
		result.push_back(strProduct);	
		return true;
	}

	std::string strDefaultValue=" ";
	std::string strADIFindKey;
	VECSTR strDefault;

	int index = strITVkey.find(',');
	if( index != -1 )
	{

		compartStr(strITVkey,',',strDefault);
		if(strDefault.size() != 2)
		{
			printf("ADIKey = %s define format error! please specify default value\n",
						strADIkey.c_str());
			result.push_back(strDefaultValue);
			return false;
		}
		strITVkey = strDefault[0];
		strDefaultValue = strDefault[1];
		strDefault.clear();
	}
	vector_type::iterator itkey;
	VECSTR KeyValue;
	bool bFindKey = false;
	for(itkey = MetaData.begin(); itkey != MetaData.end(); ++itkey)
	{
		if(itkey->first == strITVkey)
		{
			bFindKey = true;
			if(itkey->second[itkey->second.size() - 1] == ',')
			{
				result.push_back(strDefaultValue);
			}
			else
			{
				compartStr(itkey->second,',',KeyValue);
				
				ELEMENTMDMP::iterator itMD = KeyValuemap.find(strITVkey);
				if(itMD != KeyValuemap.end())
				{	
					vector_type KeyValues;
					KeyValues = itMD->second;
					for(itkey = KeyValues.begin(); itkey != KeyValues.end(); ++itkey)
					{
						if(itkey->first == KeyValue[KeyValue.size() -1])
						{
							result.push_back(itkey->second);
							bFindKey = true;
							break;
						}
					}
//					printf("ITVKey = %s value not found!\n", strITVkey.c_str());
				}
				else
				{
					result.push_back(KeyValue[KeyValue.size() -1]);
					bFindKey = true;
		
				}
		    	break;
			}		
		}
	}
	if(!bFindKey)
	{
		index = strDefaultValue.find('$');
		if(index != -1)
		{
			if(index == 0)
			{
				strADIFindKey = strDefaultValue.substr(1,strDefaultValue.size() -1);
				strDefaultValue = "";
			}
			else
			{
				compartStr(strDefaultValue,'$',strDefault);
				if(strDefault.size() != 2)
				{
					printf("ADIKey = %s define format error! please specify default value");
					result.push_back(" ");
					return false;
				}
				strDefaultValue = strDefault[0];
				strADIFindKey = strDefault[1];
			}
			STRMAP::iterator itorsys = SysDefineKey.find(strADIFindKey);
			if(itorsys != SysDefineKey.end())
			{
                strDefaultValue = strDefaultValue  + SysDefineKey[strADIFindKey];
				result.push_back(strDefaultValue);
				return true;
			}
			for(vector_type::iterator itor = ADITVMAP.begin(); itor != ADITVMAP.end(); itor++)
			{
				if(itor->first == strADIFindKey)
				{
					VECSTR defaultvaule;
					
					GetKeyValue(MetaData, strADIFindKey,itor->second, defaultvaule, ADITVMAP);
					if(defaultvaule.size() > 0)
					{	
						if(defaultvaule[0] != " ")
						strDefaultValue = strDefaultValue  +defaultvaule[0];
					}
					break;
				}
			}
		}
		result.push_back(strDefaultValue);
	}
   return true;
}
bool getKeyString(vector_type& MetaData, std::string Key, vector_type& KeyValueMap,VECSTR& strvalue)
{
	std::string type;
	bool bFindKey = false;
	vector_type::iterator itkey;
	VECSTR KeyValue;
	for(itkey = MetaData.begin(); itkey != MetaData.end(); ++itkey)
	{
		if(itkey->first == Key)
		{
			if(itkey->second[itkey->second.size() - 1] == ',')
			{
               return false;
			}
			else
			{
				compartStr(itkey->second,',',KeyValue);
				type = KeyValue[KeyValue.size() -1]; 
				break;
			}
		}
	} 

	if(type.empty())
	{
		return false;
	}
	for(itkey = KeyValueMap.begin(); itkey != KeyValueMap.end(); ++itkey)
	{
		if(itkey->first == type)
		{
           compartStr(itkey->second,',',strvalue);
		   bFindKey = true;
		   strvalue.push_back(itkey->first);

		   break;
		}
	}
	
	if(!bFindKey)
	{
		return false;
	}
	return true;
}
//按照指定的字符分隔字符串
///@param[in]  src       : 要处理的字符串
///@param[in]  delimiter : 指定的字符
///@param[out] result    : 分隔后的字符串列表
 
void compartStr( const std::string& src, char delimiter, VECSTR& result)
{
	std::string::const_iterator it, beginPos = src.begin();
	for (it = src.begin(); it != src.end(); it ++) {
		if (*it == delimiter) {
			std::string str(beginPos, it);
			beginPos = it + 1;
			result.push_back(str);
		}
	}
	if(beginPos != src.end())
	{
		std::string str(beginPos, it);
		result.push_back(str);
	}
}
///Get Current System Time 
std::string GetCurrentDatatime()
{
	char strtime[20];
	std::string strTime;
	SYSTEMTIME time; 
	GetLocalTime(&time);
	sprintf(strtime,"%04d%02d%02d%02d%02d%02d",time.wYear, time.wMonth, 
		time.wDay, time.wHour, time.wMinute, time.wSecond );
	strTime = strtime;
	return strTime;
}

///Create Export Directory
///@param[in]  strExportDir:ADI File的存放路径

int CreateOutDir(std::string& strExportDir)
{
	std::string  strCurDir;
	char           sModuleName[1025];
	
	DWORD dSize = GetModuleFileName(NULL,sModuleName,1024);
	sModuleName[dSize] = L'\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.rfind('\\');
	strCurDir = strCurDir.substr(0,nIndex + 1);

	strExportDir = "ADIExportDir" + GetCurrentDatatime(); 
	strExportDir = strCurDir + strExportDir+ "\\";
    
	// open dictionary
   	int ret = ::CreateDirectory(strExportDir.c_str(), NULL);
	return ret;
}
int  ListFile(const char *argv, std::list<std::string> &File)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH];  // directory specification
	DWORD dwError;
	
	//printf ("Target directory is %s.\n", argv);
	strncpy (DirSpec, argv, strlen(argv)+1);
	strncat (DirSpec, "\\*", 3);
	
	hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		printf( "ListFile::Invalid file handle. Error is %u",GetLastError());		 
		return -1;
	} 
	else 
	{
		//printf ("First file name is %s\n", FindFileData.cFileName);
		if ( FindFileData.dwFileAttributes == 32 )
		{
			strcpy(DirSpec,  argv);   
            strcat(DirSpec,  "\\");
			strcat(DirSpec, FindFileData.cFileName);
			File.push_back(DirSpec);
		}

		while (FindNextFileA(hFind, &FindFileData) != 0) 
		{
			//printf ("Next file name is %s\n", FindFileData.cFileName);
			if ( FindFileData.dwFileAttributes == 32 )
			{
				strcpy(DirSpec,  argv);   
				strcat(DirSpec,  "\\");
				strcat(DirSpec, FindFileData.cFileName);
				File.push_back(DirSpec);
			}
		}
		
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			printf( "ListFile::FindNextFile error. Error is %u",  GetLastError());		 
			return -1;
		}
	}
	return 0;
}
bool ListDir(const char *lpSrcFile, std::list<std::string> &Directory) 
{   
	if( FILE_ATTRIBUTE_DIRECTORY == ::GetFileAttributesA(lpSrcFile))     
	{   
		WIN32_FIND_DATAA   FindFileData;   
		char   FileName[_MAX_PATH];   

		strcpy(   FileName   ,   lpSrcFile   );   
		strcat(   FileName   ,   "\\*.*"   );   
		
		HANDLE hFindFile = ::FindFirstFileA( FileName, &FindFileData);   
		if ( INVALID_HANDLE_VALUE == hFindFile )   
		{   
			return FALSE;   
		}   
		//CString strSrcPathName;   
		while( ::FindNextFileA( hFindFile, &FindFileData ) )   
		{   
			if(!( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//不是目录   
			{   
				// CString   strSrcPathName(   lpSrcFile   );   
				// strSrcPathName+=_T("\\");   
				// strSrcPathName+=   FindFileData.cFileName;   
				//printf("%s\n", FindFileData.cFileName);
                
			}   
			else//目录   
			{   
				//记录下目录
				if ( strcmp(FindFileData.cFileName, ".") !=0 && strcmp(FindFileData.cFileName, "..") != 0 )
				{
					//printf("%s\n", FindFileData.cFileName);
					strcpy(FileName,  lpSrcFile);   
		            strcat(FileName,  "\\");
					strcat(FileName, FindFileData.cFileName);
					Directory.push_back(FileName);
					ListDir(FileName ,  Directory);
				}
			}   
		}   
		
		::FindClose(hFindFile);   
		
		if ( ERROR_NO_MORE_FILES != GetLastError() )   
			return   FALSE;   
	}   
	return   1;   
}  
