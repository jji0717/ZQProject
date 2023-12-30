#include "./db_datas.h"

#pragma warning(disable: 4786)

namespace SrvrLoad
{

db_datas::db_datas()
{
}

db_datas::~db_datas()
{
}

void db_datas::add_ote_server_rec(const ote_server_rec& value)
{
	_ote_servers.push_back(value);
}

void db_datas::add_ote_instance_rec(const ote_instance_rec& value)
{
	_ote_instances.push_back(value);
}

void db_datas::add_ote_group_rec(const ote_group_rec& value)
{
	_ote_groups.push_back(value);
}

void db_datas::add_ote_cm_app_rec(const ote_cm_app_rec& value)
{
	_ote_cm_apps.push_back(value);
}

const std::vector<ote_server_rec>& db_datas::get_ote_servers() const
{
	return _ote_servers;
}

const std::vector<ote_instance_rec>& db_datas::get_ote_instances() const
{
	return _ote_instances;
}

const std::vector<ote_group_rec>& db_datas::get_ote_groups() const
{
	return _ote_groups;
}

const std::vector<ote_cm_app_rec>& db_datas::get_ote_cm_apps() const
{
	return _ote_cm_apps;
}

void db_datas::erase_ote_servers()
{
	_ote_servers.clear();
}

void db_datas::erase_ote_instances()
{
	_ote_instances.clear();
}

void db_datas::erase_ote_groups()
{
	_ote_groups.clear();
}

void db_datas::erase_ote_cm_apps()
{
	_ote_cm_apps.clear();
}

}

