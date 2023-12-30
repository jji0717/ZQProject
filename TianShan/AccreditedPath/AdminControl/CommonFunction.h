#ifndef __COMMON_FUNCTION_H_
#define __COMMON_FUNCTION_H_

#include <windows.h>

BOOL GetPrivateDataStr(char *dest,::TianShanIce::Variant *pvar)
{
	std::string strtemp = "";
	char temp[20];
	BOOL flag = FALSE;
    switch(pvar->type)
	{
	case ::TianShanIce::vtStrings:
		{
			::TianShanIce::StrValues::iterator strpos = pvar->strs.begin();
			while(strpos != pvar->strs.end())
			{
				if(flag)
				{
					strtemp += ";";
				}
				strtemp += *strpos; 	
				strpos++;
				flag = TRUE;
			}
			strcpy(dest,strtemp.c_str());
			return TRUE;
		}
	case ::TianShanIce::vtLongs:
		{
			::TianShanIce::LValues::iterator Lpos = pvar->lints.begin();
			if(pvar->bRange == FALSE)
			{
				while(Lpos != pvar->lints.end())
				{
					if(flag)
					{
						strtemp += ";";
						
					}
					strtemp += itoa(*Lpos, temp, 10); 
					Lpos++;
					flag = TRUE;
				}
			}
			else
			{
				if(Lpos != pvar->lints.end())
				{
					strtemp += itoa(*Lpos, temp, 10);
					Lpos++;
					strtemp += "  ~  ";
					strtemp += itoa(*Lpos, temp, 10);
				}
				else 
					strtemp = "";
			
			}
			strcpy(dest,strtemp.c_str());
			return TRUE;
		}
	case ::TianShanIce::vtInts:
		{			
			::TianShanIce::IValues::iterator Ipos = pvar->ints.begin();
			if(pvar->bRange == FALSE)
			{
				while(Ipos != pvar->ints.end())
				{
					if(flag)
					{
						strtemp += ";";
					}
					strtemp += itoa(*Ipos, temp, 10); 
					Ipos++;
					flag = TRUE;
				}
			}
			else
			{
				if(Ipos != pvar->ints.end())
				{
				strtemp += itoa(*Ipos, temp, 10);
				Ipos++;
				strtemp += "  ~  ";
				strtemp += itoa(*Ipos, temp, 10);
				}
				else 
					strtemp = "";
			}
			strcpy(dest,strtemp.c_str());
			return TRUE;
		}
	case ::TianShanIce::vtBin:
		return FALSE;
	}
   return FALSE;
}
///Delete all of front Blank and end Blank in a string
///@parme[in] pstr the string which is delete blank
///@return    if the string only contain Blank or NULL ,return NULL;
///              else return Delete Blank String
char* Delblank(char *psrc)//去掉一个字符串前后的所有空格。
{
	char* pfirst, *plast;
	if(*psrc == '\0')
		return NULL;
	plast = psrc + strlen(psrc) - 1;
	
	while(*psrc == ' '&& *psrc != '\0')
	{
		psrc++;
	}
	while(*plast ==' ')
	{
		plast--;
	}
	pfirst = psrc;
	*(plast + 1) = '\0';
	
	if(*pfirst == '\0')
		return NULL;
	
	return pfirst;
	
}

///Judges a character string is not the pure digital string
///@parme psrc the Judges character string
///@return  if the string is pure digital string ,return TRUE;
///          else return FALSE
BOOL IsInt(char *psrc)//判断一个字符串是不是纯数字串。
{
	int i = strlen(psrc);
	if(i >= 2 && *psrc  == '0' )
		return FALSE;
	while(*psrc >= '0' && *psrc <= '9')
	{
		psrc++;
	}
	
	if(*psrc != '\0')
		return FALSE;
	
	return TRUE;
}

///judges a string is  only contain digital 、blank and specify character
///@parme[in] ch specify character
///@parme[in] psrc the judges string
///@return  if the string only contain digital blank and specify character, return TRUE
///            else  return FALSE
BOOL IsString(char ch, char *psrc)//判断一个字符串中是否只包含了数字空格和指定的字符。
{
	while(*psrc >= '0' && *psrc <= '9' || *psrc == ' ' || *psrc == ch)
	{
		psrc++;
	}
	
	if(*psrc != '\0')
		return FALSE;
	
	return TRUE;
}

