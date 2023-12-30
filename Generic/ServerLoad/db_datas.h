#ifndef __db_datas_H__
#define __db_datas_H__

#include <vector>

namespace SrvrLoad
{

// describe the record in ote_server table
struct ote_server_rec
{
	int cm_group_id;
	int server_type; // 1 for Axiom, 2 for TianShan
	int interval_time;
	int pg_level;
	char version[21]; // mayorVersion.minorVersion
	char data_time[26]; // YYYY-MM-DDThh:mm:ss
	bool bServerTypeEmpty;
	bool bIntervalTimeEmpty;
	bool bPGLevelEmpty;
};

// describe the record in ote_instance table
struct ote_instance_rec
{
	int cm_group_id;
	char host_name[101];
	char session_protocal_type[11];
	char ip_address[51];
	int port;
	int server_load;
	int life_time;
	bool bPortEmpty;
	bool bServerLoadEmpty;
	bool bLifeTimeEmpty;
};

// describe the record in ote_group table
struct ote_group_rec
{
	int cm_group_id;
	__int64 node_group;
};

// describe the record in ote_cm_app table
struct ote_cm_app_rec
{
	int cm_group_id;
	char app_type[201];
};

class db_datas
{
public: 
	db_datas();
	virtual ~db_datas();

public: 
	void add_ote_server_rec(const ote_server_rec& value);
	void add_ote_instance_rec(const ote_instance_rec& value);
	void add_ote_group_rec(const ote_group_rec& value);
	void add_ote_cm_app_rec(const ote_cm_app_rec& value);

	const std::vector<ote_server_rec>& get_ote_servers() const;
	const std::vector<ote_instance_rec>& get_ote_instances() const;
	const std::vector<ote_group_rec>& get_ote_groups() const;
	const std::vector<ote_cm_app_rec>& get_ote_cm_apps() const;

	void erase_ote_servers();
	void erase_ote_instances();
	void erase_ote_groups();
	void erase_ote_cm_apps();

private: 
	std::vector<ote_server_rec> _ote_servers;
	std::vector<ote_instance_rec> _ote_instances;
	std::vector<ote_group_rec> _ote_groups;
	std::vector<ote_cm_app_rec> _ote_cm_apps;

};

}

#endif // #define __db_datas_H__

