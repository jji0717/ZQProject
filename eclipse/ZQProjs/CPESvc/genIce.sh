${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/CPE/CPE.ICE  -I .  -I ${ZQProjsPath}/TianShan/CPE  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/CPE
${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2freeze -I .  -I ${ZQProjsPath}/TianShan/CPE  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --index "TianShanIce::ContentProvision::ContentToProvision,TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" ContentToProvision ${ZQProjsPath}/TianShan/CPE/CPE.ICE --output-dir ${ZQProjsPath}/TianShan/CPE