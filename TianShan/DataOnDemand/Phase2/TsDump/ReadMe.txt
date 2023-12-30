========================================================================
       CONSOLE APPLICATION : tsdump
========================================================================
tsdump 帮助显示与解码 ts 包, 它可以来自一个文件,或者网络(现在支持 tcp udp multicast)
对于接收到的文件进行 dump\保存\解码, -o 选项用于对 dod 相关的内容进行处理, 在分析\解码 dod 流时可以打开这个选项

一 示例:

1) 将224.0.5.110:2007 处的DOD多播ts包dump到屏幕上, 并显示DOD相关的信息.
tsdump -o -m224.0.5.110:2007

2) 将224.0.5.110:2007 处的DOD多播ts包解码到c:\tscache目录中.
tsdump -o -m224.0.5.110:2007 -ec:\tscache

3) 将解码的dod包还原成文件(即在dod服务器上的原始文件), 解码的文件为 pid_555_tid_16.tab.n(n 为一个递增的数值)
tsdump -x pid_555_tid_128.tab pid_555_tid_16.tab

4) 不解码, 直接保存ts包
tsdump -m224.0.5.110:2007 -wd:\package\dod.ts

二 帮助:

1) tsdump <source> [method] [option]
source:
        -f<filename>    source is a file
        -t<ip:port>     source is a tcp endpoint
        -u<ip:port>     source is a udp endpoint
        -m<ip:port>     source is a multicast endpoint
method:
        -s[times]       statistic
        -d              dump to the screen (default)
        -e<cachedir>    decode
        -w<filename>    write to a file
option:
        -h              show help information
        -o              special information of DOD

2) tsdump -x <indexfile> <datafile>
         make up the data of DOD

