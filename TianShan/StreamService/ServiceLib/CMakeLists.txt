cmake_minimum_required(VERSION 2.8)

project(StreamService)

include(${PROJECT_SOURCE_DIR}/../../../ZQProjs.cmake)

add_definitions(-DWITH_ICESTORM)

include_directories(${icesdkfolder}/include)
include_directories(${zqcommonfolder})
include_directories(${tianshancommonfolder})
include_directories(${tianshanicefolder})
include_directories(${PROJECT_SOURCE_DIR})


foreach(icefile StreamService Playlist)
	add_custom_command( OUTPUT ${icefile}.cpp ${icefile}.h
					COMMAND ${slice2cpp_bin} -I${icesdkfolder}/slice -I${tianshanicefolder} -I${PROJECT_SOURCE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/../Ice/${icefile}.ICE 
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.cpp ${PROJECT_SOURCE_DIR}
					COMMAND ${copycommand} ${PROJECT_BINARY_DIR}/${icefile}.h ${PROJECT_SOURCE_DIR})
	set(icesource "${icesource} ${icefile}.cpp ${icefile}.h ")
endforeach(icefile)

add_library(StreamService STATIC  StreamService.cpp StreamService.h Playlist.cpp Playlist.h EventSender.cpp EventSender.h 
									renewTicket.cpp renewTicket.h Scheduler.cpp Scheduler.h SsEnvironment.cpp SsEnvironment.h
									SsServiceImpl.cpp SsServiceImpl.h SsStreamBase.cpp SsStreamBase.h 
									SsStreamImpl.cpp SsStreamImpl.h SsStreamEventCenter.cpp SsStreamImplControl.cpp
									StreamFactory.cpp StreamFactory.h StreamService.cpp StreamService.h 	)