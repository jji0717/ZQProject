
#include "PoormanOpt.h"
#include <sstream>
#include <strHelper.h>
#include <assert.h>

OptInit::OptInit( OptDescription* owner )
:mOwner(owner)
{
	assert(mOwner != NULL );
}


OptInit& OptInit::operator( )( const char* shortname , bool value , const char* description  , const char* longname ,bool bStatic  )
{
	assert(mOwner != NULL );
	assert( shortname != NULL );
	std::string strShortName = shortname ? shortname : ("");
	std::string strLongName = longname ? longname : ("");
	std::string strDecription = description ? description : ("");
	mOwner->add(strShortName,strDecription,value,strLongName,bStatic);
	return *this;
}
OptInit& OptInit::operator( )( const char* shortname , const char* defaultValue , const char* description , const char* longname , bool bStatic  )
{
	assert(mOwner != NULL );
	assert( shortname != NULL );
	std::string strShortName = shortname ? shortname : ("");
	std::string strLongName = longname ? longname : ("");
	std::string strDecription = description ? description : ("");
	std::string strDefaultValue = defaultValue ? defaultValue : ("");
	mOwner->add(strShortName,strDecription,strDefaultValue,strLongName,bStatic);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
OptDescription::OptDescription( )
{

}
OptDescription::~OptDescription( )
{

}

std::string	OptDescription::fillBlank( size_t maxBlank ) const
{
	std::string str;
	for( int i = 0 ; i < (int)maxBlank ; i++ )
		str = str + (' ');
	return str;
}

std::string	 OptDescription::format( ) const
{
	std::map< std::string , std::string> tmpRelation;//short name to long name map;
	SlRelation::const_iterator itTempLongName = mRelation.begin( );
	for( ; itTempLongName != mRelation.end() ; itTempLongName++ )
	{
		tmpRelation.insert( std::map< std::string , std::string>::value_type( itTempLongName->second , itTempLongName->first ) );
	}
	//round one
	//scan the whole options and find the longest short name and long name
	size_t	maxShortNameLen		= 0;
	size_t	maxLongNameLen		= 0;

	OptionS::const_iterator itOpt = mOpts.begin();
	for( ; itOpt != mOpts.end() ; itOpt ++ )
	{
		size_t tmpSize = itOpt->first.length();
		maxShortNameLen = maxShortNameLen > tmpSize ? maxShortNameLen : tmpSize;
		std::map< std::string , std::string>::const_iterator itLongName = tmpRelation.find(itOpt->first);
		if( itLongName != tmpRelation.end())
		{
			tmpSize = itLongName->second.length();
			maxLongNameLen = maxLongNameLen > tmpSize ? maxLongNameLen : tmpSize;
		}
	}

	maxLongNameLen	+= 4;
	maxShortNameLen	+= 4;
	
	//int minCommentLen = 10;
	int maxCommentLen = (int)( 75 - maxLongNameLen - maxShortNameLen - 3 );
	maxCommentLen = maxCommentLen < 15 ? 15 : maxCommentLen;
	
	//round two
	itOpt = mOpts.begin();
	std::stringstream oss;
	
	//fill static names
	StaticOptNames::const_iterator itStaticName = mStaticNames.begin();
	for( ; itStaticName != mStaticNames.end() ; itStaticName ++ )
	{
		oss << *itStaticName <<" ";
	}
	oss<<"\n";
	oss<<mDesciption << "\n\n";
	std::string commentBlank = fillBlank( maxShortNameLen + maxLongNameLen + 3 );
	for( ; itOpt != mOpts.end() ; itOpt++ )
	{
		if( itOpt->second.bStatic )
			continue;

		const std::string& shortName = itOpt->first;

		oss << ("-") << shortName << fillBlank( maxShortNameLen - shortName.length() ) ;

		//size tmpBlankCount = maxLongNameLen;
		std::map< std::string , std::string>::const_iterator itLongName = tmpRelation.find( itOpt->first );
		if( itLongName != tmpRelation.end() )
		{
			const std::string& strLongName = itLongName->second;
			oss << ("--")<< strLongName<< fillBlank(maxLongNameLen - strLongName.length() );
		}
		else
		{
			oss << fillBlank( maxLongNameLen);
		}

		//display description
		std::string desc = itOpt->second.description;
		int descLen = (int)desc.length();
		bool bFirst = true;
		while( descLen > 0 )
		{
			if( !bFirst )
				oss << commentBlank;
			int displayLen = descLen > maxCommentLen ? maxCommentLen : descLen;
			std::string str = desc.substr(0,displayLen);
			desc = desc.substr(displayLen );
			descLen -= displayLen;
			oss << str << std::endl;						
			bFirst = false;
		}
	}	
	return oss.str();
}

OptInit	OptDescription::addOptions( const std::string& desc )
{
	mDesciption = desc;
	OptInit o(this);
	return o;
}
bool OptDescription::add( const std::string& shortName , const std::string& description , 
					const bool& defaultValue, const std::string& longName,
					bool bStatic)
{
	if( shortName.length() <= 0 ) return false;

	OptionValue optValue;
	optValue.bStatic		= bStatic;
	optValue.defaultValue	= defaultValue ? ("1") : ("0");
	optValue.bFlag			= true;
	optValue.description	= description;

	mOpts.insert(OptionS::value_type( shortName , optValue ));
	if( longName.length() > 0 )
	{
		mRelation.insert(SlRelation::value_type(longName,shortName));
	}
	if( bStatic )
	{
		mStaticNames.push_back(shortName);
	}
	return true;
}

bool OptDescription::add( const std::string& shortName , const std::string& description , 
					const std::string& defaultValue, const std::string& longName ,
					bool bStatic ) 
{
	if( shortName.length() <= 0 ) return false;

	OptionValue optValue;
	optValue.bStatic		= bStatic;
	optValue.defaultValue	= defaultValue;
	optValue.bFlag			= false;
	optValue.description	= description;
	
	mOpts.insert(OptionS::value_type( shortName , optValue ));
	if( longName.length() > 0 )
	{
		mRelation.insert(SlRelation::value_type(longName,shortName));
	}
	if( bStatic )
	{
		mStaticNames.push_back(shortName);
	}
	return true;
}

bool OptDescription::findOptionWithShortName( const std::string& name , bool *bStatic ,bool* bFlag ,std::string* defaultValue ) const
{
	OptionS::const_iterator itShort = mOpts.find(name);
	if( itShort != mOpts.end() )
	{
		if( bStatic )
			*bStatic = itShort->second.bStatic;
		if( bFlag )
			*bFlag = itShort->second.bFlag;
		if( defaultValue )
			*defaultValue = itShort->second.defaultValue;
		return true;
	}
	return false;
}

bool OptDescription::findOption( std::string& name , bool *bStatic ,bool* bFlag ,std::string* defaultValue ) const
{
	if( findOptionWithShortName(name,bStatic,bFlag ,defaultValue ))
		return true;
	SlRelation::const_iterator itRelation = mRelation.find(name);
	if( itRelation != mRelation.end() )
	{
		name = itRelation->second;
		return findOptionWithShortName( itRelation->second ,bStatic ,bFlag , defaultValue ) ;
	}
	return false;
}


size_t OptDescription::getOptionsCount( ) const
{
	return mOpts.size();
}

size_t OptDescription::getStaticOptionCount( ) const
{
	return mStaticNames.size();
}



std::string OptDescription::getStaticOptionName( size_t index ) const
{
	if (index < mStaticNames.size() )
	{
		return mStaticNames[index];
	}
	else
	{
		return std::string((""));
	}
}


//////////////////////////////////////////////////////////////////////////
OptResult::OptResult( )
{

}
OptResult::~OptResult( )
{

}

void OptResult::addResult( const std::string& name , const std::string& value)
{
	if( name.length() <= 0 )	return;		
	mResults.insert(ResultMap::value_type(name,value));
}

size_t OptResult::import( const OptDescription& optDescription )
{
	const OptDescription::OptionS& opts = optDescription.mOpts;
	OptDescription::OptionS::const_iterator it = opts.begin();
	for( ; it != opts.end() ; it ++ )
	{
		if( it->second.defaultValue.length() > 0 )
			mResults.insert(ResultMap::value_type(it->first,it->second.defaultValue));
	}
	return mResults.size();
}

///////////////////////////////////////////////////////////////////////


#define VALIDITER(it) (it != vars.end() )
#define VALIDSTRING(x) ( x.length() > 0 )

bool FindOptionStart( const char* p ,const char*& pOut )
{
	pOut = NULL;
	if( strncmp(p , ("--") , 2 ) == 0 )
	{//long name option
		
		pOut = p + 2;
	}
	else if( *p == ('-') )
	{//short name option
		
		pOut = p + 1;
	}
	else if( *p == ('/') )
	{//short name option or long name option
		
		pOut = p + 1;
	}
	return pOut != NULL ;
}
const char* applyTurnOnOption( const std::string& defaultValue )
{
	const char* pTmp = ("0");
	if( !defaultValue.empty() )
	{
		if( atoi(defaultValue.c_str() ) != 0 )
		{
			pTmp = ("0");
		}
		else
		{
			pTmp = ("1");
		}
	}
	else
	{
		pTmp = ("1");
	}
	return pTmp;
}
bool ParseOptions( const OptDescription& desc , OptResult& result ,const std::vector<std::string>& vars )
{
	size_t staticOptionIndex  = 0;
	std::vector<std::string>::const_iterator it = vars.begin() ;
	std::string defaultValue;
	for( ; it != vars.end() ; it ++ )
	{
		defaultValue.clear();

		const char* pOpt = NULL;
		//const char* p = it->c_str();
		if( it->length() > 1 )
		{
			if( FindOptionStart( it->c_str() , pOpt ) )
			{
				it ++;
			}
			if(  pOpt )
			{
				if( VALIDITER(it) )
				{
					//const char* pTmpOut = NULL;
					bool bFlag = false;
					std::string optName = pOpt;
					if( desc.findOption(optName,NULL,&bFlag,&defaultValue) && bFlag )
					{
						it--;
						
						result.addResult( optName,applyTurnOnOption(defaultValue));
					}
					else
					{
						result.addResult( optName , *it);						
					}					
				}
				else
				{					
					std::string optName = pOpt ;
					//get default value 
					desc.findOption(optName,NULL,NULL,&defaultValue);
					result.addResult(optName,applyTurnOnOption(defaultValue));
				
					it--;
				}
				continue;
			}			
		}
		
		//static parameter without option
		std::string strName = desc.getStaticOptionName( staticOptionIndex );
		if( strName.length() > 0 )
		{
			result.addResult( strName , *it );
		}
		staticOptionIndex++;
	}

	if( staticOptionIndex < desc.getStaticOptionCount() )
	{
		size_t optCount = desc.getStaticOptionCount();
		size_t i = 0;
		for( i = staticOptionIndex; i < optCount ; i ++ )
		{
			std::string strName = desc.getStaticOptionName( i );
			if( strName.length() <= 0 )
				continue;
			std::string tmpDefaultValue;
			if(!desc.findOption( strName ,NULL , NULL ,&tmpDefaultValue ) )
				continue;			
			if( tmpDefaultValue.empty())
				continue;
			result.addResult( strName , tmpDefaultValue );			
		}
		if( i < optCount)
			return false;
	}

	//check all options with default value
	if( result.import(desc) < desc.getOptionsCount() )
		return false;

	return true;
}

bool ParseOptions( const OptDescription& desc , OptResult& result , int argc , const char* argv[] )
{
	std::vector<std::string>	paras;
	for ( int  i = 1 ;i < argc ; i++ )
	{
		std::string tmp = argv[i];
		ZQ::common::stringHelper::TrimExtra( tmp  ) ;
		paras.push_back( tmp );
	}
	return ParseOptions(  desc , result , paras );
}

