#!/bin/bash

${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/StreamService/StreamPumper/StreamService.ICE -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/StreamService/StreamPumper
${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/StreamService/StreamPumper/Playlist.ICE -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/StreamService/StreamPumper
${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/Ice/TsStreamer.ICE -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/Ice

