if exists (select 1
            from  sysobjects
           where  id = object_id('ote_server')
            and   type = 'U')
   drop table ote_server
go

/*==============================================================*/
/* Table : ote_server                                           */
/*==============================================================*/
create table ote_server (
cm_group_id          int                  not null,
server_type          int                  null,
version              varchar(20)          null,
date_time            varchar(25)          null,
interval_time        int                  null,
pg_level             numeric(1)           null,
constraint PK_OTE_SERVER primary key  (cm_group_id)
)
go



if exists (select 1
            from  sysobjects
           where  id = object_id('ote_nodegroup')
            and   type = 'U')
   drop table ote_nodegroup
go

/*==============================================================*/
/* Table : ote_nodegroup                                        */
/*==============================================================*/
create table ote_nodegroup (
cm_group_id          int		  not null,
nodegroup            bigint                  not null,
constraint PK_OTE_NODEGROUP primary key  (cm_group_id, nodegroup)
)
go


if exists (select 1
            from  sysobjects
           where  id = object_id('ote_cm_app')
            and   type = 'U')
   drop table ote_cm_app
go

/*==============================================================*/
/* Table : ote_cm_app                                        */
/*==============================================================*/
create table ote_cm_app (
cm_group_id         int		      not null,
app_type            varchar(200)      not null,
constraint PK_OTE_CM_APP primary key  (cm_group_id, app_type)
)
go


if exists (select 1
            from  sysobjects
           where  id = object_id('ote_instance')
            and   type = 'U')
   drop table ote_instance
go

/*==============================================================*/
/* Table : ote_instance                                         */
/*==============================================================*/
create table ote_instance (
cm_group_id            int                  not null,
host_name              varchar(100)         null,
ip_address             varchar(50)          not null,
port                   int                  not null,
server_load            numeric(4)           null,
life_time              numeric(8)           null,
session_protocal_type  varchar(10)          null,
constraint PK_OTE_INSTANCE primary key  (cm_group_id, ip_address, port)
)
go