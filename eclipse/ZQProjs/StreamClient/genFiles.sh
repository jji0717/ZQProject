
if [ -e ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/unistd.h ]; then
rm -vf ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/unistd.h;
fi

 
rm -vf ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSGrammer.hpp ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSGrammer.cpp

bison -dvtl -o  ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSGrammer.cpp ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSGrammer.y




rm -vf ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSScanner.cpp

flex -o ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSScanner.cpp ${ZQProjsPath}/TianShan/StreamSmith/StreamClient/SSScanner.l


${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp ${ZQProjsPath}/TianShan/StreamSmith/Service/StreamSmithAdmin.ICE -I . -I ${ZQProjsPath}/TianShan/StreamSmith/Service -I ${ZQProjsPath}/TianShan/StreamSmith/StreamClient -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice --output-dir ${ZQProjsPath}/TianShan/StreamSmith/StreamClient
