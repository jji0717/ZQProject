1. Extract the zip file into your destination directory

2. install WinPcap, run "WinPcap_3_1.exe" under the extracted directory

3. run IngestSim, usage: 
IngestSim [-s multiIP:port] [-d ftp://username:passwd@server:port/filename] [-f filename] [-h]
Forward received stream package from one interface to another ftp server interface.
options:
        -b ip             specify bind local ip
        -s ip:port  specify the listening ip and port, defaut is 225.25.25.25:1234
        -d ftp://username:passwd@server:port/filename
        -f filename, save to local machine, disable default, for test
        -l logfile log file name, default is c:\IngestSim.log
        -t timeout timeout value in second, default means infinite
        -h            display this help