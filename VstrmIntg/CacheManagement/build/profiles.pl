# -----------------
# global declares
# -----------------

use vars qw(
	$VSSROOT $VSSUSER $VSSPASSWD $VSSSERVER $SOSDATABASE
	$BUILD_SETTINGS $BUILD_ROOT $RELEASE_ROOT %BUILD_BATCH $SSCMD $MSDEV6CMD $MSDEV7CMD $MSDEV9CMD
	$EMAILATTNS
	);

($VSSROOT, $VSSSERVER, $VSSUSER, $VSSPASSWD, $SOSDATABASE, $BUILD_SETTINGS, $BUILD_ROOT, $RELEASE_ROOT)
 =  ("\\\\192.168.87.4\\vss\\vssmore\\CacheManagement","192.168.87.4:8080", "build", "nightly", "D:\\VSS\\vssmore\\CacheManagement\\srcsafe.ini", "CacheManagement/build", "e:\\CMEV2Build", "\\\\127.0.0.1\\e\$\\CMEV2Release");


$MSVS6_INST = "C:\\Program Files\\Microsoft Visual Studio";
$MSVS7_INST = "C:\\Program Files\\Microsoft Visual Studio .NET 2003";
$MSVS8_INST = "C:\\Program Files\\Microsoft Visual Studio 8";
$MSVS9_INST = "C:\\Program Files\\Microsoft Visual Studio 9.0";
$SSCMD = "\"C:\\Program Files\\SourceOffSite\\soscmd.exe\"";
$MSDEV6CMD = "\"$MSVS6_INST\\Common\\MSDev98\\Bin\\MSDEV.EXE\"";
$MSDEV7CMD = "\"$MSVS7_INST\\Common7\\IDE\\DEVENV.EXE\"";
$MSDEV8CMD = "\"$MSVS8_INST\\Common7\\IDE\\DEVENV.EXE\"";
$MSDEV9CMD = "\"$MSVS9_INST\\Common7\\IDE\\DEVENV.EXE\"";

$EMAILATTNS='ken.qian@xor-media.tv';

use vars qw (
	@NIGHLY_PROJLIST,
	@WORK_PROJLIST,
);
	
# -----------------
# projects build profiles
# -----------------

my %ProjTemplate = (
	'name'			=> 'ProjectTemplate',	# short name of the project, no white spaces allowed here
	'environment'           => {
        	                     'variable'  => 'path',
        	                },   # set private environment variable for this project
	'prod_ver'		=> '0.1.0',		# <major_ver>.<minor_ver>.<patch_number>
	'sources'		=> ['ZQProjs/Common',
						'ZQProjs/ProjTemplate'], # necessary source sub-tree to build this project
	'work_path'		=> 'ZQProjs\\ProjTemplate',		 # the working path when build this project
	'rc_h_files'	=> ['ZQProjs\\ProjTemplate\\console\\ZQResource.h'], # the ZQResource.h files that must be updated with the version info
	'build_cmds'	=> ["$MSDEV6CMD ProjTemplate.dsw /MAKE \"console - Win32 Release\" /REBUILD" ], # the command lines to run the build processes
	'output_pkg'	=> ["ZQProjs\\ProjTemplate\\console\\Release\\console.exe" ], # the final packages from the build process(es)
);



%CMEV2 = (
	'name'		=> 'CMEV2',
	'desc'		=> 'XOR CMEV2 Serivce',
	'dependence'    => ['CMEV2_x64'],
	'xml_path'	=> ['CacheManagement\\CMEV2\\'],	
	'environment'           => {
                                     'RegExppKit'       => 'D:\\SDK4CME\\3rdPartyKits\\regexpp1331',
                                     'ExpatPath'        => 'D:\\SDK4CME\\3rdPartyKits\\expat',
                                     'TianShanSDK'      => 'D:\\SDK4CME\\XORMedia\\TianShanSDK',
                                     'GSOAPPATH'        => 'D:\\SDK4CME\\3rdPartyKits\\gsoap-2.7.10',
		                },
	'owner'		=> ['ken.qian@xor-media.tv'],
	'prod_ver'	=> '2.0.0',
	'sources'	=> ['CacheManagement/build','CacheManagement/CMEV2','CacheManagement/common','CacheManagement/etc'],
	'work_path'	=> "CacheManagement\\CMEV2",
	'rc_h_files'	=> ['CacheManagement/CMEV2/ZQResource.h' ],
	'build_cmds'=> [
	                "$MSDEV8CMD /Rebuild  \"Release|win32\" /project CMEV2 CMEV2.sln",

			".\\script\\CMEV2PKG.bat"],
	'output_pkg'	=> ["CacheManagement\\CMEV2_setup.zip", "CacheManagement\\CMEV2Symbols.zip" ],
);

%CMEV2_x64 = (
	'name'		=> 'CMEV2_x64',
	'desc'		=> 'XOR CMEV2 Serivce',
	'xml_path'	=> ['CacheManagement\\CMEV2\\'],	
	'environment'           => {
                                     'RegExppKit'       => 'D:\\SDK4CME\\3rdPartyKits\\regexpp1331',
                                     'ExpatPath'        => 'D:\\SDK4CME\\3rdPartyKits\\expat',
                                     'TianShanSDK'      => 'D:\\SDK4CME\\XORMedia\\TianShanSDK',
                                     'GSOAPPATH'        => 'D:\\SDK4CME\\3rdPartyKits\\gsoap-2.7.10',		                     
		                },
	'owner'		=> ['ken.qian@xor-media.tv'],
	'prod_ver'	=> '2.0.0',
	'sources'	=> ['CacheManagement/build','CacheManagement/CMEV2','CacheManagement/common','CacheManagement/etc'],
	'work_path'	=> "CacheManagement\\CMEV2",
	'rc_h_files'	=> ['CacheManagement/CMEV2/ZQResource.h' ],
	'build_cmds'=> [
	                "$MSDEV8CMD /Rebuild  \"Release|x64\" /project CMEV2 CMEV2.sln",

			".\\script\\CMEV2PKG_x64.bat"],
);

@NIGHLY_PROJLIST = (
#	ProjTemplate,
	CMEV2,
	CMEV2_x64,
);

@ALL_PROJS =(
	ProjTemplate,
	CMEV2,
	CMEV2_x64,
);
