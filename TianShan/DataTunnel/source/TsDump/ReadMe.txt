========================================================================
       CONSOLE APPLICATION : tsdump
========================================================================
tsdump ������ʾ����� ts ��, ����������һ���ļ�,��������(����֧�� tcp udp multicast)
���ڽ��յ����ļ����� dump\����\����, -o ѡ�����ڶ� dod ��ص����ݽ��д���, �ڷ���\���� dod ��ʱ���Դ����ѡ��

һ ʾ��:

1) ��224.0.5.110:2007 ����DOD�ಥts��dump����Ļ��, ����ʾDOD��ص���Ϣ.
tsdump -o -m224.0.5.110:2007

2) ��224.0.5.110:2007 ����DOD�ಥts�����뵽c:\tscacheĿ¼��.
tsdump -o -m224.0.5.110:2007 -ec:\tscache

3) �������dod����ԭ���ļ�(����dod�������ϵ�ԭʼ�ļ�), ������ļ�Ϊ pid_555_tid_16.tab.n(n Ϊһ����������ֵ)
tsdump -x pid_555_tid_128.tab pid_555_tid_16.tab

4) ������, ֱ�ӱ���ts��
tsdump -m224.0.5.110:2007 -wd:\package\dod.ts

�� ����:

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

