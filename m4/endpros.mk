if DBGFILE
dbgfile:=1
else
dbgfile:=0
endif

define processall
@case $1 in  \
	"elf")	\
	echo ".........$(words $2)[$2] "	;\
	for z in $2	; \
	do \
		echo "---------z=$$z" ;\
		TEMPZ=$$z ;\
	if [ $3 = "1" ]; then \
		objcopy --only-keep-debug  $$z $$z.dbg ;\
		objcopy --strip-debug $$z ;\
		objcopy --add-gnu-debuglink=$$z.dbg $$z ;\
		install -D $$z.dbg $(cssyb_dir)/$$z@celfv@.dbg ;\
	fi ;\
		install -D $$z $(csbin_dir)/$$z@celfv@ ;cd $(csbin_dir);ln -sf $$z@celfv@ $$z;cd - ;\
	done \
	;;	\
	"so")	\
	echo "this is dynamic library......"	;\
	if [ $3 = "1" ]; then \
		objcopy --only-keep-debug .libs/$2.0.0.0 .libs/$2.0.0.0.dbg	;\
		objcopy --strip-debug  .libs/$2.0.0.0	;\
		objcopy --add-gnu-debuglink=.libs/$2.0.0.0.dbg .libs/$2.0.0.0 ;\
		install -D .libs/$2.0.0.0.dbg $(cssyb_dir)/$2@clibv@.dbg ;\
	fi ;\
	install -D .libs/$2.0.0.0  $(csbin_dir)/$2@clibv@;cd $(csbin_dir);ln -sf $2@clibv@ $2 ;\
	;;	\
	"ar")	\
	echo "this is static library......"	;\
	install -D  $2 $(cslib_dir)/$2; \
	;;	\
	*)	\
	echo "ignorant......"	;\
	;;	\
esac
endef
all:
	$(call processall,$(tsflag),$(goal),$(dbgfile))
	-rm -rf rubbish