///judges a string is contain double ';' or the first character is ';'
///@parme[in] psrc the judges string;
///@return  if the string contain double';' or the first character is';',return FALSE;
///         else return TRUE
BOOL DoubleChar(char *psrc)
{
	char *pstr = psrc;
	int i = strlen(psrc);
	
	if(i == 1 && *psrc == ';')
		return FALSE;
	
	for(; *pstr != '\0'; pstr++)
	{
		if(*pstr == *(pstr +1))
		{
			if(*pstr == ';')
				return FALSE;
		}
	}
	return TRUE;
}

///Check the string  is effect or not
///@param[in] strKey   VariantMap key
///@param[in]type  Variant Type
///@param[in] psrc the string is checked 
///@param[out] pvar receive the Variant
///@return	if the string is effect, return TRUE
///          else return FALSE
BOOL CheckPrivateData(const std::string strKey,INT type, char *psrc,::TianShanIce::Variant *pvar)
{
	char sepStr[] = ";";
	char sepIntTrue[] = "~";
	char *token, *pstr, *ptemp;
	switch(type)
	{
	case ::TianShanIce::vtStrings:
		pvar->type = TianShanIce::vtStrings;
		pvar->bRange = FALSE;
		
		if(!DoubleChar(psrc))
			return FALSE;
		
		if(*psrc == ';')
			return FALSE;
		token = strtok(psrc,sepStr );
		while( token != NULL)
		{
			pvar->strs.push_back(token);
			token = strtok(NULL, sepStr);
		}
		break;
	case ::TianShanIce::vtLongs:
		pvar->type = TianShanIce::vtLongs;
		if(IsString(';',psrc))//判断是不是只有数字空格和';'
		{
			pvar->bRange = FALSE;
			if(!DoubleChar(psrc))
				return FALSE;
			if(*psrc == ';')
				return FALSE;
			
			token = strtok(psrc,sepStr );
			while( token != NULL)
			{
				pstr =  Delblank(token);// " ; ;"
				
				if(!pstr)
					return FALSE;
				
				if(!IsInt(pstr))
					return FALSE;
				pvar->lints.push_back(atoi(pstr));
				token = strtok(NULL,sepStr );
			}
		}
		else 
			if(IsString('~',psrc))//判断是不是只有数字空格和'-'
			{
				pvar->bRange = TRUE;
				if(*psrc == '-')
					return FALSE;
				
				token = strtok(psrc,sepIntTrue );
				
				while( token != NULL)
				{
					pstr =  Delblank(token);
					
					if(!pstr)
						return FALSE;
					
					if(!IsInt(pstr))
						return FALSE;
					pvar->ints.push_back(atoi(pstr));
					token = strtok(NULL,sepIntTrue );
				}
			}
			
			else
				return FALSE;
			break;
	case ::TianShanIce::vtInts:
		
		pvar->type = TianShanIce::vtInts;
		
		if(IsString(';',psrc))//判断是不是只有数字空格和';'
		{
			pvar->bRange = FALSE;
			if(!DoubleChar(psrc))
				return FALSE;
			if(*psrc == ';')
				return FALSE;
			
			token = strtok(psrc,sepStr );
			while( token != NULL)
			{
				pstr =  Delblank(token);// " ; ;"
				
				if(!pstr)
					return FALSE;
				
				if(!IsInt(pstr))
					return FALSE;
				pvar->ints.push_back(atoi(pstr));
				token = strtok(NULL,sepStr );
			}
		}
		else 
			if(IsString('~',psrc))//判断是不是只有数字空格和'-'
			{
				int cout = 0;
				pvar->bRange = TRUE;
                for(ptemp = psrc; *ptemp != '\0'; ptemp++)
				{
					if(*ptemp == '~')
						cout++;
				}
				if(cout != 1)
					return FALSE;
				if(*psrc == '~')
					return FALSE;
				
				token = strtok(psrc,sepIntTrue );
				
				while( token != NULL)
				{
					pstr =  Delblank(token);
					
					if(!pstr)
						return FALSE;
					
					if(!IsInt(pstr))
						return FALSE;
					pvar->ints.push_back(atoi(pstr));
					token = strtok(NULL,sepIntTrue );
				}
			}
			
			else
				return FALSE;
			break;
	case ::TianShanIce::vtBin:
		return FALSE;
	default:
		break;
 }
 return TRUE;
}
#endif
