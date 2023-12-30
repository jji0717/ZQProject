
PlaylistSoapUDTMapperps.dll: dlldata.obj PlaylistSoapUDTMapper_p.obj PlaylistSoapUDTMapper_i.obj
	link /dll /out:PlaylistSoapUDTMapperps.dll /def:PlaylistSoapUDTMapperps.def /entry:DllMain dlldata.obj PlaylistSoapUDTMapper_p.obj PlaylistSoapUDTMapper_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del PlaylistSoapUDTMapperps.dll
	@del PlaylistSoapUDTMapperps.lib
	@del PlaylistSoapUDTMapperps.exp
	@del dlldata.obj
	@del PlaylistSoapUDTMapper_p.obj
	@del PlaylistSoapUDTMapper_i.obj
