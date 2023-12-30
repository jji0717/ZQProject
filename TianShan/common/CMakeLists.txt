cmake_minimum_required(VERSION 2.8)

project(TianShanCommon)

include(${PROJECT_SOURCE_DIR}/../../ZQProjs.cmake)

add_definitions(-DWITH_ICESTORM)

include_directories(${icesdkfolder}/include)
include_directories(${zqcommonfolder})
include_directories(${tianshancommonfolder})

foreach(icefile ZqSentryIce TianShanUtils)
	add_custom_command( OUTPUT ${icefile}.cpp ${icefile}.h
					COMMAND ${slice2cpp_bin} -I${icesdkfolder}/slice -I${tianshanicefolder} -I${PROJECT_SOURCE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/${icefile}.ICE 
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.cpp ${PROJECT_SOURCE_DIR}
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.h ${PROJECT_SOURCE_DIR})
	set(icesource "${icesource} ${icefile}.cpp ${icefile}.h ")
endforeach(icefile)

add_library(TianShanCommon STATIC  EventChannel.cpp TianShanUtils.cpp TianShanDefines.cpp ZqSentryIce.cpp
									ZqAdapter.cpp HtmlTempl.cpp IceLog.cpp TimerWatchdog.cpp TianShanIceHelper.cpp)
