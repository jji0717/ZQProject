cmake_minimum_required(VERSION 2.8)

project(CDNSS)

include(${PROJECT_SOURCE_DIR}/../../../ZQProjs.cmake)
include_directories(${zqcommonfolder})
include_directories(${zqcommonfolder}/DataPostHouse)
include_directories(${zqcommonfolder}/HttpEngine)
include_directories(${zqprojsroot})
include_directories(${icesdkfolder}/include)
include_directories(${tianshanicefolder})
include_directories(${tianshanicefolder}/Shell/ZQSNMPManPkg)
include_directories(${tianshancommonfolder})
include_directories(${zqtianshanfolder}/StreamService/ServiceLib)
include_directories(${zqtianshanfolder}/ContentStore)
include_directories(${zqtianshanfolder}/CPE)
include_directories(${zqtianshanfolder}/ContentStore/MediaClusterCS)
include_directories(${vstrmsdkfolder}/include)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/C2Streamer)

add_definitions(-DNAS -DEXCLUDE_VSTRM_API)

foreach(icefile C2Locator)
	add_custom_command( OUTPUT ${icefile}.cpp ${icefile}.h
					COMMAND ${slice2cpp_bin} -I${icesdkfolder}/slice -I${tianshanicefolder} -I${tianshancommonfolder} -I${PROJECT_SOURCE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/../CRM_C2Locator/${icefile}.ice
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.cpp ${PROJECT_SOURCE_DIR}
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.h ${PROJECT_SOURCE_DIR})
	set(icesource "${icesource} ${icefile}.cpp ${icefile}.h ")
endforeach(icefile)

add_executable(CDNSS  C2Locator.cpp C2Locator.h CdnEnv.cpp CdnEnv.h 						
						CdnSsCommand.cpp CdnSsCommand.h CdnSSConfig.h 
						CdnStreamerManager.cpp CdnStreamerManager.h CdnSvc.cpp CdnSvc.h
						cspNative.cpp SimpleXMLParser.cpp SimpleXMLParser.h
						ProvEventSink.cpp ProvEventSink.h EmbedContentStore.cpp
						IdxFileParserEnvironment.cpp IdxFileParserEnvironment.h
						IndexFileParser.cpp IndexFileParser.h
						${zqcommonfolder}/ConfigHelper.cpp 
						${zqcommonfolder}/ZQDaemon.cpp 
						${zqcommonfolder}/HttpClient.cpp
						${zqtianshanfolder}/ContentStore/CPCImpl.cpp
						${zqtianshanfolder}/ContentStore/MediaClusterCS/ContentProvisionWrapper.cpp )

target_link_libraries(CDNSS  C2Streamer ZQCommon HttpEngine DataPostHouse  SnmpManPkg ServiceShell
							TianShanIce TianShanCommon StreamService ContentStore boost_regex nl
							${libice} ${libiceutil} ${libfreeze} ${libicestorm} ${libdb} )

subdirs(C2Streamer)