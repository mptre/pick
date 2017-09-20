description: do not match inside a CSI escape sequence
keys: 32 \n
stdin:
\033[32m33\033[m
3aaa2
stdout:
3aaa2
