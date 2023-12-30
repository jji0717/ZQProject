
ColorBarps.dll: dlldata.obj ColorBar_p.obj ColorBar_i.obj
	link /dll /out:ColorBarps.dll /def:ColorBarps.def /entry:DllMain dlldata.obj ColorBar_p.obj ColorBar_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del ColorBarps.dll
	@del ColorBarps.lib
	@del ColorBarps.exp
	@del dlldata.obj
	@del ColorBar_p.obj
	@del ColorBar_i.obj
