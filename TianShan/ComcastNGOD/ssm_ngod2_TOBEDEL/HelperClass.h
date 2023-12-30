#ifndef _SSM_NGOD2_PLUGIN_HELPER_CLASS_HEADER_FILE_H__
#define _SSM_NGOD2_PLUGIN_HELPER_CLASS_HEADER_FILE_H__

#include <TianShanDefines.h>
#include <TsStreamer.h>


class HelperClass
{
public:
	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Int& valueOut );
	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Int>& valueOut ,bool& bRange );
	static	void	getValueMapDataWithdefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Int& valueDefault , Ice::Int& valueOut );


	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Long& valueOut );
	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Long>& valueOut ,bool& bRange );
	static	void	getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Long& valueDefault , Ice::Long& valueOut );

	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Float& valueOut );
	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Float>& valueOut ,bool& bRange );
	static	void	getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Float& valueDefault , Ice::Float& valueOut );

	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::string& valueOut );
	static	void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<std::string>& valueOut ,bool& bRange );
	static	void	getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key ,const std::string& valueDefault , std::string& valueOut );

};

class NgodUtilsClass
{
public:
	/// add by zjm to support session history
	static std::string generatorISOTime();

	/// add by zjm to modify Notice header string
	static 	std::string generatorNoticeString(const std::string strNoticeCode, 
		const std::string strNoticeString, const std::string strNpt = "");

};

#endif//_SSM_NGOD2_PLUGIN_HELPER_CLASS_HEADER_FILE_H__
