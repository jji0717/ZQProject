cmake_minimum_required(VERSION 2.8)

project(TianShanProgram)

set(icesdkfolder "/opt/Ice-3.3")
set(vstrmsdkfolder "/opt/VstrmKit6.1.0-RC16")
#get slice2cpp slice2freeze binary
find_program( slice2cpp_bin slice2cpp PATHS "${icesdkfolder}/bin")
find_program( slice2freeze_bin slice2freeze PATHS "${icesdkfolder}/bin" )
find_library( libice libIce.so PATHS "${icesdkfolder}/lib")
find_library( libiceutil libIceUtil.so PATHS "${icesdkfolder}/lib")
find_library( libfreeze libFreeze.so PATHS "${icesdkfolder}/lib")
find_library( libicestorm libIceStorm.so PATHS "${icesdkfolder}/lib")
find_library( libdb libdb_cxx-4.6.so PATHS "/usr/local/BerkeleyDB.4.6/lib")

SET(CMAKE_SKIP_BUILD_RPATH  TRUE)

#get cp command
find_program( copycommand cp )

#set ice relative folder
set(iceincludefolder "${icesdkfolder}/include")
set(icebinfolder "${icesdkfolder}/bin")
set(icelibfolder "${icesdkfolder}/lib")
set(iceslicefolder "${icesdkfolder}/slice")

#TianShan bin output folder
set( binoutputfolder "$ENV{ZQPROJSPATH}/TianShan/bin" )

#TianShan lib output folder
set( liboutputfolder "$ENV{ZQPROJSPATH}/TianShan/bin" )

#TianShanIce folder
set( tianshanicefolder "$ENV{ZQPROJSPATH}/TianShan/Ice")
set( tianshancommonfolder "$ENV{ZQPROJSPATH}/TianShan/common")


#top source folder
set(zqprojsroot "$ENV{ZQPROJSPATH}")

#zqcommon folder
set(zqcommonfolder "${zqprojsroot}/Common")

#tianshan folder
set(zqtianshanfolder "${zqprojsroot}/TianShan")

#TianShan Shell snmp folder
set(tianshansnmpfolder "${zqprojsroot}/TianShan/Shell/ZQSNMPManPkg")

#TianShan Shell CfgPkg folder
set(tianshancfgpkgfolder "${zqprojsroot}/TianShan/Shell/ZQCfgPkg")

#TianShan Shell ZQ App shell
set(zqappshellfolder "${zqprojsroot}/TianShan/Shell/ZQAppShell")

#TianShan Shell ZQ Shell
set(zqshellfolder "${zqprojsroot}/TianShan/Shell/ZQShell")



#set include directory list
include_directories("${zqprojsroot}/Common")
include_directories("${zqprojsroot}/TianShan/Include")
include_directories("${zqprojsroot}/TianShan/Shell/ZQSNMPManPkg")
include_directories("${zqprojsroot}/TianShan/Shell/ZQCfgPkg")
include_directories("${zqprojsroot}/TianShan/Ice")
include_directories("${zqprojsroot}/TianShan/common")
include_directories("${zqprojsroot}/TianShan/common")


