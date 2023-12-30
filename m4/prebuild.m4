dnl #### waiting for me
if test -z "$ZQProjsPath" 
then
        echo "env-variable ZQProjsPath not defined! and script exit!"
        exit 99
fi

if test  -z "$ZQSdkPath" 
then
        echo "env-variable ZQSdkPath  not defined! and script exit!"
        exit  99
fi
if ! test -d ./TianShan/lib64
then  mkdir ./TianShan/lib64
fi

if ! test -d ./TianShan/bin64
then mkdir -p  ./TianShan/bin64/.libs
fi

dnl  test AC_CONFIG_COMMANDS commands.
dnl AC_CONFIG_COMMANDS (tag..., [cmds], [init-cmds])
dnl AC_CONFIG_COMMANDS_PRE (cmds)
dnl AC_CONFIG_COMMANDS_POST (cmds)
AC_CONFIG_COMMANDS(predelunistdh,[rm -rf "./TianShan/ContentStore/ContentClient/unistd.h"  \
					"./TianShan/StreamSmith/StreamClient/unistd.h"  \
					"./TianShan/CPE/service/CPEClient" ])
dnl AC_CONFIG_COMMANDS(timestamp1, [date > timestamp1])

