========================================================================
    CONSOLE APPLICATION : DupMIBSvc Project Overview
========================================================================

DupMIBSvc -s <ServiceName or ServiceOID> -n <SvcSeqNum> -o SvcXXX_SeqNum.MIB TianShan.MIB";
   '-s' to specify a service type available in TianShan.MIB";
   '-n' to specify the [0~9] sequence number of the service to duplicate as"; 
   '-o' is the output new MIB file";   
   
   
Special function:
    "DupMIBSvc.exe -s 1000 -n 3 -o debug.MIB TianShan.MIB"
    "DupMIBSvc.exe -s tianShanService -f tianShanComponents -n 3 -p DupMIBSvc -o debug.MIB TianShan.MIB"
    "DupMIBSvc.exe -s tianShanService -f tianShanComponents -n 3 -p TreeDupMIBSvc -o SvcXXX_SeqNum.MIB TianShan.MIB"
    "DupMIBSvc.exe -s tianShanService -f tianShanComponents -n 3 -p TreeDupMIBSvc -o SvcXXX_SeqNum.MIB -c TianShan.MIB"
    "DupMIBSvc.exe -s tianShanService -f tianShanComponents -n 3 -p TreeDupMIBSvc -o SvcXXX_SeqNum.MIB -c TianShan.MIB"
    "DupMIBSvc.exe -s tianShanService -f tianShanComponents -n 0 -p TreeDupMIBSvc -o SvcXXX_SeqNum.MIB -c TianShan.MIB"
    "DupMIBSvc.exe -s tianShanService -f tianShanComponents -n 0 -p DupMIBSvc -o SvcXXX_SeqNum.MIB -c TianShan.MIB"
