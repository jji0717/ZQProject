bison -dvtl -o ${ZQProjsPath}/TianShan/ContentLib/ContentLibClient/ContentLibGrammer.cpp ${ZQProjsPath}/TianShan/ContentLib/ContentLibClient/ContentLibGrammer.y
flex -o ${ZQProjsPath}/TianShan/ContentLib/ContentLibClient/ContentLibScanner.cpp ${ZQProjsPath}/TianShan/ContentLib/ContentLibClient/ContentLibScanner.l
${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/MetaLib/MetaLib.ICE  -I .  -I ${ZQProjsPath}/TianShan/MetaLib  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/MetaLib
${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/ContentLib/ContentReplicaEx.ICE  -I .  -I ${ZQProjsPath}/TianShan/MetaLib  -I ${ZQProjsPath}/TianShan/ContentLib  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/ContentLib

