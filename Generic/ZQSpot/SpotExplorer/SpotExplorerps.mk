
ATL_INFOps.dll: dlldata.obj ATL_INFO_p.obj ATL_INFO_i.obj
	link /dll /out:ATL_INFOps.dll /def:ATL_INFOps.def /entry:DllMain dlldata.obj ATL_INFO_p.obj ATL_INFO_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del ATL_INFOps.dll
	@del ATL_INFOps.lib
	@del ATL_INFOps.exp
	@del dlldata.obj
	@del ATL_INFO_p.obj
	@del ATL_INFO_i.obj
