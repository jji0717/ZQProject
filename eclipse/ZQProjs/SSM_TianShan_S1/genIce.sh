${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/StreamSmith/Modules/SsmTianShanS1_Now/SessionContext_ice.ICE  -I .  -I ${ZQProjsPath}/TianShan/StreamSmith/Modules/SsmTianShanS1_Now  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/StreamSmith/Modules/SsmTianShanS1_Now

${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2freeze -I .  -I ${ZQProjsPath}/TianShan/StreamSmith/Modules/SsmTianShanS1_Now  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --index "TianShanS1::StreamIdx, TianShanS1::SessionContext, streamID" StreamIdx ${ZQProjsPath}/TianShan/StreamSmith/Modules/SsmTianShanS1_Now/SessionContext_ice.ICE --output-dir ${ZQProjsPath}/TianShan/StreamSmith/Modules/SsmTianShanS1_Now

