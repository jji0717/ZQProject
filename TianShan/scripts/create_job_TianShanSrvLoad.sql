Declare @JobID BINARY(16)
Declare @db_name varchar(100)
Declare @db_job_name varchar(200)
Declare @db_job_schedule_name varchar(200)
Declare @db_job_command varchar(200)
Declare @db_job_starttime varchar(20)

Set @db_name = 'kongzhe_pm'
Set @db_job_backup_path = 'e:\zzzz\'
Set @db_job_starttime = '050000'
Set @db_job_name = 'DailyBackupJob_' + @db_name
Set @db_job_schedule_name = 'DailyBackupJobSchedule_' + @db_name
Set @db_job_command = 'exec sp_backup_db ''' + @db_name + ''', ''' + @db_job_backup_path + ''''


SELECT @JobID = job_id     
	FROM   msdb.dbo.sysjobs    
  	WHERE (name = @db_job_name)

IF (@JobID IS NOT NULL) 
Begin  	  
	EXEC msdb.dbo.sp_delete_job @job_name = @db_job_name
End

exec msdb.dbo.sp_add_job
    @job_name = @db_job_name,
    @enabled = 1, -- enable
    @description = 'Backup database daily',
    @notify_level_eventlog = 3, -- always
    @delete_level = 0, -- never delete
    @start_step_id = 1


exec msdb.dbo.sp_add_jobstep
    @job_name = @db_job_name,
    @step_id = 1,
    @step_name = 'backup',
    @subsystem = 'TSQL',
    @command = @db_job_command,
    @database_name = @db_name


exec msdb.dbo.sp_add_jobschedule
    @job_name = @db_job_name,
    @name = @db_job_schedule_name,
    @enabled = 1, -- enable
    @freq_type = 4, -- daily
    @freq_interval = 1, -- do job once per day
    @active_start_time = @db_job_starttime

exec msdb.dbo.sp_add_jobserver
    @job_name = @db_job_name,
    @server_name = N'(LOCAL)'



