cmake_minimum_required(VERSION 2.8)

project(TianShanIce)

include(${PROJECT_SOURCE_DIR}/../../ZQProjs.cmake)

include_directories(${icesdkfolder}/include)

set(icesource "")

foreach(icefile ZQIceVersion TianShanIce TsSRM TsEvents TsStorage TsTransport TsSite TsApplication TsStreamer TsAppBcast TsEdgeResource TsContentProv)
	add_custom_command( OUTPUT ${icefile}.cpp ${icefile}.h
					COMMAND ${slice2cpp_bin} -I${icesdkfolder}/slice -I${PROJECT_SOURCE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/${icefile}.ICE 
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.cpp ${PROJECT_SOURCE_DIR}
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.h ${PROJECT_SOURCE_DIR})
	set(icesource "${icesource} ${icefile}.cpp ${icefile}.h ")
endforeach(icefile)

#add_custom_command( OUTPUT ZQIceVersion.cpp ZQIceVersion.h 
#					COMMAND ${slice2cpp_bin} -I${icesdkfolder}/slice -I${PROJECT_SOURCE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/ZQIceVersion.ICE 
#					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/ZQIceVersion.* ${PROJECT_SOURCE_DIR})

#add_custom_command( OUTPUT TianShanIce.cpp TianShanIce.h 
#					COMMAND ${slice2cpp_bin} -I${icesdkfolder}/slice -I${PROJECT_SOURCE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/TianShanIce.ICE 
#					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/TianShanIce.* ${PROJECT_SOURCE_DIR} )

add_library(TianShanIce SHARED ZQIceVersion.cpp ZQIceVersion.h
								TianShanIce.cpp TianShanIce.h
								TsSRM.cpp TsSRM.h
								TsEvents.cpp TsEvents.h
								TsStorage.cpp TsStorage.h
								TsTransport.cpp TsTransport.h
								TsSite.cpp TsSite.h
								TsApplication.cpp TsApplication.h
								TsStreamer.cpp TsStreamer.h
								TsAppBcast.cpp TsAppBcast.h
								TsEdgeResource.cpp TsEdgeResource.h
								TsContentProv.cpp TsContentProv.h )
#add_library(TianShanIce SHARED ${icesource})

