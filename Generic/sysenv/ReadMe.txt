========================================================================
       CONSOLE APPLICATION : sysenv
========================================================================
examples:

; show all system environment variable
sysenv

; show all user environment variable
sysenv -u

; show a system environment variable
sysenv PATH

; show a users environment variable
sysenv -u _NT_SYMBOL_PATH

; add a system environment variable
sysenv TEST=HELLO

; add a user environment variable
sysenv -u USER_TEST=HELLO

; remove a system environment variable
sysenv TEST=-

; remove a user environment variable
sysenv -u USER_TEST=-
