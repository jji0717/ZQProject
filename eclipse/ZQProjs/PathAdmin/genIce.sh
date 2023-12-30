rm -vf ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleGrammar.hpp ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleGrammar.cpp
bison -dvtl -o  ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleGrammar.cpp ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleGrammar.y

rm -vf ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleScanner.cpp
flex -o ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleScanner.cpp ${ZQProjsPath}/TianShan/AccreditedPath/admin/ConsoleScanner.l

${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/AccreditedPath/TsPathAdmin.ICE  -I .  -I ${ZQProjsPath}/TianShan/AccreditedPath  -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/AccreditedPath
