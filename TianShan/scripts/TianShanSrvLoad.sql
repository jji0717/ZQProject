
IF EXISTS (SELECT * FROM sysobjects WHERE id = object_id('dbo.updateTianShanSrvLoad') AND sysstat & 0xf = 4)
	drop procedure dbo.updateTianShanSrvLoad
GO
CREATE PROCEDURE updateTianShanSrvLoad
	@rtspprxyIP1 varchar(15),
	@rtspprxyIP2 varchar(15)
AS
BEGIN
	DECLARE @intLoad1 int
	DECLARE @intLoad2 int	

	SELECT @intLoad1 = 100 * RAND( (DATEPART(mm, GETDATE()) * 100000 )
 	           	                + (DATEPART(ss, GETDATE()) * 1000 )
	                                + DATEPART(ms, GETDATE()) )
	
	SELECT @intLoad2 = 100 * RAND( (DATEPART(mm, GETDATE()) * 1000 )
	           + (DATEPART(ss, GETDATE()) * 100000 )
	           + DATEPART(ms, GETDATE()) )


	SELECT @intLoad1 = @intLoad1 % 10
	SELECT @intLoad2 = @intLoad2 % 10


	-- delete existing data to avoid dumplication
	DELETE FROM OTE_INSTANCE

	-- insert ote-server information
	INSERT INTO ote_server (Version,Date_Time,interval_time,App_Type,PG_Level) 
        	VALUES ('1.6', GETDATE(), '300', '0X5F00001', '0')

	-- delete existing data to avoid dumplication
	DELETE FROM ote_instance
	
	--reset the ip for RTSPProxy Instance 1
	INSERT INTO ote_instance (instance_number,ip_address,port,server_load,life_time) 
			VALUES   (1, @rtspprxyIP1, 5541, @intLoad1, 330)
	
	--reset the ip for RTSPProxy Instance 2
	INSERT INTO ote_instance (instance_number,ip_address,port,server_load,life_time) 
			VALUES   (2, @rtspprxyIP2, 5541, @intLoad2, 330)
	
END
