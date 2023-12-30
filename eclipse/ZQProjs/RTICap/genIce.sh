#!/bin/bash

${SYS_INTERP} --library-path ${ICE_ROOT}/lib ${ICE_ROOT}/bin/slice2cpp  ${ZQProjsPath}/TianShan/CPE/CPE.ICE -I ${ZQProjsPath}/TianShan/Ice -I ${ICE_ROOT}/slice   --output-dir ${ZQProjsPath}/TianShan/CPE

