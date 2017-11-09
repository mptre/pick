description: matching is case insensitive
keys: a \n # ENTER
stdin:
A
stdout:
A

description: it favors the shortest match
keys: aa/aa \n # ENTER
stdin:
aa/åå/aa
aa/åå/aa/aa
stdout:
aa/åå/aa/aa

description: do not match inside a CSI escape sequence
keys: 32 \n
stdin:
\033[32m33\033[m
3aaa2
stdout:
3aaa2

description: do not match inside a OSC escape sequence
keys: example \n # ENTER
stdin:
\033]8;;example.com\aOSC Hyperlink\033]8;;\a
favored match since the query is not inside the escape sequence example.com
stdout:
favored match since the query is not inside the escape sequence example.com
