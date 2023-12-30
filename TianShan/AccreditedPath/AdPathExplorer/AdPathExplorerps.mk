
AdPathExplorerps.dll: dlldata.obj AdPathExplorer_p.obj AdPathExplorer_i.obj
	link /dll /out:AdPathExplorerps.dll /def:AdPathExplorerps.def /entry:DllMain dlldata.obj AdPathExplorer_p.obj AdPathExplorer_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del AdPathExplorerps.dll
	@del AdPathExplorerps.lib
	@del AdPathExplorerps.exp
	@del dlldata.obj
	@del AdPathExplorer_p.obj
	@del AdPathExplorer_i.obj
