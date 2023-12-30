


注意 WebServer.cpp:254
当data是内容不是头部参数时，name是什么
frag.name什么意思
看看dll中调用GetRequestVar()和GetRequestVars()方法的地方


SentryCommand.cpp中屏蔽了日志监控
174,200

SentryPages.h中 140行，proxyPage实现在WebProxy.cpp




编译相关错误解决方法

1.Error	3	error C2628: 'intptr_t' followed by 'int' is illegal (did you forget a ';'?)	d:\git_zqprojs\zqproj\common\eloop\libuv_1.9.1\include\uv-win.h	27	

解决:头文件包含问题，HttpServer.h必须放到BaseZQServiceApplication.h前面

Neighborhood.cpp中的#include "SentryEnv.h"必须放在最前面