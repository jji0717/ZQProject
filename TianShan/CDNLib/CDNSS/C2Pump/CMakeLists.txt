cmake_minimum_required(VERSION 2.8)

project(C2Streamer)

include(${PROJECT_SOURCE_DIR}/../../../../ZQProjs.cmake)

include_directories(${zqcommonfolder})
include_directories(${zqcommonfolder}/HttpEngine)
include_directories(${PROJECT_SOURCE_DIR})



add_library(C2Streamer STATIC AioFile.cpp AioFile.h C2EventUpdater.cpp C2EventUpdater.h
								C2HttpHandler.cpp C2HttpHandler.h
								C2Session.cpp C2Session.h C2SessionDataRunner.cpp C2SessionHelper.cpp 
								C2SessionHelper.h C2SessionManager.cpp C2SessionManager.h 
								C2StreamerEnv.cpp C2StreamerEnv.h C2StreamerLib.cpp C2StreamerLib.h
								C2StreamerService.cpp C2StreamerService.h C2TransferInit.cpp 
								C2TransferSession.cpp C2TransferTerm.cpp PortManager.cpp PortManager.h 
								LocateSparseTsHeader.cpp LocateSparseTsHeader.h )
