Build:
The CRM_SymbolServer need a new lib: zlib. Unzip the SymbolServer/tools/zib-1.2.5.zip to a folder and add an environment variable ZLIB_ROOT with the path as value. Then open the SymbolServer.sln and build the CRM_SymbolServer.


Server Side:
1. Install HttpCRG from TianShan kit.
2. Copy the CRM_SymbolServer.dll to the "TianShan/modules/". And configure the HttpCRG.xml to load this module.
3. Copy the SymbolServer.ini to the "TianShan/etc/". Then configure the DBRoot with the TianShan symbols' publish directory. And specify a temporary directory by TMPDir.
4. Start the HttpCRG. Then the client can request the PDB file via url: "http://HOST:PORT/symbols?f=<ModuleName>&v=<Version>[&x64]".


Client Side:
We use the curl.exe utility as the HTTP client.
1. Copy the curl.exe to one of your PATH folder.
2. Download file by this command:
   curl <URL> -s --compressed -o <SaveAsName>  -w %{http_code}
   This command will request the resource specified in the URL and save it to a local file. Then curl print out the response code to the screen.
3. There is a script named "getpdb.bat" which can ease the downloading.
   1) Copy the getpdb.bat to one of your PATH folder.
   2) Specify the symbol server endpoint and saving directory via the environment variables SymbolServer(default hawkeye:10080) and SymbolDir(default C:\sym).
   3) Call the getpdb.bat in format of:
      getpdb <ModuleName> <Version> [x64]
   3) The screen output is the HTTP status code. 200 means successfully downloading. 404 means file not found.
   For example:
   set SymbolDir=C:\abc
   getpdb ssm_ngod2 1.10.2.26 x64
   If 200 shows up, the 64-bits ssm_ngod2.pdb of TianShan v1.10.2.26 will be downloaded from http://hawkeye:10080/ and be saved as C:\abc\ssm_ngod2.pdb.
